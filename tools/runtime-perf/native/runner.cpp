// Standalone benchmark harness, following compile-perf's native driver conventions.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <slang-rhi.h>
#include <slang-rhi/shader-cursor.h>
#include <string>
#include <vector>

using namespace rhi;
using Clock = std::chrono::steady_clock;

static double elapsed(Clock::time_point start)
{
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

static bool check(Result result, const char* operation)
{
    if (SLANG_SUCCEEDED(result))
        return true;
    fprintf(stderr, "%s failed: 0x%08x\n", operation, unsigned(result));
    return false;
}
#define CHECK(call)            \
    if (!check((call), #call)) \
    return false

static std::string quote(const char* value)
{
    std::string result = "\"";
    for (const unsigned char* p = (const unsigned char*)(value ? value : ""); *p; ++p)
    {
        if (*p == '"' || *p == '\\')
            result += '\\';
        if (*p < 32)
        {
            char escaped[7];
            snprintf(escaped, sizeof(escaped), "\\u%04x", *p);
            result += escaped;
        }
        else
            result += char(*p);
    }
    return result + '"';
}

static double median(std::vector<double> values)
{
    std::sort(values.begin(), values.end());
    size_t n = values.size();
    return n % 2 ? values[n / 2] : (values[n / 2 - 1] + values[n / 2]) * 0.5;
}

static double spread(const std::vector<double>& values)
{
    double center = median(values);
    std::vector<double> deviations;
    for (double value : values)
        deviations.push_back(std::abs(value - center));
    return center > 0 ? median(deviations) / center : 1.0;
}

struct Options
{
    std::string backend = "vulkan", caseName, directory;
    uint32_t adapter = 0, complexity = 4, elements = 0, maxElements = 16777216;
    uint32_t samples = 10, maxSamples = 30, warmup = 5;
    uint32_t batch = 0, maxBatch = 1024;
    double targetMs = 25.0, maxSpread = 0.05;
    bool strictPhases = false;
};

static DeviceType deviceType(const std::string& name)
{
    if (name == "vulkan")
        return DeviceType::Vulkan;
    if (name == "metal")
        return DeviceType::Metal;
    if (name == "cuda")
        return DeviceType::CUDA;
    if (name == "d3d12")
        return DeviceType::D3D12;
    return DeviceType::Default;
}

static float inputValue(uint32_t i)
{
    return 0.1f + float(i % 251) * 0.002f;
}

static float matrixValue(uint32_t matrix, uint32_t row, uint32_t column)
{
    return row == column ? 0.90f + float(matrix) * 0.001f : float(row + 2 * column + 1) * 0.001f;
}

struct Stage
{
    std::string name;
    ComPtr<IComputePipeline> pipeline;
    double loadMs = 0, linkMs = 0, codeMs = 0, pipelineMs = 0;
};

struct Sample
{
    double total = 0, record = 0, submit = 0, wait = 0, endToEnd = 0;
    std::vector<double> stages;
};

static bool stableSamples(const std::vector<Sample>& samples, double limit, bool strictPhases)
{
    std::vector<double> values;
    for (const auto& sample : samples)
        values.push_back(sample.total);
    if (spread(values) > limit)
        return false;
    if (!strictPhases)
        return true;
    for (size_t stage = 0; stage < samples.front().stages.size(); ++stage)
    {
        values.clear();
        for (const auto& sample : samples)
            values.push_back(sample.stages[stage]);
        if (spread(values) > limit)
            return false;
    }
    return true;
}

struct Benchmark
{
    Options options;
    ComPtr<IDevice> device;
    ComPtr<ICommandQueue> queue;
    ComPtr<IQueryPool> queries;
    ComPtr<slang::ISession> session;
    std::vector<Stage> stages;
    std::vector<ComPtr<IBuffer>> buffers, resources;
    ComPtr<IBuffer> matrices;
    uint32_t count = 0;
    uint32_t width = 256;
    uint32_t batch = 1;
    double deviceMs = 0, resourceMs = 0, readbackMs = 0;
    bool unsupported = false;

    bool loadStage(const char* name)
    {
        Stage stage;
        stage.name = name;
        ComPtr<slang::IBlob> diagnostics;
        auto start = Clock::now();
        auto module = session->loadModule(name, diagnostics.writeRef());
        stage.loadMs = elapsed(start);
        if (diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        if (!module)
            return false;
        ComPtr<slang::IEntryPoint> entry;
        CHECK(module->findEntryPointByName("computeMain", entry.writeRef()));
        slang::IComponentType* components[] = {module, entry.get()};
        ComPtr<slang::IComponentType> composite, linked;
        start = Clock::now();
        CHECK(session->createCompositeComponentType(
            components,
            2,
            composite.writeRef(),
            diagnostics.writeRef()));
        Result result = composite->link(linked.writeRef(), diagnostics.writeRef());
        if (diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        CHECK(result);
        stage.linkMs = elapsed(start);
        // Materialize target code before constructing the RHI program. This
        // separates Slang code generation from backend pipeline preparation.
        ComPtr<slang::IBlob> code;
        start = Clock::now();
        result = linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
        if (diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        CHECK(result);
        stage.codeMs = elapsed(start);
        std::string artifact = options.directory + "/" + name + ".target-code";
        FILE* file = fopen(artifact.c_str(), "wb");
        if (!file)
            return false;
        size_t written = fwrite(code->getBufferPointer(), 1, code->getBufferSize(), file);
        bool saved = fclose(file) == 0 && written == code->getBufferSize();
        if (!saved)
            return false;
        ShaderProgramDesc programDesc = {};
        programDesc.slangGlobalScope = linked.get();
        ComPtr<IShaderProgram> program;
        start = Clock::now();
        result =
            device->createShaderProgram(programDesc, program.writeRef(), diagnostics.writeRef());
        if (diagnostics)
            fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
        CHECK(result);
        ComputePipelineDesc pipelineDesc = {};
        pipelineDesc.program = program;
        pipelineDesc.compilationPolicy = PipelineCompilationPolicy::Immediate;
        CHECK(device->createComputePipeline(pipelineDesc, stage.pipeline.writeRef()));
        stage.pipelineMs = elapsed(start);
        stages.push_back(stage);
        return true;
    }

    bool initialize()
    {
        DeviceDesc desc = {};
        desc.deviceType = deviceType(options.backend);
        auto adapter = getRHI()->getAdapter(desc.deviceType, options.adapter);
        if (!adapter)
            return false;
        desc.adapter = adapter;
        const char* path = options.directory.c_str();
        desc.slang.searchPaths = &path;
        desc.slang.searchPathCount = 1;
        desc.slang.optimizationLevel = SLANG_OPTIMIZATION_LEVEL_MAXIMAL;
        desc.slang.targetProfile = "sm_6_6";
        auto start = Clock::now();
        CHECK(getRHI()->createDevice(desc, device.writeRef()));
        deviceMs = elapsed(start);
        if (!device->hasFeature(Feature::TimestampQuery) || !device->getInfo().timestampFrequency)
        {
            fprintf(stderr, "Device does not support GPU timestamp queries\n");
            unsupported = true;
            return false;
        }
        queue = device->getQueue(QueueType::Graphics);
        CHECK(device->getSlangSession(session.writeRef()));
        if (options.caseName == "image_pipeline")
        {
            if (!loadStage("horizontal") || !loadStage("vertical") || !loadStage("tone"))
                return false;
        }
        else if (!loadStage("main"))
            return false;
        return true;
    }

    bool configureQueries(uint32_t iterations)
    {
        batch = iterations;
        queries.setNull();
        QueryPoolDesc queryDesc = {};
        queryDesc.type = QueryType::Timestamp;
        queryDesc.count = uint32_t(stages.size() * 2 * batch + 2);
        CHECK(device->createQueryPool(queryDesc, queries.writeRef()));
        return true;
    }

    bool buffer(const std::vector<float>& data, uint32_t stride, ComPtr<IBuffer>& out)
    {
        BufferDesc desc = {};
        desc.size = data.size() * sizeof(float);
        desc.elementSize = stride;
        desc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                     BufferUsage::CopySource | BufferUsage::CopyDestination;
        desc.memoryType = MemoryType::DeviceLocal;
        CHECK(device->createBuffer(desc, data.data(), out.writeRef()));
        return true;
    }

    bool allocate(uint32_t elements)
    {
        count = elements;
        buffers.clear();
        resources.clear();
        matrices.setNull();
        auto start = Clock::now();
        std::vector<float> data(count);
        for (uint32_t i = 0; i < count; ++i)
            data[i] = inputValue(i);
        buffers.resize(stages.size() + 1);
        if (!buffer(data, 4, buffers[0]))
            return false;
        std::fill(data.begin(), data.end(), -12345.0f);
        for (size_t i = 1; i < buffers.size(); ++i)
            if (!buffer(data, 4, buffers[i]))
                return false;
        if (options.caseName.find("resources_") == 0)
        {
            resources.resize(options.complexity);
            data.resize(256);
            for (uint32_t j = 0; j < options.complexity; ++j)
            {
                for (uint32_t i = 0; i < 256; ++i)
                    data[i] = float(j + 1) * 0.01f + float(i) * 0.001f;
                if (!buffer(data, 4, resources[j]))
                    return false;
            }
        }
        if (options.caseName.find("matrix_") == 0)
        {
            data.resize(16 * 16);
            for (uint32_t m = 0; m < 16; ++m)
                for (uint32_t r = 0; r < 4; ++r)
                    for (uint32_t c = 0; c < 4; ++c)
                    {
                        uint32_t offset =
                            options.caseName == "matrix_column" ? c * 4 + r : r * 4 + c;
                        data[m * 16 + offset] = matrixValue(m, r, c);
                    }
            if (!buffer(data, 64, matrices))
                return false;
        }
        CHECK(queue->waitOnHost());
        resourceMs = elapsed(start);
        return true;
    }

    bool run(Sample& sample)
    {
        CHECK(queries->reset());
        auto start = Clock::now();
        auto encoder = queue->createCommandEncoder();
        if (!encoder)
            return false;
        encoder->writeTimestamp(queries, 0);
        for (uint32_t iteration = 0; iteration < batch; ++iteration)
        {
            // Preserve ordering when reusing output storage between iterations.
            if (iteration)
                encoder->globalBarrier();
            for (uint32_t s = 0; s < stages.size(); ++s)
            {
                // Explicit states also order the producer/consumer boundary of image passes.
                encoder->setBufferState(buffers[s], ResourceState::ShaderResource);
                encoder->setBufferState(buffers[s + 1], ResourceState::UnorderedAccess);
                uint32_t queryBase = (iteration * uint32_t(stages.size()) + s) * 2;
                encoder->writeTimestamp(queries, queryBase + 1);
                // RHI timestamps use backend-defined pipeline stages. Serialize the
                // measured region so later compute cannot run ahead of its start
                // timestamp (notably Vulkan's bottom-of-pipe timestamp writes).
                encoder->globalBarrier();
                auto pass = encoder->beginComputePass();
                auto root = pass->bindPipeline(stages[s].pipeline);
                if (!root)
                    return false;
                ShaderCursor cursor(root);
                if (cursor["inputData"].isValid())
                    CHECK(cursor["inputData"].setBinding(buffers[s]));
                CHECK(cursor["outputData"].setBinding(buffers[s + 1]));
                CHECK(cursor["count"].setData(count));
                uint32_t dispatchWidth = 256 * 64;
                CHECK(cursor["dispatchWidth"].setData(dispatchWidth));
                if (cursor["width"].isValid())
                    CHECK(cursor["width"].setData(width));
                if (matrices)
                    CHECK(cursor["matrices"].setBinding(matrices));
                for (uint32_t j = 0; j < resources.size(); ++j)
                    CHECK(cursor["resources"][j].setBinding(resources[j]));
                // Two-dimensional dispatches stay within the portable workgroup
                // count limits even for the largest calibrated element counts.
                pass->dispatchCompute(256, (count + dispatchWidth - 1) / dispatchWidth, 1);
                pass->end();
                encoder->globalBarrier();
                encoder->writeTimestamp(queries, queryBase + 2);
            }
        }
        encoder->writeTimestamp(queries, uint32_t(stages.size() * 2 * batch + 1));
        auto commands = encoder->finish();
        if (!commands)
            return false;
        sample.record = elapsed(start);
        auto submitStart = Clock::now();
        CHECK(queue->submit(commands));
        sample.submit = elapsed(submitStart);
        auto waitStart = Clock::now();
        CHECK(queue->waitOnHost());
        sample.wait = elapsed(waitStart);
        sample.endToEnd = elapsed(start);
        std::vector<uint64_t> ticks(stages.size() * 2 * batch + 2);
        CHECK(queries->getResult(0, uint32_t(ticks.size()), ticks.data()));
        double scale = 1000.0 / double(device->getInfo().timestampFrequency);
        if (ticks.back() <= ticks.front())
            return false;
        sample.total = double(ticks.back() - ticks.front()) * scale;
        sample.stages.resize(stages.size(), 0);
        for (uint32_t iteration = 0; iteration < batch; ++iteration)
            for (size_t s = 0; s < stages.size(); ++s)
            {
                size_t q = (iteration * stages.size() + s) * 2;
                if (ticks[q + 2] < ticks[q + 1])
                    return false;
                sample.stages[s] += double(ticks[q + 2] - ticks[q + 1]) * scale;
            }
        return true;
    }

    bool validate()
    {
        std::vector<float> expected(count), actual(count), temporary(count);
        uint32_t n = options.complexity;
        for (uint32_t i = 0; i < count; ++i)
            expected[i] = inputValue(i);
        if (options.caseName == "image_pipeline")
        {
            for (int direction = 0; direction < 2; ++direction)
            {
                for (uint32_t i = 0; i < count; ++i)
                {
                    int x = int(i % width), y = int(i / width), height = int(count / width);
                    float sum = 0;
                    for (int offset = -int(n); offset <= int(n); ++offset)
                    {
                        int index = direction == 0
                                        ? y * int(width) + (x + offset + int(width)) % int(width)
                                        : ((y + offset + height) % height) * int(width) + x;
                        sum += expected[index];
                    }
                    temporary[i] = sum / float(2 * n + 1);
                }
                expected.swap(temporary);
            }
            for (float& x : expected)
                x = x / (1.0f + x);
        }
        else
            for (uint32_t i = 0; i < count; ++i)
            {
                float x = inputValue(i), derivative = 1;
                if (options.caseName.find("matrix_") == 0)
                {
                    float v[] = {x, x + 0.1f, x + 0.2f, x + 0.3f};
                    for (uint32_t j = 0; j < n; ++j)
                    {
                        float next[4] = {};
                        for (uint32_t r = 0; r < 4; ++r)
                            for (uint32_t c = 0; c < 4; ++c)
                                next[r] += matrixValue(i % 16, r, c) * v[c];
                        memcpy(v, next, sizeof(v));
                    }
                    x = v[0] + 2 * v[1] + 3 * v[2] + 4 * v[3];
                }
                else if (options.caseName.find("resources_") == 0)
                {
                    x = 0;
                    for (uint32_t j = 0; j < n; ++j)
                        x += float(j + 1) * 0.01f + float(i % 256) * 0.001f;
                }
                else
                    for (uint32_t j = 0; j < n; ++j)
                    {
                        if (options.caseName == "arithmetic_basic")
                            x = x * 0.91f + 0.03f;
                        else if (options.caseName == "arithmetic_transcendental")
                            x = std::sin(x) * 0.4f + std::cos(x * 0.7f) * 0.2f + 0.1f;
                        else if (options.caseName.find("dispatch_") == 0)
                            x = x * 0.75f + float((i + j) % n + 1) * 0.005f;
                        else
                        {
                            derivative *= 0.8f + 0.1f * x;
                            x = 0.8f * x + 0.05f * x * x;
                        }
                    }
                if (options.caseName.find("autodiff_") == 0 &&
                    options.caseName != "autodiff_primal")
                    x = derivative;
                expected[i] = x;
            }
        auto start = Clock::now();
        CHECK(device->readBuffer(buffers.back(), 0, uint64_t(count) * 4, actual.data()));
        readbackMs = elapsed(start);
        for (uint32_t i = 0; i < count; ++i)
        {
            if (!std::isfinite(actual[i]) ||
                std::abs(actual[i] - expected[i]) > 2e-4f * (1 + std::abs(expected[i])))
            {
                fprintf(
                    stderr,
                    "Validation failed at %u: got %.9g, expected %.9g\n",
                    i,
                    actual[i],
                    expected[i]);
                return false;
            }
        }
        return true;
    }
};

static int devices()
{
    printf("[");
    bool first = true;
    for (const char* backend : {"metal", "vulkan", "cuda", "d3d12"})
    {
        auto adapters = getRHI()->getAdapters(deviceType(backend));
        for (uint32_t i = 0; i < adapters.getCount(); ++i)
        {
            const auto& info = adapters.getAdapters()[i];
            printf(
                "%s{\"backend\":%s,\"index\":%u,\"name\":%s,\"vendor_id\":%u,\"device_id\":%u}",
                first ? "" : ",",
                quote(backend).c_str(),
                i,
                quote(info.name).c_str(),
                info.vendorID,
                info.deviceID);
            first = false;
        }
    }
    printf("]\n");
    return 0;
}

int main(int argc, char** argv)
{
    if (argc == 2 && strcmp(argv[1], "--devices") == 0)
        return devices();
    Benchmark bench;
    auto& o = bench.options;
    for (int i = 1; i < argc; ++i)
    {
        std::string key = argv[i];
        if (++i == argc)
        {
            fprintf(stderr, "Missing option value\n");
            return 1;
        }
        const char* value = argv[i];
        if (key == "--backend")
            o.backend = value;
        else if (key == "--case")
            o.caseName = value;
        else if (key == "--directory")
            o.directory = value;
        else if (key == "--adapter")
            o.adapter = uint32_t(strtoul(value, nullptr, 10));
        else if (key == "--complexity")
            o.complexity = uint32_t(strtoul(value, nullptr, 10));
        else if (key == "--elements")
            o.elements = uint32_t(strtoul(value, nullptr, 10));
        else if (key == "--max-elements")
            o.maxElements = uint32_t(strtoul(value, nullptr, 10));
        else if (key == "--batch")
            o.batch = uint32_t(strtoul(value, nullptr, 10));
        else if (key == "--max-batch")
            o.maxBatch = uint32_t(strtoul(value, nullptr, 10));
        else if (key == "--samples")
            o.samples = uint32_t(strtoul(value, nullptr, 10));
        else if (key == "--max-samples")
            o.maxSamples = uint32_t(strtoul(value, nullptr, 10));
        else if (key == "--warmup")
            o.warmup = uint32_t(strtoul(value, nullptr, 10));
        else if (key == "--target-ms")
            o.targetMs = strtod(value, nullptr);
        else if (key == "--max-spread")
            o.maxSpread = strtod(value, nullptr);
        else if (key == "--strict-phases" && (strcmp(value, "0") == 0 || strcmp(value, "1") == 0))
            o.strictPhases = strcmp(value, "1") == 0;
        else
        {
            fprintf(stderr, "Unknown option: %s\n", key.c_str());
            return 1;
        }
    }
    if (deviceType(o.backend) == DeviceType::Default || o.caseName.empty() || o.directory.empty() ||
        o.complexity < 1 || o.complexity > 64 || o.samples < 3 || o.maxSamples < o.samples ||
        o.maxSamples > 1000 || o.warmup < 1 || o.warmup > 1000 || o.maxBatch < 1 ||
        o.maxBatch > 1024 || o.batch > o.maxBatch || o.maxElements < 65536 ||
        o.maxElements > 16777216 || o.maxElements % 256 ||
        (o.elements && (o.elements < 65536 || o.elements > o.maxElements || o.elements % 256)) ||
        !std::isfinite(o.targetMs) || o.targetMs <= 0 || !std::isfinite(o.maxSpread) ||
        o.maxSpread <= 0)
    {
        fprintf(
            stderr,
            "Invalid configuration (elements must be multiples of 256, between 65536 and "
            "16777216)\n");
        return 1;
    }
    if (!getRHI()->getAdapter(deviceType(o.backend), o.adapter))
    {
        printf("{\"status\":\"unsupported\",\"reason\":\"requested adapter unavailable\"}\n");
        return 77;
    }
    if (!bench.initialize())
    {
        if (bench.unsupported)
        {
            printf(
                "{\"status\":\"unsupported\",\"reason\":\"GPU timestamp queries unavailable\"}\n");
            return 77;
        }
        return 1;
    }
    uint32_t elements = o.elements ? o.elements : 65536;
    uint32_t batch = o.batch ? o.batch : 1;
    struct Calibration
    {
        uint32_t elements, batch;
        double duration;
    };
    std::vector<Calibration> calibration;
    for (;;)
    {
        if (!bench.allocate(elements))
            return 1;
        if (!bench.configureQueries(batch))
            return 1;
        for (uint32_t i = 0; i < o.warmup; ++i)
        {
            Sample s;
            if (!bench.run(s))
                return 1;
        }
        std::vector<double> times;
        std::vector<Sample> pilot;
        for (int i = 0; i < 5; ++i)
        {
            Sample s;
            if (!bench.run(s))
                return 1;
            times.push_back(s.total);
            pilot.push_back(s);
        }
        double duration = median(times);
        calibration.push_back({elements, batch, duration});
        // Leave headroom so ordinary frequency drift does not put the final
        // batch just below the minimum duration. Never resize a frozen run.
        if ((duration >= o.targetMs * 1.25 && stableSamples(pilot, o.maxSpread, o.strictPhases)) ||
            (o.elements && o.batch))
            break;
        if (!o.elements && elements < o.maxElements)
            elements = std::min(elements * 2, o.maxElements);
        else if (!o.batch && batch < o.maxBatch)
            batch = std::min(batch * 2, o.maxBatch);
        else
            break;
    }
    if (!bench.validate())
        return 1;
    // CPU reference validation is deliberately outside timing; warm up again afterward.
    for (uint32_t i = 0; i < o.warmup; ++i)
    {
        Sample s;
        if (!bench.run(s))
            return 1;
    }
    std::vector<Sample> samples;
    std::vector<double> durations;
    for (uint32_t i = 0; i < o.maxSamples; ++i)
    {
        Sample sample;
        if (!bench.run(sample))
            return 1;
        durations.push_back(sample.total);
        samples.push_back(sample);
        if (samples.size() >= o.samples && stableSamples(samples, o.maxSpread, o.strictPhases))
            break;
    }
    // Validate the measured execution too, not only the warmup output.
    if (!bench.validate())
        return 1;
    bool stable =
        stableSamples(samples, o.maxSpread, o.strictPhases) && median(durations) >= o.targetMs;
    auto globalSession = bench.session->getGlobalSession();
    printf(
        "{\"status\":\"%s\",\"validated\":true,\"elements\":%u,\"width\":%u,\"batch\":%u,"
        "\"measurement_mode\":%s,"
        "\"relative_mad\":%.12g,\"backend\":%s,\"adapter\":%s,\"compiler\":%s,\"rhi_revision\":%s,"
        "\"timestamp_frequency\":%llu,\"device_ms\":%.12g,\"resource_setup_ms\":%.12g,\"validation_"
        "readback_ms\":%.12g,",
        stable ? "ok" : "unstable",
        elements,
        bench.width,
        batch,
        quote(batch == 1 ? "single_execution" : "batched_throughput").c_str(),
        spread(durations),
        quote(o.backend.c_str()).c_str(),
        quote(bench.device->getInfo().adapterName).c_str(),
        quote(globalSession->getBuildTagString()).c_str(),
        quote(RUNTIME_PERF_RHI_REV).c_str(),
        (unsigned long long)bench.device->getInfo().timestampFrequency,
        bench.deviceMs,
        bench.resourceMs,
        bench.readbackMs);
    printf("\"runner_source_sha256\":%s,\"calibration\":[", quote(RUNTIME_PERF_SOURCE_SHA).c_str());
    for (size_t i = 0; i < calibration.size(); ++i)
        printf(
            "%s{\"elements\":%u,\"batch\":%u,\"gpu_ms\":%.12g}",
            i ? "," : "",
            calibration[i].elements,
            calibration[i].batch,
            calibration[i].duration);
    printf("],\"preparation\":[");
    for (size_t i = 0; i < bench.stages.size(); ++i)
    {
        auto& s = bench.stages[i];
        printf(
            "%s{\"stage\":%s,\"load_ms\":%.12g,\"link_ms\":%.12g,\"codegen_ms\":%.12g,\"program_"
            "pipeline_ms\":%.12g}",
            i ? "," : "",
            quote(s.name.c_str()).c_str(),
            s.loadMs,
            s.linkMs,
            s.codeMs,
            s.pipelineMs);
    }
    printf("],\"samples\":[");
    for (size_t i = 0; i < samples.size(); ++i)
    {
        auto& s = samples[i];
        printf(
            "%s{\"gpu_total_ms\":%.12g,\"gpu_iteration_ms\":%.12g,\"cpu_record_ms\":%.12g,\"cpu_"
            "submit_ms\":%.12g,\"cpu_wait_ms\":%.12g,\"cpu_end_to_end_ms\":%.12g",
            i ? "," : "",
            s.total,
            s.total / batch,
            s.record,
            s.submit,
            s.wait,
            s.endToEnd);
        for (size_t j = 0; j < s.stages.size(); ++j)
            printf(
                ",%s:%.12g",
                quote(("gpu_" + bench.stages[j].name + "_ms").c_str()).c_str(),
                s.stages[j]);
        printf("}");
    }
    printf("]}\n");
    return stable ? 0 : 2;
}
