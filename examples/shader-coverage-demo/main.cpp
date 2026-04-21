// shader-coverage-demo — end-to-end demonstration of the
// `-trace-coverage` compiler flag + `slang-coverage-rt` host helper.
//
// This prototype demo runs in two modes:
//
//   --mode=compile (default)
//       Compiles `simulate.slang` with `-trace-coverage` via slang-rhi.
//       Produces `simulate.slangcov` (the counter->source manifest).
//       This demonstrates that the compiler flag is reachable from a
//       normal slang-rhi host and that the manifest ships alongside
//       compiled output.
//
//   --mode=report
//       Parses an existing `.slangcov` manifest + a counter buffer
//       (binary uint32 little-endian) and writes an LCOV `.info` file
//       via the `slang-coverage-rt` helper. Demonstrates that the
//       full manifest/counters → LCOV pipeline is runnable end-to-end
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
    std::string manifest = "simulate.slangcov";
    std::string countersFile;
    std::string outputLcov = "coverage.lcov";
    std::string testName = "shader_coverage_demo";
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

// Compile the demo shader with `-trace-coverage` pinned on the
// session so the coverage pass runs and writes the manifest sidecar.
// We stop before building the ComputePipeline because on current
// slang-rhi, the synthesized coverage buffer trips pipeline builder
// reflection. The manifest is still produced during shader-program
// linking, which is all this mode needs to demonstrate.
int runCompile(const Options& opt)
{
    // Tell the coverage pass where to write the sidecar manifest.
    static std::string manifestEnv;
    manifestEnv = "SLANG_COVERAGE_MANIFEST_PATH=" + opt.manifest;
    putenv(const_cast<char*>(manifestEnv.c_str()));

    slang::CompilerOptionEntry covOption = {};
    covOption.name = slang::CompilerOptionName::TraceCoverage;
    covOption.value.kind = slang::CompilerOptionValueKind::Int;
    covOption.value.intValue0 = 1;

    DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = pickDeviceType(opt.backend);
    deviceDesc.slang.compilerOptionEntries = &covOption;
    deviceDesc.slang.compilerOptionEntryCount = 1;

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
    module.attach(slangSession->loadModule(shaderPath.getBuffer(), diagnostics.writeRef()));
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

    // Force codegen so the coverage pass runs and the manifest is
    // written.  This triggers the full IR pipeline for the selected
    // target; the coverage pass writes the `.slangcov` sidecar during
    // that pipeline.
    ComPtr<slang::IBlob> codeBlob;
    linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef());
    if (diagnostics)
        std::fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());

    std::printf("compile: manifest written to %s\n", opt.manifest.c_str());
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
    static std::string manifestEnv;
    manifestEnv = "SLANG_COVERAGE_MANIFEST_PATH=" + opt.manifest;
    putenv(const_cast<char*>(manifestEnv.c_str()));

    slang::CompilerOptionEntry covOption = {};
    covOption.name = slang::CompilerOptionName::TraceCoverage;
    covOption.value.kind = slang::CompilerOptionValueKind::Int;
    covOption.value.intValue0 = 1;

    DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = pickDeviceType(opt.backend);
    deviceDesc.slang.compilerOptionEntries = &covOption;
    deviceDesc.slang.compilerOptionEntryCount = 1;

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
    module.attach(slangSession->loadModule(shaderPath.getBuffer(), diagnostics.writeRef()));
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
    ComPtr<IShaderProgram> shaderProgram = device->createShaderProgram(progDesc);
    if (!shaderProgram)
        return 1;

    std::fprintf(stderr, "[dispatch] createComputePipeline...\n");
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
    std::fprintf(stderr, "[dispatch] pipeline OK\n");

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
    std::fprintf(stderr, "[dispatch] partBuf OK\n");

    // Size the coverage buffer from the manifest the compile just wrote.
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
    std::fprintf(stderr, "[dispatch] covBuf OK, dispatching...\n");

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
    std::fprintf(stderr, "[dispatch] dispatched, reading back...\n");

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
