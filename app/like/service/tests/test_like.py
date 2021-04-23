# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

import time
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.like_client import Client as LikeClient


IP_ADDRESS = "localhost"
PORT = 9092


class TestService(unittest.TestCase):
  def test_like_post_and_retrieve_standard_like(self):
    with LikeClient(IP_ADDRESS, PORT) as client:
      # Like a post and check the returned object's attributes.
      like = client.like_post(1, 2)
      self.assertAlmostEqual(time.time(), like.created_at, delta=60)
      self.assertEqual(1, like.account_id)
      self.assertEqual(2, like.post_id)
      # Retrieve that like and check its attributes.
      retrieved_like = client.retrieve_standard_like(1, like.id)
      self.assertEqual(like.id, retrieved_like.id)
      self.assertEqual(like.created_at, retrieved_like.created_at)
      self.assertEqual(like.account_id, retrieved_like.account_id)
      self.assertEqual(like.post_id, retrieved_like.post_id)

  def retrieve_expanded_like(self):
    # TODO
    pass

  def delete_like(self):
    # TODO
    pass

  def list_likes(self):
    # TODO
    pass

  def count_likes_by_account(self):
    # TODO
    pass

  def count_likes_of_post(self):
    # TODO
    pass


if __name__ == "__main__":
  unittest.main()
