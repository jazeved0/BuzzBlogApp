# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from .gen.buzzblog import TPostService


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

  def create_post(self, requester_id, text):
    return self._tclient.create_post(requester_id=requester_id, text=text)

  def retrieve_standard_post(self, requester_id, post_id):
    return self._tclient.retrieve_standard_post(requester_id=requester_id,
        post_id=post_id)

  def retrieve_expanded_post(self, requester_id, post_id):
    return self._tclient.retrieve_expanded_post(requester_id=requester_id,
        post_id=post_id)

  def delete_post(self, requester_id, post_id):
    return self._tclient.delete_post(requester_id=requester_id, post_id=post_id)

  def list_posts(self, requester_id, author_id):
    return self._tclient.list_posts(requester_id=requester_id,
        author_id=author_id)
