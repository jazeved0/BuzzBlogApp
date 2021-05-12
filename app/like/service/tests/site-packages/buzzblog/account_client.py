# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from buzzblog.gen import TAccountService


class Client:
  def __init__(self, ip_address, port, timeout=10000):
    self._socket = TSocket.TSocket(ip_address, port)
    self._socket.setTimeout(timeout)
    self._transport = TTransport.TBufferedTransport(self._socket)
    self._protocol = TBinaryProtocol.TBinaryProtocol(self._transport)
    self._tclient = TAccountService.Client(self._protocol)
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

  def authenticate_user(self, request_metadata, username, password):
    return self._tclient.authenticate_user(request_metadata=request_metadata,
        username=username, password=password)

  def create_account(self, request_metadata, username, password, first_name,
      last_name):
    return self._tclient.create_account(request_metadata=request_metadata,
        username=username, password=password, first_name=first_name,
        last_name=last_name)

  def retrieve_standard_account(self, request_metadata, account_id):
    return self._tclient.retrieve_standard_account(
        request_metadata=request_metadata, account_id=account_id)

  def retrieve_expanded_account(self, request_metadata, account_id):
    return self._tclient.retrieve_expanded_account(
        request_metadata=request_metadata, account_id=account_id)

  def update_account(self, request_metadata, account_id, password, first_name,
      last_name):
    return self._tclient.update_account(request_metadata=request_metadata,
        account_id=account_id, password=password, first_name=first_name,
        last_name=last_name)

  def delete_account(self, request_metadata, account_id):
    return self._tclient.delete_account(request_metadata=request_metadata,
        account_id=account_id)
