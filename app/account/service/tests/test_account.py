# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

import time
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.account_client import Client as AccountClient


IP_ADDRESS = "localhost"
PORT = 9090


class TestService(unittest.TestCase):
  def test_create_and_retrieve_standard_account(self):
    with AccountClient(IP_ADDRESS, PORT) as client:
      # Create account and check its attributes.
      account = client.create_account(TRequestMetadata(id="1"), "john_doe",
          "strongpasswd", "John", "Doe")
      self.assertAlmostEqual(time.time(), account.created_at, delta=60)
      self.assertTrue(account.active)
      self.assertEqual("john_doe", account.username)
      self.assertEqual("John", account.first_name)
      self.assertEqual("Doe", account.last_name)
      # Retrieve that account and check its attributes.
      retrieved_account = client.retrieve_standard_account(
          TRequestMetadata(id="2", requester_id=account.id), account.id)
      self.assertEqual(account.id, retrieved_account.id)
      self.assertEqual(account.created_at, retrieved_account.created_at)
      self.assertEqual(account.active, retrieved_account.active)
      self.assertEqual(account.username, retrieved_account.username)
      self.assertEqual(account.first_name, retrieved_account.first_name)
      self.assertEqual(account.last_name, retrieved_account.last_name)

  def test_authenticate_user(self):
    # TODO
    pass

  def test_retrieve_expanded_account(self):
    # TODO
    pass

  def test_update_account(self):
    # TODO
    pass

  def test_delete_account(self):
    # TODO
    pass


if __name__ == "__main__":
  unittest.main()
