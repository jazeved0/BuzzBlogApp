// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>

#include <cxxopts.hpp>
#include <pqxx/pqxx>
#include <spdlog/sinks/basic_file_sink.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TPostService.h>
#include <buzzblog/base_server.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;


class TPostServiceHandler : public BaseServer, public TPostServiceIf {
private:
  bool validate_attributes(const std::string& text) {
    return (text.size() > 0 && text.size() <= 200);
  }

  std::string build_where_clause(const TPostQuery& query) {
    std::ostringstream where_clause;
    where_clause << "active = true";
    if (query.__isset.author_id)
      where_clause << " AND author_id = " << query.author_id;
    return where_clause.str();
  }

public:
  TPostServiceHandler(const std::string& backend_filepath,
      const std::string& postgres_user, const std::string& postgres_password,
      const std::string& postgres_dbname)
  : BaseServer(backend_filepath, postgres_user, postgres_password,
      postgres_dbname) {
  }

  void create_post(TPost& _return, const TRequestMetadata& request_metadata,
      const std::string& text) {
    auto _trace = BaseServer::TraceHandle("post_server", __FUNCTION__, request_metadata);
    // Validate attributes.
    if (!validate_attributes(text))
      throw TPostInvalidAttributesException();

    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "INSERT INTO Posts (text, author_id, created_at) "
        "VALUES ('%s', %d, extract(epoch from now())) "
        "RETURNING id, created_at";
    sprintf(query_str, query_fmt, text.c_str(), request_metadata.requester_id);

    // Execute query.
    pqxx::connection conn(post_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    // Build account (standard mode).
    _return.id = db_res[0][0].as<int>();
    _return.created_at = db_res[0][1].as<int>();
    _return.active = true;
    _return.text = text;
    _return.author_id = request_metadata.requester_id;
  }

  void retrieve_standard_post(TPost& _return,
      const TRequestMetadata& request_metadata, const int32_t post_id) {
    auto _trace = BaseServer::TraceHandle("post_server", __FUNCTION__, request_metadata);
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT created_at, active, text, author_id "
        "FROM Posts "
        "WHERE id = %d";
    sprintf(query_str, query_fmt, post_id);

    // Execute query.
    pqxx::connection conn(post_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    // Check if post exists.
    if (db_res.begin() == db_res.end())
      throw TPostNotFoundException();

    // Build post (standard mode).
    _return.id = post_id;
    _return.created_at = db_res[0][0].as<int>();
    _return.active = db_res[0][1].as<bool>();
    _return.text = db_res[0][2].as<std::string>();
    _return.author_id = db_res[0][3].as<int>();
  }

  void retrieve_expanded_post(TPost& _return,
      const TRequestMetadata& request_metadata, const int32_t post_id) {
    auto _trace = BaseServer::TraceHandle("post_server", __FUNCTION__, request_metadata);
    // Retrieve standard post.
    retrieve_standard_post(_return, request_metadata, post_id);

    // Retrieve author.
    auto account_client = get_account_client();
    auto author = account_client->retrieve_standard_account(request_metadata,
        _return.author_id);
    account_client->close();

    // Retrieve like activity.
    auto like_client = get_like_client();
    auto n_likes = like_client->count_likes_of_post(request_metadata, post_id);
    like_client->close();

    // Build post (expanded mode).
    _return.__set_author(author);
    _return.__set_n_likes(n_likes);
  }

  void delete_post(const TRequestMetadata& request_metadata,
      const int32_t post_id) {
    auto _trace = BaseServer::TraceHandle("post_server", __FUNCTION__, request_metadata);
    {
      // Retrieve standard post.
      TPost post;
      retrieve_standard_post(post, request_metadata, post_id);

      // Check if requester is authorized.
      if (request_metadata.requester_id != post.author_id)
        throw TPostNotAuthorizedException();
    }

    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "UPDATE Posts "
        "SET active = FALSE "
        "WHERE id = %d";
    sprintf(query_str, query_fmt, post_id);

    // Execute query.
    pqxx::connection conn(post_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();
  }

  void list_posts(std::vector<TPost>& _return,
      const TRequestMetadata& request_metadata, const TPostQuery& query,
      const int32_t limit, const int32_t offset) {
    auto _trace = BaseServer::TraceHandle("post_server", __FUNCTION__, request_metadata);
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT id, created_at, active, text, author_id "
        "FROM Posts "
        "WHERE %s "
        "ORDER BY created_at DESC "
        "LIMIT %d "
        "OFFSET %d";
    sprintf(query_str, query_fmt, build_where_clause(query).c_str(), limit,
        offset);

    // Execute query.
    pqxx::connection conn(post_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    // Build posts.
    auto account_client = get_account_client();
    auto like_client = get_like_client();
    for (auto row : db_res) {
      // Retrieve author.
      auto author = account_client->retrieve_standard_account(request_metadata,
          row["author_id"].as<int>());

      // Retrieve like activity.
      auto n_likes = like_client->count_likes_of_post(request_metadata,
          row["id"].as<int>());

      // Build post (expanded mode).
      TPost post;
      post.id = row["id"].as<int>();
      post.created_at = row["created_at"].as<int>();
      post.active = row["active"].as<bool>();
      post.text = row["text"].as<std::string>();
      post.author_id = row["author_id"].as<int>();
      post.__set_author(author);
      post.__set_n_likes(n_likes);
      _return.push_back(post);
    }
    account_client->close();
    like_client->close();
  }

  int32_t count_posts_by_author(const TRequestMetadata& request_metadata,
      const int32_t author_id) {
    auto _trace = BaseServer::TraceHandle("post_server", __FUNCTION__, request_metadata);
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT COUNT(*) "
        "FROM Posts "
        "WHERE author_id = %d";
    sprintf(query_str, query_fmt, author_id);

    // Execute query.
    pqxx::connection conn(post_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    return db_res[0][0].as<int>();
  }
};


int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("post_server", "Post server");
  options.add_options()
      ("host", "", cxxopts::value<std::string>()->default_value("0.0.0.0"))
      ("port", "", cxxopts::value<int>())
      ("threads", "", cxxopts::value<int>())
      ("backend_filepath", "", cxxopts::value<std::string>()->default_value(
          "/etc/opt/BuzzBlogApp/backend.yml"))
      ("postgres_user", "", cxxopts::value<std::string>()->default_value(
          "postgres"))
      ("postgres_password", "", cxxopts::value<std::string>()->default_value(
          "postgres"))
      ("postgres_dbname", "", cxxopts::value<std::string>()->default_value(
          "postgres"));

  // Parse command-line arguments.
  auto result = options.parse(argc, argv);
  std::string host = result["host"].as<std::string>();
  int port = result["port"].as<int>();
  int threads = result["threads"].as<int>();
  std::string backend_filepath = result["backend_filepath"].as<std::string>();
  std::string postgres_user = result["postgres_user"].as<std::string>();
  std::string postgres_password = result["postgres_password"].as<std::string>();
  std::string postgres_dbname = result["postgres_dbname"].as<std::string>();

  // Initialize logger.
  auto logger = spdlog::basic_logger_mt("logger", "/tmp/calls.log");
  logger->set_pattern("[%H:%M:%S.%F] pid=%P tid=%t %v");

  // Create server.
  TThreadedServer server(
      std::make_shared<TPostServiceProcessor>(
          std::make_shared<TPostServiceHandler>(backend_filepath,
              postgres_user, postgres_password, postgres_dbname)),
      std::make_shared<TServerSocket>(host, port),
      std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
