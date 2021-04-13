// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>

#include <cxxopts.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TLikeService.h>
#include <buzzblog/base_server.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace buzzblog;


class TLikeServiceHandler : public BaseServer, public TLikeServiceIf {
public:
  TLikeServiceHandler() {
  }

  void like_post(TLike& _return, const int32_t requester_id,
      const int32_t post_id) {
    auto _trace = BaseServer::TraceHandle("like_server", __FUNCTION__);
    // Add unique pair (account, post).
    auto uniquepair_client = get_uniquepair_client();
    TUniquepair uniquepair;
    try {
      uniquepair = uniquepair_client->add("like", requester_id, post_id);
    }
    catch (TUniquepairAlreadyExistsException e) {
      throw TLikeAlreadyExistsException();
    }
    uniquepair_client->close();

    // Build like (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.account_id = requester_id;
    _return.post_id = post_id;
  }

  void retrieve_standard_like(TLike& _return, const int32_t requester_id,
      const int32_t like_id) {
    auto _trace = BaseServer::TraceHandle("like_server", __FUNCTION__);
    // Get unique pair.
    auto uniquepair_client = get_uniquepair_client();
    TUniquepair uniquepair;
    try {
      uniquepair = uniquepair_client->get(like_id);
    }
    catch (TUniquepairNotFoundException e) {
      throw TLikeNotFoundException();
    }
    uniquepair_client->close();

    // Build like (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.account_id = uniquepair.first_elem;
    _return.post_id = uniquepair.second_elem;
  }

  void retrieve_expanded_like(TLike& _return, const int32_t requester_id,
      const int32_t like_id) {
    auto _trace = BaseServer::TraceHandle("like_server", __FUNCTION__);
    // Retrieve standard like.
    retrieve_standard_like(_return, requester_id, like_id);

    // Retrieve account.
    auto account_client = get_account_client();
    auto account = account_client->retrieve_standard_account(requester_id,
        _return.account_id);
    account_client->close();

    // Retrieve post.
    auto post_client = get_post_client();
    auto post = post_client->retrieve_expanded_post(requester_id,
        _return.post_id);
    post_client->close();

    // Build like (expanded mode).
    _return.__set_account(account);
    _return.__set_post(post);
  }

  void delete_like(const int32_t requester_id, const int32_t like_id) {
    auto _trace = BaseServer::TraceHandle("like_server", __FUNCTION__);
    {
      // Get unique pair.
      auto uniquepair_client = get_uniquepair_client();
      TUniquepair uniquepair;
      try {
        uniquepair = uniquepair_client->get(like_id);
      }
      catch (TUniquepairNotFoundException e) {
        throw TLikeNotFoundException();
      }
      uniquepair_client->close();

      // Check if requester is authorized.
      if (requester_id != uniquepair.first_elem)
        throw TLikeNotAuthorizedException();
    }

    // Remove unique pair.
    auto uniquepair_client = get_uniquepair_client();
    try {
      uniquepair_client->remove(like_id);
    }
    catch (TUniquepairNotFoundException e) {
      throw TLikeNotFoundException();
    }
    uniquepair_client->close();
  }

  void list_likes(std::vector<TLike>& _return, const int32_t requester_id,
      const int32_t account_id, const int32_t post_id) {
    auto _trace = BaseServer::TraceHandle("like_server", __FUNCTION__);
    // List unique pairs.
    auto uniquepair_client = get_uniquepair_client();
    std::vector<TUniquepair> uniquepairs;
    if (account_id < 0 && post_id < 0) {
      // List all unique pairs.
      uniquepairs = uniquepair_client->all("like");
    }
    else if (account_id >= 0 && post_id < 0) {
      // List unique pairs filtered by the 1st element.
      uniquepairs = uniquepair_client->filter_by_first_elem("like", account_id);
    }
    else if (account_id < 0 && post_id >= 0) {
      // List unique pairs filtered by the 2nd element.
      uniquepairs = uniquepair_client->filter_by_second_elem("like", post_id);
    }
    else {
      // Find unique pair.
      try {
        uniquepairs.push_back(uniquepair_client->find("like", account_id,
            post_id));
      }
      catch (TUniquepairNotFoundException e) {
      }
    }
    uniquepair_client->close();

    // Build likes.
    auto account_client = get_account_client();
    auto post_client = get_post_client();
    for (auto it : uniquepairs) {
      // Retrieve account.
      auto account = account_client->retrieve_standard_account(requester_id,
          it.first_elem);

      // Retrieve post.
      auto post = post_client->retrieve_expanded_post(requester_id,
          it.second_elem);

      // Build like (expanded mode).
      TLike like;
      like.id = it.id;
      like.created_at = it.created_at;
      like.account_id = it.first_elem;
      like.post_id = it.second_elem;
      like.__set_account(account);
      like.__set_post(post);
      _return.push_back(like);
    }
    account_client->close();
    post_client->close();
  }

  int32_t count_likes_by_account(const int32_t requester_id,
      const int32_t account_id) {
    auto _trace = BaseServer::TraceHandle("like_server", __FUNCTION__);
    auto uniquepair_client = get_uniquepair_client();
    auto count = uniquepair_client->count_first_elem("like", account_id);
    uniquepair_client->close();
    return count;
  }

  int32_t count_likes_of_post(const int32_t requester_id,
      const int32_t post_id) {
    auto _trace = BaseServer::TraceHandle("like_server", __FUNCTION__);
    auto uniquepair_client = get_uniquepair_client();
    auto count = uniquepair_client->count_second_elem("like", post_id);
    uniquepair_client->close();
    return count;
  }
};


int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("like_server", "Like server");
  options.add_options()
      ("host", "", cxxopts::value<std::string>()->default_value("0.0.0.0"))
      ("port", "", cxxopts::value<int>())
      ("threads", "", cxxopts::value<int>());

  // Parse command-line arguments.
  auto result = options.parse(argc, argv);
  std::string host = result["host"].as<std::string>();
  int port = result["port"].as<int>();
  int threads = result["threads"].as<int>();

  // Create server.
  TThreadedServer server(
      std::make_shared<TLikeServiceProcessor>(
          std::make_shared<TLikeServiceHandler>()),
      std::make_shared<TServerSocket>(host, port),
      std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
