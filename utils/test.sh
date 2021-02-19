#!/bin/bash

# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

# This script deploys BuzzBlog in your local machine using the standard
# configuration (in the 'conf' directory), as described in 'docs/MANUAL.md'.
# Then it runs unit tests for all services and the API Gateway.

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

# Clean Docker artifacts.
utils/clean_docker.sh

# Generate Thrift code and copy service client libraries.
utils/generate_and_copy_code.sh

# Deploy Load Balancer (1 NGINX server).
docker run \
    --name loadbalancer \
    --publish 8888:80 \
    --volume $(pwd)/conf/nginx.conf:/etc/nginx/nginx.conf \
    --detach \
    nginx:1.18.0

# Deploy API Gateway (2 uWSGI servers).
cd app/apigateway/server
docker build -t apigateway:latest .
cd ../../..
docker run \
    --name apigateway1 \
    --publish 8080:81 \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlogApp/backend.yml \
    --volume $(pwd)/conf/uwsgi.ini:/etc/uwsgi/uwsgi.ini \
    --detach \
    apigateway:latest
docker run \
    --name apigateway2 \
    --publish 8081:81 \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlogApp/backend.yml \
    --volume $(pwd)/conf/uwsgi.ini:/etc/uwsgi/uwsgi.ini \
    --detach \
    apigateway:latest

# Deploy Account Service (1 PostgreSQL database server + 2 Thrift multithreaded
# servers).
docker volume create pg_account
docker run \
    --name account_database \
    --publish 5433:5432 \
    --volume pg_account:/var/lib/postgresql/data \
    --env POSTGRES_USER=postgres \
    --env POSTGRES_PASSWORD=postgres \
    --env POSTGRES_DB=postgres \
    --env POSTGRES_HOST_AUTH_METHOD=trust \
    --detach \
    postgres:13.1 \
    -c max_connections=128
sleep 4
psql -U postgres -h localhost -p 5433 -f app/account/database/account_schema.sql
cd app/account/service/server
docker build -t account:latest .
cd ../../../..
docker run \
    --name account_service1 \
    --publish 9090:9090 \
    --env port=9090 \
    --env threads=8 \
    --env backend_filepath=/etc/opt/BuzzBlogApp/backend.yml \
    --env postgres_user=postgres \
    --env postgres_password=postgres \
    --env postgres_dbname=postgres \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlogApp/backend.yml \
    --detach \
    account:latest
docker run \
    --name account_service2 \
    --publish 9091:9091 \
    --env port=9091 \
    --env threads=8 \
    --env backend_filepath=/etc/opt/BuzzBlogApp/backend.yml \
    --env postgres_user=postgres \
    --env postgres_password=postgres \
    --env postgres_dbname=postgres \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlogApp/backend.yml \
    --detach \
    account:latest

# Deploy Follow Service (1 Thrift multithreaded server).
cd app/follow/service/server
docker build -t follow:latest .
cd ../../../..
docker run \
    --name follow_service \
    --publish 9092:9092 \
    --env port=9092 \
    --env threads=8 \
    --env backend_filepath=/etc/opt/BuzzBlogApp/backend.yml \
    --env postgres_user=postgres \
    --env postgres_password=postgres \
    --env postgres_dbname=postgres \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlogApp/backend.yml \
    --detach \
    follow:latest

# Deploy Like Service (1 Thrift multithreaded server).
cd app/like/service/server
docker build -t like:latest .
cd ../../../..
docker run \
    --name like_service \
    --publish 9093:9093 \
    --env port=9093 \
    --env threads=8 \
    --env backend_filepath=/etc/opt/BuzzBlogApp/backend.yml \
    --env postgres_user=postgres \
    --env postgres_password=postgres \
    --env postgres_dbname=postgres \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlogApp/backend.yml \
    --detach \
    like:latest

# Deploy Post Service (1 PostgreSQL database server + 1 Thrift multithreaded
# server).
docker volume create pg_post
docker run \
    --name post_database \
    --publish 5434:5432 \
    --volume pg_post:/var/lib/postgresql/data \
    --env POSTGRES_USER=postgres \
    --env POSTGRES_PASSWORD=postgres \
    --env POSTGRES_DB=postgres \
    --env POSTGRES_HOST_AUTH_METHOD=trust \
    --detach \
    postgres:13.1 \
    -c max_connections=128
sleep 4
psql -U postgres -h localhost -p 5434 -f app/post/database/post_schema.sql
cd app/post/service/server
docker build -t post:latest .
cd ../../../..
docker run \
    --name post_service \
    --publish 9094:9094 \
    --env port=9094 \
    --env threads=8 \
    --env backend_filepath=/etc/opt/BuzzBlogApp/backend.yml \
    --env postgres_user=postgres \
    --env postgres_password=postgres \
    --env postgres_dbname=postgres \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlogApp/backend.yml \
    --detach \
    post:latest

# Deploy Uniquepair Service (1 PostgreSQL database server + 1 Thrift
# multithreaded server).
docker volume create pg_uniquepair
docker run \
    --name uniquepair_database \
    --publish 5435:5432 \
    --volume pg_uniquepair:/var/lib/postgresql/data \
    --env POSTGRES_USER=postgres \
    --env POSTGRES_PASSWORD=postgres \
    --env POSTGRES_DB=postgres \
    --env POSTGRES_HOST_AUTH_METHOD=trust \
    --detach \
    postgres:13.1 \
    -c max_connections=128
sleep 4
psql -U postgres -h localhost -p 5435 -f app/uniquepair/database/uniquepair_schema.sql
cd app/uniquepair/service/server
docker build -t uniquepair:latest .
cd ../../../..
docker run \
    --name uniquepair_service \
    --publish 9095:9095 \
    --env port=9095 \
    --env threads=8 \
    --env backend_filepath=/etc/opt/BuzzBlogApp/backend.yml \
    --env postgres_user=postgres \
    --env postgres_password=postgres \
    --env postgres_dbname=postgres \
    --volume $(pwd)/conf/backend.yml:/etc/opt/BuzzBlogApp/backend.yml \
    --detach \
    uniquepair:latest

# Run unit tests for all services and the API Gateway.
for service in account follow like post uniquepair
do
  export PYTHONPATH=app/$service/service/tests/site-packages/
  python3 app/$service/service/tests/test_$service.py
done
python3 app/apigateway/tests/test_api.py
