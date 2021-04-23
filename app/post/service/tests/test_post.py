# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

import time
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.post_client import Client as PostClient


IP_ADDRESS = "localhost"
PORT = 9093


class TestService(unittest.TestCase):
  def test_create_and_retrieve_standard_post(self):
    with PostClient(IP_ADDRESS, PORT) as client:
      # Create post and check its attributes.
      post = client.create_post(1, "Test message")
      self.assertAlmostEqual(time.time(), post.created_at, delta=60)
      self.assertTrue(post.active)
      self.assertEqual("Test message", post.text)
      self.assertEqual(1, post.author_id)
      # Retrieve that post and check its attributes.
      retrieved_post = client.retrieve_standard_post(1, post.id)
      self.assertEqual(post.id, retrieved_post.id)
      self.assertEqual(post.created_at, retrieved_post.created_at)
      self.assertEqual(post.active, retrieved_post.active)
      self.assertEqual(post.text, retrieved_post.text)
      self.assertEqual(post.author_id, retrieved_post.author_id)

  def test_retrieve_expanded_post(self):
    # TODO
    pass

  def test_delete_post(self):
    # TODO
    pass

  def test_list_posts(self):
    # TODO
    pass

  def test_count_posts_by_author(self):
    # TODO
    pass


if __name__ == "__main__":
  unittest.main()
