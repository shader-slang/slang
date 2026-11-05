// Image-pipeline coverage demo — slang-rhi Vulkan host driver.
//
// Single-dispatch compute pipeline: denoise → tonemap → gamma applied
// to a synthetic test image (320x180 by default). Runs in
// "smoke" mode (single operator/boundary/gamma) or "full" mode (full
// parameter sweep). The coverage delta between the two runs is the
// demo's headline.
//
// The host uses slang-rhi synthetic bindings for the hidden coverage buffer.
// For direct CPU/CUDA/Vulkan/Metal binding, see shader-coverage-backends.

#include "shader-coverage-common/coverage-counters.h"
#include "shader-coverage-common/coverage-rhi.h"

#include <charconv>
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
#include <slang-rhi/shader-cursor.h>
#include <slang.h>
#include <string>
#include <string_view>
#include <vector>

using Slang::ComPtr;
using coverageDemo::decodeCoverageCounters;
using coverageDemo::createStorageBuffer;

namespace
{

// The application's compute resources live on descriptor set 0. The
// synthesized `__slang_coverage` buffer is placed on its own set by the
// compiler; rather than hardcode that location, this demo *discovers* it
// from `ISyntheticResourceMetadata` after compilation (see
// `compileShader`) — the "metadata-derived binding" approach. Contrast
// the BVH-traversal demo, which dictates the slot up front via
// `-trace-coverage-binding` (the "explicit placement" approach). Both
// are valid; this pair exists to show each one end-to-end.

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
    // Row offset of the current dispatch's tile (see the dispatch loop
    // in main). Must match `PipelineParams` in pipeline.slang.
    uint32_t tileOriginY;
};

[[noreturn]] void fail(const std::string& message)
{
    std::cerr << "error: " << message << "\n";
    std::exit(1);
}

// Parse the entire argument so negative, overflowing, or trailing text is rejected.
uint32_t parseUnsigned(std::string_view value, const char* option)
{
    uint32_t result = 0;
    auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
        fail(std::string(option) + " requires an unsigned integer");
    return result;
}

void checkSlang(SlangResult result, const char* what)
{
    if (SLANG_FAILED(result))
        fail(std::string(what) + " failed with SlangResult " + std::to_string(result));
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

// Set by `main()` from `--demo-dir=<path>` when supplied; otherwise
// stays empty and `getDemoDirectory()` falls back to its default
// `__FILE__`-then-CWD discovery. Process-scope (anonymous-namespace)
// because every caller of `getDemoDirectory()` lives in the same
// translation unit and threading the override through compileShader
// and friends would clutter unrelated signatures.
std::filesystem::path g_demoDirOverride;

// Resolves the directory containing the demo's `.slang` assets.
// Discovery order:
//   1. `--demo-dir=<path>` from the CLI, if supplied.
//   2. The compile-time `__FILE__` parent directory, if the anchor
//      `pipeline.slang` is still there. Works for the intended
//      "run from source tree" workflow.
//   3. The current working directory, as a last-resort fallback for
//      a user who runs the binary from a directory that happens to
//      contain the demo shaders (e.g. a CI runner that `cd`s in).
// If none of these point at the shaders, the subsequent
// `loadModule` call surfaces a clear path-not-found error.
std::filesystem::path getDemoDirectory()
{
    if (!g_demoDirOverride.empty())
        return g_demoDirOverride;
    std::filesystem::path sourceDir = std::filesystem::path(__FILE__).parent_path();
    if (std::filesystem::exists(sourceDir / "pipeline.slang"))
        return sourceDir;
    return std::filesystem::current_path();
}

struct CompiledShader
{
    // Components borrow their session. Keep it alive while RHI queries layouts
    // and retrieves code from the linked program after compileShader returns.
    ComPtr<slang::ISession> session;
    ComPtr<slang::IComponentType> linkedProgram;
    ComPtr<slang::IMetadata> metadata;
    slang::ICoverageTracingMetadata* coverageMetadata = nullptr;
    rhi::SyntheticResourceBindingDesc coverageResource = {};
};

// Bundles the demo's compile-time choices so call sites name each
// option at the call site instead of relying on positional `bool`s.
// Members:
//   - `enableCoverage` — toggles `-trace-coverage*` flags so the same
//     binary can produce a baseline (uncovered) timing measurement.
//   - `counterWidthBits` — forwarded to `-trace-coverage-counter-width`.
//     `64` is the default on any Vulkan driver that supports
//     `VK_KHR_shader_atomic_int64`; pass `32` on runtimes that do not
//     (most notably MoltenVK on Apple Silicon as of MoltenVK 1.4).
//   - `booleanMode` — selects between the two coverage recording modes:
//       false (Count, default): each counter is incremented with an
//       atomic add per hit; the slot holds an exact execution count.
//       true (Boolean, opted into via `-trace-coverage-boolean`):
//       each counter is written non-atomically with `1`; the slot is
//       `0` if the entry never executed, non-zero otherwise. This
//       removes all atomic contention (the dominant cost on hot
//       loops like the bilateral filter), at the price of losing
//       exact counts. The full sweep in this demo runs roughly
//       an order of magnitude faster in boolean mode; LCOV output
//       is identical because the converter treats any positive count
//       as "covered" either way.
struct CompileOptions
{
    bool enableCoverage = true;
    int counterWidthBits = 32;
    bool booleanMode = false;
};

CompiledShader compileShader(const CompileOptions& options)
{
    ComPtr<slang::IGlobalSession> globalSession;
    checkSlang(slang::createGlobalSession(globalSession.writeRef()), "createGlobalSession");

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    // Renamed from `options` to avoid shadowing the `const
    // CompileOptions& options` parameter.
    std::vector<slang::CompilerOptionEntry> optionEntries;
    auto pushBool = [&](slang::CompilerOptionName name)
    {
        slang::CompilerOptionEntry e = {};
        e.name = name;
        e.value.kind = slang::CompilerOptionValueKind::Int;
        e.value.intValue0 = 1;
        optionEntries.push_back(e);
    };
    pushBool(slang::CompilerOptionName::EmitSpirvDirectly);
    if (options.enableCoverage)
    {
        pushBool(slang::CompilerOptionName::TraceCoverage);
        pushBool(slang::CompilerOptionName::TraceFunctionCoverage);
        pushBool(slang::CompilerOptionName::TraceBranchCoverage);

        // Note: this demo deliberately does NOT pass
        // `TraceCoverageBinding`. It lets the compiler auto-assign a
        // descriptor slot for the `__slang_coverage` buffer, then reads
        // that slot back from `ISyntheticResourceMetadata` after
        // compilation (the metadata-derived binding path, below). The
        // BVH-traversal demo shows the alternative: dictate the slot here
        // with `TraceCoverageBinding`.

        // Forward the counter-width selection. The slangc CLI parser
        // converts `-trace-coverage-counter-width <bits>` to a byte
        // width (4 or 8); the API option matches that convention.
        slang::CompilerOptionEntry widthOpt = {};
        widthOpt.name = slang::CompilerOptionName::TraceCoverageCounterByteWidth;
        widthOpt.value.kind = slang::CompilerOptionValueKind::Int;
        widthOpt.value.intValue0 = options.counterWidthBits / 8;
        optionEntries.push_back(widthOpt);

        // Hit/miss mode opts the coverage pass into non-atomic stores
        // of `1` instead of atomic adds. Off (count mode) by default.
        if (options.booleanMode)
            pushBool(slang::CompilerOptionName::TraceCoverageBoolean);
    }

    const std::string searchPath = getDemoDirectory().string();
    const char* searchPaths[] = {searchPath.c_str()};

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 1;
    sessionDesc.compilerOptionEntries = optionEntries.data();
    sessionDesc.compilerOptionEntryCount = (uint32_t)optionEntries.size();

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

    CompiledShader out;
    out.session = session;
    out.linkedProgram = linked;

    if (options.enableCoverage)
    {
        // Metadata retrieval emits the shader and describes its generated resources.
        // Without coverage, RHI requests the code when it builds the program.
        diagnostics.setNull();
        checkSlang(
            linked->getEntryPointMetadata(0, 0, out.metadata.writeRef(), diagnostics.writeRef()),
            "getEntryPointMetadata");
        diagnoseIfNeeded(diagnostics);
        out.coverageMetadata = (slang::ICoverageTracingMetadata*)out.metadata->castAs(
            slang::ICoverageTracingMetadata::getTypeGuid());
        if (!out.coverageMetadata)
            fail("expected coverage metadata");
        checkSlang(
            coverageDemo::getCoverageResourceDesc(out.metadata, out.coverageResource),
            "getCoverageResourceDesc");
    }
    return out;
}

// Deterministic synthetic HDR test image: smooth gradient +
// hard-edged strips + an HDR spike.
std::vector<float> generateTestImage(uint32_t width, uint32_t height)
{
    std::vector<float> pixels(size_t(width) * height * 4);
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

std::vector<DispatchConfig> buildFullConfigs()
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
    if (!out)
        fail("cannot open for writing: " + path.string());
    out.write(
        static_cast<const char*>(manifestBlob->getBufferPointer()),
        (std::streamsize)manifestBlob->getBufferSize());
    if (!out)
        fail("write failed: " + path.string());
}

void writeCountersBinary(const std::vector<uint8_t>& rawBytes, const std::filesystem::path& path)
{
    // Writes the raw readback bytes verbatim; this function does not see
    // the slot width. The caller guarantees the layout by sizing
    // `rawBytes` as `counterCount * counterByteWidth`, so the resulting
    // file is N little-endian unsigned integers each of `counterByteWidth`
    // bytes (mirrored in `manifest.buffer.element_stride`). Downstream
    // tools (the LCOV converter, the HTML renderer, slang-rhi consumers)
    // read the manifest and this file as a pair.
    std::ofstream out(path, std::ios::binary);
    if (!out)
        fail("cannot open for writing: " + path.string());
    out.write(reinterpret_cast<const char*>(rawBytes.data()), (std::streamsize)rawBytes.size());
    if (!out)
        fail("write failed: " + path.string());
}

// Write a full LCOV file with line (DA), function (FN/FNDA), and branch
// (BRDA) records directly from the Slang coverage metadata API. This is the
// in-process conversion path: no external tool is required because
// ICoverageTracingMetadata::getEntryInfo() exposes the kind, source location,
// function name, and branch site/arm for every counter slot.
//
// Contrast the BVH-traversal demo, which writes a line-only LCOV and uses
// tools/shader-coverage/slang-coverage-to-lcov.py for the full conversion.
void writeLcov(
    slang::ICoverageTracingMetadata* coverage,
    const std::vector<uint64_t>& hits,
    const std::filesystem::path& path,
    const char* testName)
{
    // Collect entries grouped by source file, separated by kind.
    struct FuncRecord
    {
        uint32_t line;
        std::string name;
        uint64_t count;
    };
    struct BranchRecord
    {
        uint32_t line;
        uint32_t siteId; // identifies the branch point; groups its arms
        uint32_t armId;  // arm index within the site (0, 1, …)
        uint64_t count;
    };
    struct FileRecords
    {
        std::map<uint32_t, uint64_t> lines; // line → accumulated count
        std::vector<FuncRecord> funcs;
        std::vector<BranchRecord> branches;
    };
    std::map<std::string, FileRecords> byFile;

    const uint32_t entryCount = coverage->getEntryCount();
    for (uint32_t i = 0; i < entryCount; ++i)
    {
        slang::CoverageEntryInfo entry = {};
        if (SLANG_FAILED(coverage->getEntryInfo(i, &entry)))
            continue;
        if (!entry.file || !*entry.file || entry.line == 0)
            continue;
        // Each entry's counter slot is identified by counterIndex; use it to
        // look up the actual hit count. kInvalidCoverageCounterIndex marks
        // metadata-only entries that have no runtime counter. The range guard
        // (`< hits.size()`) is a defensive check against a hypothetical
        // compiler bug returning an out-of-bounds index.
        const uint64_t count = (entry.counterIndex != slang::kInvalidCoverageCounterIndex &&
                                entry.counterIndex < (uint32_t)hits.size())
                                   ? hits[entry.counterIndex]
                                   : 0;
        auto& rec = byFile[entry.file];
        switch (entry.kind)
        {
        case slang::CoverageEntryKind::Line:
            rec.lines[entry.line] += count;
            break;
        case slang::CoverageEntryKind::Function:
            rec.funcs.push_back({entry.line, entry.functionName ? entry.functionName : "", count});
            break;
        case slang::CoverageEntryKind::Branch:
            rec.branches.push_back({entry.line, entry.branchSiteID, entry.branchArmID, count});
            break;
        default:
            break;
        }
    }

    std::ofstream f(path, std::ios::binary);
    if (!f)
        fail("cannot open for writing: " + path.string());
    f << "TN:" << testName << "\n";
    for (const auto& fp : byFile)
    {
        const auto& rec = fp.second;
        f << "SF:" << fp.first << "\n";

        // Function records: declaration line then per-function hit count.
        for (const auto& fn : rec.funcs)
            f << "FN:" << fn.line << "," << fn.name << "\n";
        for (const auto& fn : rec.funcs)
            f << "FNDA:" << fn.count << "," << fn.name << "\n";
        if (!rec.funcs.empty())
        {
            uint32_t fnHit = 0;
            for (const auto& fn : rec.funcs)
                if (fn.count > 0)
                    ++fnHit;
            f << "FNF:" << rec.funcs.size() << "\n";
            f << "FNH:" << fnHit << "\n";
        }

        // Line records.
        uint32_t lHit = 0;
        for (const auto& lp : rec.lines)
        {
            f << "DA:" << lp.first << "," << lp.second << "\n";
            if (lp.second > 0)
                ++lHit;
        }
        f << "LF:" << rec.lines.size() << "\n";
        f << "LH:" << lHit << "\n";

        // Branch records: siteId groups arms of the same branch point.
        if (!rec.branches.empty())
        {
            uint32_t bHit = 0;
            for (const auto& br : rec.branches)
            {
                f << "BRDA:" << br.line << "," << br.siteId << "," << br.armId << "," << br.count
                  << "\n";
                if (br.count > 0)
                    ++bHit;
            }
            f << "BRF:" << rec.branches.size() << "\n";
            f << "BRH:" << bHit << "\n";
        }

        f << "end_of_record\n";
    }
    if (!f)
        fail("write failed: " + path.string());
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
    // Iterate entries ([0, getEntryCount())), not counters: several
    // entries can share one counterIndex once line coverage coalesces
    // markers, so a counter-indexed loop both under-visits (stops at
    // getCounterCount() < getEntryCount()) and misattributes hits (entry
    // i's counter is not generally counter i). Look each entry's counter
    // up by its own counterIndex instead, mirroring writeLcov() below.
    const uint32_t n = coverage->getEntryCount();
    for (uint32_t i = 0; i < n; ++i)
    {
        slang::CoverageEntryInfo entry = {};
        if (SLANG_FAILED(coverage->getEntryInfo(i, &entry)))
            continue;
        const bool covered = entry.counterIndex != slang::kInvalidCoverageCounterIndex &&
                             entry.counterIndex < (uint32_t)hits.size() &&
                             hits[entry.counterIndex] > 0;
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

int exampleMain(int argc, char** argv)
{
    // Wrap the demo body in try/catch so a Vulkan/Slang failure (no
    // device, allocation failure, shader-module rejection, ...) exits
    // with a diagnostic line instead of letting `std::terminate` fire.
    // RHI buffer allocation and standard-library operations can throw
    // `std::runtime_error`, so catching `std::exception` covers both.
    try
    {
        std::string mode = "smoke";
        // Keep the 48-configuration counting demo small; opt into benchmark-sized inputs.
        uint32_t imageWidth = 320;
        uint32_t imageHeight = 180;
        bool enableCoverage = true;
        // This demo intentionally defaults to 32-bit counters even though
        // the compiler default is 64-bit (see
        // `CompilerOptionName::TraceCoverageCounterByteWidth`): MoltenVK on
        // Apple Silicon does not expose `shaderBufferInt64Atomics`, and we
        // want the demo to run there out of the box. Pass
        // `--counter-width=64` on any Vulkan driver that does support
        // `VK_KHR_shader_atomic_int64` to exercise the wider counters and
        // match the compiler-side default.
        int counterWidthBits = 32;
        // `--coverage-mode=count` (default) records exact execution
        // counts via atomic add; `--coverage-mode=boolean` records
        // covered-or-not via non-atomic stores of `1`, which removes
        // all atomic contention. Bilateral filter dispatches in this
        // demo have thousands of weighted samples per pixel — atomic
        // counters become the dominant cost, and boolean mode runs ~15×
        // faster on the full sweep at the price of losing
        // exact counts. The LCOV report is identical either way (any
        // positive count is "covered").
        bool coverageBoolean = false;
        // `--tile-rows=N`: split each config dispatch into horizontal bands
        // of N rows to avoid OS watchdog resets (Windows TDR /
        // VK_ERROR_DEVICE_LOST) on coverage-instrumented runs. The
        // bilateral filter's inner loop creates heavy atomic contention on
        // a few counters, especially with large images. Full mode defaults to
        // 128-row tiles to limit work per submission. Smoke mode defaults
        // to whole-image dispatch. Pass an explicit value to
        // tune (smaller if you still observe TDR), or `--tile-rows=0` to
        // force whole-image dispatch in full mode. Batching does not limit
        // total process runtime; use the smaller default image for quick runs.
        constexpr uint32_t kTileRowsUnset = UINT32_MAX;
        constexpr uint32_t kFullModeTileRows = 128;
        uint32_t tileRows = kTileRowsUnset; // resolved per mode after parsing
        constexpr std::string_view kTileRowsFlag = "--tile-rows=";
        // `--output-dir=<path>`: explicit output location for the three
        // coverage artifacts (manifest JSON, LCOV, raw counter buffer).
        // Empty (default) means use `getDemoDirectory()` — source dir
        // when running from the tree, current working directory as the
        // robustness fallback. When set, the demo creates the directory
        // if needed.
        std::filesystem::path outputDir;
        // `--demo-dir=<path>`: explicit override for the directory
        // containing the demo's `.slang` assets. Empty (default) means
        // `getDemoDirectory()` discovers them itself (`__FILE__`
        // parent, falling back to CWD). When set, the demo trusts the
        // override unconditionally and a missing `pipeline.slang`
        // there surfaces as a `loadModule` error rather than silently
        // falling back to a different directory.
        std::filesystem::path demoDir;
        constexpr std::string_view kOutputDirFlag = "--output-dir=";
        constexpr std::string_view kDemoDirFlag = "--demo-dir=";
        for (int i = 1; i < argc; ++i)
        {
            std::string_view a = argv[i];
            if (a == "--mode=smoke")
                mode = "smoke";
            else if (a == "--mode=full")
                mode = "full";
            else if (a.substr(0, 8) == "--width=")
                imageWidth = parseUnsigned(a.substr(8), "--width");
            else if (a.substr(0, 9) == "--height=")
                imageHeight = parseUnsigned(a.substr(9), "--height");
            else if (a == "--no-coverage")
                enableCoverage = false;
            else if (a == "--coverage")
                enableCoverage = true;
            else if (a == "--counter-width=32")
                counterWidthBits = 32;
            else if (a == "--counter-width=64")
                counterWidthBits = 64;
            else if (a == "--coverage-mode=count")
                coverageBoolean = false;
            else if (a == "--coverage-mode=boolean")
                coverageBoolean = true;
            else if (a.substr(0, kTileRowsFlag.size()) == kTileRowsFlag)
            {
                tileRows = parseUnsigned(a.substr(kTileRowsFlag.size()), "--tile-rows");
                if (tileRows % 8 != 0)
                    fail("--tile-rows must be 0 or a multiple of 8 to avoid overlapping tiles");
            }
            else if (a.substr(0, kOutputDirFlag.size()) == kOutputDirFlag)
                outputDir = std::string(a.substr(kOutputDirFlag.size()));
            else if (a.substr(0, kDemoDirFlag.size()) == kDemoDirFlag)
                demoDir = std::string(a.substr(kDemoDirFlag.size()));
            else
            {
                std::cerr << "unknown arg: " << a << "\n";
                return 1;
            }
        }

        // Denoise uses signed pixel indices; keep width * height representable in int32_t.
        if (imageWidth == 0 || imageWidth > 65535 || imageHeight == 0 || imageHeight > 65535)
            fail("--width and --height must be in [1, 65535]");
        if (uint64_t(imageWidth) * imageHeight > INT32_MAX)
            fail("image pixel count must not exceed INT32_MAX");

        // Resolve the tile-rows default per mode (see the flag comment
        // above): full mode tiles by default, smoke mode does not. An
        // explicit `--tile-rows=` always wins, including `=0`.
        if (tileRows == kTileRowsUnset)
            tileRows = (mode == "full") ? kFullModeTileRows : 0;

        // Publish the `--demo-dir` override to file-scope storage so
        // `getDemoDirectory()` (called from `compileShader` and from
        // the output-dir resolution below) sees it without each call
        // site having to thread an override parameter.
        g_demoDirOverride = demoDir;

        // Build the parenthesized status in one place so the parentheses are
        // self-evidently balanced and a future edit to one branch can't
        // silently unbalance the line.
        const char* coverageModeLabel = coverageBoolean ? "boolean" : "count";
        const std::string coverageStatus =
            enableCoverage ? (" (coverage on, " + std::to_string(counterWidthBits) +
                              "-bit counters, " + coverageModeLabel + " mode)")
                           : " (no coverage)";
        std::cout << "compiling pipeline.slang" << coverageStatus << "\n";
        CompileOptions compileOpts;
        compileOpts.enableCoverage = enableCoverage;
        compileOpts.counterWidthBits = counterWidthBits;
        compileOpts.booleanMode = coverageBoolean;
        auto shader = compileShader(compileOpts);

        uint32_t counterCount = 0;
        uint32_t counterByteWidth = 4;
        if (enableCoverage)
        {
            counterCount = shader.coverageMetadata->getCounterCount();
            // Match the GPU-side buffer's per-slot byte width to whatever
            // the compiler synthesized. The default is uint64 (8); the
            // `-trace-coverage-counter-width 32` opt-down produces uint32
            // (4). The host must allocate and read back the matching
            // layout, so a missing or zero `elementByteWidth` here is a
            // hard error — silently falling back to 4 would mis-size the
            // buffer whenever the user requested 64-bit counters.
            slang::CoverageBufferInfo bufferInfo = {};
            if (SLANG_FAILED(shader.coverageMetadata->getBufferInfo(&bufferInfo)) ||
                bufferInfo.elementByteWidth == 0)
            {
                std::cerr << "coverage: getBufferInfo returned no element width; cannot size "
                             "the readback buffer to match the compiled counter layout\n";
                return 1;
            }
            counterByteWidth = bufferInfo.elementByteWidth;
            std::cout << "coverage counter count: " << counterCount << " ("
                      << (counterByteWidth * 8) << "-bit slots), __slang_coverage bound at set="
                      << shader.coverageResource.space
                      << " binding=" << shader.coverageResource.binding
                      << " (discovered from synthetic-resource metadata)\n";
        }

        coverageDemo::DiagnosticCallback diagnosticCallback;
        rhi::DeviceDesc deviceDesc = {};
        deviceDesc.debugCallback = &diagnosticCallback;
        deviceDesc.deviceType = rhi::DeviceType::Vulkan;
        const rhi::Feature atomicInt64 = rhi::Feature::AtomicInt64;
        if (enableCoverage && counterByteWidth == 8)
        {
            deviceDesc.requiredFeatures = &atomicInt64;
            deviceDesc.requiredFeatureCount = 1;
        }
        rhi::ComPtr<rhi::IDevice> device;
        checkSlang(rhi::getRHI()->createDevice(deviceDesc, device.writeRef()), "createDevice");

        // Register the hidden resource before program creation so RHI can extend
        // the pipeline layout. Coverage-disabled programs use the ordinary path.
        rhi::ShaderProgramSyntheticResourcesDesc syntheticDesc = {};
        syntheticDesc.resources = &shader.coverageResource;
        syntheticDesc.resourceCount = 1;
        rhi::ShaderProgramDesc programDesc = {};
        programDesc.slangGlobalScope = shader.linkedProgram;
        if (enableCoverage)
            programDesc.next = &syntheticDesc;
        rhi::ComPtr<rhi::IShaderProgram> program;
        ComPtr<slang::IBlob> diagnostics;
        const auto programResult =
            device->createShaderProgram(programDesc, program.writeRef(), diagnostics.writeRef());
        diagnoseIfNeeded(diagnostics);
        checkSlang(programResult, "createShaderProgram");

        rhi::ComputePipelineDesc pipelineDesc = {};
        pipelineDesc.program = program;
        rhi::ComPtr<rhi::IComputePipeline> pipeline;
        checkSlang(
            device->createComputePipeline(pipelineDesc, pipeline.writeRef()),
            "createComputePipeline");
        rhi::ComPtr<rhi::IShaderObject> root;
        checkSlang(
            device->createRootShaderObject(program, root.writeRef()),
            "createRootShaderObject");
        rhi::ComPtr<rhi::ICommandQueue> queue;
        checkSlang(device->getQueue(rhi::QueueType::Graphics, queue.writeRef()), "getQueue");

        std::cout << "image: " << imageWidth << "x" << imageHeight << "\n";
        const auto image = generateTestImage(imageWidth, imageHeight);
        auto inputBuf = createStorageBuffer(
            device,
            "inputImage",
            image.size() * sizeof(float),
            4 * sizeof(float),
            image.data());
        auto outputBuf =
            createStorageBuffer(device, "output", image.size() * sizeof(float), 4 * sizeof(float));
        auto paramsBuf = createStorageBuffer(
            device,
            "paramsBuffer",
            sizeof(PipelineParams),
            sizeof(PipelineParams));
        rhi::ShaderCursor cursor(root);
        checkSlang(cursor["inputImage"].setBinding(inputBuf), "bind inputImage");
        checkSlang(cursor["outputImage"].setBinding(outputBuf), "bind outputImage");
        checkSlang(cursor["paramsBuffer"].setBinding(paramsBuf), "bind paramsBuffer");

        // Zero once, then accumulate across all dispatches. The metadata ID is
        // opaque; it is not the descriptor binding number or a reflected name.
        rhi::ComPtr<rhi::IBuffer> coverageBuf;
        if (enableCoverage)
        {
            std::vector<uint8_t> zero(size_t(counterCount) * counterByteWidth, 0);
            coverageBuf =
                createStorageBuffer(device, "coverage", zero.size(), counterByteWidth, zero.data());
            checkSlang(
                rhi::bindSyntheticResource(
                    program,
                    root,
                    shader.coverageResource.id,
                    rhi::Binding(coverageBuf)),
                "bindSyntheticResource");
        }

        auto configs = (mode == "smoke") ? buildSmokeConfigs() : buildFullConfigs();

        // `--tile-rows=N` splits each config into horizontal bands of N
        // rows; the shader recovers the real pixel row as
        // `tid.y + tileOriginY`. Tiling caps the per-submission GPU time,
        // which prevents OS watchdog resets (Windows TDR /
        // VK_ERROR_DEVICE_LOST) under coverage count mode on this demo's hot
        // bilateral filter. Full mode defaults to 128-row tiles; smoke mode
        // and `--tile-rows=0` dispatch the whole image per config.
        // --coverage-mode=boolean also eliminates TDR risk without tiling.
        //
        // Neither shape affects coverage results: tiles partition the image,
        // every pixel is processed exactly once, and counters accumulate
        // across all submissions.
        const uint32_t effectiveTileRows =
            (tileRows == 0) ? imageHeight : std::min(tileRows, imageHeight);
        const uint32_t groupsX = (imageWidth + 7) / 8;
        uint32_t dispatchCount = 0;

        std::cout << "running " << configs.size() << " config(s) in mode=" << mode
                  << " (coverage=" << (enableCoverage ? "on" : "off") << ", dispatch=";
        if (tileRows > 0)
            std::cout << tileRows << "-row tiles";
        else
            std::cout << "whole-image";
        std::cout << ")\n";

        const auto renderStart = std::chrono::steady_clock::now();
        for (const auto& cfg : configs)
        {
            PipelineParams p = {};
            p.imageWidth = imageWidth;
            p.imageHeight = imageHeight;
            p.tonemapOp = cfg.tonemapOp;
            p.boundaryMode = cfg.boundaryMode;
            p.gammaMode = cfg.gammaMode;
            p.bilateralSigmaS = cfg.sigmaS;
            p.bilateralSigmaR = cfg.sigmaR;
            p.exposure = cfg.exposure;
            for (uint32_t y0 = 0; y0 < imageHeight; y0 += effectiveTileRows)
            {
                p.tileOriginY = y0;
                rhi::ComPtr<rhi::ICommandEncoder> encoder;
                checkSlang(queue->createCommandEncoder(encoder.writeRef()), "createCommandEncoder");
                checkSlang(
                    encoder->uploadBufferData(paramsBuf, 0, sizeof(p), &p),
                    "upload parameters");
                const uint32_t bandRows =
                    (imageHeight - y0 < effectiveTileRows) ? (imageHeight - y0) : effectiveTileRows;
                const uint32_t groupsY = (bandRows + 7) / 8;
                auto pass = encoder->beginComputePass();
                pass->bindPipeline(pipeline, root);
                pass->dispatchCompute(groupsX, groupsY, 1);
                pass->end();
                rhi::ComPtr<rhi::ICommandBuffer> commands;
                checkSlang(encoder->finish(commands.writeRef()), "finish commands");
                checkSlang(queue->submit(commands), "submit");
                checkSlang(queue->waitOnHost(), "waitOnHost");
                ++dispatchCount;
            }
        }
        const auto renderEnd = std::chrono::steady_clock::now();
        const double renderMs =
            std::chrono::duration<double, std::milli>(renderEnd - renderStart).count();
        std::cout << "render wall time: " << renderMs << " ms (" << configs.size() << " config(s), "
                  << dispatchCount << " dispatches, " << (renderMs / configs.size())
                  << " ms/config)\n";

        if (!enableCoverage)
        {
            std::cout << "no-coverage baseline complete\n";
            return 0;
        }

        // Read the GPU buffer back as raw bytes, then widen each slot to
        // a uint64_t so the summary/LCOV writers don't have to care about
        // the on-GPU width. The host file (`<mode>.counters.bin`) is
        // written out at the original byte width so downstream tools that
        // consume the manifest's `element_stride` see consistent layout.
        std::vector<uint8_t> rawBytes((size_t)counterCount * counterByteWidth);
        ComPtr<slang::IBlob> readback;
        checkSlang(
            device->readBuffer(coverageBuf, 0, rawBytes.size(), readback.writeRef()),
            "read coverage buffer");
        std::memcpy(rawBytes.data(), readback->getBufferPointer(), rawBytes.size());
        auto hits = decodeCoverageCounters(rawBytes.data(), rawBytes.size(), counterByteWidth);

        auto summary = summarize(shader.coverageMetadata, hits);
        printSummary(mode.c_str(), summary);

        // Resolve the output directory. When `--output-dir` is unset,
        // fall back to the demo directory (source tree during
        // development, current working directory under the robustness
        // fallback). When set, `create_directories` makes the path on
        // demand — `fail()` on error so a typo doesn't silently land
        // artifacts somewhere unexpected.
        std::filesystem::path outDir = outputDir.empty() ? getDemoDirectory() : outputDir;
        if (!outputDir.empty())
        {
            std::error_code ec;
            std::filesystem::create_directories(outDir, ec);
            if (ec)
                fail("could not create output directory " + outDir.string() + ": " + ec.message());
        }
        writeManifest(shader.coverageMetadata, outDir / (mode + ".coverage-manifest.json"));
        writeLcov(
            shader.coverageMetadata,
            hits,
            outDir / (mode + ".lcov"),
            ("image-pipeline-" + mode).c_str());
        writeCountersBinary(rawBytes, outDir / (mode + ".counters.bin"));
        std::cout << "wrote " << (outDir / (mode + ".coverage-manifest.json")) << "\n";
        std::cout << "wrote " << (outDir / (mode + ".lcov")) << "\n";
        std::cout << "wrote " << (outDir / (mode + ".counters.bin")) << "\n";

        std::cout << "done\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
}
