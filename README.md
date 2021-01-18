# BuzzBlog
BuzzBlog is an open-source microblogging application. As with Twitter, users can
write short posts, follow other users, and like their favorite posts.

BuzzBlog is part of [Wise](https://www.cc.gatech.edu/systems/projects/Elba/Wise/),
a toolkit for the experimental study of systems performance. Specifically,
BuzzBlog was developed for the study of performance bugs caused by sub-second
resource contention in microservice architectures.

BuzzBlog was developed by Rodrigo Alves Lima (<ral@gatech.edu>) and is licensed
under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0).

## Architecture
[TODO: Add updated architecture figure]

### API Gateway
The API Gateway sits between clients and backend services. It receives HTTP
requests representing calls to the BuzzBlog API, calls the backend services
needed to fulfil each request, and sends HTTP responses back to the clients
containing JSON-encoded textual representations of the results of these calls.

To enable horizontal scalability (i.e., the ability to increase computing
capacity by adding more API Gateway servers), NGINX can be deployed as a load
balancer in front of multiple API Gateway servers. In this way, requests first
arrive at the NGINX server, which then chooses one of the API Gateway servers to
forward each request.

To learn how to build HTTP requests representing calls to the BuzzBlog API and
what to expect as response, see the [BuzzBlog API Reference](docs/API.md).
It lists all API endpoints available, their parameters, and their response data
format.

### Backend Services
The backend services have small scope, conforming to the microservice
architectural style. Four services implement business capabilities: *account* is
responsible for user authentication and account management, *post* for posting,
*like* for post liking, and *follow* for user following. The other service,
*uniquepair*, implements an abstract data type (sets of unique pairs).

Three of these backend services (*account*, *post*, and *uniquepair*) store data
in PostgreSQL databases. To help reduce the latency of requests, Redis can be
deployed as cache in front of the databases.

## Request Lifecycle
The BuzzBlog API is organized around REST, using standard HTTP methods and
authentication to access and manipulate objects through predictable URLs. For
example, a request whose HTTP method is `GET` and URL path is
`/account/:account_id` receives, as response, the JSON-encoded textual
representation of the account identified by `account_id`.

A call to the BuzzBlog API starts with the client sending an HTTP request.
If deployed, the NGINX load balancer is the first to receive incoming requests.
NGINX uses a load-balancing algorithm (for example, Round Robin) for choosing an
API Gateway server to forward each request.

Upon the arrival of an HTTP request, the API Gateway selects the proper request
handler based on its URL path and HTTP method, as defined in the
[BuzzBlog API Reference](docs/API.md). In the previous example, an HTTP request
whose method is `GET` and URL path is `/account/:account_id` is handled by
function `retrieve_account` (see `app/api_gateway/server/src/api_gateway.py`).

A request handler of the API Gateway accesses and manipulates objects through
calls to the backend services. Continuing with the same example,
`retrieve_account` retrieves account information by calling method
`retrieve_expanded_account` of the *account* service (see
`app/account/service/server/src/account_server.cpp`).

Backend services commonly call other backend services to retrieve related data,
and services with persisent state also query their associated Redis cache and
PostgreSQL database. For example, that `retrieve_expanded_account` method
queries the Redis cache and, if needed, the PostgreSQL database to retrieve
account information. It also calls services *post*, *like*, and *follow* to
retrieve a summary of the posting, liking, and following activities of the
account, respectively.

With the results of successful calls to the backend services, the request
handler of the API Gateway builds an HTTP response containing the JSON-encoded
textual representation of one or more objects and sends it back to the client.

[TODO: Add a new request lifecycle figure]

## User Manual

### Deployment
This running example shows how to deploy BuzzBlog in a single host, with all
servers running in Docker containers. The topology used is:
* Load Balancer: 1 NGINX server
* API Gateway: 2 uWSGI servers
* Account service: 2 Thrift multithreaded servers
* Account database: 1 PostgreSQL database server
* Follow service: 1 Thrift multithreaded server
* Like service: 1 Thrift multithreaded server
* Post service: 2 Thrift multithreaded servers
* Post database: 1 PostgreSQL database server
* Uniquepair service: 2 Thrift multithreaded servers
* Uniquepair database: 1 PostgreSQL database server

After running this example, adapting files and commands to deploy BuzzBlog in
multiple hosts using different configurations should be trivial. Also, note that
databases are limited to a single instance per service and that load balancer
and caches are optional.

#### Configuration
First, update `conf/topology.yml` with the hostnames and ports on which all
servers will listen to connections. This file is read by most servers in order
to connect to other servers.
```
load_balancer: "172.17.0.1:8080"
api_gateway:
  - "172.17.0.1:8081"
  - "172.17.0.1:8082"
account:
  service:
    - "172.17.0.1:9090"
    - "172.17.0.1:9091"
  database: "172.17.0.1:5433"
  cache: "172.17.0.1:6479"
follow:
  service:
    - "172.17.0.1:9092"
like:
  service:
    - "172.17.0.1:9093"
post:
  service:
    - "172.17.0.1:9094"
    - "172.17.0.1:9095"
  database: "172.17.0.1:5434"
  cache: "172.17.0.1:6480"
uniquepair:
  service:
    - "172.17.0.1:9096"
    - "172.17.0.1:9097"
  database: "172.17.0.1:5435"
  cache: "172.17.0.1:6481"
```

### Load Balancer
#### Deployment
To deploy the NGINX server running in a Docker container:
1. Create the NGINX configuration file `app/load_balancer/etc/nginx.conf`. Here
we set the server to listen on port 80, use 8 worker processes, and limit the
number of simultaneous connections that can be opened by a worker process to
512.
```
worker_processes 8;

events {
  worker_connections 512;
}

http {
  include /etc/nginx/mime.types;
  default_type application/octet-stream;
  keepalive_timeout 0;
  upstream backend {
    server 172.17.0.1:8081;
    server 172.17.0.1:8082;
  }
  server {
    listen 80;
    location / {
      proxy_pass http://backend;
    }
  }
}
```
2. Run a Docker container based on the official NGINX image. Here we name the
container `load_balancer`, publish its port 80 to the host port 8080, and
bind-mount the NGINX configuration file.
```
sudo docker run \
    --name load_balancer \
    --publish 8080:80 \
    --volume $(pwd)/app/load_balancer/etc/nginx.conf:/etc/nginx/nginx.conf \
    --detach \
    nginx:1.18.0
```

### API Gateway
The API Gateway is implemented in Python using the Flask web framework. Its
source code is in `app/api_gateway/server`.

#### Deployment
To deploy the API Gateway with a uWSGI server running in a Docker container:
1. Generate Thrift code and copy dependencies, such as service client libraries.
```
./utils/generate_and_copy_code.sh
```
2. Build the Docker image. Here we name the image `api_gateway:latest`.
```
cd app/api_gateway/server
sudo docker build -t api_gateway:latest .
```
3. Run Docker containers based on the newly built image. Here we name these
containers `api_gateway1` and `api_gateway2`, publish their ports 81 to the host
ports 8081 and 8082, respectively, and bind-mount the topology configuration
file.
```
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
```
To configure the uWSGI server, pass environment variables as defined in the
[documentation](https://uwsgi-docs.readthedocs.io/en/latest/Configuration.html).
```
...
--env UWSGI_MASTER=1
--env UWSGI_WORKERS=32
...
```

### Account Service
The *account* service is implemented in C++ using the Thrift framework and have
client libraries available in Python and C++. Its source code is in
`app/account/service`.

The *account* service persists data in a PostgreSQL database whose schema is in
`app/account/database`.

#### Deployment
To deploy the PostgreSQL server running in a Docker container and set up the
database:
1. Create a Docker volume. Here we name the volume `pg_account`.
```
sudo docker volume create pg_account
```

2. Run a Docker container based on the official PostgreSQL image. Here we name
the container `account_database`, publish its port 5432 to the host port 5433,
create superuser and database named `postgres`, enable the `trust`
authentication mode, and limit the number of concurrent connections to 128.
```
sudo docker run \
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
```
3. Set up the database.
```
psql -U postgres -h localhost -p 5433 -f app/account/database/account_schema.sql
```

To deploy the *account* service with a multithreaded Thrift server running in a
Docker container:
1. Generate Thrift code and copy dependencies, such as service client libraries.
```
./utils/generate_and_copy_code.sh
```
2. Build the Docker image. Here we name the image `account:latest`.
```
cd app/account/service/server
sudo docker build -t account:latest .
```
3. Run Docker containers based on the newly built image. Here we name these
containers `account_service1` and `account_service2`, publish their ports 9090
and 9091, respectively, to the same host ports, set their number of Thrift
server threads to 8, and bind-mount the topology configuration file.
```
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
```

### Follow Service
The *follow* service is implemented in C++ using the Thrift framework and have
client libraries available in Python and C++. Its source code is in
`app/follow/service`.

#### Deployment
To deploy the *follow* service with a multithreaded Thrift server running in a
Docker container:
1. Generate Thrift code and copy dependencies, such as service client libraries.
```
./utils/generate_and_copy_code.sh
```
2. Build the Docker image. Here we name the image `follow:latest`.
```
cd app/follow/service/server
sudo docker build -t follow:latest .
```
3. Run a Docker container based on the newly built image. Here we name the
container `follow_service`, publish its port 9092 to the same host port, set its
number of Thrift server threads to 8, and bind-mount the topology configuration
file.
```
sudo docker run \
    --name follow_service \
    --publish 9092:9092 \
    --env port=9092 \
    --env threads=8 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    follow:latest
```

### Like Service
The *like* service is implemented in C++ using the Thrift framework and have
client libraries available in Python and C++. Its source code is in
`app/like/service`.

#### Deployment
To deploy the *like* service with a multithreaded Thrift server running in a
Docker container:
1. Generate Thrift code and copy dependencies, such as service client libraries.
```
./utils/generate_and_copy_code.sh
```
2. Build the Docker image. Here we name the image `like:latest`.
```
cd app/like/service/server
sudo docker build -t like:latest .
```
3. Run a Docker container based on the newly built image. Here we name the
container `like_service`, publish its port 9093 to the same host port, set its
number of Thrift server threads to 8, and bind-mount the topology configuration
file.
```
sudo docker run \
    --name like_service \
    --publish 9093:9093 \
    --env port=9093 \
    --env threads=8 \
    --volume $(pwd)/conf/topology.yml:/usr/local/etc/buzzblog/topology.yml \
    --detach \
    like:latest
```

### Post Service
The *post* service is implemented in C++ using the Thrift framework and have
client libraries available in Python and C++. Its source code is in
`app/post/service`.

The *post* service persists data in a PostgreSQL database whose schema is in
`app/post/database`.

#### Deployment
To deploy the PostgreSQL server running in a Docker container and set up the
database:
1. Create a Docker volume. Here we name the volume `pg_post`.
```
sudo docker volume create pg_post
```

2. Run a Docker container based on the official PostgreSQL image. Here we name
the container `post_database`, publish its port 5432 to the host port 5434,
create superuser and database named `postgres`, enable the `trust`
authentication mode, and limit the number of concurrent connections to 128.
```
sudo docker run \
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
```
3. Set up the database.
```
psql -U postgres -h localhost -p 5434 -f app/post/database/post_schema.sql
```

To deploy the *post* service with a multithreaded Thrift server running in a
Docker container:
1. Generate Thrift code and copy dependencies, such as service client libraries.
```
./utils/generate_and_copy_code.sh
```
2. Build the Docker image. Here we name the image `post:latest`.
```
cd app/post/service/server
sudo docker build -t post:latest .
```
3. Run Docker containers based on the newly built image. Here we name these
containers `post_service1` and `post_service2`, publish their ports 9094
and 9095, respectively, to the same host ports, set their number of Thrift
server threads to 8, and bind-mount the topology configuration file.
```
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
```

### Uniquepair Service
The *uniquepair* service is implemented in C++ using the Thrift framework and
have client libraries available in Python and C++. Its source code is in
`app/uniquepair/service`.

The *uniquepair* service persists data in a PostgreSQL database whose schema is
in `app/uniquepair/database`.

#### Deployment
To deploy the PostgreSQL server running in a Docker container and set up the
database:
1. Create a Docker volume. Here we name the volume `pg_uniquepair`.
```
sudo docker volume create pg_uniquepair
```

2. Run a Docker container based on the official PostgreSQL image. Here we name
the container `uniquepair_database`, publish its port 5432 to the host port
5435, create superuser and database named `postgres`, enable the `trust`
authentication mode, and limit the number of concurrent connections to 128.
```
sudo docker run \
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
```
3. Set up the database.
```
psql -U postgres -h localhost -p 5435 -f app/uniquepair/database/uniquepair_schema.sql
```

To deploy the *uniquepair* service with a multithreaded Thrift server running in
a Docker container:
1. Generate Thrift code and copy dependencies, such as service client libraries.
```
./utils/generate_and_copy_code.sh
```
2. Build the Docker image. Here we name the image `uniquepair:latest`.
```
cd app/uniquepair/service/server
sudo docker build -t uniquepair:latest .
```
3. Run Docker containers based on the newly built image. Here we name these
containers `uniquepair_service1` and `uniquepair_service2`, publish their ports
9096 and 9097, respectively, to the same host ports, set their number of Thrift
server threads to 8, and bind-mount the topology configuration file.
```
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
```
