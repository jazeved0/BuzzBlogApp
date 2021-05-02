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

  def follow_account(self, requester_id, account_id):
    return self._tclient.follow_account(requester_id=requester_id,
        account_id=account_id)

  def retrieve_standard_follow(self, requester_id, follow_id):
    return self._tclient.retrieve_standard_follow(requester_id=requester_id,
        follow_id=follow_id)

  def retrieve_expanded_follow(self, requester_id, follow_id):
    return self._tclient.retrieve_expanded_follow(requester_id=requester_id,
        follow_id=follow_id)

  def delete_follow(self, requester_id, follow_id):
    return self._tclient.delete_follow(requester_id=requester_id,
        follow_id=follow_id)

  def list_follows(self, requester_id, query, limit, offset):
    return self._tclient.list_follows(requester_id=requester_id,
        query=query, limit=limit, offset=offset)

  def check_follow(self, requester_id, follower_id, followee_id):
    return self._tclient.check_follow(requester_id=requester_id,
        follower_id=follower_id, followee_id=followee_id)

  def count_followers(self, requester_id, account_id):
    return self._tclient.count_followers(requester_id=requester_id,
        account_id=account_id)

  def count_followees(self, requester_id, account_id):
    return self._tclient.count_followees(requester_id=requester_id,
        account_id=account_id)
