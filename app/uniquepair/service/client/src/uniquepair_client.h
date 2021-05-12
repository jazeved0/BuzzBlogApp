// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <memory>
#include <string>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <buzzblog/gen/TUniquepairService.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace gen;


namespace uniquepair_service {
  class Client {
   private:
    std::shared_ptr<TSocket> _socket;
    std::shared_ptr<TTransport> _transport;
    std::shared_ptr<TProtocol> _protocol;
    std::shared_ptr<TUniquepairServiceClient> _client;
   public:
    Client(const std::string& ip_address, int port, int conn_timeout_ms) {
      _socket = std::make_shared<TSocket>(ip_address, port);
      _socket->setConnTimeout(conn_timeout_ms);
      _transport = std::make_shared<TBufferedTransport>(_socket);
      _protocol = std::make_shared<TBinaryProtocol>(_transport);
      _client = std::make_shared<TUniquepairServiceClient>(_protocol);
      _transport->open();
    }

    ~Client() {
      close();
    }

    void close() {
      if (_transport->isOpen())
        _transport->close();
    }

    TUniquepair get(const TRequestMetadata& request_metadata,
        const int32_t uniquepair_id) {
      TUniquepair _return;
      _client->get(_return, request_metadata, uniquepair_id);
      return _return;
    }

    TUniquepair add(const TRequestMetadata& request_metadata,
        const std::string& domain, const int32_t first_elem,
        const int32_t second_elem) {
      TUniquepair _return;
      _client->add(_return, request_metadata, domain, first_elem, second_elem);
      return _return;
    }

    void remove(const TRequestMetadata& request_metadata,
        const int32_t uniquepair_id) {
      _client->remove(request_metadata, uniquepair_id);
    }

    TUniquepair find(const TRequestMetadata& request_metadata,
        const std::string& domain, const int32_t first_elem,
        const int32_t second_elem) {
      TUniquepair _return;
      _client->find(_return, request_metadata, domain, first_elem, second_elem);
      return _return;
    }

    std::vector<TUniquepair> fetch(const TRequestMetadata& request_metadata,
        const TUniquepairQuery& query, const int32_t limit,
        const int32_t offset) {
      std::vector<TUniquepair> _return;
      _client->fetch(_return, request_metadata, query, limit, offset);
      return _return;
    }

    int32_t count(const TRequestMetadata& request_metadata,
        const TUniquepairQuery& query) {
      return _client->count(request_metadata, query);
    }
  };
}
