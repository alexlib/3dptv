# Dockerfile for 3DPTV Application
# Base image: Ubuntu 20.04 LTS
FROM ubuntu:20.04

# Set DEBIAN_FRONTEND to noninteractive to avoid prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies, build, and clean up in one layer
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        tcl-dev \
        tk-dev \
        libtiff-dev \
        tklib \
        bwidget \
        dos2unix && \
    rm -rf /var/lib/apt/lists/*

# Create the application directory
WORKDIR /opt/3dptv

# Copy the entire project context into the WORKDIR
COPY . .

# Build the application
# This assumes your CMakeLists.txt is in the root and builds in a 'build' subdirectory
RUN dos2unix /usr/local/bin/entrypoint.sh || true && \
    mkdir build && cd build && cmake .. && make && \
    apt-get purge -y build-essential cmake git dos2unix && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/*

# Copy the entrypoint script and ensure correct line endings and permissions
COPY entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

# Set the entrypoint
ENTRYPOINT ["/bin/bash", "-c", "\
  WORK_DIR=\"${1:-/opt/3dptv/test}\" && \
  if [ ! -d \"$WORK_DIR\" ]; then \
    echo \"Error: Specified working directory '$WORK_DIR' does not exist inside the container.\"; \
    echo \"Please ensure the directory is correctly mounted and the path is valid.\"; \
    exit 1; \
  fi && \
  cd \"$WORK_DIR\" && \
  [ -d res ] || mkdir res && \
  exec /opt/3dptv/build/bin/3dptv /opt/3dptv/ptv.tcl \
", "--"]

# Default command can be an empty array or a default path for the entrypoint
# The entrypoint.sh script already defines a default working directory if none is provided.
CMD []

