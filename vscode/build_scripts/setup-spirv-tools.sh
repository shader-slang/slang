#!/bin/bash

# Shared script to setup SPIRV Tools with pinned version
SPIRV_TOOLS_TAG="vulkan-sdk-1.3.290.0"

echo "[$(date)] Setup SPIRV Tools ..."
if [ ! -d spirv-tools ]; then
	git clone https://github.com/KhronosGroup/SPIRV-Tools.git spirv-tools
fi

pushd spirv-tools
	git checkout $SPIRV_TOOLS_TAG
popd 