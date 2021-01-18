# Load Balancer

python3 utils/generate_nginx_conf.py \
    --topology_filepath=conf/topology.yml \
    --worker_processes 8 \
    --worker_connections 512 \
    --outdir app/load_balancer/etc

sudo docker run \
    --name load_balancer \
    --publish 8080:80 \
    --volume $(pwd)/app/load_balancer/etc/nginx.conf:/etc/nginx/nginx.conf \
    --detach \
    nginx:1.18.0

# API Gateway

./utils/generate_and_copy_code.sh

cd app/api_gateway/server
sudo docker build -t api_gateway:latest .
cd ../../..

sudo docker run \
    --name api_gateway1 \
    --publish 8081:81 \
    --env port=81 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    api_gateway:latest

sudo docker run \
    --name api_gateway2 \
    --publish 8082:81 \
    --env port=81 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    api_gateway:latest

# Account

sudo docker volume create pg_account

sudo docker run \
    --name account_postgres \
    --publish 5433:5432 \
    --volume pg_account:/var/lib/postgresql/data \
    --env POSTGRES_USER=postgres \
    --env POSTGRES_PASSWORD=postgres \
    --env POSTGRES_DB=postgres \
    --env POSTGRES_HOST_AUTH_METHOD=trust \
    --detach \
    postgres:13.1 \
    -c max_connections=128

psql -U postgres -h localhost -p 5433 -f app/account/database/account_schema.sql

./utils/generate_and_copy_code.sh

cd app/account/service/server
sudo docker build -t account:latest .
cd ../../../..

sudo docker run \
    --name account_service1 \
    --publish 9090:9090 \
    --env port=9090 \
    --env threads=8 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    account:latest

sudo docker run \
    --name account_service2 \
    --publish 9091:9091 \
    --env port=9091 \
    --env threads=8 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    account:latest

# Follow

./utils/generate_and_copy_code.sh

cd app/follow/service/server
sudo docker build -t follow:latest .
cd ../../../..

sudo docker run \
    --name follow_service \
    --publish 9092:9092 \
    --env port=9092 \
    --env threads=8 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    follow:latest

# Like

./utils/generate_and_copy_code.sh

cd app/like/service/server
sudo docker build -t like:latest .
cd ../../../..

sudo docker run \
    --name like_service \
    --publish 9093:9093 \
    --env port=9093 \
    --env threads=8 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    like:latest

# Post

sudo docker volume create pg_post

sudo docker run \
    --name post_postgres \
    --publish 5434:5432 \
    --volume pg_post:/var/lib/postgresql/data \
    --env POSTGRES_USER=postgres \
    --env POSTGRES_PASSWORD=postgres \
    --env POSTGRES_DB=postgres \
    --env POSTGRES_HOST_AUTH_METHOD=trust \
    --detach \
    postgres:13.1 \
    -c max_connections=128

psql -U postgres -h localhost -p 5434 -f app/post/database/post_schema.sql

./utils/generate_and_copy_code.sh

cd app/post/service/server
sudo docker build -t post:latest .
cd ../../../..

sudo docker run \
    --name post_service1 \
    --publish 9094:9094 \
    --env port=9094 \
    --env threads=8 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    post:latest

sudo docker run \
    --name post_service2 \
    --publish 9095:9095 \
    --env port=9095 \
    --env threads=8 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    post:latest

# Uniquepair

sudo docker volume create pg_uniquepair

sudo docker run \
    --name uniquepair_postgres \
    --publish 5435:5432 \
    --volume pg_uniquepair:/var/lib/postgresql/data \
    --env POSTGRES_USER=postgres \
    --env POSTGRES_PASSWORD=postgres \
    --env POSTGRES_DB=postgres \
    --env POSTGRES_HOST_AUTH_METHOD=trust \
    --detach \
    postgres:13.1 \
    -c max_connections=128

psql -U postgres -h localhost -p 5435 -f app/uniquepair/database/uniquepair_schema.sql

./utils/generate_and_copy_code.sh

cd app/uniquepair/service/server
sudo docker build -t uniquepair:latest .
cd ../../../..

sudo docker run \
    --name uniquepair_service1 \
    --publish 9096:9096 \
    --env port=9096 \
    --env threads=8 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    uniquepair:latest

sudo docker run \
    --name uniquepair_service2 \
    --publish 9097:9097 \
    --env port=9097 \
    --env threads=8 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    uniquepair:latest
