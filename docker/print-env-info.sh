#!/bin/bash
# Print environment configuration in structured format
# Displays GPU, driver, CUDA, and Vulkan information
# Output can be parsed programmatically for analysis

echo "ENV_INFO_START"
echo "driver_version=$(nvidia-smi --query-gpu=driver_version --format=csv,noheader 2>/dev/null || echo 'unknown')"
echo "gpu_name=$(nvidia-smi --query-gpu=name --format=csv,noheader 2>/dev/null || echo 'unknown')"
echo "gpu_memory=$(nvidia-smi --query-gpu=memory.total --format=csv,noheader 2>/dev/null || echo 'unknown')"
echo "cuda_version=$(nvcc --version 2>&1 | grep -o 'release [0-9.]*' | cut -d' ' -f2 || echo 'unknown')"
echo "vulkan_sdk=${VULKAN_SDK:-not_set}"
echo "vulkan_layer_path=${VK_LAYER_PATH:-not_set}"
echo "vulkan_validation_api=$(grep -o '"api_version": "[^"]*"' "${VK_LAYER_PATH}/VkLayer_khronos_validation.json" 2>/dev/null | head -1 | cut -d'"' -f4 || echo 'unknown')"
echo "vulkan_icd_api=$(grep -o '"api_version" : "[^"]*"' /etc/vulkan/icd.d/nvidia_icd.json 2>/dev/null | cut -d'"' -f4 || echo 'unknown')"
echo "container_image=${CONTAINER_IMAGE:-ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0}"
echo "runner_name=${RUNNER_NAME:-unknown}"
echo "ENV_INFO_END"
