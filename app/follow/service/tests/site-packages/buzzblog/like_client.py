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

  def like_post(self, request_metadata, post_id):
    return self._tclient.like_post(request_metadata=request_metadata,
        post_id=post_id)

  def retrieve_standard_like(self, request_metadata, like_id):
    return self._tclient.retrieve_standard_like(
        request_metadata=request_metadata, like_id=like_id)

  def retrieve_expanded_like(self, request_metadata, like_id):
    return self._tclient.retrieve_expanded_like(
        request_metadata=request_metadata, like_id=like_id)

  def delete_like(self, request_metadata, like_id):
    return self._tclient.delete_like(request_metadata=request_metadata,
        like_id=like_id)

  def list_likes(self, request_metadata, query, limit, offset):
    return self._tclient.list_likes(request_metadata=request_metadata,
        query=query, limit=limit, offset=offset)

  def count_likes_by_account(self, request_metadata, account_id):
    return self._tclient.count_likes_by_account(
        request_metadata=request_metadata, account_id=account_id)

  def count_likes_of_post(self, request_metadata, post_id):
    return self._tclient.count_likes_of_post(request_metadata=request_metadata,
        post_id=post_id)
