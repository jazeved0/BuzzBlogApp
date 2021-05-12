# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from buzzblog.gen import TPostService


class Client:
  def __init__(self, ip_address, port, timeout=10000):
    self._socket = TSocket.TSocket(ip_address, port)
    self._socket.setTimeout(timeout)
    self._transport = TTransport.TBufferedTransport(self._socket)
    self._protocol = TBinaryProtocol.TBinaryProtocol(self._transport)
    self._tclient = TPostService.Client(self._protocol)
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

  def create_post(self, request_metadata, text):
    return self._tclient.create_post(request_metadata=request_metadata,
        text=text)

  def retrieve_standard_post(self, request_metadata, post_id):
    return self._tclient.retrieve_standard_post(
        request_metadata=request_metadata, post_id=post_id)

  def retrieve_expanded_post(self, request_metadata, post_id):
    return self._tclient.retrieve_expanded_post(
        request_metadata=request_metadata, post_id=post_id)

  def delete_post(self, request_metadata, post_id):
    return self._tclient.delete_post(request_metadata=request_metadata,
        post_id=post_id)

  def list_posts(self, request_metadata, query, limit, offset):
    return self._tclient.list_posts(request_metadata=request_metadata,
        query=query, limit=limit, offset=offset)
