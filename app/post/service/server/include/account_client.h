// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <memory>
#include <string>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen/TAccountService.h"


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace buzzblog;


namespace account_service {
  class Client {
   private:
    std::shared_ptr<TSocket> _socket;
    std::shared_ptr<TTransport> _transport;
    std::shared_ptr<TProtocol> _protocol;
    std::shared_ptr<TAccountServiceClient> _client;
   public:
    Client(const std::string& ip_address, int port, int conn_timeout_ms) {
      _socket = std::make_shared<TSocket>(ip_address, port);
      _socket->setConnTimeout(conn_timeout_ms);
      _transport = std::make_shared<TBufferedTransport>(_socket);
      _protocol = std::make_shared<TBinaryProtocol>(_transport);
      _client = std::make_shared<TAccountServiceClient>(_protocol);
      _transport->open();
    }

    ~Client() {
      close();
    }

    void close() {
      if (_transport->isOpen())
        _transport->close();
    }

    TAccount authenticate_user(const std::string& username,
        const std::string& password) {
      TAccount _return;
      _client->authenticate_user(_return, username, password);
      return _return;
    }

    TAccount create_account(const std::string& username,
        const std::string& password, const std::string& first_name,
        const std::string& last_name) {
      TAccount _return;
      _client->create_account(_return, username, password, first_name,
          last_name);
      return _return;
    }

    TAccount retrieve_standard_account(const int32_t requester_id,
        const int32_t account_id) {
      TAccount _return;
      _client->retrieve_standard_account(_return, requester_id, account_id);
      return _return;
    }

    TAccount retrieve_expanded_account(const int32_t requester_id,
        const int32_t account_id) {
      TAccount _return;
      _client->retrieve_expanded_account(_return, requester_id, account_id);
      return _return;
    }

    TAccount update_account(const int32_t requester_id,
        const int32_t account_id, const std::string& password,
        const std::string& first_name, const std::string& last_name) {
      TAccount _return;
      _client->update_account(_return, requester_id, account_id, password,
          first_name, last_name);
      return _return;
    }

    void delete_account(const int32_t requester_id, const int32_t account_id) {
      _client->delete_account(requester_id, account_id);
    }
  };
}
