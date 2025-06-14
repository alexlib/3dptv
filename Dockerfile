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
# - tklib, bwidget: For Tcl/Tk extensions (e.g., tooltip)
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
    # Clean up apt cache to reduce image size
    && rm -rf /var/lib/apt/lists/*

# Create the application directory
WORKDIR /opt/3dptv

# Copy the entire project context into the WORKDIR
COPY . .

# Build the application
# This assumes your CMakeLists.txt is in the root and builds in a 'build' subdirectory
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make

# Copy the entrypoint script and make it executable
COPY entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

# Set the entrypoint
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

# Default command can be an empty array or a default path for the entrypoint
# The entrypoint.sh script already defines a default working directory if none is provided.
CMD []

