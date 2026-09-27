// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Executes the fixed synthetic tiled-brass eval and sample contracts. Build with the selected CUDA
// headers; loading the driver dynamically avoids introducing a CUDA dependency into Slang itself.
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cuda.h>
#include <dlfcn.h>

struct Driver
{
    void* library = nullptr;
    decltype(&cuInit) init = nullptr;
    decltype(&cuDriverGetVersion) getVersion = nullptr;
    decltype(&cuDeviceGet) getDevice = nullptr;
    decltype(&cuDeviceGetName) getName = nullptr;
    decltype(&cuDeviceGetAttribute) getAttribute = nullptr;
    decltype(&cuDevicePrimaryCtxRetain) retainContext = nullptr;
    decltype(&cuDevicePrimaryCtxRelease) releaseContext = nullptr;
    decltype(&cuCtxSetCurrent) setCurrent = nullptr;
    decltype(&cuCtxSynchronize) synchronize = nullptr;
    decltype(&cuArray3DCreate) createArray = nullptr;
    decltype(&cuArrayDestroy) destroyArray = nullptr;
    decltype(&cuMemcpy2D) copy2D = nullptr;
    decltype(&cuTexObjectCreate) createTexture = nullptr;
    decltype(&cuTexObjectDestroy) destroyTexture = nullptr;
    decltype(&cuGetErrorName) getErrorName = nullptr;
    decltype(&cuModuleLoad) loadModule = nullptr;
    decltype(&cuModuleUnload) unloadModule = nullptr;
    decltype(&cuModuleGetFunction) getFunction = nullptr;
    decltype(&cuModuleGetGlobal) getGlobal = nullptr;
    decltype(&cuMemAlloc) allocate = nullptr;
    decltype(&cuMemFree) freeMemory = nullptr;
    decltype(&cuMemcpyHtoD) copyToDevice = nullptr;
    decltype(&cuMemcpyDtoH) copyToHost = nullptr;
    decltype(&cuLaunchKernel) launch = nullptr;
    decltype(&cuMemsetD8) reset = nullptr;
    decltype(&cuEventCreate) createEvent = nullptr;
    decltype(&cuEventRecord) recordEvent = nullptr;
    decltype(&cuEventSynchronize) synchronizeEvent = nullptr;
    decltype(&cuEventElapsedTime_v2) elapsedEvent = nullptr;
    decltype(&cuEventDestroy) destroyEvent = nullptr;

    template<typename T>
    bool load(T& function, const char* name)
    {
        function = reinterpret_cast<T>(dlsym(library, name));
        if (!function)
            std::fprintf(stderr, "Missing CUDA driver symbol: %s\n", name);
        return function != nullptr;
    }

    bool open(bool extended, bool measure)
    {
        library = dlopen("libcuda.so.1", RTLD_NOW | RTLD_LOCAL);
        if (!library)
        {
            std::fprintf(stderr, "Cannot load CUDA driver: %s\n", dlerror());
            return false;
        }
        return load(init, "cuInit") && load(getVersion, "cuDriverGetVersion") &&
               load(getDevice, "cuDeviceGet") && load(getName, "cuDeviceGetName") &&
               load(getAttribute, "cuDeviceGetAttribute") &&
               load(retainContext, "cuDevicePrimaryCtxRetain") &&
               load(releaseContext, "cuDevicePrimaryCtxRelease_v2") &&
               load(setCurrent, "cuCtxSetCurrent") && load(synchronize, "cuCtxSynchronize") &&
               load(createArray, "cuArray3DCreate_v2") && load(destroyArray, "cuArrayDestroy") &&
               load(copy2D, "cuMemcpy2D_v2") && load(createTexture, "cuTexObjectCreate") &&
               load(destroyTexture, "cuTexObjectDestroy") && load(getErrorName, "cuGetErrorName") &&
               load(loadModule, "cuModuleLoad") && load(unloadModule, "cuModuleUnload") &&
               load(getFunction, "cuModuleGetFunction") &&
               load(getGlobal, "cuModuleGetGlobal_v2") && load(allocate, "cuMemAlloc_v2") &&
               load(freeMemory, "cuMemFree_v2") && load(copyToDevice, "cuMemcpyHtoD_v2") &&
               load(copyToHost, "cuMemcpyDtoH_v2") && load(launch, "cuLaunchKernel") &&
               (!extended || load(reset, "cuMemsetD8_v2")) &&
               (!measure ||
                (load(createEvent, "cuEventCreate") && load(recordEvent, "cuEventRecord") &&
                 load(synchronizeEvent, "cuEventSynchronize") &&
                 load(elapsedEvent, "cuEventElapsedTime_v2") &&
                 load(destroyEvent, "cuEventDestroy_v2")));
    }

    bool check(CUresult result, const char* operation) const
    {
        if (result == CUDA_SUCCESS)
            return true;
        const char* name = "unknown";
        getErrorName(result, &name);
        std::fprintf(stderr, "%s failed: %d (%s)\n", operation, int(result), name);
        return false;
    }

    ~Driver()
    {
        if (library)
            dlclose(library);
    }
};

// Clean up partially created resources too, and make cleanup errors fail the probe.
struct Resources
{
    Driver& driver;
    bool& cleanupOk;
    CUdevice device = 0;
    CUcontext context = nullptr;
    CUarray arrays[2] = {};
    CUtexObject textures[2] = {};
    CUmodule module = nullptr;
    CUdeviceptr buffers[3] = {};
    CUevent events[2] = {};

    ~Resources()
    {
        if (context)
        {
            cleanupOk &= driver.check(driver.setCurrent(context), "cleanup cuCtxSetCurrent");
            cleanupOk &= driver.check(driver.synchronize(), "cleanup cuCtxSynchronize");
            for (CUevent event : events)
                if (event)
                    cleanupOk &= driver.check(driver.destroyEvent(event), "cuEventDestroy_v2");
            for (CUdeviceptr buffer : buffers)
                if (buffer)
                    cleanupOk &= driver.check(driver.freeMemory(buffer), "cuMemFree");
            if (module)
                cleanupOk &= driver.check(driver.unloadModule(module), "cuModuleUnload");
            for (int i = 1; i >= 0; --i)
            {
                if (textures[i])
                    cleanupOk &=
                        driver.check(driver.destroyTexture(textures[i]), "cuTexObjectDestroy");
                if (arrays[i])
                    cleanupOk &= driver.check(driver.destroyArray(arrays[i]), "cuArrayDestroy");
            }
            cleanupOk &= driver.check(driver.releaseContext(device), "cuDevicePrimaryCtxRelease");
        }
    }
};

// Own heap storage for the reviewed large workloads, including partially initialized failures.
struct HostBuffer
{
    unsigned char* data;
    explicit HostBuffer(size_t size)
        : data(static_cast<unsigned char*>(std::malloc(size)))
    {
    }
    ~HostBuffer() { std::free(data); }
    HostBuffer(const HostBuffer&) = delete;
    HostBuffer& operator=(const HostBuffer&) = delete;
};
constexpr unsigned char kSentinel = 0xa5;

struct alignas(8) Float2
{
    float x, y;
};
struct Float3
{
    float x, y, z;
};
struct alignas(8) EvalInput
{
    Float2 uv;
    Float3 wi;
    Float3 wo;
    uint32_t seed;
    uint32_t padding;
};
struct EvalOutput
{
    Float3 value;
    float pdf;
};
struct alignas(8) SampleInput
{
    Float2 uv;
    Float3 wi;
    uint32_t seed;
};
struct SampleOutput
{
    Float3 outgoing;
    float pdf;
    Float3 weight;
    uint32_t flags;
};
struct Material
{
    uint32_t color, roughness;
};
struct Buffer
{
    CUdeviceptr pointer;
    uint64_t count;
};
struct Globals
{
    Buffer inactiveLuts[5];
    Buffer material, evalInput, evalOutput, sampleInput, sampleOutput;
    uint32_t count;
    uint32_t padding;
};
static_assert(sizeof(CUtexObject) == 8 && sizeof(Buffer) == 16);
static_assert(sizeof(Float3) == 12 && sizeof(Material) == 8 && sizeof(EvalOutput) == 16);
static_assert(sizeof(EvalInput) == 40 && alignof(EvalInput) == 8);
static_assert(offsetof(EvalInput, wi) == 8 && offsetof(EvalInput, wo) == 20);
static_assert(offsetof(EvalInput, seed) == 32);
static_assert(sizeof(SampleInput) == 24 && alignof(SampleInput) == 8);
static_assert(offsetof(SampleInput, wi) == 8 && offsetof(SampleInput, seed) == 20);
static_assert(sizeof(SampleOutput) == 32 && offsetof(SampleOutput, pdf) == 12);
static_assert(offsetof(SampleOutput, weight) == 16 && offsetof(SampleOutput, flags) == 28);
static_assert(offsetof(Globals, sampleInput) == 128 && offsetof(Globals, sampleOutput) == 144);
static_assert(sizeof(Globals) == 168 && alignof(Globals) == 8);
static_assert(offsetof(Globals, material) == 80 && offsetof(Globals, evalInput) == 96);
static_assert(offsetof(Globals, evalOutput) == 112 && offsetof(Globals, count) == 160);

// Transfers one fixed-size artifact and rejects trailing bytes on every input.
bool transferFile(const char* directory, const char* name, void* data, size_t size, bool write)
{
    char path[4096];
    int length = std::snprintf(path, sizeof(path), "%s/%s", directory, name);
    if (length < 0 || size_t(length) >= sizeof(path))
        return false;
    FILE* file = std::fopen(path, write ? "wb" : "rb");
    if (!file)
    {
        std::fprintf(stderr, "Cannot open %s\n", path);
        return false;
    }
    bool ok =
        write ? std::fwrite(data, 1, size, file) == size : std::fread(data, 1, size, file) == size;
    if (!write)
        ok &= std::fgetc(file) == EOF && !std::ferror(file);
    ok &= std::fclose(file) == 0;
    if (!ok)
        std::fprintf(stderr, "Invalid size or failed I/O: %s\n", path);
    return ok;
}

int execute(
    Driver& driver,
    Resources& resources,
    const char* cubin,
    const char* directory,
    const char* entry,
    bool sample,
    uint32_t activeCount,
    bool measure,
    bool extended)
{
    // The bounded protocol admits only these reviewed counts. All are 1 mod 64 and
    // leave exactly 63 guard records in the final launched block.
    if (activeCount != 65 && activeCount != 65537 && activeCount != 1048577)
        return 2;
    const uint32_t outputCount = activeCount + 63;
    const uint32_t blocks = outputCount / 64;
    int version = 0;
    char deviceName[256] = {};
    int major = 0;
    int minor = 0;
    if (!driver.check(driver.init(0), "cuInit") ||
        !driver.check(driver.getVersion(&version), "cuDriverGetVersion") ||
        !driver.check(driver.getDevice(&resources.device, 0), "cuDeviceGet") ||
        !driver.check(
            driver.getName(deviceName, sizeof(deviceName), resources.device),
            "cuDeviceGetName") ||
        !driver.check(
            driver.getAttribute(
                &major,
                CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR,
                resources.device),
            "cuDeviceGetAttribute major") ||
        !driver.check(
            driver.getAttribute(
                &minor,
                CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR,
                resources.device),
            "cuDeviceGetAttribute minor"))
        return 2;
    std::printf(
        "cuda_header_version=%d driver_api_version=%d device_ordinal=0 device=%s sm=%d%d\n",
        CUDA_VERSION,
        version,
        deviceName,
        major,
        minor);
    if (!driver.check(
            driver.retainContext(&resources.context, resources.device),
            "cuDevicePrimaryCtxRetain") ||
        !driver.check(driver.setCurrent(resources.context), "cuCtxSetCurrent"))
        return 2;

    if (extended)
    {
        int multiprocessors = 0, l2Bytes = 0;
        if (!driver.check(
                driver.getAttribute(
                    &multiprocessors,
                    CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT,
                    resources.device),
                "SM count") ||
            !driver.check(
                driver.getAttribute(&l2Bytes, CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE, resources.device),
                "L2 bytes"))
            return 2;
        std::printf("device_resources sm_count=%d l2_bytes=%d\n", multiprocessors, l2Bytes);
    }
    float pixels[2][16];
    const size_t inputStride = sample ? sizeof(SampleInput) : sizeof(EvalInput);
    const size_t outputStride = sample ? sizeof(SampleOutput) : sizeof(EvalOutput);
    if (activeCount > SIZE_MAX / inputStride || outputCount > SIZE_MAX / outputStride)
        return 2;
    const size_t inputBytes = activeCount * inputStride;
    const size_t outputBytes = outputCount * outputStride;
    HostBuffer inputStorage(inputBytes), outputStorage(outputBytes),
        referenceStorage(measure ? outputBytes : 1);
    if (!inputStorage.data || !outputStorage.data || !referenceStorage.data)
        return 2;
    unsigned char* inputs = inputStorage.data;
    unsigned char* outputs = outputStorage.data;
    if (measure &&
        !transferFile(directory, "reference.bin", referenceStorage.data, outputBytes, false))
        return 2;
    std::memset(outputs, kSentinel, outputBytes);
    if (!transferFile(directory, "color.bin", pixels[0], sizeof(pixels[0]), false) ||
        !transferFile(directory, "roughness.bin", pixels[1], sizeof(pixels[1]), false) ||
        !transferFile(directory, "inputs.bin", inputs, inputBytes, false))
        return 2;
    bool allFit = true;
    for (int i = 0; i < 2; ++i)
    {
        CUDA_ARRAY3D_DESCRIPTOR arrayDesc = {};
        arrayDesc.Width = 2;
        arrayDesc.Height = 2;
        arrayDesc.Format = CU_AD_FORMAT_FLOAT;
        arrayDesc.NumChannels = 4;
        if (!driver.check(driver.createArray(&resources.arrays[i], &arrayDesc), "cuArray3DCreate"))
            return 2;
        CUDA_MEMCPY2D copy = {};
        copy.srcMemoryType = CU_MEMORYTYPE_HOST;
        copy.srcHost = pixels[i];
        copy.srcPitch = 2 * 4 * sizeof(float);
        copy.dstMemoryType = CU_MEMORYTYPE_ARRAY;
        copy.dstArray = resources.arrays[i];
        copy.WidthInBytes = copy.srcPitch;
        copy.Height = 2;
        if (!driver.check(driver.copy2D(&copy), "cuMemcpy2D"))
            return 2;
        CUDA_RESOURCE_DESC resourceDesc = {};
        resourceDesc.resType = CU_RESOURCE_TYPE_ARRAY;
        resourceDesc.res.array.hArray = resources.arrays[i];
        CUDA_TEXTURE_DESC textureDesc = {};
        for (int axis = 0; axis < 3; ++axis)
            textureDesc.addressMode[axis] = CU_TR_ADDRESS_MODE_WRAP;
        textureDesc.filterMode = CU_TR_FILTER_MODE_LINEAR;
        textureDesc.flags = CU_TRSF_NORMALIZED_COORDINATES;
        if (!driver.check(
                driver.createTexture(&resources.textures[i], &resourceDesc, &textureDesc, nullptr),
                "cuTexObjectCreate"))
            return 2;
        const auto handle = static_cast<unsigned long long>(resources.textures[i]);
        const bool fits = (handle & ~0x3fffffffULL) == 0;
        const bool valid = handle != 0;
        std::printf(
            "texture=%s handle_decimal=%llu handle_hex=0x%016llx low30_fit=%s nonzero=%s\n",
            i == 0 ? "color" : "roughness",
            handle,
            handle,
            fits ? "true" : "false",
            valid ? "true" : "false");
        allFit &= fits && valid;
    }
    std::printf(
        "handle_encoding_gate=%s arrays=RGBA32F_2x2 normalized=true filter=linear address=wrap\n",
        allFit ? "PASS" : "FAIL");
    if (!allFit)
        return 1;
    // The source casts its low30 application handle directly to a CUDA texture object.
    // Narrow only after proving that the full opaque driver handle survives unchanged.
    Material material{uint32_t(resources.textures[0]), uint32_t(resources.textures[1])};
    const size_t sizes[] = {sizeof(material), inputBytes, outputBytes};
    const void* hostData[] = {&material, inputs, outputs};
    for (int i = 0; i < 3; ++i)
    {
        if (!driver.check(driver.allocate(&resources.buffers[i], sizes[i]), "cuMemAlloc") ||
            !driver.check(
                driver.copyToDevice(resources.buffers[i], hostData[i], sizes[i]),
                "cuMemcpyHtoD"))
            return 2;
    }
    Globals globals = {};
    globals.material = {resources.buffers[0], 1};
    Buffer& input = sample ? globals.sampleInput : globals.evalInput;
    Buffer& output = sample ? globals.sampleOutput : globals.evalOutput;
    input = {resources.buffers[1], activeCount};
    output = {resources.buffers[2], outputCount};
    globals.count = activeCount;
    CUdeviceptr symbol = 0;
    size_t symbolSize = 0;
    CUfunction function = nullptr;
    if (!driver.check(driver.loadModule(&resources.module, cubin), "cuModuleLoad") ||
        !driver.check(
            driver.getGlobal(&symbol, &symbolSize, resources.module, "SLANG_globalParams"),
            "cuModuleGetGlobal") ||
        !driver.check(
            driver.getFunction(&function, resources.module, entry),
            "cuModuleGetFunction"))
        return 2;
    if (symbolSize != sizeof(globals))
    {
        std::fprintf(
            stderr,
            "Global ABI mismatch: expected %zu, found %zu\n",
            sizeof(globals),
            symbolSize);
        return 2;
    }
    if (!transferFile(directory, "material.bin", &material, sizeof(material), true) ||
        !transferFile(directory, "globals.bin", &globals, sizeof(globals), true) ||
        !driver.check(driver.copyToDevice(symbol, &globals, sizeof(globals)), "upload globals"))
        return 2;
    if (measure)
        for (int i = 0; i < 2; ++i)
            if (!driver.check(
                    driver.createEvent(&resources.events[i], CU_EVENT_DEFAULT),
                    "cuEventCreate"))
                return 2;
    const int launches = measure ? 12 : 1;
    for (int iteration = 0; iteration < launches; ++iteration)
    {
        // Reset both active output and every guard before every launch, outside event timing.
        if (extended && (!driver.check(
                             driver.reset(resources.buffers[2], kSentinel, outputBytes),
                             "cuMemsetD8_v2") ||
                         !driver.check(driver.synchronize(), "reset completion")))
            return 2;
        using Clock = std::chrono::steady_clock;
        const auto hostStart = Clock::now();
        if (measure &&
            !driver.check(driver.recordEvent(resources.events[0], nullptr), "start event"))
            return 2;
        const auto submitStart = Clock::now();
        if (!driver.check(
                driver.launch(function, blocks, 1, 1, 64, 1, 1, 0, nullptr, nullptr, nullptr),
                "cuLaunchKernel"))
            return 2;
        const auto submitEnd = Clock::now();
        if (measure)
        {
            if (!driver.check(driver.recordEvent(resources.events[1], nullptr), "stop event") ||
                !driver.check(driver.synchronizeEvent(resources.events[1]), "stop completion"))
                return 2;
        }
        else if (!driver.check(driver.synchronize(), "cuCtxSynchronize"))
            return 2;
        const auto hostStop = Clock::now();
        float deviceMilliseconds = 0;
        if (measure &&
            !driver.check(
                driver.elapsedEvent(&deviceMilliseconds, resources.events[0], resources.events[1]),
                "cuEventElapsedTime_v2"))
            return 2;
        if (!driver.check(
                driver.copyToHost(outputs, resources.buffers[2], outputBytes),
                "cuMemcpyDtoH"))
            return 2;
        const bool equal =
            !measure || std::memcmp(outputs, referenceStorage.data, outputBytes) == 0;
        const bool validTime =
            !measure || (std::isfinite(deviceMilliseconds) && deviceMilliseconds > 0);
        if (measure)
        {
            const auto submissionNs =
                std::chrono::duration_cast<std::chrono::nanoseconds>(submitEnd - submitStart)
                    .count();
            const auto untilStopNs =
                std::chrono::duration_cast<std::chrono::nanoseconds>(hostStop - hostStart).count();
            std::printf(
                "measurement index=%d phase=%s device_ms=%.9g host_submit_ns=%lld "
                "host_until_stop_ns=%lld reset_bytes=%zu compared_bytes=%zu equal=%s "
                "valid_time=%s\n",
                iteration,
                iteration < 3 ? "warmup" : "sample",
                double(deviceMilliseconds),
                static_cast<long long>(submissionNs),
                static_cast<long long>(untilStopNs),
                outputBytes,
                outputBytes,
                equal ? "true" : "false",
                validTime ? "true" : "false");
            std::fflush(stdout);
        }
        if (!equal || !validTime)
        {
            char name[64];
            std::snprintf(name, sizeof(name), "failed-output-%02d.bin", iteration);
            if (!transferFile(directory, name, outputs, outputBytes, true))
                return 2;
            return 1;
        }
        if (!measure && !transferFile(directory, "outputs.bin", outputs, outputBytes, true))
            return 2;
    }
    if (extended)
    {
        std::printf(
            "execution launches=%d active=%u output_capacity=%u global_bytes=%zu input_stride=%zu "
            "output_stride=%zu blocks=%u phase=%s\n",
            launches,
            activeCount,
            outputCount,
            symbolSize,
            inputStride,
            outputStride,
            blocks,
            measure ? "measure" : "qualify");
    }
    else
    {
        // Preserve the original 65-record validator's execution evidence and CLI contract.
        std::printf(
            "execution launches=1 active=%u output_capacity=%u global_bytes=%zu input_stride=%zu\n",
            activeCount,
            outputCount,
            symbolSize,
            inputStride);
    }
    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 4 && argc != 6)
    {
        std::fprintf(
            stderr,
            "Usage: material-driver shader.cubin artifact-directory eval_buffer|sample_buffer "
            "[count qualify|measure]\n");
        return 2;
    }
    const bool sample = std::strcmp(argv[3], "sample_buffer") == 0;
    if (!sample && std::strcmp(argv[3], "eval_buffer") != 0)
    {
        std::fprintf(stderr, "Unknown material entry: %s\n", argv[3]);
        return 2;
    }
    const bool extended = argc == 6;
    uint32_t activeCount = 65;
    bool measure = false;
    if (extended)
    {
        if (std::strcmp(argv[4], "65") == 0)
            activeCount = 65;
        else if (std::strcmp(argv[4], "65537") == 0)
            activeCount = 65537;
        else if (std::strcmp(argv[4], "1048577") == 0)
            activeCount = 1048577;
        else
            return 2;
        measure = std::strcmp(argv[5], "measure") == 0;
        if (!measure && std::strcmp(argv[5], "qualify") != 0)
            return 2;
    }
    static_assert(sizeof(CUtexObject) == sizeof(uint64_t));
    Driver driver;
    if (!driver.open(extended, measure))
        return 2;
    bool cleanupOk = true;
    int result = 2;
    {
        Resources resources{driver, cleanupOk};
        result = execute(
            driver,
            resources,
            argv[1],
            argv[2],
            argv[3],
            sample,
            activeCount,
            measure,
            extended);
    }
    std::printf("cleanup=%s\n", cleanupOk ? "PASS" : "FAIL");
    return cleanupOk ? result : 2;
}
