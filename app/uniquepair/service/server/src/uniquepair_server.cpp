// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <sstream>
#include <string>

#include <cxxopts.hpp>
#include <pqxx/pqxx>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TUniquepairService.h>
#include <buzzblog/base_server.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace gen;


class TUniquepairServiceHandler : public BaseServer,
    public TUniquepairServiceIf {
private:
  std::string build_where_clause(const TUniquepairQuery& query) {
    std::ostringstream where_clause;
    where_clause << "domain = '" << query.domain << "'";
    if (query.__isset.first_elem)
      where_clause << " AND first_elem = " << query.first_elem;
    if (query.__isset.second_elem)
      where_clause << " AND second_elem = " << query.second_elem;
    return where_clause.str();
  }

public:
  TUniquepairServiceHandler(const std::string& backend_filepath,
      const std::string& postgres_user, const std::string& postgres_password,
      const std::string& postgres_dbname)
  : BaseServer(backend_filepath, postgres_user, postgres_password,
      postgres_dbname) {
  }

  void get(TUniquepair& _return, const int32_t uniquepair_id) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT created_at, domain, first_elem, second_elem "
        "FROM Uniquepairs "
        "WHERE id = %d";
    sprintf(query_str, query_fmt, uniquepair_id);

    // Execute query.
    pqxx::connection conn(uniquepair_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    // Check if unique pair exists.
    if (db_res.begin() == db_res.end())
      throw TUniquepairNotFoundException();

    // Build unique pair.
    _return.id = uniquepair_id;
    _return.created_at = db_res[0][0].as<int>();
    _return.domain = db_res[0][1].as<std::string>();
    _return.first_elem = db_res[0][2].as<int>();
    _return.second_elem = db_res[0][3].as<int>();
  }

  void add(TUniquepair& _return, const std::string& domain,
      const int32_t first_elem, const int32_t second_elem) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "INSERT INTO Uniquepairs (domain, first_elem, second_elem, created_at) "
        "VALUES ('%s', %d, %d, extract(epoch from now())) "
        "RETURNING id, created_at";
    sprintf(query_str, query_fmt, domain.c_str(), first_elem, second_elem);

    // Execute query.
    pqxx::connection conn(uniquepair_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res;
    try {
      db_res = txn.exec(query_str);
    }
    catch (pqxx::sql_error& e) {
      throw TUniquepairAlreadyExistsException();
    }
    txn.commit();
    conn.disconnect();

    // Build unique pair.
    _return.id = db_res[0][0].as<int>();
    _return.created_at = db_res[0][1].as<int>();
    _return.domain = domain;
    _return.first_elem = first_elem;
    _return.second_elem = second_elem;
  }

  void remove(const int32_t uniquepair_id) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "DELETE FROM Uniquepairs "
        "WHERE id = %d "
        "RETURNING id";
    sprintf(query_str, query_fmt, uniquepair_id);

    // Execute query.
    pqxx::connection conn(uniquepair_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    // Check if unique pair exists.
    if (db_res.begin() == db_res.end())
      throw TUniquepairNotFoundException();
  }

  void find(TUniquepair& _return, const std::string& domain,
      const int32_t first_elem, const int32_t second_elem) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT id, created_at "
        "FROM Uniquepairs "
        "WHERE domain = '%s' AND first_elem = %d AND second_elem = %d";
    sprintf(query_str, query_fmt, domain.c_str(), first_elem, second_elem);

    // Execute query.
    pqxx::connection conn(uniquepair_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    // Check if unique pair exists.
    if (db_res.begin() == db_res.end())
      throw TUniquepairNotFoundException();

    // Build unique pair.
    _return.id = db_res[0][0].as<int>();
    _return.created_at = db_res[0][1].as<int>();
    _return.domain = domain;
    _return.first_elem = first_elem;
    _return.second_elem = second_elem;
  }

  void fetch(std::vector<TUniquepair>& _return, const TUniquepairQuery& query,
      const int32_t limit, const int32_t offset) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT id, created_at, first_elem, second_elem "
        "FROM Uniquepairs "
        "WHERE %s "
        "ORDER BY created_at DESC "
        "LIMIT %d "
        "OFFSET %d";
    sprintf(query_str, query_fmt, build_where_clause(query).c_str(), limit,
        offset);

    // Execute query.
    pqxx::connection conn(uniquepair_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    // Build unique pairs.
    for (auto row : db_res) {
      // Build unique pair.
      TUniquepair uniquepair;
      uniquepair.id = row["id"].as<int>();
      uniquepair.created_at = row["created_at"].as<int>();
      uniquepair.domain = query.domain;
      uniquepair.first_elem = row["first_elem"].as<int>();
      uniquepair.second_elem = row["second_elem"].as<int>();
      _return.push_back(uniquepair);
    }
  }

  int32_t count_first_elem(const std::string& domain,
      const int32_t first_elem) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT COUNT(*) "
        "FROM Uniquepairs "
        "WHERE domain = '%s' AND first_elem = %d";
    sprintf(query_str, query_fmt, domain.c_str(), first_elem);

    // Execute query.
    pqxx::connection conn(uniquepair_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    return db_res[0][0].as<int>();
  }

  int32_t count_second_elem(const std::string& domain,
      const int32_t second_elem) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT COUNT(*) "
        "FROM Uniquepairs "
        "WHERE domain = '%s' AND second_elem = %d";
    sprintf(query_str, query_fmt, domain.c_str(), second_elem);

    // Execute query.
    pqxx::connection conn(uniquepair_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    return db_res[0][0].as<int>();
  }
};


int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("uniquepair_server", "Uniquepair server");
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

  // Create server.
  TThreadedServer server(
      std::make_shared<TUniquepairServiceProcessor>(
          std::make_shared<TUniquepairServiceHandler>(backend_filepath,
              postgres_user, postgres_password, postgres_dbname)),
      std::make_shared<TServerSocket>(host, port),
      std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
