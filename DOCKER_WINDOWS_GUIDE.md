## Running 3DPTV using Docker on Windows

This guide explains how to use Docker to create a consistent Linux environment for building and running the 3DPTV application on a Windows host. This method avoids native compilation issues on Windows and ensures all dependencies are correctly managed.

### Prerequisites

1.  **Docker Desktop for Windows:**
    *   Download and install Docker Desktop from the [official Docker website](https://www.docker.com/products/docker-desktop).
    *   Ensure Docker Desktop is running and configured to use the WSL 2 backend (recommended for performance) or Hyper-V.

2.  **X Server for Windows:**
    *   To display the Tcl/Tk GUI from the Linux container on your Windows desktop, you need an X server. Popular choices:
        *   **VcXsrv:** Download from [SourceForge](https://sourceforge.net/projects/vcxsrv/).
            *   When launching VcXsrv, use default settings but ensure "Disable access control" is checked on the "Extra settings" page.
        *   **X410:** Available from the Microsoft Store (paid).
    *   Start your X server before running the Docker container.

3.  **Project Files:**
    *   Ensure you have the 3DPTV project code (including the `Dockerfile` created in the project root) on your Windows machine. For example, in `C:\Users\YourUser\Documents\repos\3dptv`.

### `Dockerfile` Content

The `Dockerfile` (located in the project root) sets up an Ubuntu 20.04 environment, copies the project code, and **builds the application** with the necessary tools and libraries:

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
    *   Open a terminal (PowerShell, Command Prompt, or Git Bash) in the root directory of the 3DPTV project on your Windows machine (e.g., `C:\Users\YourUser\Documents\repos\3dptv`).
    *   Run the following command to build the Docker image. This will include compiling the application and might take some time.
        ```bash
        docker build -t 3dptv-app .
        ```
        This tags the image as `3dptv-app`.

2.  **Run the Docker Container:**
    *   Ensure your X Server (e.g., VcXsrv) is running on Windows.
    *   Run the following command in your terminal. Replace `C:\Users\YourUser\Documents\repos\3dptv` with the actual path to your project directory on Windows.

        ```bash
        docker run -it --rm \
            -v "C:\Users\YourUser\Documents\repos\3dptv:/3dptv_workspace" \
            -e DISPLAY=host.docker.internal:0.0 \
            3dptv-app
        ```
        *   `-it`: Runs the container in interactive mode with a terminal.
        *   `--rm`: Automatically removes the container when you exit it.
        *   `-v "C:\Users\YourUser\Documents\repos\3dptv:/3dptv_workspace"`: Mounts your project directory on Windows to `/3dptv_workspace` inside the container. **Adjust the Windows path accordingly.**
        *   `-e DISPLAY=host.docker.internal:0.0`: Sets the `DISPLAY` environment variable so GUI applications inside the container can connect to your X server on Windows. If `host.docker.internal` doesn't work, try replacing it with your Windows machine's local IP address (find it using `ipconfig` in a Windows command prompt).
        *   `3dptv-app`: The name of the Docker image to use.

    *   You should now be inside a bash shell within the Linux container, in the `/3dptv_workspace` directory. The application has already been compiled during the image build.

3.  **Compile the 3DPTV Application (inside the container):**
    *   This step is **no longer needed** if the `Dockerfile` includes the build process. The application is compiled when the image is built.
    *   If you make changes to the C/C++ source code on your host machine, you will need to **rebuild the Docker image** using `docker build -t 3dptv-app .` for those changes to take effect.

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
        The Tcl/Tk GUI should appear on your Windows desktop.

### Troubleshooting

*   **Cannot connect to X server:**
    *   Ensure your X Server (VcXsrv, X410) is running on Windows.
    *   Verify VcXsrv has "Disable access control" checked.
    *   Check your Windows Firewall settings; it might be blocking connections to the X server. You might need to allow connections for VcXsrv or your Docker network.
    *   Try using your explicit Windows IP address for the `DISPLAY` variable: `-e DISPLAY=<your_windows_ip>:0.0`.
*   **File permission issues:** If you encounter permission errors when the container tries to write files to the mounted volume, ensure Docker Desktop has the necessary permissions to access the shared drive in Docker Desktop settings (Settings -> Resources -> File Sharing).
*   **`docker build` fails:** Check the output for errors. It's often due to typos in package names or network issues preventing package downloads.
*   **CMake errors during build:** Ensure all necessary `-dev` packages are listed in the `Dockerfile`.

