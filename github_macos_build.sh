#!/usr/bin/env bash
# Get premake
wget https://github.com/shader-slang/slang-binaries/blob/master/premake/premake-5.0.0-alpha16/bin/osx/premake5?raw=true -O premake5
chmod u+x premake5

# generate slang-tag-version.h 
git describe --tags | sed -e "s/\(.*\)/\#define SLANG_TAG_VERSION \"\1\"/" > slang-tag-version.h
cat slang-tag-version.h 

if [[ "" == "${TARGETARCH}" ]]; then
TARGETARCH=${ARCH}
fi

if [[ "${ARCH}" != "${TARGETARCH}" ]]; then

# Create the makefile
./premake5 gmake2 --cc=${CC} --enable-xlib=false --enable-embed-stdlib=true --arch=${ARCH} --deps=true --no-progress=true

# Build the configuration
make config=${CONFIGURATION}_${ARCH} -j`sysctl -n hw.ncpu`

rm -rf ./bin

ARCH="arm64"
# Create the makefile
./premake5 gmake2 --cc=${CC} --enable-xlib=false --enable-embed-stdlib=true --arch=${TARGETARCH} --deps=true --build-glslang=true --no-progress=true  --skip-source-generation=true --deploy-slang-llvm=false --deploy-slang-glslang=false
make config=${CONFIGURATION}_${TARGETARCH} -j`sysctl -n hw.ncpu`
else
# Create the makefile
./premake5 gmake2 --cc=${CC} --enable-xlib=false --enable-embed-stdlib=true --arch=${TARGETARCH} --deps=true --no-progress=true
# Build the configuration
make config=${CONFIGURATION}_${TARGETARCH}  -j`sysctl -n hw.ncpu`
fi
