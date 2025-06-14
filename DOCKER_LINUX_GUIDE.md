## Running 3DPTV using Docker on Linux

This guide explains how to use Docker to create a consistent environment for building and running the 3DPTV application on a Linux host. This method helps manage dependencies and ensures a reproducible setup.

### Prerequisites

1.  **Docker Engine for Linux:**
    *   Install Docker Engine on your Linux distribution. Follow the official instructions for your specific distribution at the [Docker website](https://docs.docker.com/engine/install/).
    *   Ensure the Docker daemon is running (`sudo systemctl start docker`).
    *   Add your user to the `docker` group to run Docker commands without `sudo` (optional but recommended): `sudo usermod -aG docker $USER` (you'll need to log out and back in for this to take effect).

2.  **X11 Server (Usually already running):**
    *   Most Linux desktop environments run an X11 server by default. No extra installation is typically needed for GUI forwarding from Docker containers.

3.  **Project Files:**
    *   Ensure you have the 3DPTV project code (including the `Dockerfile` located in the project root) on your Linux machine. For example, in `/home/youruser/Documents/repos/3dptv`.

### `Dockerfile` Content

The `Dockerfile` (located in the project root) sets up an Ubuntu 20.04 environment, copies the project code, and **builds the application** with the necessary tools and libraries. (The content is the same as for the Windows guide, so it's referenced here for brevity. Ensure `Dockerfile` exists in your project root).

```dockerfile
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
```

### Steps to Build and Run

1.  **Build the Docker Image:**
    *   Open a terminal in the root directory of the 3DPTV project on your Linux machine (e.g., `/home/youruser/Documents/repos/3dptv`).
    *   Run the following command to build the Docker image. This will include compiling the application and might take some time.
        ```bash
        docker build -t 3dptv-app .
        ```
        This tags the image as `3dptv-app`.

2.  **Run the Docker Container:**
    *   Run the following command in your terminal. **Crucially, replace `/home/youruser/Documents/repos/3dptv` with the correct absolute path to your project directory on your Linux host.**

        ```bash
        docker run -it --rm \
            -v "/tmp/.X11-unix:/tmp/.X11-unix" \
            -v "/home/youruser/Documents/repos/3dptv:/3dptv_workspace" # <-- VERIFY THIS HOST PATH
            -e DISPLAY=$DISPLAY \
            --user $(id -u):$(id -g) \
            3dptv-app
        ```
        *   `-it`: Runs the container in interactive mode with a terminal.
        *   `--rm`: Automatically removes the container when you exit it.
        *   `-v "/tmp/.X11-unix:/tmp/.X11-unix"`: Mounts the X11 socket from the host into the container. This allows GUI applications in the container to display on your host's X server.
        *   `-v "/home/youruser/Documents/repos/3dptv:/3dptv_workspace"`: Mounts your project directory on the host to `/3dptv_workspace` inside the container. **Adjust the host path accordingly.**
        *   `-e DISPLAY=$DISPLAY`: Passes your host's `DISPLAY` environment variable to the container. This tells GUI applications where to connect.
        *   `--user $(id -u):$(id -g)`: (Recommended) Runs the container with your current host user's ID and group ID. This helps avoid permission issues with files created in the mounted volume, ensuring they are owned by you on the host.
        *   `3dptv-app`: The name of the Docker image to use.

    *   You should now be inside a bash shell within the Linux container. Your `Dockerfile` sets the `WORKDIR` to `/3dptv_workspace`. **Verify you are in this directory by running `pwd`. Then, list its contents with `ls -la` to ensure your project files are visible.**

3.  **Compile the 3DPTV Application (inside the container):**
    *   This step is **no longer needed** if the `Dockerfile` includes the build process. The application is compiled when the image is built.
    *   If you make changes to the C/C++ source code on your host machine (and these changes are part of the files copied by `COPY . .`), you will need to **rebuild the Docker image** using `docker build -t 3dptv-app .` for those changes to take effect.

4.  **Run the 3DPTV Application (inside the container):**
    *   Navigate to the directory where your main Tcl script (e.g., `ptv.tcl`) is located, or where the compiled executable is (usually within the `build` subdirectory in `/3dptv_workspace`).
    *   Launch the application. The exact command depends on how your Tcl application is started. It might be something like:
        ```bash
        # Assuming ptv.tcl is in /3dptv_workspace and your Tcl extension is found
        cd /3dptv_workspace
        wish ptv.tcl
        ```
        Or, if your executable is in `build/bin/3dptv` and it handles the Tcl initialization:
        ```bash
        cd /3dptv_workspace/build/bin # Or appropriate build output directory
        ./3dptv # Or the name of your main executable
        ```
        The Tcl/Tk GUI should appear on your Linux desktop.

### Troubleshooting

*   **Cannot connect to X server / GUI not displaying:**
    *   Ensure the `-v "/tmp/.X11-unix:/tmp/.X11-unix"` and `-e DISPLAY=$DISPLAY` options are correctly included in your `docker run` command.
    *   On some systems, you might need to run `xhost +local:docker` on the host before running the container to allow local connections from Docker. (Use with caution, as it can be less secure: `xhost +` allows any local user to connect to your X server. `xhost -local:docker` to revoke).
*   **Message: "groups: cannot find name for group ID XXXX"**:
    *   When using the `--user $(id -u):$(id -g)` option, you might see this message upon entering the container. It means that your host's numeric group ID (e.g., 1000) doesn't have a corresponding group *name* defined in the container's `/etc/group` file.
    *   This is usually a cosmetic warning and does **not** affect the functionality of the application or file permissions (which are based on numeric IDs). You can typically ignore this message if the application runs correctly and file ownership in mounted volumes is correct.
*   **File permission issues:** Using `--user $(id -u):$(id -g)` in the `docker run` command should prevent most permission issues for files created in the mounted volume. If you still have problems, check the ownership and permissions of the mounted directory on the host.
*   **`docker build` fails:** Check the output for errors. It's often due to typos in package names or network issues preventing package downloads.
*   **CMake errors during build:** Ensure all necessary `-dev` packages are listed in the `Dockerfile`.

