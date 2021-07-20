#!/usr/bin/env bash

# CONFIGURATION=release or debug
if [ "${CC}" == "gcc" ] && [ "${CONFIGURATION}" == "release" ]
then
    SLANG_TEST_CATEGORY=full
else
    SLANG_TEST_CATEGORY=smoke
fi

PLATFORM=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCHITECTURE=$(uname -p)

if [ "${ARCHITECTURE}" == "x86_64" ]; then
    ARCHITECTURE="x64"
fi

TARGET=${PLATFORM}-${ARCHITECTURE}

OUTPUTDIR=bin/${TARGET}/${CONFIGURATION}/

if [ "${ARCHITECTURE}" == "x64" ]; then
    LOCATION=$(curl -s https://api.github.com/repos/shader-slang/swiftshader/releases/latest \
    | grep "tag_name" \
    | awk '{print "https://github.com/shader-slang/swiftshader/releases/download/" substr($2, 2, length($2)-3) "/vk_swiftshader_linux_x64.zip"}')
    curl -L -o libswiftshader.zip $LOCATION
    unzip libswiftshader.zip -d $OUTPUTDIR
fi

SLANG_TEST=${OUTPUTDIR}slang-test

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OUTPUTDIR
${SLANG_TEST} -bindir ${OUTPUTDIR} -travis -category ${SLANG_TEST_CATEGORY} ${SLANG_TEST_FLAGS}
