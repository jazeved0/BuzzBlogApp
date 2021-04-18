# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from buzzblog.gen import TLikeService


class Client:
  def __init__(self, ip_address, port, timeout=10000):
    self._socket = TSocket.TSocket(ip_address, port)
    self._socket.setTimeout(timeout)
    self._transport = TTransport.TBufferedTransport(self._socket)
    self._protocol = TBinaryProtocol.TBinaryProtocol(self._transport)
    self._tclient = TLikeService.Client(self._protocol)
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

  def like_post(self, requester_id, post_id):
    return self._tclient.like_post(requester_id=requester_id, post_id=post_id)

  def retrieve_standard_like(self, requester_id, like_id):
    return self._tclient.retrieve_standard_like(requester_id=requester_id,
        like_id=like_id)

  def retrieve_expanded_like(self, requester_id, like_id):
    return self._tclient.retrieve_expanded_like(requester_id=requester_id,
        like_id=like_id)

  def delete_like(self, requester_id, like_id):
    return self._tclient.delete_like(requester_id=requester_id, like_id=like_id)

  def list_likes(self, requester_id, account_id, post_id):
    return self._tclient.list_likes(requester_id=requester_id,
        account_id=account_id, post_id=post_id)

  def count_likes_by_account(self, requester_id, account_id):
    return self._tclient.count_likes_by_account(requester_id=requester_id,
        account_id=account_id)

  def count_likes_of_post(self, requester_id, post_id):
    return self._tclient.count_likes_of_post(requester_id=requester_id,
        post_id=post_id)
