#!/usr/bin/env bash

# Get premake
wget https://github.com/shader-slang/slang-binaries/blob/master/premake/premake-5.0.0-alpha13/bin/linux-64/premake5?raw=true -O premake5
chmod u+x premake5

# Create the makefile
./premake5 gmake --cc=${CC}

# Build the configuration
make config=${CONFIGURATION}_x64


