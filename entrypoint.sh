#!/bin/bash
set -e

# Default working directory if none is provided
DEFAULT_WORK_DIR="/opt/3dptv/test"
# Path to the 3dptv executable
PTV_EXECUTABLE="/opt/3dptv/build/bin/3dptv"
# Path to the main ptv.tcl script
PTV_TCL_SCRIPT="/opt/3dptv/ptv.tcl"

# Use the first argument as the working directory, or the default
WORK_DIR="${1:-$DEFAULT_WORK_DIR}"

if [ "$#" -eq 0 ]; then
  echo "No working directory specified. Using default: $DEFAULT_WORK_DIR"
  echo "Usage: docker run <your_image_name> [path_to_your_data_directory_in_container]"
  echo "Example: docker run -v /path/on/host/my_test_data:/data <your_image_name> /data"
fi

if [ ! -d "$WORK_DIR" ]; then
  echo "Error: Specified working directory '$WORK_DIR' does not exist inside the container."
  echo "Please ensure the directory is correctly mounted and the path is valid."
  exit 1
fi

echo "Changing to working directory: $WORK_DIR"
cd "$WORK_DIR"

# Create a 'res' directory if it doesn't exist in the working directory
if [ ! -d "res" ]; then
  echo "Directory 'res' not found in $WORK_DIR. Creating it."
  mkdir res
fi

echo "Starting 3DPTV application..."
echo "Executing: $PTV_EXECUTABLE $PTV_TCL_SCRIPT"

# Execute the application
# The application will run in the context of $WORK_DIR
# and use the ptv.tcl script from its fixed location.
exec "$PTV_EXECUTABLE" "$PTV_TCL_SCRIPT"
