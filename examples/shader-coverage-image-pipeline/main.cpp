// Image-pipeline coverage demo — raw-Vulkan host driver.
//
// Single-dispatch compute pipeline: denoise → tonemap → gamma applied
// to a synthetic 1080p test image. Runs in "smoke" mode (single
// operator/boundary/gamma) or "exhaustive" mode (full sweep). The
// coverage delta between the two runs is the demo's headline.
//
// All GPU-runtime calls go through `vk_compute_demo.h` so the entire
// raw-Vulkan path is isolated to one file. When slang-rhi PR #739
// lands, the migration replaces only `vk_compute_demo.h` + this file's
// Vulkan touch points; the slang sources and demo logic stay
// unchanged. See `vk_compute_demo.h`'s file-level comment for the
// step-by-step swap procedure.

#include "vk_compute_demo.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <slang-com-ptr.h>
#include <slang.h>
#include <string>
#include <string_view>
#include <vector>

using Slang::ComPtr;

namespace
{

constexpr uint32_t kCoverageBinding = 0;
constexpr uint32_t kCoverageSet = 1;
constexpr uint32_t kImageWidth = 1920;
constexpr uint32_t kImageHeight = 1080;

struct PipelineParams
{
    uint32_t imageWidth;
    uint32_t imageHeight;
    uint32_t tonemapOp;
    uint32_t boundaryMode;
    uint32_t gammaMode;
    float bilateralSigmaS;
    float bilateralSigmaR;
    float exposure;
};

[[noreturn]] void fail(const std::string& message)
{
    std::cerr << "error: " << message << "\n";
    std::exit(1);
}

void checkSlang(SlangResult result, const char* what)
{
    if (SLANG_FAILED(result))
        fail(std::string(what) + " failed");
}

void diagnoseIfNeeded(slang::IBlob* diagnostics)
{
    if (diagnostics && diagnostics->getBufferSize())
    {
        std::cerr.write(
            reinterpret_cast<const char*>(diagnostics->getBufferPointer()),
            diagnostics->getBufferSize());
        std::cerr << "\n";
    }
}

std::filesystem::path getDemoDirectory()
{
    return std::filesystem::path(__FILE__).parent_path();
}

struct CompiledShader
{
    std::vector<uint8_t> spirv;
    ComPtr<slang::IMetadata> metadata;
    slang::ICoverageTracingMetadata* coverageMetadata = nullptr;
};

// Compile pipeline.slang to SPIR-V plus optional coverage metadata.
// `enableCoverage` toggles the `-trace-coverage*` flags so the same
// binary can produce a baseline (uncovered) timing measurement.
// `counterWidthBits` is forwarded to `-trace-coverage-counter-width`
// when coverage is enabled; valid values are 32 and 64. The default
// (64) is correct on any Vulkan driver that supports
// `VK_KHR_shader_atomic_int64`; pass 32 on runtimes that do not
// (most notably MoltenVK on Apple Silicon as of MoltenVK 1.4).
CompiledShader compileShader(bool enableCoverage, int counterWidthBits)
{
    ComPtr<slang::IGlobalSession> globalSession;
    checkSlang(slang::createGlobalSession(globalSession.writeRef()), "createGlobalSession");

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    std::vector<slang::CompilerOptionEntry> options;
    auto pushBool = [&](slang::CompilerOptionName name)
    {
        slang::CompilerOptionEntry e = {};
        e.name = name;
        e.value.kind = slang::CompilerOptionValueKind::Int;
        e.value.intValue0 = 1;
        options.push_back(e);
    };
    pushBool(slang::CompilerOptionName::EmitSpirvDirectly);
    if (enableCoverage)
    {
        pushBool(slang::CompilerOptionName::TraceCoverage);
        pushBool(slang::CompilerOptionName::TraceFunctionCoverage);
        pushBool(slang::CompilerOptionName::TraceBranchCoverage);

        slang::CompilerOptionEntry pin = {};
        pin.name = slang::CompilerOptionName::TraceCoverageBinding;
        pin.value.kind = slang::CompilerOptionValueKind::Int;
        pin.value.intValue0 = kCoverageBinding;
        pin.value.intValue1 = kCoverageSet;
        options.push_back(pin);

        // Forward the counter-width selection. The slangc CLI parser
        // converts `-trace-coverage-counter-width <bits>` to a byte
        // width (4 or 8); the API option matches that convention.
        slang::CompilerOptionEntry widthOpt = {};
        widthOpt.name = slang::CompilerOptionName::TraceCoverageCounterWidth;
        widthOpt.value.kind = slang::CompilerOptionValueKind::Int;
        widthOpt.value.intValue0 = counterWidthBits / 8;
        options.push_back(widthOpt);
    }

    const std::string searchPath = getDemoDirectory().string();
    const char* searchPaths[] = {searchPath.c_str()};

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 1;
    sessionDesc.compilerOptionEntries = options.data();
    sessionDesc.compilerOptionEntryCount = (uint32_t)options.size();

    ComPtr<slang::ISession> session;
    checkSlang(globalSession->createSession(sessionDesc, session.writeRef()), "createSession");

    ComPtr<slang::IBlob> diagnostics;
    const std::string modulePath = (getDemoDirectory() / "pipeline.slang").string();
    slang::IModule* loaded = session->loadModule(modulePath.c_str(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics);
    if (!loaded)
        fail("loadModule(pipeline.slang)");
    ComPtr<slang::IModule> module(loaded);

    ComPtr<slang::IEntryPoint> entryPoint;
    checkSlang(
        module->findEntryPointByName("computeMain", entryPoint.writeRef()),
        "findEntryPointByName");

    slang::IComponentType* parts[] = {module.get(), entryPoint.get()};
    ComPtr<slang::IComponentType> composed;
    diagnostics.setNull();
    checkSlang(
        session
            ->createCompositeComponentType(parts, 2, composed.writeRef(), diagnostics.writeRef()),
        "createCompositeComponentType");
    diagnoseIfNeeded(diagnostics);

    ComPtr<slang::IComponentType> linked;
    diagnostics.setNull();
    checkSlang(composed->link(linked.writeRef(), diagnostics.writeRef()), "link");
    diagnoseIfNeeded(diagnostics);

    ComPtr<slang::IBlob> code;
    diagnostics.setNull();
    checkSlang(
        linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()),
        "getEntryPointCode");
    diagnoseIfNeeded(diagnostics);

    CompiledShader out;
    out.spirv.assign(
        (const uint8_t*)code->getBufferPointer(),
        (const uint8_t*)code->getBufferPointer() + code->getBufferSize());

    if (enableCoverage)
    {
        diagnostics.setNull();
        checkSlang(
            linked->getEntryPointMetadata(0, 0, out.metadata.writeRef(), diagnostics.writeRef()),
            "getEntryPointMetadata");
        diagnoseIfNeeded(diagnostics);
        out.coverageMetadata = (slang::ICoverageTracingMetadata*)out.metadata->castAs(
            slang::ICoverageTracingMetadata::getTypeGuid());
        if (!out.coverageMetadata)
            fail("expected coverage metadata");
    }
    return out;
}

// Deterministic synthetic 1080p HDR test image: smooth gradient +
// hard-edged strips + an HDR spike.
std::vector<float> generateTestImage(uint32_t width, uint32_t height)
{
    std::vector<float> pixels(width * height * 4);
    std::mt19937 rng(12345);
    std::normal_distribution<float> noise(0.0f, 0.05f);
    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            float u = float(x) / float(width);
            float v = float(y) / float(height);
            float r = u;
            float g = v;
            float b = 0.5f * (u + v);
            int stripX = int(u * 8.0f);
            int stripY = int(v * 8.0f);
            if ((stripX + stripY) & 1)
            {
                r *= 1.2f;
                g *= 0.8f;
            }
            float dx = u - 0.75f;
            float dy = v - 0.25f;
            float d2 = dx * dx + dy * dy;
            if (d2 < 0.01f)
            {
                float gain = 10.0f * (1.0f - d2 * 100.0f);
                r *= gain;
                g *= gain;
                b *= gain;
            }
            r += noise(rng);
            g += noise(rng);
            b += noise(rng);
            size_t off = (size_t(y) * width + x) * 4;
            pixels[off + 0] = std::max(0.0f, r);
            pixels[off + 1] = std::max(0.0f, g);
            pixels[off + 2] = std::max(0.0f, b);
            pixels[off + 3] = 1.0f;
        }
    }
    return pixels;
}

struct DispatchConfig
{
    uint32_t tonemapOp;
    uint32_t boundaryMode;
    uint32_t gammaMode;
    float sigmaS;
    float sigmaR;
    float exposure;
};

std::vector<DispatchConfig> buildSmokeConfigs()
{
    DispatchConfig c = {};
    c.tonemapOp = 0;
    c.boundaryMode = 0;
    c.gammaMode = 0;
    c.sigmaS = 1.0f;
    c.sigmaR = 0.1f;
    c.exposure = 1.0f;
    return {c};
}

std::vector<DispatchConfig> buildExhaustiveConfigs()
{
    std::vector<DispatchConfig> configs;
    for (uint32_t op = 0; op < 4; ++op)
        for (uint32_t boundary = 0; boundary < 4; ++boundary)
            for (uint32_t gamma = 0; gamma < 3; ++gamma)
            {
                DispatchConfig c = {};
                c.tonemapOp = op;
                c.boundaryMode = boundary;
                c.gammaMode = gamma;
                c.sigmaS = (op % 2 == 0) ? 1.0f : 2.5f;
                c.sigmaR = 0.1f;
                c.exposure = 1.0f;
                configs.push_back(c);
            }
    return configs;
}

void writeManifest(slang::ICoverageTracingMetadata* coverage, const std::filesystem::path& path)
{
    ComPtr<ISlangBlob> manifestBlob;
    checkSlang(
        slang_writeCoverageManifestJson(coverage, manifestBlob.writeRef()),
        "slang_writeCoverageManifestJson");
    std::ofstream out(path, std::ios::binary);
    out.write(
        static_cast<const char*>(manifestBlob->getBufferPointer()),
        (std::streamsize)manifestBlob->getBufferSize());
}

void writeCountersBinary(const std::vector<uint8_t>& rawBytes, const std::filesystem::path& path)
{
    // The host file mirrors the on-GPU memory layout exactly: N
    // little-endian unsigned integers, each of byte width
    // `manifest.buffer.element_stride`. Downstream tools (the LCOV
    // converter, the HTML renderer, slang-rhi consumers) read both
    // the manifest and this file as a pair.
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(rawBytes.data()), (std::streamsize)rawBytes.size());
}

void writeLcov(
    slang::ICoverageTracingMetadata* coverage,
    const std::vector<uint64_t>& hits,
    const std::filesystem::path& path,
    const char* testName)
{
    const uint32_t counterCount = coverage->getCounterCount();
    std::map<std::string, std::map<uint32_t, uint64_t>> byFile;
    for (uint32_t i = 0; i < counterCount; ++i)
    {
        slang::CoverageEntryInfo entry = {};
        if (SLANG_FAILED(coverage->getEntryInfo(i, &entry)))
            continue;
        if (!entry.file || !*entry.file || entry.line == 0)
            continue;
        byFile[entry.file][entry.line] += hits[i];
    }
    std::ofstream f(path, std::ios::binary);
    f << "TN:" << testName << "\n";
    for (const auto& fp : byFile)
    {
        f << "SF:" << fp.first << "\n";
        for (const auto& lp : fp.second)
            f << "DA:" << lp.first << "," << lp.second << "\n";
        f << "end_of_record\n";
    }
}

struct CoverageSummary
{
    uint32_t lineCovered = 0;
    uint32_t lineTotal = 0;
    uint32_t functionCovered = 0;
    uint32_t functionTotal = 0;
    uint32_t branchCovered = 0;
    uint32_t branchTotal = 0;
};

CoverageSummary summarize(
    slang::ICoverageTracingMetadata* coverage,
    const std::vector<uint64_t>& hits)
{
    CoverageSummary s = {};
    const uint32_t n = coverage->getCounterCount();
    for (uint32_t i = 0; i < n; ++i)
    {
        slang::CoverageEntryInfo entry = {};
        if (SLANG_FAILED(coverage->getEntryInfo(i, &entry)))
            continue;
        const bool covered = hits[i] > 0;
        switch (entry.kind)
        {
        case slang::CoverageEntryKind::Line:
            ++s.lineTotal;
            if (covered)
                ++s.lineCovered;
            break;
        case slang::CoverageEntryKind::Function:
            ++s.functionTotal;
            if (covered)
                ++s.functionCovered;
            break;
        case slang::CoverageEntryKind::Branch:
            ++s.branchTotal;
            if (covered)
                ++s.branchCovered;
            break;
        default:
            break;
        }
    }
    return s;
}

void printSummary(const char* label, const CoverageSummary& s)
{
    std::cout << label << ":\n"
              << "  line     : " << s.lineCovered << " / " << s.lineTotal << "\n"
              << "  function : " << s.functionCovered << " / " << s.functionTotal << "\n"
              << "  branch   : " << s.branchCovered << " / " << s.branchTotal << "\n";
}

} // namespace

int main(int argc, char** argv)
{
    std::string mode = "smoke";
    bool enableCoverage = true;
    // 32 by default so the demo runs on MoltenVK out of the box.
    // Pass `--counter-width=64` on any Vulkan driver that supports
    // `VK_KHR_shader_atomic_int64` to exercise the wider counters.
    int counterWidthBits = 32;
    for (int i = 1; i < argc; ++i)
    {
        std::string_view a = argv[i];
        if (a == "--mode=smoke")
            mode = "smoke";
        else if (a == "--mode=exhaustive")
            mode = "exhaustive";
        else if (a == "--no-coverage")
            enableCoverage = false;
        else if (a == "--coverage")
            enableCoverage = true;
        else if (a == "--counter-width=32")
            counterWidthBits = 32;
        else if (a == "--counter-width=64")
            counterWidthBits = 64;
        else
        {
            std::cerr << "unknown arg: " << a << "\n";
            return 1;
        }
    }

    std::cout << "compiling pipeline.slang"
              << (enableCoverage ? " (coverage on, " : " (no coverage")
              << (enableCoverage ? (std::to_string(counterWidthBits) + "-bit counters)\n") : ")\n");
    auto shader = compileShader(enableCoverage, counterWidthBits);

    uint32_t counterCount = 0;
    uint32_t counterByteWidth = 4;
    if (enableCoverage)
    {
        counterCount = shader.coverageMetadata->getCounterCount();
        // Match the GPU-side buffer's per-slot byte width to whatever
        // the compiler synthesized. The default is uint64 (8); the
        // `-trace-coverage-counter-width 32` opt-down produces uint32
        // (4). Either way the host must allocate and read back the
        // matching layout.
        slang::CoverageBufferInfo bufferInfo = {};
        if (SLANG_SUCCEEDED(shader.coverageMetadata->getBufferInfo(&bufferInfo)) &&
            bufferInfo.elementByteWidth != 0)
        {
            counterByteWidth = bufferInfo.elementByteWidth;
        }
        std::cout << "coverage counter count: " << counterCount << " (" << (counterByteWidth * 8)
                  << "-bit slots)\n";
    }

    vkdemo::Context ctx;
    ctx.init();

    // Application bindings at set=0; coverage at set=kCoverageSet binding=kCoverageBinding.
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> setBindings;
    setBindings.resize(2);
    auto pushBinding = [](std::vector<VkDescriptorSetLayoutBinding>& v, uint32_t b)
    {
        VkDescriptorSetLayoutBinding lb = {};
        lb.binding = b;
        lb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        lb.descriptorCount = 1;
        lb.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        v.push_back(lb);
    };
    pushBinding(setBindings[0], 0); // inputImage
    pushBinding(setBindings[0], 1); // outputImage
    pushBinding(setBindings[0], 2); // paramsBuffer
    if (enableCoverage)
    {
        pushBinding(setBindings[1], kCoverageBinding); // coverage buffer
    }
    else
    {
        setBindings.pop_back(); // no coverage set
    }

    // Slang's SPIR-V emit renames the entry point to "main" by default.
    auto pipe =
        ctx.createComputePipeline(shader.spirv.data(), shader.spirv.size(), setBindings, "main");

    const auto image = generateTestImage(kImageWidth, kImageHeight);
    auto inputBuf =
        ctx.createBuffer(image.size() * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    ctx.upload(inputBuf, image.data(), inputBuf.size);
    auto outputBuf =
        ctx.createBuffer(image.size() * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    auto paramsBuf = ctx.createBuffer(sizeof(PipelineParams), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    vkdemo::Buffer coverageBuf = {};
    if (enableCoverage)
    {
        coverageBuf = ctx.createBuffer(
            (size_t)counterCount * counterByteWidth,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        std::vector<uint8_t> zero((size_t)counterCount * counterByteWidth, 0u);
        ctx.upload(coverageBuf, zero.data(), coverageBuf.size);
    }

    auto configs = (mode == "smoke") ? buildSmokeConfigs() : buildExhaustiveConfigs();
    std::cout << "running " << configs.size() << " dispatch(es) in mode=" << mode
              << " (coverage=" << (enableCoverage ? "on" : "off") << ")\n";

    const auto renderStart = std::chrono::steady_clock::now();
    for (const auto& cfg : configs)
    {
        PipelineParams p = {};
        p.imageWidth = kImageWidth;
        p.imageHeight = kImageHeight;
        p.tonemapOp = cfg.tonemapOp;
        p.boundaryMode = cfg.boundaryMode;
        p.gammaMode = cfg.gammaMode;
        p.bilateralSigmaS = cfg.sigmaS;
        p.bilateralSigmaR = cfg.sigmaR;
        p.exposure = cfg.exposure;
        ctx.upload(paramsBuf, &p, sizeof(p));

        VkDescriptorSet set0 = ctx.allocateDescriptorSet(pipe.setLayouts[0]);
        ctx.writeStorageBuffer(set0, 0, inputBuf);
        ctx.writeStorageBuffer(set0, 1, outputBuf);
        ctx.writeStorageBuffer(set0, 2, paramsBuf);

        std::vector<VkDescriptorSet> sets = {set0};
        if (enableCoverage)
        {
            VkDescriptorSet set1 = ctx.allocateDescriptorSet(pipe.setLayouts[1]);
            ctx.writeStorageBuffer(set1, kCoverageBinding, coverageBuf);
            sets.push_back(set1);
        }

        const uint32_t groupsX = (kImageWidth + 7) / 8;
        const uint32_t groupsY = (kImageHeight + 7) / 8;
        ctx.dispatch(pipe, sets, groupsX, groupsY, 1);
    }
    const auto renderEnd = std::chrono::steady_clock::now();
    const double renderMs =
        std::chrono::duration<double, std::milli>(renderEnd - renderStart).count();
    std::cout << "render wall time: " << renderMs << " ms (" << configs.size() << " dispatches, "
              << (renderMs / configs.size()) << " ms/dispatch)\n";

    if (!enableCoverage)
    {
        ctx.destroyBuffer(inputBuf);
        ctx.destroyBuffer(outputBuf);
        ctx.destroyBuffer(paramsBuf);
        ctx.destroyPipeline(pipe);
        std::cout << "no-coverage baseline complete\n";
        return 0;
    }

    // Read the GPU buffer back as raw bytes, then widen each slot to
    // a uint64_t so the summary/LCOV writers don't have to care about
    // the on-GPU width. The host file (`<mode>.counters.bin`) is
    // written out at the original byte width so downstream tools that
    // consume the manifest's `element_stride` see consistent layout.
    std::vector<uint8_t> rawBytes((size_t)counterCount * counterByteWidth);
    ctx.download(coverageBuf, rawBytes.data(), coverageBuf.size);
    std::vector<uint64_t> hits(counterCount, 0);
    for (uint32_t i = 0; i < counterCount; ++i)
    {
        uint64_t value = 0;
        const uint8_t* slot = rawBytes.data() + (size_t)i * counterByteWidth;
        for (uint32_t b = 0; b < counterByteWidth; ++b)
            value |= (uint64_t)slot[b] << (b * 8);
        hits[i] = value;
    }

    auto summary = summarize(shader.coverageMetadata, hits);
    printSummary(mode.c_str(), summary);

    const auto outDir = getDemoDirectory();
    writeManifest(shader.coverageMetadata, outDir / (mode + ".coverage-mapping.json"));
    writeLcov(
        shader.coverageMetadata,
        hits,
        outDir / (mode + ".lcov"),
        ("image-pipeline-" + mode).c_str());
    writeCountersBinary(rawBytes, outDir / (mode + ".counters.bin"));
    std::cout << "wrote " << (outDir / (mode + ".coverage-mapping.json")) << "\n";
    std::cout << "wrote " << (outDir / (mode + ".lcov")) << "\n";
    std::cout << "wrote " << (outDir / (mode + ".counters.bin")) << "\n";

    ctx.destroyBuffer(inputBuf);
    ctx.destroyBuffer(outputBuf);
    ctx.destroyBuffer(paramsBuf);
    ctx.destroyBuffer(coverageBuf);
    ctx.destroyPipeline(pipe);
    std::cout << "done\n";
    return 0;
}
