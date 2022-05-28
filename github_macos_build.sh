#!/usr/bin/env bash
# Get premake
wget https://github.com/shader-slang/slang-binaries/blob/master/premake/premake-5.0.0-alpha16/bin/osx/premake5?raw=true -O premake5
chmod u+x premake5

# generate slang-tag-version.h 
git describe --tags | sed -e "s/\(.*\)/\#define SLANG_TAG_VERSION \"\1\"/" > slang-tag-version.h
cat slang-tag-version.h 

# Create the makefile
./premake5 gmake --cc=${CC} --enable-xlib=false --enable-embed-stdlib=true --arch=${ARCH} --deps=true --no-progress=true

# Build the configuration
make config=${CONFIGURATION}_x64 -j`sysctl -n hw.ncpu`


