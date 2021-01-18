#!/bin/bash

# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

# Define constants.
SERVICES="account follow like post uniquepair"

# Change to the parent directory.
cd "$(dirname "$(dirname "$(readlink -fm "$0")")")"

# Process command-line arguments.
set -u
while [[ $# > 1 ]]; do
  case $1 in
    * )
      echo "Invalid argument: $1"
      exit 1
  esac
  shift
  shift
done

# Remove old files.
rm -rf app/api_gateway/server/site-packages
for service in $SERVICES
do
  rm -rf app/$service/service/server/include
done

# Generate Thrift code.
mkdir -p app/api_gateway/server/site-packages/gen
thrift -r --gen py -out app/api_gateway/server/site-packages/gen app/common/thrift/buzzblog.thrift
for service in $SERVICES
do
  mkdir -p app/$service/service/server/include/gen
  thrift -r --gen cpp -out app/$service/service/server/include/gen app/common/thrift/buzzblog.thrift
done

# Copy service client libraries.
touch app/api_gateway/server/site-packages/__init__.py
for service in $SERVICES
do
  cp app/common/include/base_server.h app/$service/service/server/include/
  cp app/$service/service/client/src/*.py app/api_gateway/server/site-packages/
  for service_to_copy in $SERVICES
  do
    cp app/$service_to_copy/service/client/src/*.h app/$service/service/server/include/
  done
done
