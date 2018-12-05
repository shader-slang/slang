#!/usr/bin/env bash

# Get premake
wget https://github.com/shader-slang/slang-binaries/blob/master/premake/premake-5.0.0-alpha13/bin/linux-64/premake5?raw=true -O premake5
chmod u+x premake5
./premake5 gmake -build-location=build.linux
cd build.linux        
make config=release_x64
make config=debug_x64
cd ..

# Setup to run release tests
bash ./travis_test.sh release smoke

