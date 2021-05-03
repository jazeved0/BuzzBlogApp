// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <memory>
#include <string>
#include <vector>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <buzzblog/gen/TLikeService.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace gen;


namespace like_service {
  class Client {
   private:
    std::shared_ptr<TSocket> _socket;
    std::shared_ptr<TTransport> _transport;
    std::shared_ptr<TProtocol> _protocol;
    std::shared_ptr<TLikeServiceClient> _client;
   public:
    Client(const std::string& ip_address, int port, int conn_timeout_ms) {
      _socket = std::make_shared<TSocket>(ip_address, port);
      _socket->setConnTimeout(conn_timeout_ms);
      _transport = std::make_shared<TBufferedTransport>(_socket);
      _protocol = std::make_shared<TBinaryProtocol>(_transport);
      _client = std::make_shared<TLikeServiceClient>(_protocol);
      _transport->open();
    }

    ~Client() {
      close();
    }

    void close() {
      if (_transport->isOpen())
        _transport->close();
    }

    TLike like_post(const int32_t requester_id, const int32_t post_id) {
      TLike _return;
      _client->like_post(_return, requester_id, post_id);
      return _return;
    }

    TLike retrieve_standard_like(const int32_t requester_id,
        const int32_t like_id) {
      TLike _return;
      _client->retrieve_standard_like(_return, requester_id, like_id);
      return _return;
    }

    TLike retrieve_expanded_like(const int32_t requester_id,
        const int32_t like_id) {
      TLike _return;
      _client->retrieve_expanded_like(_return, requester_id, like_id);
      return _return;
    }

    void delete_like(const int32_t requester_id, const int32_t like_id) {
      _client->delete_like(requester_id, like_id);
    }

    std::vector<TLike> list_likes(const int32_t requester_id,
        const TLikeQuery& query, const int32_t limit, const int32_t offset) {
      std::vector<TLike> _return;
      _client->list_likes(_return, requester_id, query, limit, offset);
      return _return;
    }

    int32_t count_likes_by_account(const int32_t requester_id,
        const int32_t account_id) {
      return _client->count_likes_by_account(requester_id, account_id);
    }

    int32_t count_likes_of_post(const int32_t requester_id,
        const int32_t post_id) {
      return _client->count_likes_of_post(requester_id, post_id);
    }
  };
}
