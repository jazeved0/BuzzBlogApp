# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

# Define base configuration.
FROM ubuntu:20.04
MAINTAINER ral@gatech.edu
WORKDIR /usr/local/

# Declare environment variables.
ENV threads null
ENV port null

# Install software dependencies.
RUN apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    automake \
    bison \
    flex \
    g++ \
    git \
    gnupg2 \
    libboost-all-dev \
    libevent-dev \
    libpq-dev \
    libssl-dev \
    libtool \
    lsb-core \
    make \
    pkg-config \
    wget \
    unzip

# Install Thrift 0.13.
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  libthrift-0.13.0=0.13.0-2build2 \
  libthrift-dev=0.13.0-2build2

# Install libyaml 0.6.2.
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  libyaml-cpp0.6=0.6.2-4ubuntu1 \
  libyaml-cpp-dev=0.6.2-4ubuntu1
    
# Copy cxxopts 2.2.1.
RUN cd lib \
  && wget https://github.com/jarro2783/cxxopts/archive/v2.2.1.zip \
  && unzip v2.2.1.zip \
  && cp cxxopts-2.2.1/include/cxxopts.hpp /usr/local/include \
  && rm v2.2.1.zip

# Copy service client libraries.
COPY include include/buzzblog

# Copy source code.
COPY src src/buzzblog

# Compile source code.
RUN g++ -o bin/follow_server src/buzzblog/follow_server.cpp \
  include/buzzblog/gen/buzzblog_types.cpp \
  include/buzzblog/gen/buzzblog_constants.cpp \
  include/buzzblog/gen/TAccountService.cpp \
  include/buzzblog/gen/TFollowService.cpp \
  include/buzzblog/gen/TLikeService.cpp \
  include/buzzblog/gen/TPostService.cpp \
  include/buzzblog/gen/TUniquepairService.cpp \
  -std=c++14 -lthrift -lyaml-cpp \
  -I/usr/local/include -L/usr/local/lib

# Start the server.
CMD ["/bin/bash", "-c", "follow_server --host 0.0.0.0 --port $port --threads $threads"]
