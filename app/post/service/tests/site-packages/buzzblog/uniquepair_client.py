# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from buzzblog.gen import TUniquepairService


class Client:
  def __init__(self, ip_address, port, timeout=10000):
    self._socket = TSocket.TSocket(ip_address, port)
    self._socket.setTimeout(timeout)
    self._transport = TTransport.TBufferedTransport(self._socket)
    self._protocol = TBinaryProtocol.TBinaryProtocol(self._transport)
    self._tclient = TUniquepairService.Client(self._protocol)
    self._transport.open()

  def __enter__(self):
    return self

  def __exit__(self, exception_type, exception_value, exception_traceback):
    self.close()

  def __del__(self):
    self.close()

  def close(self):
    if self._transport.isOpen():
      self._transport.close()

  def get(self, request_metadata, uniquepair_id):
    return self._tclient.get(request_metadata=request_metadata,
        uniquepair_id=uniquepair_id)

  def add(self, request_metadata, domain, first_elem, second_elem):
    return self._tclient.add(request_metadata=request_metadata, domain=domain,
        first_elem=first_elem, second_elem=second_elem)

  def remove(self, request_metadata, uniquepair_id):
    return self._tclient.remove(request_metadata=request_metadata,
        uniquepair_id=uniquepair_id)

  def find(self, request_metadata, domain, first_elem, second_elem):
    return self._tclient.find(request_metadata=request_metadata, domain=domain,
        first_elem=first_elem, second_elem=second_elem)

  def fetch(self, request_metadata, query, limit, offset):
    return self._tclient.fetch(request_metadata=request_metadata, query=query,
        limit=limit, offset=offset)

  def count(self, request_metadata, query):
    return self._tclient.count(request_metadata=request_metadata, query=query)
