# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

import time
import unittest

import requests
from requests.auth import HTTPBasicAuth


SERVER_HOSTNAME = "localhost"
SERVER_PORT = 8080
URL = "{hostname}:{port}".format(hostname=SERVER_HOSTNAME, port=SERVER_PORT)


class TestService(unittest.TestCase):
  def test_create_account_ok(self):
    r = requests.post("http://{url}/account".format(url=URL),
        json={
          "username": "test_mQUO8",
          "password": "abracadabra",
          "first_name": "Test",
          "last_name": "mQUO8"
        }
    )
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual("account", response["object"])
    self.assertEqual("standard", response["mode"])
    self.assertIsInstance(response["id"], int)
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertEqual(True, response["active"])
    self.assertEqual("test_mQUO8", response["username"])
    self.assertEqual("Test", response["first_name"])
    self.assertEqual("mQUO8", response["last_name"])

  def test_create_account_duplicate(self):
    r = requests.post("http://{url}/account".format(url=URL),
        json={
          "username": "test",
          "password": "abracadabra",
          "first_name": "Test",
          "last_name": "Test"
        }
    )
    self.assertEqual(400, r.status_code)

  def test_create_account_missing_parameter(self):
    r = requests.post("http://{url}/account".format(url=URL),
        json={
          "username": "test_x6AnM",
          "first_name": "Test",
          "last_name": "x6AnM"
        }
    )
    self.assertEqual(400, r.status_code)

  def test_create_account_invalid_parameter(self):
    r = requests.post("http://{url}/account".format(url=URL),
        json={
          "username": "",
          "password": "abracadabra",
          "first_name": "Test",
          "last_name": "5nwkF"
        }
    )
    self.assertEqual(400, r.status_code)

  def test_retrieve_account_ok(self):
    r = requests.get("http://{url}/account/{account_id}".format(url=URL,
        account_id=TEST_ACCOUNT_ID),
        auth=HTTPBasicAuth("test", "abracadabra"))
    self.assertEqual(200, r.status_code)
    response = r.json()
    self.assertEqual("account", response["object"])
    self.assertEqual("expanded", response["mode"])
    self.assertEqual(TEST_ACCOUNT_ID, response["id"])
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertEqual(True, response["active"])
    self.assertEqual("test", response["username"])
    self.assertEqual("Test", response["first_name"])
    self.assertEqual("Test", response["last_name"])
    # [TODO] Test expanded fields.

  def test_retrieve_account_unauthorized(self):
    r = requests.get("http://{url}/account/{account_id}".format(url=URL,
        account_id=TEST_ACCOUNT_ID))
    self.assertEqual(401, r.status_code)

  def test_retrieve_account_not_found(self):
    r = requests.get("http://{url}/account/123456789".format(url=URL),
        auth=HTTPBasicAuth("test", "abracadabra"))
    self.assertEqual(404, r.status_code)

  def test_update_account_ok(self):
    r1 = requests.post("http://{url}/account".format(url=URL),
        json={
          "username": "test_kPxhO",
          "password": "abracadabra",
          "first_name": "Test",
          "last_name": "kPxhO"
        }
    )
    self.assertEqual(200, r1.status_code)
    r2 = requests.put("http://{url}/account/{account_id}".format(url=URL,
        account_id=r1.json()["id"]),
        auth=HTTPBasicAuth("test_kPxhO", "abracadabra"),
        json={
          "password": "abracadabra",
          "first_name": "Test",
          "last_name": "zUXAs"
        }
    )
    self.assertEqual(200, r2.status_code)
    response = r2.json()
    self.assertEqual("account", response["object"])
    self.assertEqual("standard", response["mode"])
    self.assertEqual(r1.json()["id"], response["id"])
    self.assertAlmostEqual(time.time(), response["created_at"], delta=60)
    self.assertEqual(True, response["active"])
    self.assertEqual("test_kPxhO", response["username"])
    self.assertEqual("Test", response["first_name"])
    self.assertEqual("zUXAs", response["last_name"])

  def test_update_account_missing_parameter(self):
    r = requests.put("http://{url}/account/{account_id}".format(url=URL,
        account_id=TEST_ACCOUNT_ID),
        auth=HTTPBasicAuth("test", "abracadabra"),
        json={
          "first_name": "Test",
          "last_name": "Test"
        }
    )
    self.assertEqual(400, r.status_code)

  def test_update_account_invalid_parameter(self):
    r = requests.put("http://{url}/account/{account_id}".format(url=URL,
        account_id=TEST_ACCOUNT_ID),
        auth=HTTPBasicAuth("test", "abracadabra"),
        json={
          "password": "",
          "first_name": "Test",
          "last_name": "Test"
        }
    )
    self.assertEqual(400, r.status_code)

  def test_update_account_unauthorized(self):
    r = requests.put("http://{url}/account/{account_id}".format(url=URL,
        account_id=TEST_ACCOUNT_ID),
        json={
          "password": "abracadabra",
          "first_name": "Test",
          "last_name": "Test"
        }
    )
    self.assertEqual(401, r.status_code)

  def test_update_account_forbidden(self):
    r1 = requests.post("http://{url}/account".format(url=URL),
        json={
          "username": "test_FDz8u",
          "password": "abracadabra",
          "first_name": "Test",
          "last_name": "FDz8u"
        }
    )
    self.assertEqual(200, r1.status_code)
    r2 = requests.put("http://{url}/account/{account_id}".format(url=URL,
        account_id=r1.json()["id"]),
        auth=HTTPBasicAuth("test", "abracadabra"),
        json={
          "password": "abracadabra",
          "first_name": "Test",
          "last_name": "1WEVh"
        }
    )
    self.assertEqual(403, r2.status_code)

  def test_delete_account_ok(self):
    # Create account.
    r1 = requests.post("http://{url}/account".format(url=URL),
        json={
          "username": "test_VmGSL",
          "password": "abracadabra",
          "first_name": "Test",
          "last_name": "VmGSL"
        }
    )
    self.assertEqual(200, r1.status_code)
    # Delete account.
    r2 = requests.delete("http://{url}/account/{account_id}".format(url=URL,
        account_id=r1.json()["id"]),
        auth=HTTPBasicAuth("test_VmGSL", "abracadabra"))
    self.assertEqual(200, r2.status_code)
    # Check account's active status.
    r3 = requests.get("http://{url}/account/{account_id}".format(url=URL,
        account_id=r1.json()["id"]),
        auth=HTTPBasicAuth("test", "abracadabra"))
    self.assertEqual(200, r3.status_code)
    response = r3.json()
    self.assertEqual(False, response["active"])

  def test_delete_account_unauthorized(self):
    r = requests.delete("http://{url}/account/{account_id}".format(url=URL,
        account_id=TEST_ACCOUNT_ID))
    self.assertEqual(401, r.status_code)

  def test_delete_account_forbidden(self):
    r1 = requests.post("http://{url}/account".format(url=URL),
        json={
          "username": "test_wQYE9",
          "password": "abracadabra",
          "first_name": "Test",
          "last_name": "wQYE9"
        }
    )
    self.assertEqual(200, r1.status_code)
    r2 = requests.delete("http://{url}/account/{account_id}".format(url=URL,
        account_id=r1.json()["id"]),
        auth=HTTPBasicAuth("test", "abracadabra"))
    self.assertEqual(403, r2.status_code)


if __name__ == "__main__":
  r = requests.post("http://{url}/account".format(url=URL),
      json={
        "username": "test",
        "password": "abracadabra",
        "first_name": "Test",
        "last_name": "Test"
      }
  )
  global TEST_ACCOUNT_ID
  TEST_ACCOUNT_ID = r.json()["id"]
  unittest.main()
