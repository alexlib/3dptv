## Running 3DPTV using Docker on Linux

This guide explains how to use Docker to create a consistent environment for running the 3DPTV application on a Linux host. The Docker image now includes the pre-built application and an entrypoint script for easier execution.

### Prerequisites

1.  **Docker Engine for Linux:**
    *   Install Docker Engine on your Linux distribution. Follow the official instructions for your specific distribution at the [Docker website](https://docs.docker.com/engine/install/).
    *   Ensure the Docker daemon is running (`sudo systemctl start docker`).
    *   Add your user to the `docker` group to run Docker commands without `sudo` (optional but recommended): `sudo usermod -aG docker $USER` (you\'ll need to log out and back in for this to take effect).

2.  **X11 Server (Usually already running):**
    *   Most Linux desktop environments run an X11 server by default.

3.  **Project Files (for building the image):**
    *   You need the 3DPTV project source code (including the `Dockerfile` and `entrypoint.sh` in the project root) to build the image initially. For example, in `/home/youruser/Documents/repos/3dptv`.

### Steps to Build and Run

1.  **Build the Docker Image (One-time setup):**
    *   Open a terminal in the root directory of the 3DPTV project on your Linux machine (e.g., `/home/youruser/Documents/repos/3dptv`).
    *   Run the following command to build the Docker image. This will compile the application and set up the entrypoint.
        ```bash
        docker build -t 3dptv-app .
        ```
        This tags the image as `3dptv-app`.

2.  **Run the 3DPTV Application using Docker:**
    *   To run the application, you need to mount your **data directory** (where your images, parameters, and results are or will be stored) into the container. The application will then run using this directory as its working context.
    *   **Replace `/path/to/your/data/directory` with the absolute path to your actual data directory on your Linux host.**

        ```bash
        docker run -it --rm \\
            -v "/tmp/.X11-unix:/tmp/.X11-unix" \\
            -v "./test:/data" \\
            -e DISPLAY=$DISPLAY \\
            --user $(id -u):$(id -g) \\
            3dptv-app /data
        ```
        *   `-it`: Runs the container in interactive mode with a terminal.
        *   `--rm`: Automatically removes the container when you exit it.
        *   `-v "/tmp/.X11-unix:/tmp/.X11-unix"`: Mounts the X11 socket for GUI forwarding.
        *   `-v "/path/to/your/data/directory:/data"`: Mounts your host data directory to `/data` inside the container. The application will use `/data` as its working directory. **This is the crucial part for your input/output files.**
        *   `-e DISPLAY=$DISPLAY`: Passes your host\'s `DISPLAY` environment variable.
        *   `--user $(id -u):$(id -g)`: (Recommended) Runs the container with your host user\'s ID and group ID to avoid file permission issues in the mounted `/data` volume.
        *   `3dptv-app`: The name of the Docker image.
        *   `/data`: This is passed to the `entrypoint.sh` script inside the container, telling it to use `/data` (which is your mounted host directory) as the working directory for the application.

    *   The 3DPTV application GUI should appear on your Linux desktop, operating on the files within your specified data directory.

    *   **Example using the `test` directory from the project:**
        If your project is at `/home/youruser/Documents/repos/3dptv`, and you want to run with the `test` subdirectory as the data source:
        ```bash
        docker run -it --rm \\
            -v "/tmp/.X11-unix:/tmp/.X11-unix" \\
            -v "/home/youruser/Documents/repos/3dptv/test:/data" \\
            -e DISPLAY=$DISPLAY \\
            --user $(id -u):$(id -g) \\
            3dptv-app /data
        ```

3.  **If no data directory is specified:**
    *   If you run the container without specifying a directory argument at the end (e.g., `docker run ... 3dptv-app`), the `entrypoint.sh` script will default to using `/opt/3dptv/test` (the `test` directory copied from the project during the image build) as the working directory. This is useful for a quick test or demo.
        ```bash
        docker run -it --rm \\
            -v "/tmp/.X11-unix:/tmp/.X11-unix" \\
            -e DISPLAY=$DISPLAY \\
            --user $(id -u):$(id -g) \\
            3dptv-app
        ```

### How it Works

*   The `Dockerfile` now copies the project source to `/opt/3dptv` inside the image and builds the application there.
*   The `entrypoint.sh` script (also copied into the image) is set as the `ENTRYPOINT`.
*   When you run `docker run ... 3dptv-app /your/container/path/to/data`, the `/your/container/path/to/data` is passed as an argument to `entrypoint.sh`.
*   The script changes to this directory and then executes `/opt/3dptv/build/bin/3dptv /opt/3dptv/ptv.tcl`.
*   Your data (mounted from the host to `/your/container/path/to/data`) is directly accessed by the application.

### Troubleshooting

*   **Cannot connect to X server / GUI not displaying:**
    *   Ensure the `-v "/tmp/.X11-unix:/tmp/.X11-unix"` and `-e DISPLAY=$DISPLAY` options are correct.
    *   On some systems, you might need `xhost +local:docker` on the host (use with caution).
*   **Message: "groups: cannot find name for group ID XXXX"**:
    *   This is usually a cosmetic warning when using `--user $(id -u):$(id -g)` and can often be ignored if the application and file permissions work correctly.
*   **File permission issues in the mounted data directory:**
    *   The `--user $(id -u):$(id -g)` option should prevent most issues. Ensure the host directory you are mounting has appropriate permissions for your user.
*   **"Error: Specified working directory \'/data\' does not exist"**:
    *   This means the volume mount might have failed or the path provided to the entrypoint script is incorrect. Double-check your `-v` mapping in the `docker run` command. The second part of the `-v` option (e.g., `/data`) must match the argument you give to `3dptv-app` (e.g., `3dptv-app /data`).
*   **If you modify the C/C++ source code or `ptv.tcl`:** You will need to rebuild the Docker image (`docker build -t 3dptv-app .`) for these changes to be included in the container. Changes to your *data files* in the mounted directory are reflected immediately.

