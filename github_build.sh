#!/usr/bin/env bash
# Get premake
if [[ "aarch64" == "${ARCH}"  ]]; then
wget https://github.com/shader-slang/slang-binaries/blob/master/premake/premake-5.0.0-alpha16/bin/linux-arm64/premake5?raw=true -O premake5
else
wget https://github.com/shader-slang/slang-binaries/blob/master/premake/premake-5.0.0-alpha16/bin/linux-64/premake5?raw=true -O premake5
fi
chmod u+x premake5

# generate slang-tag-version.h 
git describe --tags | sed -e "s/\(.*\)/\#define SLANG_TAG_VERSION \"\1\"/" > slang-tag-version.h
cat slang-tag-version.h 

if [[ "" == "${TARGETARCH}" ]]; then
TARGETARCH=${ARCH}
fi

glslangBuildFlag=""
if [[ "aarch64" == "${TARGETARCH}"  && "release" == "${CONFIGURATION}" ]]; then
    glslangBuildFlag="--build-glslang=true"
fi

if [[ "${ARCH}" != "${TARGETARCH}" ]]; then

# Create the makefile
./premake5 gmake2 --cc=${CC} --enable-embed-stdlib=true --arch=${ARCH} --deps=true --no-progress=true ${glslangBuildFlag}

# Build the configuration
make config=${CONFIGURATION}_${ARCH} -j`nproc`

rm -rf ./bin

# Create the makefile
./premake5 gmake2 --cc=${CC} --enable-embed-stdlib=true --arch=${TARGETARCH} --deps=true --no-progress=true  --skip-source-generation=true --deploy-slang-llvm=false --deploy-slang-glslang=false ${glslangBuildFlag}

else
# Create the makefile
./premake5 gmake2 --cc=${CC} --enable-embed-stdlib=true --arch=${TARGETARCH} --deps=true --no-progress=true ${glslangBuildFlag}
fi

# Build the configuration
make config=${CONFIGURATION}_${TARGETARCH} -j`nproc`

