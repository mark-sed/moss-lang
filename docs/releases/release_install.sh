#!/bin/bash
# This script can be used to install moss release to correct system paths
# for easier use.
# This script copies the binary into /usr/bin/ to be executable as a command,
# it also copies all the libraries into /usr/lib/moss, which is the Linux
# expected location.
# On MacOS the location is /usr/local/bin and /usr/local/lib/moss
# NOTE: This script requires root privileges.
#
# Usage: sudo bash install.sh

LIB_PATH=""
BIN_PATH=""
if [[ "$(uname -s)" == "Darwin" ]]; then
    LIB_PATH="/usr/local/lib/moss"
    BIN_PATH="/usr/local/bin/moss"
elif [[ "$(uname -s)" == "Linux" ]]; then
    if ! [ -w /bin ]; then
        echo "ERROR: This script requires root privileges to write into /bin/ and /usr/lib"
        echo "Usage: sudo bash install.sh"
        exit 1
    fi
    LIB_PATH="/usr/lib/moss"
    BIN_PATH="/usr/bin/moss"
else
    echo "ERROR: Detected unknown OS"
    exit 128
fi

# Copy moss binary into bin/
cp moss $BIN_PATH || { echo "ERROR: Failed copying moss into $BIN_PATH"; exit 1; }
# Create moss folder for libraries
mkdir -p $LIB_PATH
# Copy all the libraries into lib/moss/
cp *.msb $LIB_PATH || { echo "ERROR: Failed copying libraries into $LIB_PATH"; exit 1; }
cp *.css $LIB_PATH || { echo "ERROR: Failed copying styles into $LIB_PATH"; exit 1; }