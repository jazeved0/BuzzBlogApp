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

# Remove old files and generate Thrift code.
rm -rf app/apigateway/server/site-packages
mkdir -p app/apigateway/server/site-packages/buzzblog
thrift -r --gen py -out app/apigateway/server/site-packages/buzzblog app/common/thrift/buzzblog.thrift
for service in $SERVICES
do
  # C++
  rm -rf app/$service/service/server/include
  mkdir -p app/$service/service/server/include/buzzblog/gen
  thrift -r --gen cpp -out app/$service/service/server/include/buzzblog/gen app/common/thrift/buzzblog.thrift
  # Python
  rm -rf app/$service/service/tests/site-packages
  mkdir -p app/$service/service/tests/site-packages/buzzblog
  thrift -r --gen py -out app/$service/service/tests/site-packages/buzzblog app/common/thrift/buzzblog.thrift
done

# Copy 'base_server.h'.
for service in $SERVICES
do
  cp app/common/include/base_server.h app/$service/service/server/include/buzzblog
done

# Copy service client libraries.
for service in $SERVICES
do
  for service_to_copy in $SERVICES
  do
    cp app/$service_to_copy/service/client/src/*.h app/$service/service/server/include/buzzblog
    cp app/$service_to_copy/service/client/src/*.py app/$service/service/tests/site-packages/buzzblog
  done
  cp app/$service/service/client/src/*.py app/apigateway/server/site-packages/buzzblog
done
