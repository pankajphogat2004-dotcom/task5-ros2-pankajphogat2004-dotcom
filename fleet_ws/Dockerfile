# Stage 1: Builder
FROM ros:jazzy AS builder

# Set shell for sourcing
SHELL ["/bin/bash", "-c"]

# Create workspace
WORKDIR /workspace
COPY . /workspace/fleet_ws
COPY ../part1 /workspace/part1

# Install dependencies and build
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace/fleet_ws
RUN source /opt/ros/jazzy/setup.bash && \
    colcon build --merge-install

# Stage 2: Runtime
FROM ros:jazzy AS runtime

# Set shell for sourcing
SHELL ["/bin/bash", "-c"]

WORKDIR /app
# Copy only the install directory from the builder stage
COPY --from=builder /workspace/fleet_ws/install /app/install

# Entrypoint setup
RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc && \
    echo "source /app/install/setup.bash" >> /root/.bashrc

# Set the entrypoint to launch the fleet
ENTRYPOINT ["/bin/bash", "-c", "source /opt/ros/jazzy/setup.bash && source /app/install/setup.bash && ros2 launch drone_fleet fleet.launch.py"]
