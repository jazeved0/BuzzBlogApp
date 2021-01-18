# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

# Define base configuration.
FROM ubuntu:20.04
MAINTAINER ral@gatech.edu
WORKDIR /usr/local/

# Declare environment variables.
ENV port null

# Install software dependencies.
RUN apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    python3-pip
RUN pip3 install --no-cache-dir uWSGI==2.0.19.1 && \
    pip3 install --no-cache-dir Flask-HTTPAuth==4.2.0 && \
    pip3 install --no-cache-dir PyYaml==5.3.1 && \
    pip3 install --no-cache-dir thrift==0.13.0

# Copy service client libraries.
COPY site-packages lib/python3.8/site-packages/buzzblog

# Copy source code.
COPY src src/buzzblog

# Start the server.
CMD ["/bin/bash", "-c", "PYTHONPATH=/usr/local/lib/python3.8/site-packages uwsgi --http-socket 0.0.0.0:$port --wsgi-file src/buzzblog/api_gateway.py --callable app"]
