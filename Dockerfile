# Dockerfile for 3DPTV Application
# Base image: Ubuntu 20.04 LTS
FROM ubuntu:20.04

# Set DEBIAN_FRONTEND to noninteractive to avoid prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package lists and install dependencies
# - build-essential: For C/C++ compilers (gcc, g++) and make
# - cmake: For building the project
# - git: For version control (optional, but good to have)
# - tcl-dev, tk-dev: For Tcl/Tk development (headers and libraries)
# - libtiff-dev: For TIFF image support (headers and libraries)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    tcl-dev \
    tk-dev \
    libtiff-dev \
    # Clean up apt cache to reduce image size
    && rm -rf /var/lib/apt/lists/*

# Set a working directory inside the container
WORKDIR /3dptv_workspace

# Copy the entire project context into the WORKDIR
COPY . .

# Build the application
# This assumes your CMakeLists.txt is in the root and builds in a 'build' subdirectory
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make

# Default command to run when the container starts
# This will drop you into a bash shell in the WORKDIR
# The application is now pre-built
CMD ["bash"]

