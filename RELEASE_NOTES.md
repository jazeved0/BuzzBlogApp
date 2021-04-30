# v0.1:
## Software:
* BuzzBlog: A Twitter-like microblogging application that follows the
microservice architectural style. As with Twitter, users can write short posts,
follow other users, and like their favorite posts. The application comprises an
API gateway (implemented in Python using the Flask web framework) and back-end
microservices (implemented in C++ using the Thrift framework), with a load
balancer (Nginx) sitting between them.
  - `apigateway`: Responsible for handling client requests to the public
BuzzBlog API.
  - `account` microservice: Responsible for user authentication and account
management.
    - Stores data in a PostgreSQL database.
    - Microservice dependencies: `like` and `follow`.
  - `follow` microservice: Responsible for managing user following.
    - Microservice dependencies: `account` and `uniquepair`.
  - `like` microservice: Responsible for managing post liking.
    - Microservice dependencies: `account`, `post`, and `uniquepair`.
  - `post` microservice: Responsible for managing posting.
    - Stores data in a PostgreSQL database.
    - Microservice dependencies: `account` and `like`.
  - `uniquepair` microservice: Responsible for managing sets of unique pairs.
    - Stores data in a PostgreSQL database.

## Documentation:
* `API.md`: The BuzzBlog API reference lists all endpoints with parameters and
expected responses.
* `MANUAL.md`: A tutorial on how to deploy BuzzBlog in your local machine with a
simple topology.

## Scripts:
* `generate_and_copy_code.sh`: A script that generate Thrift code and copy
client libraries.
