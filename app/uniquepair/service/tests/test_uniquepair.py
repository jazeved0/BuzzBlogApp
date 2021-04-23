# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

import random
import time
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.uniquepair_client import Client as UniquepairClient


IP_ADDRESS = "localhost"
PORT = 9094


class TestService(unittest.TestCase):
  def test_add_and_get(self):
    with UniquepairClient(IP_ADDRESS, PORT) as client:
      # Add uniquepair and check its attributes.
      first_elem = random.randint(1, 2 ** 16)
      second_elem = random.randint(1, 2 ** 16)
      uniquepair = client.add("test_add_and_get", first_elem, second_elem)
      self.assertEqual("test_add_and_get", uniquepair.domain)
      self.assertAlmostEqual(time.time(), uniquepair.created_at, delta=60)
      self.assertEqual(first_elem, uniquepair.first_elem)
      self.assertEqual(second_elem, uniquepair.second_elem)
      # Get that uniquepair and check its attributes.
      gotten_uniquepair = client.get(uniquepair.id)
      self.assertEqual(uniquepair.id, gotten_uniquepair.id)
      self.assertEqual(uniquepair.created_at,
          gotten_uniquepair.created_at)
      self.assertEqual(uniquepair.domain, gotten_uniquepair.domain)
      self.assertEqual(uniquepair.first_elem,
          gotten_uniquepair.first_elem)
      self.assertEqual(uniquepair.second_elem,
          gotten_uniquepair.second_elem)

  def test_remove(self):
    with UniquepairClient(IP_ADDRESS, PORT) as client:
      # Add uniquepair.
      uniquepair = client.add("test_remove", random.randint(1, 2 ** 16),
          random.randint(1, 2 ** 16))
      # Remove that uniquepair and check that it cannot be gotten anymore.
      client.remove(uniquepair.id)
      with self.assertRaises(TUniquepairNotFoundException):
        client.get(uniquepair.id)

  def test_find(self):
    with UniquepairClient(IP_ADDRESS, PORT) as client:
      # Add uniquepair.
      first_elem = random.randint(1, 2 ** 16)
      second_elem = random.randint(1, 2 ** 16)
      uniquepair = client.add("test_find", first_elem, second_elem)
      # Find that uniquepair and check its id.
      found_uniquepair = client.find("test_find", first_elem, second_elem)
      self.assertEqual(uniquepair.id, found_uniquepair.id)

  def test_fetch(self):
    with UniquepairClient(IP_ADDRESS, PORT) as client:
      # Add 10 random uniquepairs.
      uniquepairs = []
      for i in range(10):
        uniquepairs.append(client.add("test_fetch", random.randint(1, 2 ** 16),
            random.randint(1, 2 ** 16)))
        # NOTE: 2s of sleep are needed for the consistency of results because
        # creation times are recorded in the database with precision of 1s.
        time.sleep(2)
      # Fetch the 10 first uniquepairs, reverse the list, and check their ids.
      fetched_uniquepairs = client.fetch(
          TUniquepairQuery(domain="test_fetch"), 10, 0)
      fetched_uniquepairs.reverse()
      for (uniquepair, fetched_uniquepair) in zip(uniquepairs,
          fetched_uniquepairs):
        self.assertEqual(uniquepair.id, fetched_uniquepair.id)
      # Add a uniquepair whose first element is 0.
      uniquepair = client.add("test_fetch", 0, random.randint(1, 2 ** 16))
      # Fetch uniquepairs whose first element is 0 and check their ids.
      fetched_uniquepairs = client.fetch(
          TUniquepairQuery(domain="test_fetch", first_elem=0), 10, 0)
      self.assertEqual(1, len(fetched_uniquepairs))
      self.assertEqual(uniquepair.id, fetched_uniquepairs[0].id)
      # Add a uniquepair whose second element is 0.
      uniquepair = client.add("test_fetch", random.randint(1, 2 ** 16), 0)
      # Fetch uniquepairs whose second element is 0 and check their ids.
      fetched_uniquepairs = client.fetch(
          TUniquepairQuery(domain="test_fetch", second_elem=0), 10, 0)
      self.assertEqual(1, len(fetched_uniquepairs))
      self.assertEqual(uniquepair.id, fetched_uniquepairs[0].id)

  def test_count(self):
    with UniquepairClient(IP_ADDRESS, PORT) as client:
      # Add 10 random uniquepairs.
      for i in range(10):
        client.add("test_count", random.randint(1, 2 ** 16),
            random.randint(1, 2 ** 16))
      # Add a uniquepair whose first element is 0.
      client.add("test_count", 0, random.randint(1, 2 ** 16))
      # Add a uniquepair whose second element is 0.
      client.add("test_count", random.randint(1, 2 ** 16), 0)
      # Count uniquepairs whose first element is 0.
      self.assertEqual(1, client.count(TUniquepairQuery(domain="test_count",
          first_elem=0)))
      # Count uniquepairs whose second element is 0.
      self.assertEqual(1, client.count(TUniquepairQuery(domain="test_count",
          second_elem=0)))


if __name__ == "__main__":
  unittest.main()
