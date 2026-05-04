// shader-coverage-inprocess-demo
//
// Demonstrates the in-process pattern for Slang shader coverage
// (`-trace-coverage`) using only the public C++ API. No slang-rhi,
// no graphics-API dispatch, no auxiliary host-helper library — just
// `slang.h` and a small inline LCOV writer.
//
// Pipeline shape:
//
//   1. Compile a multi-module Slang program with `-trace-coverage`.
//      The IR coverage pass instruments every executable statement,
//      synthesizes a shared `__slang_coverage` buffer, and populates
//      `slang::ICoverageTracingMetadata` on the artifact.
//
//   2. Query the metadata via the typed accessors:
//      `getCounterCount()`, `getEntryInfo(slot, ...)`, `getBufferInfo(...)`.
//      That's the canonical in-process surface — no JSON parsing, no
//      file I/O, no schema versioning.
//
//   3. Serialize the manifest in-memory via the public helper
//      `slang_writeCoverageManifestJson(metadata, &blob)` for hosts
//      that want the canonical sidecar bytes (e.g. to feed Python /
//      external tools without round-tripping through disk).
//
//   4. Synthesize counter values for the report. **In a production
//      integration, replace this step with a real GPU readback.** The
//      slot-→-source attribution is the same; only the source of the
//      counter values differs.
//
//   5. Emit LCOV. The writer below (~30 LOC) covers line coverage,
//      which is the format scope of the current coverage feature.
//      Output is byte-equivalent to slang-coverage-rt's
//      `slang_coverage_save_lcov` for the same inputs.
//
//   6. Render to HTML via `tools/coverage-html/slang-coverage-html.py`
//      (the Slang-native renderer) or `genhtml`. Both consume standard
//      LCOV `.info`.
//
// What this demo intentionally omits:
//
//   - **GPU dispatch.** Customers running real workloads bring their
//     own RHI (raw Vulkan, raw D3D12, slang-rhi, ...) — the binding
//     mechanics differ per backend, but the metadata-API and
//     attribution code are identical to what's shown here. Replace
//     the synthetic counter loop in step 4 with your readback.
//   - **The slang-coverage-rt helper library.** Tier-1 in-process
//     customers don't need it: the typed metadata API + ~30 LOC of
//     LCOV emission is the entire integration. The rt library is a
//     convenience for hosts that want richer reporting features
//     without writing them.

#include "slang-com-ptr.h"
#include "slang.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <string>
#include <vector>

using Slang::ComPtr;

// ----------------------------------------------------------------
// LCOV writer — line coverage only. ~30 LOC.
//
// Aggregates hits by `(file, line)` so multiple counter slots on the
// same source line sum together. Filters out slots without a real
// source file or with a non-positive line number, matching gcov/LCOV
// reporting semantics (and slang-coverage-to-lcov.py + rt's behavior).
// ----------------------------------------------------------------

static int writeLcovReport(
    slang::ICoverageTracingMetadata* coverage,
    const uint64_t* hits,
    uint32_t hitsLen,
    const char* outputPath,
    const char* testName)
{
    // hitsLen must equal counter count.
    uint32_t n = coverage->getCounterCount();
    if (hitsLen != n)
    {
        std::fprintf(
            stderr,
            "writeLcovReport: hits length %u != counter count %u\n",
            hitsLen,
            n);
        return -1;
    }

    // Aggregate. std::map's natural ordering produces sorted-file
    // / sorted-line output, which matches what slang-coverage-html
    // expects for stable diff-friendly LCOV.
    std::map<std::string, std::map<uint32_t, uint64_t>> byFile;
    for (uint32_t i = 0; i < n; ++i)
    {
        slang::CoverageEntryInfo entry;
        if (SLANG_FAILED(coverage->getEntryInfo(i, &entry)))
            continue;
        if (!entry.file || !*entry.file || entry.line == 0)
            continue;
        byFile[entry.file][entry.line] += hits[i];
    }

    std::ofstream f(outputPath, std::ios::binary);
    if (!f)
    {
        std::fprintf(stderr, "writeLcovReport: failed to open '%s'\n", outputPath);
        return -1;
    }
    f << "TN:" << testName << "\n";
    for (const auto& filePair : byFile)
    {
        f << "SF:" << filePair.first << "\n";
        for (const auto& lineHits : filePair.second)
            f << "DA:" << lineHits.first << "," << lineHits.second << "\n";
        f << "end_of_record\n";
    }
    return 0;
}

// ----------------------------------------------------------------
// Demo entry point.
// ----------------------------------------------------------------

int main(int /*argc*/, char** /*argv*/)
{
    // ---- 1. Create a global session and an HLSL target ----------
    //
    // We pick HLSL for compilation purely so the IR coverage pass
    // runs end-to-end (the pass is target-independent on the IR
    // side, but a target is needed to drive codegen). The emitted
    // HLSL itself is unused — we only consume the metadata.
    ComPtr<slang::IGlobalSession> globalSession;
    if (SLANG_FAILED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())))
    {
        std::fprintf(stderr, "failed to create global session\n");
        return 1;
    }

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    // Turn `-trace-coverage` on at the session level. Same as
    // passing `-trace-coverage` to slangc.
    slang::CompilerOptionEntry covOption = {};
    covOption.name = slang::CompilerOptionName::TraceCoverage;
    covOption.value.kind = slang::CompilerOptionValueKind::Int;
    covOption.value.intValue0 = 1;

    // Search path resolves `import physics;` and `import math;`
    // back to physics.slang / math.slang in this directory.
    const char* searchPaths[] = {".", "examples/shader-coverage-inprocess-demo"};

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = &covOption;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 2;

    ComPtr<slang::ISession> session;
    if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef())))
    {
        std::fprintf(stderr, "failed to create session\n");
        return 1;
    }

    // ---- 2. Load the entry-point module + link -----------------
    //
    // Loading "app" pulls in physics and math via the import chain.
    // Each module is instrumented; the IR coverage pass dedupes
    // the synthesized __slang_coverage buffer across them.
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModule("app", diagnostics.writeRef());
    if (diagnostics)
        std::fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());
    if (!module)
        return 1;

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("simulate", entryPoint.writeRef());
    if (!entryPoint)
    {
        std::fprintf(stderr, "entry point 'simulate' not found\n");
        return 1;
    }

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    session->createCompositeComponentType(components, 2, program.writeRef(), nullptr);

    ComPtr<slang::IComponentType> linked;
    if (SLANG_FAILED(program->link(linked.writeRef(), diagnostics.writeRef())))
    {
        if (diagnostics)
            std::fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());
        return 1;
    }

    // Force codegen so the IR coverage pass runs and metadata is
    // populated. We discard the emitted code blob.
    ComPtr<slang::IBlob> codeBlob;
    if (SLANG_FAILED(linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef())))
    {
        if (diagnostics)
            std::fprintf(stderr, "%s", (const char*)diagnostics->getBufferPointer());
        return 1;
    }

    // ---- 3. Query coverage metadata ---------------------------
    //
    // ICoverageTracingMetadata is the canonical in-process surface.
    // No JSON, no parsing, no schema versioning — just typed C++
    // accessors.
    ComPtr<slang::IMetadata> metadata;
    if (SLANG_FAILED(
            linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef())))
    {
        std::fprintf(stderr, "failed to get entry-point metadata\n");
        return 1;
    }

    auto* coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    if (!coverage)
    {
        std::fprintf(stderr, "no coverage tracing metadata on artifact\n");
        return 1;
    }

    uint32_t counterCount = coverage->getCounterCount();
    std::printf(
        "[in-process] %u counter slots instrumented across app.slang + "
        "physics.slang + math.slang\n",
        counterCount);

    // Buffer info: for HLSL targets the synthesized buffer lands at
    // a real (space, register) pair. Tier-1 customers declare their
    // own pipeline-layout entry at this slot.
    slang::CoverageBufferInfo bufferInfo;
    coverage->getBufferInfo(&bufferInfo);
    std::printf(
        "[in-process] coverage buffer __slang_coverage at space=%d binding=%d "
        "(declare this slot in your pipeline-layout / root-signature)\n",
        bufferInfo.space,
        bufferInfo.binding);

    // Print first few entries' source attribution.
    std::printf("[in-process] sample slot mapping (first 5 of %u):\n", counterCount);
    for (uint32_t i = 0; i < counterCount && i < 5; ++i)
    {
        slang::CoverageEntryInfo entry;
        if (SLANG_FAILED(coverage->getEntryInfo(i, &entry)))
            continue;
        std::printf("              slot %2u → %s:%u\n", i, entry.file, entry.line);
    }

    // ---- 4. Serialize the manifest via the public helper ------
    //
    // `slang_writeCoverageManifestJson` produces the same bytes that
    // slangc writes as a sidecar. Useful for cross-process workflows
    // and external tools (Python LCOV converter, custom analyzers).
    {
        ComPtr<ISlangBlob> manifest;
        if (SLANG_FAILED(slang_writeCoverageManifestJson(coverage, manifest.writeRef())))
        {
            std::fprintf(stderr, "slang_writeCoverageManifestJson failed\n");
            return 1;
        }
        std::ofstream out("coverage-mapping.json", std::ios::binary);
        out.write((const char*)manifest->getBufferPointer(), manifest->getBufferSize());
        std::printf(
            "[in-process] wrote coverage-mapping.json (%u bytes)\n",
            (unsigned)manifest->getBufferSize());
    }

    // ---- 5. SYNTHETIC counter values --------------------------
    //
    // *** Replace this block with your real GPU readback. ***
    //
    // In a production integration:
    //   - Allocate a uint32_t[counterCount] buffer.
    //   - Bind it at the (space, binding) reported above.
    //   - Dispatch the shader.
    //   - Read the buffer back to host memory.
    //   - Pass that into writeLcovReport instead of the synthetic
    //     pattern below.
    //
    // The pattern below produces a deterministic mix of zero and
    // non-zero hits so the rendered HTML report shows visually-
    // distinct hit / miss attribution. Slots whose index is divisible
    // by 7 are marked "unreached" (hit count 0); others get a
    // pseudo-random count derived from the slot index.
    std::vector<uint64_t> hits(counterCount);
    for (uint32_t i = 0; i < counterCount; ++i)
        hits[i] = (i % 7 == 0) ? 0u : (50u + (i * 37u) % 350u);

    // ---- 6. Emit LCOV ----------------------------------------
    if (writeLcovReport(coverage, hits.data(), counterCount, "coverage.lcov", "in_process") != 0)
        return 1;
    std::printf("[in-process] wrote coverage.lcov\n");

    // ---- 7. Tell the user how to render HTML ------------------
    std::printf(
        "  next: python tools/coverage-html/slang-coverage-html.py "
        "coverage.lcov --output-dir coverage-html/\n");

    return 0;
}
