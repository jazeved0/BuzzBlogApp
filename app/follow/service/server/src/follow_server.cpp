// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>

#include <cxxopts.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TFollowService.h>
#include <buzzblog/base_server.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace buzzblog;


class TFollowServiceHandler : public BaseServer, public TFollowServiceIf {
public:
  TFollowServiceHandler() {
  }

  void follow_account(TFollow& _return, const int32_t requester_id,
      const int32_t account_id) {
    // Add unique pair (follower, followee).
    auto uniquepair_client = get_uniquepair_client();
    TUniquepair uniquepair;
    try {
      uniquepair = uniquepair_client->add("follow", requester_id, account_id);
    }
    catch (TUniquepairAlreadyExistsException e) {
      throw TFollowAlreadyExistsException();
    }
    uniquepair_client->close();

    // Build follow (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.follower_id = requester_id;
    _return.followee_id = account_id;
  }

  void retrieve_standard_follow(TFollow& _return, const int32_t requester_id,
      const int32_t follow_id) {
    // Get unique pair.
    auto uniquepair_client = get_uniquepair_client();
    TUniquepair uniquepair;
    try {
      uniquepair = uniquepair_client->get(follow_id);
    }
    catch (TUniquepairNotFoundException e) {
      throw TFollowNotFoundException();
    }
    uniquepair_client->close();

    // Build follow (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.follower_id = uniquepair.first_elem;
    _return.followee_id = uniquepair.second_elem;
  }

  void retrieve_expanded_follow(TFollow& _return, const int32_t requester_id,
      const int32_t follow_id) {
    // Retrieve standard follow.
    retrieve_standard_follow(_return, requester_id, follow_id);

    // Retrieve accounts.
    auto account_client = get_account_client();
    auto follower = account_client->retrieve_standard_account(requester_id,
        _return.follower_id);
    auto followee = account_client->retrieve_standard_account(requester_id,
        _return.followee_id);
    account_client->close();

    // Build follow (expanded mode).
    _return.__set_follower(follower);
    _return.__set_followee(followee);
  }

  void delete_follow(const int32_t requester_id, const int32_t follow_id) {
    {
      // Get unique pair.
      auto uniquepair_client = get_uniquepair_client();
      TUniquepair uniquepair;
      try {
        uniquepair = uniquepair_client->get(follow_id);
      }
      catch (TUniquepairNotFoundException e) {
        throw TFollowNotFoundException();
      }
      uniquepair_client->close();

      // Check if requester is authorized.
      if (requester_id != uniquepair.first_elem)
        throw TFollowNotAuthorizedException();
    }

    // Remove unique pair.
    auto uniquepair_client = get_uniquepair_client();
    try {
      uniquepair_client->remove(follow_id);
    }
    catch (TUniquepairNotFoundException e) {
      throw TFollowNotFoundException();
    }
    uniquepair_client->close();
  }

  void list_follows(std::vector<TFollow>& _return, const int32_t requester_id,
      const int32_t follower_id, const int32_t followee_id) {
    // List unique pairs.
    auto uniquepair_client = get_uniquepair_client();
    std::vector<TUniquepair> uniquepairs;
    if (follower_id < 0 && followee_id < 0) {
      // List all unique pairs.
      uniquepairs = uniquepair_client->all("follow");
    }
    else if (follower_id >= 0 && followee_id < 0) {
      // List unique pairs filtered by the 1st element.
      uniquepairs = uniquepair_client->filter_by_first_elem("follow",
          follower_id);
    }
    else if (follower_id < 0 && followee_id >= 0) {
      // List unique pairs filtered by the 2nd element.
      uniquepairs = uniquepair_client->filter_by_second_elem("follow",
          followee_id);
    }
    else {
      // Find unique pair.
      try {
        uniquepairs.push_back(uniquepair_client->find("follow", follower_id,
            followee_id));
      }
      catch (TUniquepairNotFoundException e) {
      }
    }
    uniquepair_client->close();

    // Build follows.
    auto account_client = get_account_client();
    for (auto it : uniquepairs) {
      // Retrieve accounts.
      auto follower = account_client->retrieve_standard_account(requester_id,
          it.first_elem);
      auto followee = account_client->retrieve_standard_account(requester_id,
          it.second_elem);

      // Build follow (expanded mode).
      TFollow follow;
      follow.id = it.id;
      follow.created_at = it.created_at;
      follow.follower_id = it.first_elem;
      follow.followee_id = it.second_elem;
      follow.__set_follower(follower);
      follow.__set_followee(followee);
      _return.push_back(follow);
    }
    account_client->close();
  }

  bool check_follow(const int32_t requester_id, const int32_t follower_id,
      const int32_t followee_id) {
    bool follow_exists;
    auto uniquepair_client = get_uniquepair_client();
    try {
      uniquepair_client->find("follow", follower_id, followee_id);
      follow_exists = true;
    }
    catch (TUniquepairNotFoundException e) {
      follow_exists = false;
    }
    uniquepair_client->close();
    return follow_exists;
  }

  int32_t count_followers(const int32_t requester_id,
      const int32_t account_id) {
    auto uniquepair_client = get_uniquepair_client();
    auto count = uniquepair_client->count_second_elem("follow", account_id);
    uniquepair_client->close();
    return count;
  }

  int32_t count_followees(const int32_t requester_id,
      const int32_t account_id) {
    auto uniquepair_client = get_uniquepair_client();
    auto count = uniquepair_client->count_first_elem("follow", account_id);
    uniquepair_client->close();
    return count;
  }
};


int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("follow_server", "Follow server");
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
      std::make_shared<TFollowServiceProcessor>(
          std::make_shared<TFollowServiceHandler>()),
      std::make_shared<TServerSocket>(host, port),
      std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
