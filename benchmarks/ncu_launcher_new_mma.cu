/**
 * Standalone CUDA launcher for ncu profiling of the Slang MMAHelperNew kernel.
 *
 * Loads a PTX file compiled by slangc and launches it with the CUDA Driver API.
 * This allows profiling with: ncu --set full ./ncu_launcher_new_mma
 *
 * Build:
 *   nvcc -o ncu_launcher_new_mma ncu_launcher_new_mma.cu -lcuda -arch=sm_89
 *
 * Usage:
 *   # First compile the Slang shader to PTX:
 *   slangc benchmarks/benchmark_single_layer_forward_new_mma.slang \
 *     -target ptx -entry compute_single_layer_forward_new_mma -stage compute \
 *     -o /tmp/new_mma_profile.ptx \
 *     -I build/Release/lib/slang-standard-module-2026.3.1 \
 *     -DINPUT_SIZE=64 -DOUTPUT_SIZE=16
 *
 *   # Then run under ncu:
 *   ncu --set full ./ncu_launcher_new_mma /tmp/new_mma_profile.ptx
 */

#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_fp16.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

#define CHECK_CUDA(call) do { \
    CUresult err = (call); \
    if (err != CUDA_SUCCESS) { \
        const char* str; cuGetErrorString(err, &str); \
        std::cerr << "CUDA Driver error: " << str << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    } \
} while(0)

#define CHECK_RT(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) { \
        std::cerr << "CUDA Runtime error: " << cudaGetErrorString(err) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    } \
} while(0)

// Slang StructuredBuffer layout: {pointer, count} = 16 bytes
struct StructuredBufferHandle {
    void* data;
    uint64_t count;
};

// These are set at runtime based on --input-size / --output-size flags.
static int INPUT_SIZE = 128;
static int OUTPUT_SIZE = 128;

void linear_to_tiled(const half* weights_linear, half* tiled, int output_size, int input_size) {
    int tile_size = 16;
    int n_tile_rows = (output_size + tile_size - 1) / tile_size;
    int n_tile_cols = (input_size + tile_size - 1) / tile_size;
    int padded_m = n_tile_rows * tile_size;
    int padded_k = n_tile_cols * tile_size;

    std::vector<half> w_padded(padded_m * padded_k, __float2half(0.0f));
    for (int r = 0; r < output_size; r++)
        for (int c = 0; c < input_size; c++)
            w_padded[r * padded_k + c] = weights_linear[r * input_size + c];

    for (int tr = 0; tr < n_tile_rows; tr++) {
        for (int tc = 0; tc < n_tile_cols; tc++) {
            int tile_index = tr * n_tile_cols + tc;
            for (int r = 0; r < tile_size; r++)
                for (int c = 0; c < tile_size; c++)
                    tiled[tile_index * 256 + r * tile_size + c] =
                        w_padded[(tr * tile_size + r) * padded_k + tc * tile_size + c];
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <ptx_file> [--batch-size N]" << std::endl;
        return 1;
    }

    const char* ptx_path = argv[1];
    int batch_size = 256;
    int num_warps = 1;
    std::string mode = "forward";

    for (int i = 2; i < argc; i++) {
        if (std::strcmp(argv[i], "--batch-size") == 0 && i + 1 < argc)
            batch_size = std::atoi(argv[++i]);
        if (std::strcmp(argv[i], "--input-size") == 0 && i + 1 < argc)
            INPUT_SIZE = std::atoi(argv[++i]);
        if (std::strcmp(argv[i], "--output-size") == 0 && i + 1 < argc)
            OUTPUT_SIZE = std::atoi(argv[++i]);
        if (std::strcmp(argv[i], "--warps") == 0 && i + 1 < argc)
            num_warps = std::atoi(argv[++i]);
        if (std::strcmp(argv[i], "--mode") == 0 && i + 1 < argc)
            mode = argv[++i];
    }

    bool is_transpose = (mode == "transpose");
    bool is_bias_reduce = (mode == "bias_reduce");
    bool is_outer_product = (mode == "outer_product");
    bool is_backward = (mode == "backward");

    int NTileRows = (OUTPUT_SIZE + 15) / 16;
    int NTileCols = (INPUT_SIZE + 15) / 16;
    int TILED_WEIGHT_COUNT = NTileRows * NTileCols * 256;
    int BIAS_OFFSET = TILED_WEIGHT_COUNT;
    int PARAM_COUNT = (is_transpose || is_bias_reduce || is_outer_product || is_backward) ? TILED_WEIGHT_COUNT : TILED_WEIGHT_COUNT + OUTPUT_SIZE;

    // Load PTX
    std::ifstream ptx_file(ptx_path, std::ios::binary);
    if (!ptx_file.is_open()) {
        std::cerr << "Cannot open PTX file: " << ptx_path << std::endl;
        return 1;
    }
    std::stringstream ss;
    ss << ptx_file.rdbuf();
    std::string ptx_source = ss.str();

    // Init CUDA Driver
    CHECK_CUDA(cuInit(0));
    CUdevice device;
    CHECK_CUDA(cuDeviceGet(&device, 0));
    // Use runtime API to create context (avoids cuCtxCreate version issues)
    CHECK_RT(cudaSetDevice(0));
    CHECK_RT(cudaFree(0));  // force context creation
    CUcontext ctx;
    CHECK_CUDA(cuCtxGetCurrent(&ctx));

    // Load module from PTX
    CUmodule module;
    CHECK_CUDA(cuModuleLoadData(&module, ptx_source.c_str()));

    const char* kernel_name = is_backward
        ? "compute_backward"
        : (is_bias_reduce ? "compute_bias_reduce"
        : (is_outer_product ? "compute_outer_product"
        : (is_transpose ? "compute_single_layer_transpose"
                        : "compute_single_layer_forward_new_mma")));

    CUfunction kernel;
    CHECK_CUDA(cuModuleGetFunction(&kernel, module, kernel_name));

    int threads_per_block = 32 * num_warps;
    int num_blocks = batch_size / num_warps;

    std::cout << "Mode: " << mode << std::endl;
    std::cout << "Loaded kernel: " << kernel_name << std::endl;
    std::cout << "Batch size: " << batch_size << std::endl;
    std::cout << "Warps per block: " << num_warps << std::endl;
    std::cout << "Grid: " << num_blocks << " blocks x " << threads_per_block << " threads" << std::endl;
    std::cout << "Network: " << INPUT_SIZE << " -> " << OUTPUT_SIZE << std::endl;

    // Allocate buffers
    srand(42);
    std::vector<half> h_weights_linear(INPUT_SIZE * OUTPUT_SIZE);
    std::vector<half> h_bias(OUTPUT_SIZE);
    for (auto& v : h_weights_linear) v = __float2half((rand() / (float)RAND_MAX - 0.5f) * 0.1f);
    for (auto& v : h_bias) v = __float2half((rand() / (float)RAND_MAX - 0.5f) * 0.01f);

    std::vector<half> h_params(PARAM_COUNT, __float2half(0.0f));
    if (!is_outer_product)
        linear_to_tiled(h_weights_linear.data(), h_params.data(), OUTPUT_SIZE, INPUT_SIZE);
    if (!is_transpose && !is_bias_reduce && !is_outer_product && !is_backward) {
        for (int i = 0; i < OUTPUT_SIZE; i++)
            h_params[BIAS_OFFSET + i] = h_bias[i];
    }

    half *d_params, *d_outputs, *d_bias;
    CHECK_RT(cudaMalloc(&d_params, PARAM_COUNT * sizeof(half)));
    CHECK_RT(cudaMalloc(&d_outputs, batch_size * 32 * sizeof(half)));
    CHECK_RT(cudaMalloc(&d_bias, OUTPUT_SIZE * sizeof(half)));

    CHECK_RT(cudaMemcpy(d_params, h_params.data(), PARAM_COUNT * sizeof(half), cudaMemcpyHostToDevice));
    CHECK_RT(cudaMemset(d_outputs, 0, batch_size * 32 * sizeof(half)));
    CHECK_RT(cudaMemset(d_bias, 0, OUTPUT_SIZE * sizeof(half)));

    StructuredBufferHandle params_handle = {d_params, (uint64_t)PARAM_COUNT};
    StructuredBufferHandle outputs_handle = {d_outputs, (uint64_t)(batch_size * 32)};
    StructuredBufferHandle bias_handle = {d_bias, (uint64_t)OUTPUT_SIZE};
    int batch_size_arg = batch_size;

    // bias_reduce / outer_product kernel: (handle, batch_size)
    // forward/transpose kernel: (params, outputs, batch_size)
    // backward kernel: (params, outputs, weightGrad, biasGrad, batch_size)
    StructuredBufferHandle wgrad_handle = {d_params, (uint64_t)TILED_WEIGHT_COUNT};
    void* args_bias[] = {&bias_handle, &batch_size_arg};
    void* args_outer[] = {&wgrad_handle, &batch_size_arg};
    void* args_default[] = {&params_handle, &outputs_handle, &batch_size_arg};
    void* args_backward[] = {&params_handle, &outputs_handle, &wgrad_handle, &bias_handle, &batch_size_arg};
    void** args = is_backward ? args_backward
                : is_bias_reduce ? args_bias
                : is_outer_product ? args_outer
                : args_default;

    std::cout << "Buffers allocated. PARAM_COUNT=" << PARAM_COUNT
              << " TILED_WEIGHT_COUNT=" << TILED_WEIGHT_COUNT << std::endl;
    std::cout << "args=" << (is_backward ? "backward" : (is_bias_reduce ? "bias" : (is_outer_product ? "outer" : "default"))) << std::endl;

    // Warmup
    std::cout << "Warming up (100 iterations)..." << std::endl;
    CHECK_CUDA(cuLaunchKernel(kernel, num_blocks, 1, 1, threads_per_block, 1, 1, 0, 0, args, nullptr));
    CHECK_CUDA(cuCtxSynchronize());
    cudaError_t launch_err = cudaGetLastError();
    if (launch_err != cudaSuccess) {
        std::cerr << "Kernel launch error: " << cudaGetErrorString(launch_err) << std::endl;
        return 1;
    }
    std::cout << "First launch OK" << std::endl;
    for (int i = 1; i < 100; i++)
        CHECK_CUDA(cuLaunchKernel(kernel, num_blocks, 1, 1, threads_per_block, 1, 1, 0, 0, args, nullptr));
    CHECK_CUDA(cuCtxSynchronize());

    // Benchmark with CUDA events
    int iterations = 1000;
    std::cout << "Benchmarking (" << iterations << " iterations)..." << std::endl;
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    cudaEventRecord(start);
    for (int i = 0; i < iterations; i++)
        CHECK_CUDA(cuLaunchKernel(kernel, num_blocks, 1, 1, threads_per_block, 1, 1, 0, 0, args, nullptr));
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float total_ms;
    cudaEventElapsedTime(&total_ms, start, stop);
    double avg_ms = total_ms / iterations;
    double throughput = batch_size / (avg_ms / 1000.0);

    std::cout << "\nResults:" << std::endl;
    std::cout << "  Network: " << INPUT_SIZE << " -> " << OUTPUT_SIZE << std::endl;
    std::cout << "  Batch: " << batch_size << std::endl;
    std::cout << "  Avg time: " << std::fixed << std::setprecision(4) << avg_ms << " ms" << std::endl;
    std::cout << "  Throughput: " << std::setprecision(0) << throughput << " samples/s" << std::endl;

    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    cudaFree(d_params);
    cudaFree(d_outputs);
    cudaFree(d_bias);
    cuModuleUnload(module);
    return 0;
}
