// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <memory>
#include <string>
#include <vector>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <buzzblog/gen/TFollowService.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace gen;


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

    TFollow follow_account(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      TFollow _return;
      _client->follow_account(_return, request_metadata, account_id);
      return _return;
    }

    TFollow retrieve_standard_follow(const TRequestMetadata& request_metadata,
        const int32_t follow_id) {
      TFollow _return;
      _client->retrieve_standard_follow(_return, request_metadata, follow_id);
      return _return;
    }

    TFollow retrieve_expanded_follow(const TRequestMetadata& request_metadata,
        const int32_t follow_id) {
      TFollow _return;
      _client->retrieve_expanded_follow(_return, request_metadata, follow_id);
      return _return;
    }

    void delete_follow(const TRequestMetadata& request_metadata,
        const int32_t follow_id) {
      _client->delete_follow(request_metadata, follow_id);
    }

    std::vector<TFollow> list_follows(const TRequestMetadata& request_metadata,
        const TFollowQuery& query, const int32_t limit, const int32_t offset) {
      std::vector<TFollow> _return;
      _client->list_follows(_return, request_metadata, query, limit, offset);
      return _return;
    }

    bool check_follow(const TRequestMetadata& request_metadata,
        const int32_t follower_id, const int32_t followee_id) {
      return _client->check_follow(request_metadata, follower_id, followee_id);
    }

    int32_t count_followers(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      return _client->count_followers(request_metadata, account_id);
    }

    int32_t count_followees(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      return _client->count_followees(request_metadata, account_id);
    }
  };
}
