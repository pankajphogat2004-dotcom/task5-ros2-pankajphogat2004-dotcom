#!/bin/bash

# Build the docker image
echo "Building Docker image: drone_fleet:latest..."
docker build -t drone_fleet:latest .

# Run the docker container
# -it: Interactive terminal
# --rm: Remove container after exit
# --net=host: Use host network for ROS 2 communication
# -v: Mount current directory to /app/src (optional, but requested for volume reflected changes)
# Note: The Dockerfile copies files, but mounting allows real-time changes if the entrypoint allows it.
# This usually implies building INSIDE the container or having a separate dev environment.
# For a runtime image, changes won't reflect unless we mount over the install dir or rebuild.


docker run -it --rm \
    --net=host \
    -e ROS_DOMAIN_ID=42 \
    -v $(pwd)/src:/app/src/drone_fleet \
    -v $(pwd)/../part1:/workspace/part1 \
    drone_fleet:latest
