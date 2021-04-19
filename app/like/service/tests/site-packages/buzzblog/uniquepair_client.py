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

  def get(self, uniquepair_id):
    return self._tclient.get(uniquepair_id=uniquepair_id)

  def add(self, domain, first_elem, second_elem):
    return self._tclient.add(domain=domain, first_elem=first_elem,
        second_elem=second_elem)

  def remove(self, uniquepair_id):
    return self._tclient.remove(uniquepair_id=uniquepair_id)

  def find(self, domain, first_elem, second_elem):
    return self._tclient.find(domain=domain, first_elem=first_elem,
        second_elem=second_elem)

  def fetch(self, query, limit, offset):
    return self._tclient.fetch(query=query, limit=limit, offset=offset)

  def count(self, query):
    return self._tclient.count(query=query)
