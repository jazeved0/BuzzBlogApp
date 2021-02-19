#!/bin/bash

# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

# Stop all containers.
docker container stop $(docker container ls -aq)

# Remove all containers.
docker container rm $(docker container ls -aq)

# Remove all images.
docker rmi -f $(docker images -a -q)

# Remove all stopped containers, all networks and volumes not used by at least
# one container, all dangling and unused images, and all build cache.
docker system prune -af --volumes
