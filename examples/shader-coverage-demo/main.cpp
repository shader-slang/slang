// shader-coverage-demo — end-to-end demonstration of the
// `-trace-coverage` compiler flag + `slang-coverage-rt` host helper.
//
// This prototype demo runs in two modes:
//
//   --mode=compile (default)
//       Compiles `simulate.slang` with `-trace-coverage` via slang-rhi,
//       then queries `slang::ICoverageTracingMetadata` through the
//       Slang API to retrieve the counter → (file, line) mapping and
//       the synthesized coverage-buffer binding. Serializes that into
//       a JSON manifest at `opt.manifest`. This demonstrates that a
//       host integration can obtain coverage metadata directly from
//       the compile result, without relying on slangc's sidecar file.
//
//   --mode=report
//       Parses an existing manifest + a counter buffer (binary uint32
//       little-endian) and writes an LCOV `.info` file via the
//       `slang-coverage-rt` helper. Demonstrates that the full
//       manifest/counters → LCOV pipeline is runnable end-to-end
//       without a GPU. The counter buffer can be produced by any
//       dispatch path (slang-test CPU output, a hand-written test
//       harness, etc.).
//
// NOT YET FUNCTIONAL (known follow-up):
//   --mode=dispatch
//       Intended to allocate the coverage UAV, dispatch through
//       slang-rhi, read back, and emit LCOV in one shot. As of this
//       prototype the synthesized `__slang_coverage` buffer is not
//       reflection-visible to slang-rhi's pipeline builder, so a
//       real dispatch requires a separate host integration. The
//       option is accepted for API stability but currently reports
//       the gap to the user and falls back to `--mode=compile`.

#include "examples/example-base/example-base.h"
#include "slang-com-ptr.h"
#include "slang-coverage.h"
#include "slang-rhi.h"
#include "slang.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <slang-rhi/shader-cursor.h>
#include <string>
#include <vector>

using namespace rhi;
using Slang::ComPtr;

namespace
{

struct Options
{
    std::string mode = "compile";
    std::string backend = "cpu";
    std::string manifest = "simulate.coverage-mapping.json";
    std::string countersFile;
    std::string outputLcov = "coverage.lcov";
    std::string testName = "shader_coverage_demo";
    // -1 means "no `--coverage-binding` was passed; let parameter
    // binding auto-allocate a slot the way it does without the flag".
    int coverageBindingIndex = -1;
    int coverageBindingSpace = -1;
};

void parseArgs(int argc, char** argv, Options& opt)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        auto eq = arg.find('=');
        if (eq == std::string::npos)
            continue;
        auto k = arg.substr(0, eq);
        auto v = arg.substr(eq + 1);
        if (k == "--mode")
            opt.mode = v;
        else if (k == "--backend")
            opt.backend = v;
        else if (k == "--manifest")
            opt.manifest = v;
        else if (k == "--counters")
            opt.countersFile = v;
        else if (k == "--output")
            opt.outputLcov = v;
        else if (k == "--test-name")
            opt.testName = v;
        else if (k == "--coverage-binding")
        {
            // Parse "<index>:<space>". Both must be non-negative.
            auto colon = v.find(':');
            if (colon == std::string::npos)
            {
                std::fprintf(
                    stderr,
                    "--coverage-binding expects '<index>:<space>', got '%s'\n",
                    v.c_str());
                std::exit(1);
            }
            opt.coverageBindingIndex = std::stoi(v.substr(0, colon));
            opt.coverageBindingSpace = std::stoi(v.substr(colon + 1));
        }
    }
}

// Build the compile-option entries for the slang-rhi device. Always
// includes `-trace-coverage`; when `--coverage-binding` was given,
// also pins the synthesized buffer to the explicit slot via
// `TraceCoverageBinding`. The caller-allocated `entries` array must
// hold at least 2 slots.
void buildCoverageOptions(
    const Options& opt,
    slang::CompilerOptionEntry* entries,
    uint32_t& outCount)
{
    entries[0].name = slang::CompilerOptionName::TraceCoverage;
    entries[0].value.kind = slang::CompilerOptionValueKind::Int;
    entries[0].value.intValue0 = 1;
    outCount = 1;

    if (opt.coverageBindingIndex >= 0)
    {
        entries[1].name = slang::CompilerOptionName::TraceCoverageBinding;
        entries[1].value.kind = slang::CompilerOptionValueKind::Int;
        entries[1].value.intValue0 = opt.coverageBindingIndex;
        entries[1].value.intValue1 = opt.coverageBindingSpace;
        outCount = 2;
    }
}

DeviceType pickDeviceType(const std::string& backend)
{
    if (backend == "cpu")
        return DeviceType::CPU;
    if (backend == "vulkan" || backend == "vk")
        return DeviceType::Vulkan;
    if (backend == "d3d12")
        return DeviceType::D3D12;
    if (backend == "metal")
        return DeviceType::Metal;
    return DeviceType::CPU;
}

// ---- mode=compile ---------------------------------------------------

// JSON-escape one character into `out`. Handles `\`, `"`, control
// characters per RFC 8259 — enough for file paths.
static void appendJsonEscaped(std::string& out, char c)
{
    unsigned char uc = (unsigned char)c;
    switch (uc)
    {
    case '\\':
        out += "\\\\";
        break;
    case '"':
        out += "\\\"";
        break;
    case '\b':
        out += "\\b";
        break;
    case '\f':
        out += "\\f";
        break;
    case '\n':
        out += "\\n";
        break;
    case '\r':
        out += "\\r";
        break;
    case '\t':
        out += "\\t";
        break;
    default:
        if (uc < 0x20)
        {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned)uc);
            out += buf;
        }
        else
        {
            out.push_back((char)uc);
        }
    }
}

// Build a `.coverage-mapping.json` payload from the compiler's
// ICoverageTracingMetadata. This matches the same JSON shape that
// slangc emits via `_maybeWriteCoverageMapping`, so the resulting
// file is interchangeable with slangc's sidecar output.
static std::string buildManifestJson(slang::ICoverageTracingMetadata* coverage)
{
    std::string out;
    out += "{\n";
    out += "  \"version\": 1,\n";
    char countBuf[64];
    std::snprintf(countBuf, sizeof(countBuf), "  \"counters\": %u,\n", coverage->getCounterCount());
    out += countBuf;
    out += "  \"buffer\": {\n";
    out += "    \"name\": \"__slang_coverage\",\n";
    out += "    \"element_type\": \"uint32\",\n";
    out += "    \"element_stride\": 4,\n";
    out += "    \"synthesized\": true";
    int32_t space = coverage->getBufferSpace();
    int32_t binding = coverage->getBufferBinding();
    if (space >= 0)
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), ",\n    \"space\": %d", (int)space);
        out += buf;
    }
    if (binding >= 0)
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), ",\n    \"binding\": %d", (int)binding);
        out += buf;
    }
    out += "\n  },\n";
    out += "  \"entries\": [";
    for (uint32_t i = 0; i < coverage->getCounterCount(); ++i)
    {
        if (i > 0)
            out += ",";
        char idxBuf[64];
        std::snprintf(idxBuf, sizeof(idxBuf), "\n    {\"index\": %u, \"file\": \"", i);
        out += idxBuf;
        const char* file = coverage->getEntryFile(i);
        for (const char* p = file; p && *p; ++p)
            appendJsonEscaped(out, *p);
        char tail[64];
        std::snprintf(tail, sizeof(tail), "\", \"line\": %u}", coverage->getEntryLine(i));
        out += tail;
    }
    out += "\n  ]\n";
    out += "}\n";
    return out;
}

// Fetch ICoverageTracingMetadata from a linked program and serialize
// it to `path` as a JSON manifest in the same shape slangc writes.
// Returns the counter count on success, or 0 on failure.
//
// When `opt.coverageBindingIndex >= 0` (i.e. `--coverage-binding` was
// passed), also logs whether the metadata-reported `(index, space)`
// matches what was requested. This is the end-to-end round-trip:
// host CLI → compile-option → AST modifier → parameter binding →
// metadata API. Mismatch implies something on that path dropped or
// reinterpreted the request.
static uint32_t writeManifestFromMetadata(
    slang::IComponentType* linked,
    const std::string& path,
    const Options& opt)
{
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IMetadata> metadata;
    if (SLANG_FAILED(
            linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef())))
    {
        if (diagnostics)
            std::fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());
        std::fprintf(stderr, "failed to query entry-point metadata\n");
        return 0;
    }
    auto* coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    if (!coverage || coverage->getCounterCount() == 0)
    {
        std::fprintf(stderr, "no coverage tracing metadata on the compile result\n");
        return 0;
    }
    if (opt.coverageBindingIndex >= 0)
    {
        int32_t gotIndex = coverage->getBufferBinding();
        int32_t gotSpace = coverage->getBufferSpace();
        if (gotIndex < 0 && gotSpace < 0)
        {
            // The CPU target doesn't expose buffers as (set, register)
            // — globals get packed into a `GlobalParams` struct and
            // accessed by uniform offset. The `register(uN, spaceM)`
            // semantic the option synthesizes still applies on this
            // target's IR, but parameter binding has nothing register-
            // shaped to assign it to, so the metadata reports -1/-1.
            // The generated shader is still correct; the option just
            // has no observable effect on this backend.
            std::printf(
                "[coverage] note: backend '%s' does not use (set, register) bindings; "
                "--coverage-binding has no effect here (use vulkan/d3d12/cuda to verify "
                "the round-trip)\n",
                opt.backend.c_str());
        }
        else if (gotIndex == opt.coverageBindingIndex && gotSpace == opt.coverageBindingSpace)
        {
            std::printf(
                "[coverage] binding pinned at (index=%d, space=%d) — round-trip verified\n",
                gotIndex,
                gotSpace);
        }
        else
        {
            std::fprintf(
                stderr,
                "[coverage] requested (index=%d, space=%d) but metadata reports "
                "(index=%d, space=%d)\n",
                opt.coverageBindingIndex,
                opt.coverageBindingSpace,
                gotIndex,
                gotSpace);
        }
    }
    std::string json = buildManifestJson(coverage);
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f)
    {
        std::fprintf(stderr, "failed to open '%s' for writing\n", path.c_str());
        return 0;
    }
    std::fwrite(json.data(), 1, json.size(), f);
    std::fclose(f);
    return coverage->getCounterCount();
}

// Compile the demo shader with `-trace-coverage` pinned on the
// session so the coverage pass runs and records slot → source mapping
// in the artifact's post-emit metadata. Query
// `slang::ICoverageTracingMetadata` via the standard compile API,
// then serialize it to `opt.manifest` in the same JSON shape that
// slangc would write as a sidecar.
int runCompile(const Options& opt)
{
    slang::CompilerOptionEntry covOptions[2] = {};
    uint32_t covOptionCount = 0;
    buildCoverageOptions(opt, covOptions, covOptionCount);

    DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = pickDeviceType(opt.backend);
    deviceDesc.slang.compilerOptionEntries = covOptions;
    deviceDesc.slang.compilerOptionEntryCount = covOptionCount;

    ComPtr<IDevice> device;
    if (SLANG_FAILED(getRHI()->createDevice(deviceDesc, device.writeRef())))
    {
        std::fprintf(stderr, "failed to create device '%s'\n", opt.backend.c_str());
        return 1;
    }

    ComPtr<slang::ISession> slangSession = device->getSlangSession();
    static const ExampleResources resourceBase("shader-coverage-demo");
    Slang::String shaderPath = resourceBase.resolveResource("simulate.slang");

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module;
    module = slangSession->loadModule(shaderPath.getBuffer(), diagnostics.writeRef());
    if (diagnostics)
        std::fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());
    if (!module)
        return 1;

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    if (!entryPoint)
        return 1;

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    slangSession->createCompositeComponentType(components, 2, program.writeRef(), nullptr);

    ComPtr<slang::IComponentType> linked;
    program->link(linked.writeRef(), diagnostics.writeRef());
    if (diagnostics)
        std::fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());

    // Force codegen so the coverage pass runs. We don't need the
    // emitted code here — metadata is populated as a side effect of
    // the back-end pipeline.
    ComPtr<slang::IBlob> codeBlob;
    linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef());
    if (diagnostics)
        std::fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());

    uint32_t counterCount = writeManifestFromMetadata(linked, opt.manifest, opt);
    if (counterCount == 0)
        return 1;

    std::printf(
        "compile: %u counter slots, manifest written to %s\n",
        counterCount,
        opt.manifest.c_str());
    std::printf(
        "  run scenarios, then: %s --mode=report --counters=<file>\n",
        "shader-coverage-demo");
    return 0;
}

// ---- mode=report ----------------------------------------------------

// Take an existing manifest + counter binary and emit LCOV. This
// exercises the full slang-coverage-rt C API.
int runReport(const Options& opt)
{
    if (opt.countersFile.empty())
    {
        std::fprintf(stderr, "--mode=report requires --counters=<path to binary u32 buffer>\n");
        return 1;
    }

    SlangCoverageContext* ctx = nullptr;
    if (slang_coverage_create(opt.manifest.c_str(), &ctx) != SLANG_COVERAGE_OK)
    {
        std::fprintf(stderr, "failed to parse manifest '%s'\n", opt.manifest.c_str());
        return 1;
    }
    uint32_t n = slang_coverage_counter_count(ctx);
    const SlangCoverageBindingInfo* b = slang_coverage_binding(ctx);
    std::printf(
        "report: %u counters, buffer '%s' synthesized=%d\n",
        n,
        b->bufferName ? b->bufferName : "?",
        b->synthesized);

    FILE* f = std::fopen(opt.countersFile.c_str(), "rb");
    if (!f)
    {
        std::fprintf(stderr, "failed to open '%s'\n", opt.countersFile.c_str());
        slang_coverage_destroy(ctx);
        return 1;
    }
    std::vector<uint32_t> counters(n > 0 ? n : 1, 0);
    size_t got = std::fread(counters.data(), sizeof(uint32_t), n, f);
    std::fclose(f);
    if (got < n)
    {
        std::fprintf(
            stderr,
            "counter file '%s' has %zu values, manifest expects %u\n",
            opt.countersFile.c_str(),
            got,
            n);
        slang_coverage_destroy(ctx);
        return 1;
    }

    slang_coverage_accumulate(ctx, counters.data(), n);
    if (slang_coverage_save_lcov(ctx, opt.outputLcov.c_str(), opt.testName.c_str()) !=
        SLANG_COVERAGE_OK)
    {
        std::fprintf(stderr, "failed to write '%s'\n", opt.outputLcov.c_str());
        slang_coverage_destroy(ctx);
        return 1;
    }
    std::printf("report: wrote %s\n", opt.outputLcov.c_str());
    std::printf(
        "  render: genhtml %s -o %s-html/\n",
        opt.outputLcov.c_str(),
        opt.outputLcov.c_str());

    slang_coverage_destroy(ctx);
    return 0;
}

// ---- mode=dispatch --------------------------------------------------

// Particle type discriminants, mirroring simulate.slang.
enum : uint32_t
{
    PARTICLE_FLUID = 0,
    PARTICLE_GAS = 1,
    PARTICLE_SOLID = 2,
};

struct Particle
{
    float position[3];
    float velocity[3];
    uint32_t type;
    uint32_t flags;
};
static_assert(sizeof(Particle) == 32, "Particle layout must match simulate.slang");

// Build an input particle array for a named scenario. Different
// scenarios exercise different branches of the shader, producing
// distinguishable coverage reports.
std::vector<Particle> makeParticles(const std::string& scenario, uint32_t count)
{
    std::vector<Particle> ps(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        Particle& p = ps[i];
        p.position[0] = float(i) * 0.1f;
        p.position[1] = 1.0f;
        p.position[2] = 0.0f;
        p.velocity[0] = p.velocity[1] = p.velocity[2] = 0.0f;
        p.flags = 0;
        if (scenario == "fluid-only")
            p.type = PARTICLE_FLUID;
        else if (scenario == "edge-cases")
        {
            p.type = PARTICLE_SOLID;
            p.position[1] = -0.5f; // start below ground → bounce path
        }
        else
            p.type = i % 3; // "mixed"
    }
    return ps;
}

int runDispatch(const Options& opt)
{
    slang::CompilerOptionEntry covOptions[2] = {};
    uint32_t covOptionCount = 0;
    buildCoverageOptions(opt, covOptions, covOptionCount);

    DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = pickDeviceType(opt.backend);
    deviceDesc.slang.compilerOptionEntries = covOptions;
    deviceDesc.slang.compilerOptionEntryCount = covOptionCount;

    ComPtr<IDevice> device;
    if (SLANG_FAILED(getRHI()->createDevice(deviceDesc, device.writeRef())))
    {
        std::fprintf(stderr, "failed to create device '%s'\n", opt.backend.c_str());
        return 1;
    }

    ComPtr<slang::ISession> slangSession = device->getSlangSession();
    static const ExampleResources resourceBase("shader-coverage-demo");
    Slang::String shaderPath = resourceBase.resolveResource("simulate.slang");

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module;
    module = slangSession->loadModule(shaderPath.getBuffer(), diagnostics.writeRef());
    if (diagnostics)
        std::fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());
    if (!module)
        return 1;

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    if (!entryPoint)
        return 1;

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    slangSession->createCompositeComponentType(components, 2, program.writeRef(), nullptr);

    ComPtr<slang::IComponentType> linked;
    program->link(linked.writeRef(), diagnostics.writeRef());
    if (diagnostics)
        std::fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());

    ShaderProgramDesc progDesc = {};
    progDesc.slangGlobalScope = linked;
    ComPtr<ISlangBlob> rhiDiag;
    ComPtr<IShaderProgram> shaderProgram;
    Result spResult = device->createShaderProgram(
        progDesc,
        shaderProgram.writeRef(),
        rhiDiag.writeRef());
    if (SLANG_FAILED(spResult) || !shaderProgram)
    {
        if (rhiDiag)
            std::fprintf(stderr, "%s", (const char*)rhiDiag->getBufferPointer());
        std::fprintf(
            stderr,
            "failed to create shader program on backend '%s' (0x%08x)\n",
            opt.backend.c_str(),
            (unsigned)spResult);
        return 1;
    }

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram;
    ComPtr<IComputePipeline> pipeline = device->createComputePipeline(pipelineDesc);
    if (!pipeline)
    {
        std::fprintf(
            stderr,
            "failed to create compute pipeline on backend '%s'\n",
            opt.backend.c_str());
        return 1;
    }

    // Allocate particle & coverage buffers.
    uint32_t particleCount = 64;
    uint32_t steps = 8;
    std::vector<Particle> particles = makeParticles("mixed", particleCount);

    BufferDesc partDesc = {};
    partDesc.size = sizeof(Particle) * particles.size();
    partDesc.elementSize = sizeof(Particle);
    partDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                     BufferUsage::CopyDestination | BufferUsage::CopySource;
    partDesc.memoryType = MemoryType::DeviceLocal;
    ComPtr<IBuffer> partBuf = device->createBuffer(partDesc, particles.data());

    // Query the freshly-compiled entry point's metadata and serialize
    // a manifest JSON file, so slang_coverage_create can ingest it the
    // same way it would a slangc-produced sidecar.
    if (writeManifestFromMetadata(linked, opt.manifest, opt) == 0)
        return 1;

    // Size the coverage buffer from the manifest we just wrote.
    SlangCoverageContext* covCtx = nullptr;
    if (slang_coverage_create(opt.manifest.c_str(), &covCtx) != SLANG_COVERAGE_OK)
    {
        std::fprintf(stderr, "failed to parse manifest '%s'\n", opt.manifest.c_str());
        return 1;
    }
    uint32_t counterCount = slang_coverage_counter_count(covCtx);
    std::printf(
        "dispatch: %u counters, backend=%s, scenario=mixed\n",
        counterCount,
        opt.backend.c_str());

    BufferDesc covDesc = {};
    covDesc.size = sizeof(uint32_t) * (counterCount > 0 ? counterCount : 1);
    covDesc.elementSize = sizeof(uint32_t);
    covDesc.usage =
        BufferUsage::UnorderedAccess | BufferUsage::CopyDestination | BufferUsage::CopySource;
    covDesc.memoryType = MemoryType::DeviceLocal;
    std::vector<uint32_t> zeros(counterCount > 0 ? counterCount : 1, 0);
    ComPtr<IBuffer> covBuf = device->createBuffer(covDesc, zeros.data());

    // Dispatch `steps` times.
    auto queue = device->getQueue(QueueType::Graphics);
    for (uint32_t step = 0; step < steps; ++step)
    {
        auto encoder = queue->createCommandEncoder();
        auto passEnc = encoder->beginComputePass();
        auto rootObject = passEnc->bindPipeline(pipeline);
        ShaderCursor cursor(rootObject);
        cursor["particles"].setBinding(partBuf);
        cursor["Params"]["particleCount"].setData(particleCount);
        cursor["Params"]["dt"].setData(0.016f);
        // Probe whether the reflection system knows about the
        // synthesized coverage buffer. `ShaderCursor::isValid()`
        // lets us detect the reflection-integration gap rather than
        // dereferencing a stale cursor and crashing.
        ShaderCursor covCursor = cursor["__slang_coverage"];
        if (covCursor.isValid())
        {
            covCursor.setBinding(covBuf);
        }
        else if (step == 0)
        {
            std::fprintf(
                stderr,
                "[dispatch] NOTE: `__slang_coverage` not found in reflection;\n"
                "[dispatch] the shader will still run but counters will stay 0.\n"
                "[dispatch] (follow-up work: make synthesized buffer reflection-visible)\n");
        }
        uint32_t groups = (particleCount + 63) / 64;
        passEnc->dispatchCompute(groups, 1, 1);
        passEnc->end();
        queue->submit(encoder->finish());
        queue->waitOnHost();
    }
    // Read the coverage buffer back and accumulate into the context.
    ComPtr<ISlangBlob> blob;
    device->readBuffer(covBuf, 0, covDesc.size, blob.writeRef());
    slang_coverage_accumulate(
        covCtx,
        static_cast<const uint32_t*>(blob->getBufferPointer()),
        counterCount);

    if (slang_coverage_save_lcov(covCtx, opt.outputLcov.c_str(), opt.testName.c_str()) !=
        SLANG_COVERAGE_OK)
    {
        std::fprintf(stderr, "failed to write '%s'\n", opt.outputLcov.c_str());
        slang_coverage_destroy(covCtx);
        return 1;
    }
    std::printf("dispatch: wrote %s\n", opt.outputLcov.c_str());
    std::printf(
        "  render: genhtml %s -o %s-html/\n",
        opt.outputLcov.c_str(),
        opt.outputLcov.c_str());
    slang_coverage_destroy(covCtx);
    return 0;
}

} // anonymous namespace

int exampleMain(int argc, char** argv)
{
    Options opt;
    parseArgs(argc, argv, opt);

    if (opt.mode == "compile")
        return runCompile(opt);
    if (opt.mode == "report")
        return runReport(opt);
    if (opt.mode == "dispatch")
        return runDispatch(opt);

    std::fprintf(stderr, "unknown --mode=%s\n", opt.mode.c_str());
    return 1;
}
