// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <memory>
#include <string>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen/TPostService.h"


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace buzzblog;


namespace post_service {
  class Client {
   private:
    std::shared_ptr<TSocket> _socket;
    std::shared_ptr<TTransport> _transport;
    std::shared_ptr<TProtocol> _protocol;
    std::shared_ptr<TPostServiceClient> _client;
   public:
    Client(const std::string& ip_address, int port, int conn_timeout_ms) {
      _socket = std::make_shared<TSocket>(ip_address, port);
      _socket->setConnTimeout(conn_timeout_ms);
      _transport = std::make_shared<TBufferedTransport>(_socket);
      _protocol = std::make_shared<TBinaryProtocol>(_transport);
      _client = std::make_shared<TPostServiceClient>(_protocol);
      _transport->open();
    }

    ~Client() {
      close();
    }

    void close() {
      if (_transport->isOpen())
        _transport->close();
    }

    TPost create_post(const int32_t requester_id, const std::string& text) {
      TPost _return;
      _client->create_post(_return, requester_id, text);
      return _return;
    }

    TPost retrieve_standard_post(const int32_t requester_id,
        const int32_t post_id) {
      TPost _return;
      _client->retrieve_standard_post(_return, requester_id, post_id);
      return _return;
    }

    TPost retrieve_expanded_post(const int32_t requester_id,
        const int32_t post_id) {
      TPost _return;
      _client->retrieve_expanded_post(_return, requester_id, post_id);
      return _return;
    }

    void delete_post(const int32_t requester_id, const int32_t post_id) {
      _client->delete_post(requester_id, post_id);
    }

    std::vector<TPost> list_posts(const int32_t requester_id,
        const int32_t author_id) {
      std::vector<TPost> _return;
      _client->list_posts(_return, requester_id, author_id);
      return _return;
    }

    int32_t count_posts_by_author(const int32_t requester_id,
        const int32_t author_id) {
      return _client->count_posts_by_author(requester_id, author_id);
    }
  };
}
