#!/bin/bash

# Shared script to setup emsdk with pinned version
EMSDK_TAG="4.0.11"

echo "[$(date)] Setup emsdk ..."
if [ ! -d emsdk ]; then
	git clone --branch $EMSDK_TAG --depth 1 https://github.com/emscripten-core/emsdk.git
fi
