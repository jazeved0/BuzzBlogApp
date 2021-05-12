# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from buzzblog.gen import TFollowService


class Client:
  def __init__(self, ip_address, port, timeout=10000):
    self._socket = TSocket.TSocket(ip_address, port)
    self._socket.setTimeout(timeout)
    self._transport = TTransport.TBufferedTransport(self._socket)
    self._protocol = TBinaryProtocol.TBinaryProtocol(self._transport)
    self._tclient = TFollowService.Client(self._protocol)
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

  def follow_account(self, request_metadata, account_id):
    return self._tclient.follow_account(request_metadata=request_metadata,
        account_id=account_id)

  def retrieve_standard_follow(self, request_metadata, follow_id):
    return self._tclient.retrieve_standard_follow(
        request_metadata=request_metadata, follow_id=follow_id)

  def retrieve_expanded_follow(self, request_metadata, follow_id):
    return self._tclient.retrieve_expanded_follow(
        request_metadata=request_metadata, follow_id=follow_id)

  def delete_follow(self, request_metadata, follow_id):
    return self._tclient.delete_follow(request_metadata=request_metadata,
        follow_id=follow_id)

  def list_follows(self, request_metadata, query, limit, offset):
    return self._tclient.list_follows(request_metadata=request_metadata,
        query=query, limit=limit, offset=offset)

  def check_follow(self, request_metadata, follower_id, followee_id):
    return self._tclient.check_follow(request_metadata=request_metadata,
        follower_id=follower_id, followee_id=followee_id)

  def count_followers(self, request_metadata, account_id):
    return self._tclient.count_followers(request_metadata=request_metadata,
        account_id=account_id)

  def count_followees(self, request_metadata, account_id):
    return self._tclient.count_followees(request_metadata=request_metadata,
        account_id=account_id)
