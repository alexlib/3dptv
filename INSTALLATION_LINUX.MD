# Installation Instructions for Linux

These instructions will help you set up all required dependencies for building and running the 3dptv project on a Linux system.

## 1. Update your package list

```bash
sudo apt-get update
```

## 2. Install TCL/TK development libraries

```bash
sudo apt-get install tcl-dev tk-dev
```

This will install the necessary header files and libraries for TCL and TK, typically placing them in `/usr/include/tcl8.6` and `/usr/lib/x86_64-linux-gnu/`.

## 3. Install TIFF development library

```bash
sudo apt-get install libtiff-dev
```

This will install the TIFF library and headers required for image processing.

## 4. (Optional) Install CMake and build tools

If you do not already have CMake and build tools:

```bash
sudo apt-get install cmake build-essential
```

## 5. Build the project

From the root of the repository:

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## 6. Troubleshooting

- Ensure that the include paths for TCL/TK are set to `/usr/include/tcl8.6` in your CMake configuration.
- If you have multiple TCL/TK installations (e.g., from Anaconda/Miniforge), make sure to use the system libraries to avoid conflicts.

---

For further help, see the README or contact the maintainers.
