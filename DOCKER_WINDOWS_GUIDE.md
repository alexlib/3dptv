## Running 3DPTV using Docker on Windows

This guide explains how to use Docker to create a consistent Linux environment for running the 3DPTV application on a Windows host. The Docker image now includes the pre-built application and an entrypoint script for easier execution.

### Prerequisites

1.  **Docker Desktop for Windows:**
    *   Download and install Docker Desktop from the [official Docker website](https://www.docker.com/products/docker-desktop).
    *   Ensure Docker Desktop is running and configured to use the WSL 2 backend.

2.  **X Server for Windows:**
    *   To display the Tcl/Tk GUI from the Linux container on your Windows desktop, you need an X server.
        *   **VcXsrv:** Download from [SourceForge](https://sourceforge.net/projects/vcxsrv/). When launching, ensure "Disable access control" is checked.
        *   **X410:** Available from the Microsoft Store (paid).
    *   Start your X server before running the Docker container.

3.  **Project Files (for building the image):**
    *   You need the 3DPTV project source code (including the `Dockerfile` and `entrypoint.sh` in the project root) to build the image initially. For example, in `C:\\Users\\YourUser\\Documents\\repos\\3dptv`.

### Steps to Build and Run

1.  **Build the Docker Image (One-time setup):**
    *   Open a terminal (PowerShell or Command Prompt) in the root directory of the 3DPTV project on your Windows machine (e.g., `C:\\Users\\YourUser\\Documents\\repos\\3dptv`).
    *   Run the following command to build the Docker image. This will compile the application and set up the entrypoint.
        ```bash
        docker build -t 3dptv-app .
        ```
        This tags the image as `3dptv-app`.

2.  **Run the 3DPTV Application using Docker:**
    *   Ensure your X Server (e.g., VcXsrv) is running on Windows.
    *   To run the application, you need to mount your **data directory** (where your images, parameters, and results are or will be stored) into the container. The application will then run using this directory as its working context.
    *   **Replace `C:\\path\\to\\your\\data\\directory` with the absolute path to your actual data directory on your Windows host.**

        ```bash
        docker run -it --rm ^
            -v "C:\\path\\to\\your\\data\\directory:/data" ^
            -e DISPLAY=host.docker.internal:0.0 ^
            3dptv-app /data
        ```
        *   `-it`: Runs the container in interactive mode with a terminal.
        *   `--rm`: Automatically removes the container when you exit it.
        *   `-v "C:\\path\\to\\your\\data\\directory:/data"`: Mounts your Windows host data directory to `/data` inside the container. The application will use `/data` as its working directory. **This is the crucial part for your input/output files.**
        *   `-e DISPLAY=host.docker.internal:0.0`: Sets the `DISPLAY` environment variable for GUI forwarding. If `host.docker.internal` doesn\'t work, try your machine\'s local IP address (from `ipconfig`).
        *   `3dptv-app`: The name of the Docker image.
        *   `/data`: This is passed to the `entrypoint.sh` script inside the container, telling it to use `/data` (which is your mounted host directory) as the working directory for the application.

    *   The 3DPTV application GUI should appear on your Windows desktop, operating on the files within your specified data directory.

    *   **Example using the `test` directory from the project:**
        If your project is at `C:\\Users\\YourUser\\Documents\\repos\\3dptv`, and you want to run with the `test` subdirectory as the data source:
        ```bash
        docker run -it --rm ^
            -v "C:\\Users\\YourUser\\Documents\\repos\\3dptv\\test:/data" ^
            -e DISPLAY=host.docker.internal:0.0 ^
            3dptv-app /data
        ```
        *(Note: For PowerShell, you might need to quote the volume mount path differently if it contains spaces, e.g., `"-v \'C:\\Path With Spaces\\data:/data\'"`)*

3.  **If no data directory is specified:**
    *   If you run the container without specifying a directory argument at the end (e.g., `docker run ... 3dptv-app`), the `entrypoint.sh` script will default to using `/opt/3dptv/test` (the `test` directory copied from the project during the image build) as the working directory. This is useful for a quick test or demo.
        ```bash
        docker run -it --rm ^
            -e DISPLAY=host.docker.internal:0.0 ^
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
    *   Ensure your X Server (VcXsrv) is running and "Disable access control" is checked.
    *   Check Windows Firewall.
    *   Try your explicit Windows IP for `DISPLAY`: `-e DISPLAY=<your_windows_ip>:0.0`.
*   **File permission issues in the mounted data directory:**
    *   Ensure Docker Desktop has permissions for the shared drive (Settings -> Resources -> File Sharing).
    *   Unlike Linux, the `--user` flag is not typically used or effective in the same way on Docker Desktop for Windows with WSL2 for volume mounts from the Windows filesystem. Permissions are generally handled by the WSL2 integration. If you encounter issues, ensure the Windows user account running Docker has access to the host directory.
*   **"Error: Specified working directory \'/data\' does not exist"**:
    *   This means the volume mount might have failed or the path provided to the entrypoint script is incorrect. Double-check your `-v` mapping in the `docker run` command. The Windows path must be correct, and the container path (e.g., `/data`) must match the argument you give to `3dptv-app` (e.g., `3dptv-app /data`).
*   **Path issues with `-v` in PowerShell or CMD:**
    *   CMD: `C:\path\to\your\data` is usually fine.
    *   PowerShell: If paths have spaces, quote carefully: `'-v "C:\My Path\data:/data"'`. Using WSL-style paths like `/c/Users/...` might also work if you are running `docker` from within a WSL shell.
*   **If you modify the C/C++ source code or `ptv.tcl`:** You will need to rebuild the Docker image (`docker build -t 3dptv-app .`) for these changes to be included in the container. Changes to your *data files* in the mounted directory are reflected immediately.

