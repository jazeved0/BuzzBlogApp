// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <memory>
#include <string>
#include <vector>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen/TFollowService.h"


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace buzzblog;


namespace follow_service {
  class Client {
   private:
    std::shared_ptr<TSocket> _socket;
    std::shared_ptr<TTransport> _transport;
    std::shared_ptr<TProtocol> _protocol;
    std::shared_ptr<TFollowServiceClient> _client;
   public:
    Client(const std::string& ip_address, int port, int conn_timeout_ms) {
      _socket = std::make_shared<TSocket>(ip_address, port);
      _socket->setConnTimeout(conn_timeout_ms);
      _transport = std::make_shared<TBufferedTransport>(_socket);
      _protocol = std::make_shared<TBinaryProtocol>(_transport);
      _client = std::make_shared<TFollowServiceClient>(_protocol);
      _transport->open();
    }

    ~Client() {
      close();
    }

    void close() {
      if (_transport->isOpen())
        _transport->close();
    }

    TFollow follow_account(const int32_t requester_id,
        const int32_t account_id) {
      TFollow _return;
      _client->follow_account(_return, requester_id, account_id);
      return _return;
    }

    TFollow retrieve_standard_follow(const int32_t requester_id,
        const int32_t follow_id) {
      TFollow _return;
      _client->retrieve_standard_follow(_return, requester_id, follow_id);
      return _return;
    }

    TFollow retrieve_expanded_follow(const int32_t requester_id,
        const int32_t follow_id) {
      TFollow _return;
      _client->retrieve_expanded_follow(_return, requester_id, follow_id);
      return _return;
    }

    void delete_follow(const int32_t requester_id, const int32_t follow_id) {
      _client->delete_follow(requester_id, follow_id);
    }

    std::vector<TFollow> list_follows(const int32_t requester_id,
        const int32_t follower_id, const int32_t followee_id) {
      std::vector<TFollow> _return;
      _client->list_follows(_return, requester_id, follower_id, followee_id);
      return _return;
    }

    bool check_follow(const int32_t requester_id,
        const int32_t follower_id, const int32_t followee_id) {
      return _client->check_follow(requester_id, follower_id, followee_id);
    }

    int32_t count_followers(const int32_t requester_id,
        const int32_t account_id) {
      return _client->count_followers(requester_id, account_id);
    }

    int32_t count_followees(const int32_t requester_id,
        const int32_t account_id) {
      return _client->count_followees(requester_id, account_id);
    }
  };
}
