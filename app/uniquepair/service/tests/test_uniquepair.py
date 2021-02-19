# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

import random
import time
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.uniquepair_client import Client as UniquepairClient


IP_ADDRESS = "localhost"
PORT = 9095


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

  def test_all(self):
    with UniquepairClient(IP_ADDRESS, PORT) as client:
      # Add uniquepairs.
      uniquepairs = []
      for i in range(10):
        uniquepairs.append(client.add("test_all", random.randint(1, 2 ** 16),
            random.randint(1, 2 ** 16)))
        # NOTE: 2s of sleep are needed for the consistency of results because
        # creation times are recorded in the database with precision of 1s.
        time.sleep(2)
      # Retrieve all uniquepairs, reverse the list, and check their ids.
      retrieved_uniquepairs = client.all("test_all")
      retrieved_uniquepairs.reverse()
      for (uniquepair, retrieved_uniquepair) in zip(uniquepairs,
          retrieved_uniquepairs):
        self.assertEqual(uniquepair.id, retrieved_uniquepair.id)

  def test_filter_by_first_elem(self):
    with UniquepairClient(IP_ADDRESS, PORT) as client:
      # Add a uniquepair whose first element is a certain x.
      x = random.randint(1, 2 ** 16)
      uniquepair = client.add("test_filter_by_first_elem", x,
          random.randint(1, 2 ** 16))
      # Add random uniquepairs.
      for i in range(10):
        client.add("test_filter_by_first_elem", random.randint(1, 2 ** 16),
            random.randint(1, 2 ** 16))
      # Filter uniquepairs whose first element is x and check their ids.
      filtered_uniquepairs = client.filter_by_first_elem(
          "test_filter_by_first_elem", x)
      self.assertEqual(1, len(filtered_uniquepairs))
      self.assertEqual(uniquepair.id, filtered_uniquepairs[0].id)

  def test_filter_by_second_elem(self):
    with UniquepairClient(IP_ADDRESS, PORT) as client:
      # Add a uniquepair whose second element is a certain x.
      x = random.randint(1, 2 ** 16)
      uniquepair = client.add("test_filter_by_second_elem",
          random.randint(1, 2 ** 16), x)
      # Add random uniquepairs.
      for i in range(10):
        client.add("test_filter_by_second_elem", random.randint(1, 2 ** 16),
            random.randint(1, 2 ** 16))
      # Filter uniquepairs whose second element is x and check their ids.
      filtered_uniquepairs = client.filter_by_second_elem(
          "test_filter_by_second_elem", x)
      self.assertEqual(1, len(filtered_uniquepairs))
      self.assertEqual(uniquepair.id, filtered_uniquepairs[0].id)

  def test_count_first_elem(self):
    with UniquepairClient(IP_ADDRESS, PORT) as client:
      # Add a uniquepair whose first element is a certain x.
      x = random.randint(1, 2 ** 16)
      uniquepair = client.add("test_count_first_elem", x,
          random.randint(1, 2 ** 16))
      # Add random uniquepairs.
      for i in range(10):
        client.add("test_count_first_elem", random.randint(1, 2 ** 16),
            random.randint(1, 2 ** 16))
      # Count uniquepairs whose first element is x.
      self.assertEqual(1, client.count_first_elem("test_count_first_elem", x))

  def test_count_second_elem(self):
    with UniquepairClient(IP_ADDRESS, PORT) as client:
      # Add a uniquepair whose second element is a certain x.
      x = random.randint(1, 2 ** 16)
      uniquepair = client.add("test_count_second_elem",
          random.randint(1, 2 ** 16), x)
      # Add random uniquepairs.
      for i in range(10):
        client.add("test_count_second_elem", random.randint(1, 2 ** 16),
            random.randint(1, 2 ** 16))
      # Count uniquepairs whose second element is x.
      self.assertEqual(1, client.count_second_elem("test_count_second_elem", x))


if __name__ == "__main__":
  unittest.main()
