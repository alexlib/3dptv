\
# Running the 3DPTV Application

This document explains how to run the 3DPTV (3D Particle Tracking Velocimetry) application after it has been successfully built using CMake and Make.

## Prerequisites

1.  **Successful Build:** Ensure the application has been compiled. This typically results in an executable located at `build/bin/3dptv` relative to the project root.
2.  **Tcl/Tk Environment:** The application relies on Tcl/Tk. Ensure that Tcl/Tk (including necessary development libraries if you built from source, or runtime packages) is available in your environment. The Docker container setup handles this automatically.

## Core Concepts

The 3DPTV application consists of two main parts:
1.  **C Executable (`3dptv`):** This is the compiled program that performs the core processing tasks.
2.  **Tcl Script (`ptv.tcl`):** This script, located in the project root, drives the C executable, manages the user interface (if any active), and orchestrates the different processing modules based on parameter files.

**Working Directory is Key:** The application's behavior is highly dependent on the **current working directory** from which it is launched. This directory must contain specific subdirectories and parameter files that the application will use to find input data and store results.

## Expected Working Directory Structure

When you run the application, it expects the current working directory to be structured as follows (the `test/` directory in the project is a good example):

```
<your_working_directory>/
├── parameters/         # Contains all .par configuration files
│   ├── ptv.par         # Main parameters (image paths, calibration paths)
│   ├── sequence.par    # Image sequence definitions
│   ├── criteria.par    # Detection/tracking criteria
│   └── ... (other .par files for specific modules)
│
├── img/                # Contains input image files (e.g., .tif)
│   ├── cam1.10000
│   ├── cam1.10001
│   └── ...
│
├── cal/                # Contains calibration files (e.g., .ori, .addpar)
│   ├── cam1.tif.ori
│   └── ...
│
└── res/                # Output directory for results (created if not present)
    └── ... (results will be written here)
```

*   Paths within the `.par` files (e.g., paths to images in `ptv.par` or `sequence.par`) are typically relative to this working directory (e.g., `img/cam1.10000`).

## How to Run the Application (Natively)

1.  **Open a Terminal.**

2.  **Navigate to your chosen working directory.** This directory should have the structure described above (e.g., `parameters/`, `img/`, `cal/` subdirectories).
    *   For example, to use the sample data provided in the project:
        ```bash
        cd /path/to/your/3dptv_project/test
        ```

3.  **Execute the `3dptv` program, providing the path to the `ptv.tcl` script.**
    *   The C executable (`3dptv`) is usually in the `build/bin/` directory relative to the project root.
    *   The `ptv.tcl` script is in the project root.

    If your current working directory is `/path/to/your/3dptv_project/test`, the command would be:
    ```bash
    /path/to/your/3dptv_project/build/bin/3dptv ../ptv.tcl
    ```
    *   `../ptv.tcl` correctly points one level up from the `test/` directory to the project root where `ptv.tcl` resides.

**Explanation:**
*   The C executable `/path/to/your/3dptv_project/build/bin/3dptv` is launched.
*   It is given one argument: `../ptv.tcl`, which is the path to the main Tcl control script.
*   The application starts in the current working directory (`/path/to/your/3dptv_project/test/` in this example).
*   `ptv.tcl` then reads its configuration from `./parameters/` (i.e., `/path/to/your/3dptv_project/test/parameters/`), loads images from `./img/`, calibration from `./cal/`, and writes results to `./res/`.

## Running with Docker

The Docker setup simplifies this process by using an `entrypoint.sh` script.
*   The Docker image contains the built application at `/opt/3dptv/build/bin/3dptv` and the Tcl script at `/opt/3dptv/ptv.tcl`.
*   The `entrypoint.sh` script handles changing to the correct working directory inside the container and then executes `/opt/3dptv/build/bin/3dptv /opt/3dptv/ptv.tcl`.

You typically run it by mounting your host data directory (which should have the `parameters/`, `img/`, `cal/` structure) into the container.

**Example (Linux):**
```bash
docker run -it --rm \\
    -v "/tmp/.X11-unix:/tmp/.X11-unix" \\
    -v "/path/on/your/host/my_experiment_data:/data" \\
    -e DISPLAY=$DISPLAY \\
    --user $(id -u):$(id -g) \\
    3dptv-app /data 
```
*   Here, `/path/on/your/host/my_experiment_data` is your working directory on the host.
*   It's mounted to `/data` inside the container.
*   The `entrypoint.sh` script is told to use `/data` as the working directory.
*   The `entrypoint.sh` also ensures a `res/` directory exists within `/data`.

If no data directory is specified to `docker run`, it defaults to using the `/opt/3dptv/test` directory packaged within the image.

Refer to `DOCKER_LINUX_GUIDE.md` or `DOCKER_WINDOWS_GUIDE.md` for detailed Docker instructions.

## Summary of Arguments

*   **Direct C Executable (`3dptv`):**
    *   Expects one primary argument: the path to the `ptv.tcl` script.
    *   Example: `./build/bin/3dptv ../ptv.tcl` (when run from a working directory like `test/`)

*   **Operational Parameters:**
    *   These are **not** passed as command-line arguments to `3dptv`.
    *   They are read by `ptv.tcl` from `.par` files located in the `parameters/` subdirectory of the current working directory.

By ensuring your working directory is correctly set up and then calling the executable with the path to `ptv.tcl`, the application will proceed with the configured tasks.
