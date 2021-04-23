# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

import random
import string
import time
import unittest

from buzzblog.gen.ttypes import *
from buzzblog.account_client import Client as AccountClient
from buzzblog.follow_client import Client as FollowClient


IP_ADDRESS = "localhost"
ACCOUNT_PORT = 9090
FOLLOW_PORT = 9091


def random_id(size=16, chars=string.ascii_letters + string.digits):
  return ''.join(random.choice(chars) for _ in range(size))


class TestService(unittest.TestCase):
  def test_follow_account_and_retrieve_standard_follow(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create test accounts.
      accounts = [
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell")
      ]
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Follow an account and check the returned object's attributes.
      follow = client.follow_account(accounts[0].id, accounts[1].id)
      self.assertAlmostEqual(time.time(), follow.created_at, delta=60)
      self.assertEqual(accounts[0].id, follow.follower_id)
      self.assertEqual(accounts[1].id, follow.followee_id)
      # Check that it cannot be duplicated.
      with self.assertRaises(TFollowAlreadyExistsException):
        client.follow_account(accounts[0].id, accounts[1].id)
      # Retrieve that follow and check its attributes.
      retrieved_follow = client.retrieve_standard_follow(accounts[0].id,
          follow.id)
      self.assertEqual(follow.id, retrieved_follow.id)
      self.assertEqual(follow.created_at, retrieved_follow.created_at)
      self.assertEqual(follow.follower_id, retrieved_follow.follower_id)
      self.assertEqual(follow.followee_id, retrieved_follow.followee_id)

  def test_follow_account_and_retrieve_expanded_follow(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create test accounts.
      accounts = [
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell")
      ]
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Follow an account and check the returned object's attributes.
      follow = client.follow_account(accounts[0].id, accounts[1].id)
      self.assertAlmostEqual(time.time(), follow.created_at, delta=60)
      self.assertEqual(accounts[0].id, follow.follower_id)
      self.assertEqual(accounts[1].id, follow.followee_id)
      # Check that it cannot be duplicated.
      with self.assertRaises(TFollowAlreadyExistsException):
        client.follow_account(accounts[0].id, accounts[1].id)
      # Retrieve that follow and check its attributes.
      retrieved_follow = client.retrieve_expanded_follow(accounts[0].id,
          follow.id)
      self.assertEqual(follow.id, retrieved_follow.id)
      self.assertEqual(follow.created_at, retrieved_follow.created_at)
      self.assertEqual(follow.follower_id, retrieved_follow.follower_id)
      self.assertEqual(follow.followee_id, retrieved_follow.followee_id)
      self.assertEqual(follow.follower_id, retrieved_follow.follower.id)
      self.assertEqual(follow.followee_id, retrieved_follow.followee.id)

  def test_delete_follow(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create test accounts.
      accounts = [
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell")
      ]
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Follow an account.
      follow = client.follow_account(accounts[0].id, accounts[1].id)
      # Remove that follow and check that it cannot be gotten anymore.
      client.delete_follow(accounts[0].id, follow.id)
      with self.assertRaises(TFollowNotFoundException):
        client.retrieve_standard_follow(accounts[0].id, follow.id)

  def test_list_follows(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create test accounts.
      accounts = [
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell")
      ]
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Create follows.
      follows = []
      for followee in accounts[1:]:
        follows.append(client.follow_account(accounts[0].id, followee.id))
        # NOTE: 2s of sleep are needed for the consistency of results because
        # creation times are recorded in the database with precision of 1s.
        time.sleep(2)
      # Retrieve those follows, reverse the list, and check their ids.
      retrieved_follows = client.list_follows(accounts[0].id, accounts[0].id,
          -1)
      retrieved_follows.reverse()
      for (follow, retrieved_follow) in zip(follows, retrieved_follows):
        self.assertEqual(follow.id, retrieved_follow.id)

  def test_check_follow(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create test accounts.
      accounts = [
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell")
      ]
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Check that a follow does not exist.
      self.assertFalse(client.check_follow(accounts[0].id, accounts[0].id,
          accounts[1].id))
      # Create that follow.
      client.follow_account(accounts[0].id, accounts[1].id)
      # Check that follow now exists.
      self.assertTrue(client.check_follow(accounts[0].id, accounts[0].id,
          accounts[1].id))

  def test_count_followers(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create test accounts.
      accounts = [
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell")
      ]
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Follow an account 3 times.
      for follower in accounts[1:]:
        client.follow_account(follower.id, accounts[0].id)
      # Check that account has 3 followers.
      self.assertEqual(3,
          client.count_followers(accounts[0].id, accounts[0].id))

  def test_count_followees(self):
    with AccountClient(IP_ADDRESS, ACCOUNT_PORT) as client:
      # Create test accounts.
      accounts = [
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell"),
          client.create_account(random_id(), "passwd", "George", "Burdell")
      ]
    with FollowClient(IP_ADDRESS, FOLLOW_PORT) as client:
      # Make an account follow 3 different accounts.
      for followee in accounts[1:]:
        client.follow_account(accounts[0].id, followee.id)
      # Check that account has 3 followees.
      self.assertEqual(3,
          client.count_followees(accounts[0].id, accounts[0].id))


if __name__ == "__main__":
  unittest.main()
