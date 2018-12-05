#!/bin/bash

#!/usr/bin/env bash

CONFIGURATION="${1}"
SLANG_TEST_CATEGORY="${2}"
shift

# CONFIGURATION=release
PLATFORM=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCHITECTURE=$(uname -p)

if [ "${ARCHITECTURE}" == "x86_64" ]; then
    ARCHITECTURE="x64"
fi

TARGET=${PLATFORM}-${ARCHITECTURE}

OUTPUTDIR=bin/${TARGET}/${CONFIGURATION}/

SLANG_TEST=${OUTPUTDIR}slang-test

${SLANG_TEST} -bindir ${OUTPUTDIR} -travis -category ${SLANG_TEST_CATEGORY} ${SLANG_TEST_FLAGS}
