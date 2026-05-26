// unit-test-coverage-tracing-metadata.cpp

#include "../../source/compiler-core/slang-json-lexer.h"
#include "../../source/compiler-core/slang-json-parser.h"
#include "../../source/compiler-core/slang-json-value.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>

using namespace Slang;

namespace
{

struct ParsedJson
{
    SourceManager sourceManager;
    DiagnosticSink sink;
    RefPtr<JSONContainer> container;
    JSONValue root;

    ParsedJson()
        : sink(&sourceManager, nullptr)
    {
        sourceManager.initialize(nullptr, nullptr);
        container = new JSONContainer(&sourceManager);
    }
};

static SlangResult parseJsonBlob(ISlangBlob* blob, ParsedJson& outJson)
{
    String contents(
        UnownedStringSlice((const char*)blob->getBufferPointer(), blob->getBufferSize()));
    SourceFile* sourceFile =
        outJson.sourceManager.createSourceFileWithString(PathInfo::makeUnknown(), contents);
    SourceView* sourceView =
        outJson.sourceManager.createSourceView(sourceFile, nullptr, SourceLoc());

    JSONLexer lexer;
    lexer.init(sourceView, &outJson.sink);

    JSONBuilder builder(outJson.container, JSONBuilder::Flag::ConvertLexemes);
    JSONParser parser;
    SLANG_RETURN_ON_FAIL(parser.parse(&lexer, sourceView, &builder, &outJson.sink));
    outJson.root = builder.getRootValue();
    return SLANG_OK;
}

static JSONValue findJsonField(JSONContainer* container, const JSONValue& object, const char* key)
{
    JSONKey jsonKey = container->findKey(UnownedStringSlice(key));
    if (jsonKey == 0)
        return JSONValue::makeInvalid();
    return container->findObjectValue(object, jsonKey);
}

static void checkJsonStringField(
    JSONContainer* container,
    const JSONValue& object,
    const char* key,
    const char* expected)
{
    JSONValue value = findJsonField(container, object, key);
    SLANG_CHECK(value.isValid());
    SLANG_CHECK(container->getString(value) == UnownedStringSlice(expected));
}

static void checkJsonIntField(
    JSONContainer* container,
    const JSONValue& object,
    const char* key,
    int64_t expected)
{
    JSONValue value = findJsonField(container, object, key);
    SLANG_CHECK(value.isValid());
    SLANG_CHECK(container->asInteger(value) == expected);
}

} // namespace

// Exercises the public `slang::ICoverageTracingMetadata` COM interface
// end-to-end:
//
//   - compile a small compute shader with `-trace-coverage` via the
//     standard compile API (no slangc, no file I/O)
//   - link the program and force codegen so the coverage pass runs
//   - query the per-entry-point metadata
//   - cast to `ICoverageTracingMetadata` via the public `castAs` +
//     `getTypeGuid()` idiom
//   - invoke every virtual method in the vtable and check basic
//     invariants: counter count is non-zero for an instrumented
//     shader, per-entry `(file, line)` strings/values are populated,
//     and hidden buffer binding is available through synthetic
//     resource metadata
//
// This is an ABI-level smoke test: if a future change reorders or
// changes the signature of any virtual method in
// `ICoverageTracingMetadata`, this test will catch it at CI time
// rather than at customer-runtime time.

SLANG_UNIT_TEST(coverageTracingMetadata)
{
    const char* shaderSource = R"(
        RWStructuredBuffer<uint> outputBuffer;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            uint accum = 0;
            for (uint i = 0; i < 4; ++i)
            {
                if ((i & 1u) == 0u)
                    accum += i;
                else
                    accum += i * 2u;
            }
            switch (int(tid.x) & 1)
            {
            case 0:
                accum += 10u;
                break;
            default:
                accum += 20u;
                break;
            }
            switch (int(tid.x) & 3)
            {
            case 2:
                accum += 30u;
                break;
            }
            outputBuffer[0] = accum;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    struct MetadataBundle
    {
        ComPtr<slang::IMetadata> metadata;
        slang::ICoverageTracingMetadata* coverage = nullptr;
        slang::ISyntheticResourceMetadata* syntheticResources = nullptr;
    };

    auto createMetadataBundle = [&](SlangCompileTarget format,
                                    const char* profileName,
                                    bool lineCoverageEnabled = true,
                                    bool functionCoverageEnabled = false,
                                    bool branchCoverageEnabled = false)
    {
        MetadataBundle bundle;

        slang::TargetDesc targetDesc = {};
        targetDesc.format = format;
        targetDesc.profile = globalSession->findProfile(profileName);

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;

        slang::CompilerOptionEntry coverageOptions[3] = {};
        uint32_t coverageOptionCount = 0;
        auto addBoolOption = [&](slang::CompilerOptionName name)
        {
            auto& option = coverageOptions[coverageOptionCount++];
            option.name = name;
            option.value.kind = slang::CompilerOptionValueKind::Int;
            option.value.intValue0 = 1;
        };
        if (lineCoverageEnabled)
            addBoolOption(slang::CompilerOptionName::TraceCoverage);
        if (functionCoverageEnabled)
            addBoolOption(slang::CompilerOptionName::TraceFunctionCoverage);
        if (branchCoverageEnabled)
            addBoolOption(slang::CompilerOptionName::TraceBranchCoverage);
        sessionDesc.compilerOptionEntries = coverageOptions;
        sessionDesc.compilerOptionEntryCount = coverageOptionCount;

        ComPtr<slang::ISession> session;
        SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnostics;
        auto module = session->loadModuleFromSourceString(
            "coverageTest",
            "coverageTest.slang",
            shaderSource,
            diagnostics.writeRef());
        SLANG_CHECK(module != nullptr);

        ComPtr<slang::IEntryPoint> entryPoint;
        module->findEntryPointByName("computeMain", entryPoint.writeRef());
        SLANG_CHECK(entryPoint != nullptr);

        slang::IComponentType* components[] = {module, entryPoint};
        ComPtr<slang::IComponentType> program;
        SLANG_CHECK(
            session->createCompositeComponentType(components, 2, program.writeRef(), nullptr) ==
            SLANG_OK);

        ComPtr<slang::IComponentType> linked;
        SLANG_CHECK(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> codeBlob;
        SLANG_CHECK(
            linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef()) ==
            SLANG_OK);

        SLANG_CHECK(
            linked
                ->getEntryPointMetadata(0, 0, bundle.metadata.writeRef(), diagnostics.writeRef()) ==
            SLANG_OK);

        bundle.coverage = (slang::ICoverageTracingMetadata*)bundle.metadata->castAs(
            slang::ICoverageTracingMetadata::getTypeGuid());
        SLANG_CHECK(bundle.coverage != nullptr);
        bundle.syntheticResources = (slang::ISyntheticResourceMetadata*)bundle.metadata->castAs(
            slang::ISyntheticResourceMetadata::getTypeGuid());
        SLANG_CHECK(bundle.syntheticResources != nullptr);

        return bundle;
    };

    auto checkLineCoverageEntries = [](slang::ICoverageTracingMetadata* coverage)
    {
        // The current producer emits line entries with concrete runtime
        // counters. Keep the checks source-entry based so future modes can
        // diverge from positional entry-to-counter identity without breaking
        // this ABI smoke test.
        const uint32_t counterCount = coverage->getCounterCount();
        SLANG_CHECK(counterCount > 0);
        const uint32_t entryCount = coverage->getEntryCount();
        SLANG_CHECK(entryCount > 0);

        bool seenAnyStartColumn = false;
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            slang::CoverageEntryInfo entry;
            SLANG_CHECK(coverage->getEntryInfo(i, &entry) == SLANG_OK);
            SLANG_CHECK(entry.file != nullptr);
            SLANG_CHECK(entry.line > 0);
            SLANG_CHECK(entry.counterIndex != slang::kInvalidCoverageCounterIndex);
            SLANG_CHECK(entry.counterIndex < counterCount);
            SLANG_CHECK(entry.kind == slang::CoverageEntryKind::Line);
            SLANG_CHECK(entry.counterMode == slang::CoverageCounterMode::Count);
            seenAnyStartColumn = seenAnyStartColumn || entry.startColumn > 0;
            SLANG_CHECK(entry.endLine == 0);
            SLANG_CHECK(entry.endColumn == 0);
            SLANG_CHECK(entry.functionName == nullptr);
            SLANG_CHECK(entry.functionMangledName == nullptr);
            SLANG_CHECK(entry.branchSiteID == 0);
            SLANG_CHECK(entry.branchArmID == 0);
            SLANG_CHECK(entry.branchArmKind == slang::CoverageBranchArmKind::Unknown);
        }
        SLANG_CHECK(seenAnyStartColumn);
    };

    auto cpuBundle = createMetadataBundle(SLANG_CPP_SOURCE, "sm_5_0");
    auto* coverage = cpuBundle.coverage;
    auto* syntheticResources = cpuBundle.syntheticResources;

    // The shader has multiple instrumented statements (for loop, if,
    // else, assignments, writeback) — expect at least several slots.
    uint32_t counterCount = coverage->getCounterCount();
    SLANG_CHECK(counterCount > 0);
    uint32_t entryCount = coverage->getEntryCount();
    SLANG_CHECK(entryCount > 0);
    checkLineCoverageEntries(coverage);

    // Out-of-range query must return SLANG_E_INVALID_ARG.
    {
        slang::CoverageEntryInfo entry;
        SLANG_CHECK(coverage->getEntryInfo(entryCount, &entry) == SLANG_E_INVALID_ARG);
    }

    // Null pointer must return SLANG_E_INVALID_ARG, not crash.
    SLANG_CHECK(coverage->getEntryInfo(0, nullptr) == SLANG_E_INVALID_ARG);

    // Wrong structSize must return SLANG_E_INVALID_ARG.
    {
        slang::CoverageEntryInfo entry;
        entry.structSize = 0;
        SLANG_CHECK(coverage->getEntryInfo(0, &entry) == SLANG_E_INVALID_ARG);
    }

    // A caller compiled against the original v1 struct only has
    // storage through `line`. The implementation must accept that
    // structSize and must not write tail fields.
    {
        auto counterModeSentinel = static_cast<slang::CoverageCounterMode>(0x12345678u);
        slang::CoverageEntryInfo entry;
        entry.structSize = SLANG_OFFSET_OF(slang::CoverageEntryInfo, line) + sizeof(uint32_t);
        entry.counterIndex = 0x12345678u;
        entry.kind = slang::CoverageEntryKind::Function;
        entry.counterMode = counterModeSentinel;
        entry.startColumn = 77;
        entry.endLine = 88;
        entry.endColumn = 99;
        entry.functionName = "unchanged";
        entry.functionMangledName = "unchanged_mangled";
        entry.branchSiteID = 111;
        entry.branchArmID = 222;
        entry.branchArmKind = slang::CoverageBranchArmKind::TrueArm;
        SLANG_CHECK(coverage->getEntryInfo(0, &entry) == SLANG_OK);
        SLANG_CHECK(entry.file != nullptr);
        SLANG_CHECK(entry.line > 0);
        SLANG_CHECK(entry.counterIndex == 0x12345678u);
        SLANG_CHECK(entry.kind == slang::CoverageEntryKind::Function);
        SLANG_CHECK(entry.counterMode == counterModeSentinel);
        SLANG_CHECK(entry.startColumn == 77);
        SLANG_CHECK(entry.endLine == 88);
        SLANG_CHECK(entry.endColumn == 99);
        SLANG_CHECK(UnownedStringSlice(entry.functionName) == toSlice("unchanged"));
        SLANG_CHECK(UnownedStringSlice(entry.functionMangledName) == toSlice("unchanged_mangled"));
        SLANG_CHECK(entry.branchSiteID == 111);
        SLANG_CHECK(entry.branchArmID == 222);
        SLANG_CHECK(entry.branchArmKind == slang::CoverageBranchArmKind::TrueArm);
    }

    // Synthetic resource metadata: coverage should surface one hidden
    // mutable structured buffer resource that hosts can use as a
    // binding helper bridge.
    {
        const uint32_t resourceCount = syntheticResources->getResourceCount();
        SLANG_CHECK(resourceCount == 1);

        const uint32_t coverageResourceIndex = 0;
        slang::SyntheticResourceInfo info;
        SLANG_CHECK(syntheticResources->getResourceInfo(coverageResourceIndex, &info) == SLANG_OK);
        SLANG_CHECK(info.id != 0);
        SLANG_CHECK(info.bindingType == slang::BindingType::MutableRawBuffer);
        SLANG_CHECK(info.arraySize == 1);
        SLANG_CHECK(info.scope == slang::SyntheticResourceScope::Global);
        SLANG_CHECK(info.access == slang::SyntheticResourceAccess::ReadWrite);
        SLANG_CHECK(info.entryPointIndex == -1);
        SLANG_CHECK(info.debugName != nullptr);
        SLANG_CHECK(UnownedStringSlice(info.debugName) == toSlice("__slang_coverage"));

        // CPU/CUDA-style targets must expose the concrete
        // marshaling location in the generated global params
        // payload. Descriptor-facing binding information may
        // also be available, but it is not the primary
        // contract for these targets.
        SLANG_CHECK(info.space >= -1);
        SLANG_CHECK(info.binding >= -1);
        SLANG_CHECK(info.uniformOffset >= 0);
        SLANG_CHECK(info.uniformStride > 0);

        slang::CoverageBufferInfo bufferInfo;
        SLANG_CHECK(coverage->getBufferInfo(&bufferInfo) == SLANG_OK);
        SLANG_CHECK(bufferInfo.space == info.space);
        SLANG_CHECK(bufferInfo.binding == info.binding);

        uint32_t lookedUpIndex = ~0u;
        SLANG_CHECK(syntheticResources->findResourceIndexByID(info.id, &lookedUpIndex) == SLANG_OK);
        SLANG_CHECK(lookedUpIndex == coverageResourceIndex);
    }

    // CUDA follows the same synthetic-resource marshaling contract as
    // CPU: the coverage buffer is discoverable through a concrete
    // uniform payload offset/stride rather than through descriptor
    // metadata.
    {
        auto cudaBundle = createMetadataBundle(SLANG_CUDA_SOURCE, "sm_5_0");
        checkLineCoverageEntries(cudaBundle.coverage);
        auto* cudaSyntheticResources = cudaBundle.syntheticResources;

        SLANG_CHECK(cudaSyntheticResources->getResourceCount() == 1);

        slang::SyntheticResourceInfo info;
        SLANG_CHECK(cudaSyntheticResources->getResourceInfo(0, &info) == SLANG_OK);
        SLANG_CHECK(info.id != 0);
        SLANG_CHECK(info.bindingType == slang::BindingType::MutableRawBuffer);
        SLANG_CHECK(info.uniformOffset >= 0);
        SLANG_CHECK(info.uniformStride > 0);
    }

    // Descriptor-backed targets should expose descriptor-facing binding
    // information while leaving CPU/CUDA uniform marshaling fields at their
    // documented sentinel values.
    {
        auto spirvBundle = createMetadataBundle(SLANG_SPIRV, "spirv_1_5");
        checkLineCoverageEntries(spirvBundle.coverage);
        auto* spirvSyntheticResources = spirvBundle.syntheticResources;

        const uint32_t resourceCount = spirvSyntheticResources->getResourceCount();
        SLANG_CHECK(resourceCount == 1);

        const uint32_t coverageResourceIndex = 0;
        slang::SyntheticResourceInfo info;
        SLANG_CHECK(
            spirvSyntheticResources->getResourceInfo(coverageResourceIndex, &info) == SLANG_OK);
        SLANG_CHECK(info.binding >= 0);
        SLANG_CHECK(info.space >= 0);
        SLANG_CHECK(info.bindingType == slang::BindingType::MutableRawBuffer);
        SLANG_CHECK(info.arraySize == 1);
        SLANG_CHECK(info.scope == slang::SyntheticResourceScope::Global);
        SLANG_CHECK(info.access == slang::SyntheticResourceAccess::ReadWrite);
        SLANG_CHECK(info.entryPointIndex == -1);
        SLANG_CHECK(info.debugName != nullptr);
        SLANG_CHECK(UnownedStringSlice(info.debugName) == toSlice("__slang_coverage"));
        SLANG_CHECK(info.uniformOffset == -1);
        SLANG_CHECK(info.uniformStride == 0);

        slang::CoverageBufferInfo bufferInfo;
        SLANG_CHECK(spirvBundle.coverage->getBufferInfo(&bufferInfo) == SLANG_OK);
        SLANG_CHECK(bufferInfo.space == info.space);
        SLANG_CHECK(bufferInfo.binding == info.binding);

        uint32_t lookedUpIndex = ~0u;
        SLANG_CHECK(
            spirvSyntheticResources->findResourceIndexByID(info.id, &lookedUpIndex) == SLANG_OK);
        SLANG_CHECK(lookedUpIndex == coverageResourceIndex);

        ComPtr<ISlangBlob> manifest;
        SLANG_CHECK(
            slang_writeCoverageManifestJson(spirvBundle.coverage, manifest.writeRef()) == SLANG_OK);
        SLANG_CHECK(manifest != nullptr);
        ParsedJson parsed;
        SLANG_CHECK(parseJsonBlob(manifest, parsed) == SLANG_OK);
        auto container = parsed.container.Ptr();
        checkJsonStringField(container, parsed.root, "format", "slang-coverage");
        checkJsonIntField(container, parsed.root, "version", 2);
        checkJsonIntField(
            container,
            parsed.root,
            "counter_count",
            spirvBundle.coverage->getCounterCount());

        JSONValue buffer = findJsonField(container, parsed.root, "buffer");
        SLANG_CHECK(buffer.isValid());
        checkJsonStringField(container, buffer, "name", "__slang_coverage");
        checkJsonStringField(container, buffer, "element_type", "uint32");
        checkJsonIntField(container, buffer, "element_stride", 4);
        SLANG_CHECK(findJsonField(container, buffer, "space").isValid());
        SLANG_CHECK(findJsonField(container, buffer, "binding").isValid());
        SLANG_CHECK(!findJsonField(container, buffer, "uniform_offset").isValid());
        SLANG_CHECK(!findJsonField(container, buffer, "uniform_stride").isValid());
    }

    {
        auto hlslBundle = createMetadataBundle(SLANG_HLSL, "cs_5_0");
        checkLineCoverageEntries(hlslBundle.coverage);
        auto* hlslSyntheticResources = hlslBundle.syntheticResources;

        SLANG_CHECK(hlslSyntheticResources->getResourceCount() == 1);

        slang::SyntheticResourceInfo info;
        SLANG_CHECK(hlslSyntheticResources->getResourceInfo(0, &info) == SLANG_OK);
        SLANG_CHECK(info.binding >= 0);
        SLANG_CHECK(info.space >= 0);
        SLANG_CHECK(info.bindingType == slang::BindingType::MutableRawBuffer);
        SLANG_CHECK(info.uniformOffset == -1);
        SLANG_CHECK(info.uniformStride == 0);
    }

    {
        auto metalBundle = createMetadataBundle(SLANG_METAL, "metal");
        checkLineCoverageEntries(metalBundle.coverage);
        auto* metalSyntheticResources = metalBundle.syntheticResources;

        SLANG_CHECK(metalSyntheticResources->getResourceCount() == 1);

        slang::SyntheticResourceInfo info;
        SLANG_CHECK(metalSyntheticResources->getResourceInfo(0, &info) == SLANG_OK);
        SLANG_CHECK(info.space == -1);
        SLANG_CHECK(info.binding >= 0);
        SLANG_CHECK(info.bindingType == slang::BindingType::MutableRawBuffer);
        SLANG_CHECK(info.uniformOffset == -1);
        SLANG_CHECK(info.uniformStride == 0);

        slang::CoverageBufferInfo bufferInfo;
        SLANG_CHECK(metalBundle.coverage->getBufferInfo(&bufferInfo) == SLANG_OK);
        SLANG_CHECK(bufferInfo.space == info.space);
        SLANG_CHECK(bufferInfo.binding == info.binding);
    }

    {
        uint32_t index = 0;
        SLANG_CHECK(syntheticResources->findResourceIndexByID(0, &index) == SLANG_E_NOT_FOUND);
        SLANG_CHECK(syntheticResources->findResourceIndexByID(1, nullptr) == SLANG_E_INVALID_ARG);
    }
    // Coverage disabled: metadata interface remains queryable, but it
    // should report no synthetic coverage bindings.
    {
        auto noCoverageBundle = createMetadataBundle(SLANG_CPP_SOURCE, "sm_5_0", false);
        auto* noCoverageSyntheticResources = noCoverageBundle.syntheticResources;
        SLANG_CHECK(noCoverageBundle.coverage->getCounterCount() == 0);
        SLANG_CHECK(noCoverageBundle.coverage->getEntryCount() == 0);
        SLANG_CHECK(noCoverageSyntheticResources->getResourceCount() == 0);
        uint32_t index = 0;
        SLANG_CHECK(
            noCoverageSyntheticResources->findResourceIndexByID(1, &index) == SLANG_E_NOT_FOUND);
        slang::CoverageBufferInfo bufferInfo;
        SLANG_CHECK(noCoverageBundle.coverage->getBufferInfo(&bufferInfo) == SLANG_OK);
        SLANG_CHECK(bufferInfo.space == -1);
        SLANG_CHECK(bufferInfo.binding == -1);
    }
    SLANG_CHECK(coverage->getBufferInfo(nullptr) == SLANG_E_INVALID_ARG);
    {
        slang::CoverageBufferInfo bufferInfo;
        bufferInfo.structSize = 0;
        SLANG_CHECK(coverage->getBufferInfo(&bufferInfo) == SLANG_E_INVALID_ARG);
    }
    {
        slang::SyntheticResourceInfo info;
        SLANG_CHECK(
            syntheticResources->getResourceInfo(syntheticResources->getResourceCount(), &info) ==
            SLANG_E_INVALID_ARG);
    }
    SLANG_CHECK(syntheticResources->getResourceInfo(0, nullptr) == SLANG_E_INVALID_ARG);
    {
        slang::SyntheticResourceInfo info;
        info.structSize = 0;
        SLANG_CHECK(syntheticResources->getResourceInfo(0, &info) == SLANG_E_INVALID_ARG);
    }

    // Manifest serializer: produce JSON bytes and sanity-check the
    // canonical fields are present. Detailed shape is covered by the
    // existing tests on the slangc sidecar; what this assertion locks
    // in is the in-process API path producing a non-empty,
    // structurally valid manifest with the source-entry contract.
    {
        ComPtr<ISlangBlob> manifest;
        SLANG_CHECK(slang_writeCoverageManifestJson(coverage, manifest.writeRef()) == SLANG_OK);
        SLANG_CHECK(manifest != nullptr);
        SLANG_CHECK(manifest->getBufferSize() > 0);
        ParsedJson parsed;
        SLANG_CHECK(parseJsonBlob(manifest, parsed) == SLANG_OK);
        auto container = parsed.container.Ptr();
        checkJsonStringField(container, parsed.root, "format", "slang-coverage");
        checkJsonIntField(container, parsed.root, "version", 2);
        checkJsonIntField(container, parsed.root, "counter_count", coverage->getCounterCount());

        JSONValue buffer = findJsonField(container, parsed.root, "buffer");
        SLANG_CHECK(buffer.isValid());
        checkJsonStringField(container, buffer, "name", "__slang_coverage");
        checkJsonStringField(container, buffer, "element_type", "uint32");
        checkJsonIntField(container, buffer, "element_stride", 4);
        SLANG_CHECK(findJsonField(container, buffer, "uniform_offset").isValid());
        SLANG_CHECK(findJsonField(container, buffer, "uniform_stride").isValid());

        JSONValue entriesValue = findJsonField(container, parsed.root, "entries");
        SLANG_CHECK(entriesValue.isValid());
        auto entries = container->getArray(entriesValue);
        SLANG_CHECK(entries.getCount() > 0);
        JSONValue firstEntry = entries[0];
        checkJsonStringField(container, firstEntry, "kind", "line");
        checkJsonStringField(container, firstEntry, "mode", "count");
        JSONValue counter = findJsonField(container, firstEntry, "counter");
        SLANG_CHECK(counter.isValid());
        SLANG_CHECK(container->asInteger(counter) >= 0);
        SLANG_CHECK((uint32_t)container->asInteger(counter) < coverage->getCounterCount());
    }

    // Function and branch coverage use the same hidden buffer/binding
    // contract but populate richer source-entry kinds. They are
    // intentionally opt-in and do not require line coverage to be
    // enabled, so future region coverage can compose as another
    // source-entry mode instead of being forced through line probes.
    {
        auto semanticBundle = createMetadataBundle(SLANG_CPP_SOURCE, "sm_5_0", false, true, true);
        auto* semanticCoverage = semanticBundle.coverage;
        SLANG_CHECK(semanticCoverage->getCounterCount() > 0);
        SLANG_CHECK(semanticCoverage->getEntryCount() > 0);
        SLANG_CHECK(semanticBundle.syntheticResources->getResourceCount() == 1);

        bool seenFunction = false;
        bool seenBranchTrue = false;
        bool seenBranchFalse = false;
        bool seenBranchCase = false;
        uint32_t branchDefaultCount = 0;
        bool seenLine = false;
        for (uint32_t i = 0; i < semanticCoverage->getEntryCount(); ++i)
        {
            slang::CoverageEntryInfo entry;
            SLANG_CHECK(semanticCoverage->getEntryInfo(i, &entry) == SLANG_OK);
            SLANG_CHECK(entry.file != nullptr);
            SLANG_CHECK(entry.line > 0);
            SLANG_CHECK(entry.counterIndex != slang::kInvalidCoverageCounterIndex);
            SLANG_CHECK(entry.counterIndex < semanticCoverage->getCounterCount());

            if (entry.kind == slang::CoverageEntryKind::Function)
            {
                seenFunction = true;
                SLANG_CHECK(entry.functionName != nullptr);
                SLANG_CHECK(entry.functionMangledName != nullptr);
            }
            else if (entry.kind == slang::CoverageEntryKind::Branch)
            {
                SLANG_CHECK(entry.branchSiteID != 0);
                SLANG_CHECK(entry.branchArmID != 0);
                if (entry.branchArmKind == slang::CoverageBranchArmKind::TrueArm)
                    seenBranchTrue = true;
                if (entry.branchArmKind == slang::CoverageBranchArmKind::FalseArm)
                    seenBranchFalse = true;
                if (entry.branchArmKind == slang::CoverageBranchArmKind::CaseArm)
                    seenBranchCase = true;
                if (entry.branchArmKind == slang::CoverageBranchArmKind::DefaultArm)
                    branchDefaultCount++;
            }
            else if (entry.kind == slang::CoverageEntryKind::Line)
            {
                seenLine = true;
            }
        }
        SLANG_CHECK(seenFunction);
        SLANG_CHECK(seenBranchTrue);
        SLANG_CHECK(seenBranchFalse);
        SLANG_CHECK(seenBranchCase);
        // One switch has an explicit default and one omits default;
        // the latter still emits a synthetic DefaultArm so the metadata
        // can distinguish "no label selected" from "switch not reached".
        SLANG_CHECK(branchDefaultCount >= 2);
        SLANG_CHECK(!seenLine);

        ComPtr<ISlangBlob> manifest;
        SLANG_CHECK(
            slang_writeCoverageManifestJson(semanticCoverage, manifest.writeRef()) == SLANG_OK);
        ParsedJson parsed;
        SLANG_CHECK(parseJsonBlob(manifest, parsed) == SLANG_OK);
        auto container = parsed.container.Ptr();
        JSONValue entriesValue = findJsonField(container, parsed.root, "entries");
        SLANG_CHECK(entriesValue.isValid());
        auto entries = container->getArray(entriesValue);
        bool seenFunctionJson = false;
        bool seenBranchJson = false;
        for (auto entryValue : entries)
        {
            JSONValue kindValue = findJsonField(container, entryValue, "kind");
            SLANG_CHECK(kindValue.isValid());
            auto kind = container->getString(kindValue);
            if (kind == toSlice("function"))
            {
                seenFunctionJson = true;
                SLANG_CHECK(findJsonField(container, entryValue, "function").isValid());
                SLANG_CHECK(findJsonField(container, entryValue, "function_mangled").isValid());
            }
            if (kind == toSlice("branch"))
            {
                seenBranchJson = true;
                SLANG_CHECK(findJsonField(container, entryValue, "branch_site").isValid());
                SLANG_CHECK(findJsonField(container, entryValue, "branch_arm").isValid());
                SLANG_CHECK(findJsonField(container, entryValue, "branch_arm_kind").isValid());
            }
        }
        SLANG_CHECK(seenFunctionJson);
        SLANG_CHECK(seenBranchJson);
    }

    // All three coverage modes must compose into one metadata object
    // and one counter namespace. This catches mode-ordering bugs where
    // line, function, and branch producers collide on counter slots.
    {
        auto allModesBundle = createMetadataBundle(SLANG_CPP_SOURCE, "sm_5_0", true, true, true);
        auto* allModesCoverage = allModesBundle.coverage;
        SLANG_CHECK(allModesCoverage->getCounterCount() > 0);
        SLANG_CHECK(allModesCoverage->getEntryCount() > 0);

        bool seenLine = false;
        bool seenFunction = false;
        bool seenBranch = false;
        List<uint32_t> seenCounterSlots;
        for (uint32_t i = 0; i < allModesCoverage->getEntryCount(); ++i)
        {
            slang::CoverageEntryInfo entry;
            SLANG_CHECK(allModesCoverage->getEntryInfo(i, &entry) == SLANG_OK);
            SLANG_CHECK(entry.counterIndex != slang::kInvalidCoverageCounterIndex);
            SLANG_CHECK(entry.counterIndex < allModesCoverage->getCounterCount());
            for (auto seenCounterSlot : seenCounterSlots)
                SLANG_CHECK(seenCounterSlot != entry.counterIndex);
            seenCounterSlots.add(entry.counterIndex);

            if (entry.kind == slang::CoverageEntryKind::Line)
                seenLine = true;
            else if (entry.kind == slang::CoverageEntryKind::Function)
                seenFunction = true;
            else if (entry.kind == slang::CoverageEntryKind::Branch)
                seenBranch = true;
        }

        SLANG_CHECK(seenLine);
        SLANG_CHECK(seenFunction);
        SLANG_CHECK(seenBranch);
        SLANG_CHECK(seenCounterSlots.getCount() == allModesCoverage->getEntryCount());
    }

    // Argument validation on the serializer.
    {
        ComPtr<ISlangBlob> dummy;
        SLANG_CHECK(
            slang_writeCoverageManifestJson(nullptr, dummy.writeRef()) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(slang_writeCoverageManifestJson(coverage, nullptr) == SLANG_E_INVALID_ARG);

        struct OutOfRangeCounterMetadata : slang::ICoverageTracingMetadata
        {
            SLANG_NO_THROW SlangResult SLANG_MCALL
            queryInterface(SlangUUID const&, void** outObject) SLANG_OVERRIDE
            {
                if (outObject)
                    *outObject = nullptr;
                return SLANG_E_NO_INTERFACE;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
            {
                return nullptr;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL getCounterCount() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW SlangResult SLANG_MCALL
            getEntryInfo(uint32_t index, slang::CoverageEntryInfo* outInfo) SLANG_OVERRIDE
            {
                if (!outInfo || index != 0)
                    return SLANG_E_INVALID_ARG;
                outInfo->file = "bad.slang";
                outInfo->line = 1;
                outInfo->counterIndex = 1;
                outInfo->kind = slang::CoverageEntryKind::Line;
                outInfo->counterMode = slang::CoverageCounterMode::Count;
                return SLANG_OK;
            }
            SLANG_NO_THROW SlangResult SLANG_MCALL getBufferInfo(slang::CoverageBufferInfo* outInfo)
                SLANG_OVERRIDE
            {
                if (!outInfo)
                    return SLANG_E_INVALID_ARG;
                *outInfo = {};
                outInfo->structSize = sizeof(slang::CoverageBufferInfo);
                outInfo->space = -1;
                outInfo->binding = -1;
                return SLANG_OK;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL getEntryCount() SLANG_OVERRIDE { return 1; }
        };

        OutOfRangeCounterMetadata badMetadata;
        SLANG_CHECK(slang_writeCoverageManifestJson(&badMetadata, dummy.writeRef()) != SLANG_OK);

        struct NullCounterMetadata : slang::ICoverageTracingMetadata
        {
            SLANG_NO_THROW SlangResult SLANG_MCALL
            queryInterface(SlangUUID const&, void** outObject) SLANG_OVERRIDE
            {
                if (outObject)
                    *outObject = nullptr;
                return SLANG_E_NO_INTERFACE;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
            {
                return nullptr;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL getCounterCount() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW SlangResult SLANG_MCALL
            getEntryInfo(uint32_t index, slang::CoverageEntryInfo* outInfo) SLANG_OVERRIDE
            {
                if (!outInfo || index != 0)
                    return SLANG_E_INVALID_ARG;
                outInfo->file = "counterless.slang";
                outInfo->line = 1;
                outInfo->counterIndex = slang::kInvalidCoverageCounterIndex;
                outInfo->kind = slang::CoverageEntryKind::Region;
                outInfo->counterMode = slang::CoverageCounterMode::Count;
                return SLANG_OK;
            }
            SLANG_NO_THROW SlangResult SLANG_MCALL getBufferInfo(slang::CoverageBufferInfo* outInfo)
                SLANG_OVERRIDE
            {
                if (!outInfo)
                    return SLANG_E_INVALID_ARG;
                *outInfo = {};
                outInfo->structSize = sizeof(slang::CoverageBufferInfo);
                outInfo->space = -1;
                outInfo->binding = -1;
                return SLANG_OK;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL getEntryCount() SLANG_OVERRIDE { return 1; }
        };

        NullCounterMetadata nullCounterMetadata;
        ComPtr<ISlangBlob> nullCounterManifest;
        SLANG_CHECK(
            slang_writeCoverageManifestJson(&nullCounterMetadata, nullCounterManifest.writeRef()) ==
            SLANG_OK);
        ParsedJson parsed;
        SLANG_CHECK(parseJsonBlob(nullCounterManifest, parsed) == SLANG_OK);
        auto container = parsed.container.Ptr();
        JSONValue entriesValue = findJsonField(container, parsed.root, "entries");
        SLANG_CHECK(entriesValue.isValid());
        auto entries = container->getArray(entriesValue);
        SLANG_CHECK(entries.getCount() == 1);
        JSONValue counter = findJsonField(container, entries[0], "counter");
        SLANG_CHECK(counter.isValid());
        SLANG_CHECK(counter.type == JSONValue::Type::Null);

        struct MalformedFunctionMetadata : slang::ICoverageTracingMetadata
        {
            SLANG_NO_THROW SlangResult SLANG_MCALL
            queryInterface(SlangUUID const&, void** outObject) SLANG_OVERRIDE
            {
                if (outObject)
                    *outObject = nullptr;
                return SLANG_E_NO_INTERFACE;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
            {
                return nullptr;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL getCounterCount() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW SlangResult SLANG_MCALL
            getEntryInfo(uint32_t index, slang::CoverageEntryInfo* outInfo) SLANG_OVERRIDE
            {
                if (!outInfo || index != 0)
                    return SLANG_E_INVALID_ARG;
                outInfo->file = "bad-function.slang";
                outInfo->line = 1;
                outInfo->counterIndex = 0;
                outInfo->kind = slang::CoverageEntryKind::Function;
                outInfo->counterMode = slang::CoverageCounterMode::Count;
                return SLANG_OK;
            }
            SLANG_NO_THROW SlangResult SLANG_MCALL getBufferInfo(slang::CoverageBufferInfo* outInfo)
                SLANG_OVERRIDE
            {
                if (!outInfo)
                    return SLANG_E_INVALID_ARG;
                *outInfo = {};
                outInfo->structSize = sizeof(slang::CoverageBufferInfo);
                outInfo->space = -1;
                outInfo->binding = -1;
                return SLANG_OK;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL getEntryCount() SLANG_OVERRIDE { return 1; }
        };

        MalformedFunctionMetadata malformedFunctionMetadata;
        SLANG_CHECK(
            slang_writeCoverageManifestJson(&malformedFunctionMetadata, dummy.writeRef()) !=
            SLANG_OK);

        struct MangledOnlyFunctionMetadata : slang::ICoverageTracingMetadata
        {
            SLANG_NO_THROW SlangResult SLANG_MCALL
            queryInterface(SlangUUID const&, void** outObject) SLANG_OVERRIDE
            {
                if (outObject)
                    *outObject = nullptr;
                return SLANG_E_NO_INTERFACE;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
            {
                return nullptr;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL getCounterCount() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW SlangResult SLANG_MCALL
            getEntryInfo(uint32_t index, slang::CoverageEntryInfo* outInfo) SLANG_OVERRIDE
            {
                if (!outInfo || index != 0)
                    return SLANG_E_INVALID_ARG;
                outInfo->file = "mangled-only-function.slang";
                outInfo->line = 1;
                outInfo->counterIndex = 0;
                outInfo->kind = slang::CoverageEntryKind::Function;
                outInfo->counterMode = slang::CoverageCounterMode::Count;
                outInfo->functionMangledName = "_S6helperv";
                return SLANG_OK;
            }
            SLANG_NO_THROW SlangResult SLANG_MCALL getBufferInfo(slang::CoverageBufferInfo* outInfo)
                SLANG_OVERRIDE
            {
                if (!outInfo)
                    return SLANG_E_INVALID_ARG;
                *outInfo = {};
                outInfo->structSize = sizeof(slang::CoverageBufferInfo);
                outInfo->space = -1;
                outInfo->binding = -1;
                return SLANG_OK;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL getEntryCount() SLANG_OVERRIDE { return 1; }
        };

        MangledOnlyFunctionMetadata mangledOnlyFunctionMetadata;
        ComPtr<ISlangBlob> mangledOnlyManifest;
        SLANG_CHECK(
            slang_writeCoverageManifestJson(
                &mangledOnlyFunctionMetadata,
                mangledOnlyManifest.writeRef()) == SLANG_OK);
        ParsedJson mangledOnlyParsed;
        SLANG_CHECK(parseJsonBlob(mangledOnlyManifest, mangledOnlyParsed) == SLANG_OK);
        auto mangledOnlyContainer = mangledOnlyParsed.container.Ptr();
        JSONValue mangledOnlyEntriesValue =
            findJsonField(mangledOnlyContainer, mangledOnlyParsed.root, "entries");
        SLANG_CHECK(mangledOnlyEntriesValue.isValid());
        auto mangledOnlyEntries = mangledOnlyContainer->getArray(mangledOnlyEntriesValue);
        SLANG_CHECK(mangledOnlyEntries.getCount() == 1);
        SLANG_CHECK(
            !findJsonField(mangledOnlyContainer, mangledOnlyEntries[0], "function").isValid());
        checkJsonStringField(
            mangledOnlyContainer,
            mangledOnlyEntries[0],
            "function_mangled",
            "_S6helperv");

        struct MalformedBranchMetadata : slang::ICoverageTracingMetadata
        {
            SLANG_NO_THROW SlangResult SLANG_MCALL
            queryInterface(SlangUUID const&, void** outObject) SLANG_OVERRIDE
            {
                if (outObject)
                    *outObject = nullptr;
                return SLANG_E_NO_INTERFACE;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
            {
                return nullptr;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL getCounterCount() SLANG_OVERRIDE { return 1; }
            SLANG_NO_THROW SlangResult SLANG_MCALL
            getEntryInfo(uint32_t index, slang::CoverageEntryInfo* outInfo) SLANG_OVERRIDE
            {
                if (!outInfo || index != 0)
                    return SLANG_E_INVALID_ARG;
                outInfo->file = "bad-branch.slang";
                outInfo->line = 1;
                outInfo->counterIndex = 0;
                outInfo->kind = slang::CoverageEntryKind::Branch;
                outInfo->counterMode = slang::CoverageCounterMode::Count;
                outInfo->branchSiteID = 0;
                outInfo->branchArmID = 1;
                outInfo->branchArmKind = slang::CoverageBranchArmKind::TrueArm;
                return SLANG_OK;
            }
            SLANG_NO_THROW SlangResult SLANG_MCALL getBufferInfo(slang::CoverageBufferInfo* outInfo)
                SLANG_OVERRIDE
            {
                if (!outInfo)
                    return SLANG_E_INVALID_ARG;
                *outInfo = {};
                outInfo->structSize = sizeof(slang::CoverageBufferInfo);
                outInfo->space = -1;
                outInfo->binding = -1;
                return SLANG_OK;
            }
            SLANG_NO_THROW uint32_t SLANG_MCALL getEntryCount() SLANG_OVERRIDE { return 1; }
        };

        MalformedBranchMetadata malformedBranchMetadata;
        SLANG_CHECK(
            slang_writeCoverageManifestJson(&malformedBranchMetadata, dummy.writeRef()) !=
            SLANG_OK);
    }
}

SLANG_UNIT_TEST(coverageTracingSpecializedEntryPointMetadata)
{
    const char* shaderSource = R"(
        RWStructuredBuffer<uint> outputBuffer;

        [noinline]
        __generic<T>
        uint genericHelper(uint value)
        {
            uint scale = uint(sizeof(T));
            if ((value & 1u) != 0u)
                return value + scale;
            return value + scale + 1u;
        }

        [shader("compute")]
        [numthreads(groupSize, 1, 1)]
        void computeMain<int groupSize>(uint3 tid : SV_DispatchThreadID)
        {
            uint accum = genericHelper<int>(tid.x);
            accum += genericHelper<float2>(tid.x);
            accum += genericHelper<float4>(tid.x);
            if ((tid.x & 1u) != 0u)
                accum += uint(groupSize);
            else
                accum += uint(groupSize + 1);
            outputBuffer[tid.x] = accum;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CPP_SOURCE;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    slang::CompilerOptionEntry coverageOptions[2] = {};
    coverageOptions[0].name = slang::CompilerOptionName::TraceFunctionCoverage;
    coverageOptions[0].value.kind = slang::CompilerOptionValueKind::Int;
    coverageOptions[0].value.intValue0 = 1;
    coverageOptions[1].name = slang::CompilerOptionName::TraceBranchCoverage;
    coverageOptions[1].value.kind = slang::CompilerOptionValueKind::Int;
    coverageOptions[1].value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(coverageOptions);
    sessionDesc.compilerOptionEntries = coverageOptions;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "coverageSpecialized",
        "coverageSpecialized.slang",
        shaderSource,
        diagnostics.writeRef());
    if (!module && diagnostics)
    {
        fprintf(
            stderr,
            "coverageTracingSpecializedEntryPointMetadata module diagnostics:\n%s\n",
            (const char*)diagnostics->getBufferPointer());
    }
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    slang::SpecializationArg arg = slang::SpecializationArg::fromExpr("4");
    ComPtr<slang::IComponentType> specializedEntryPoint;
    SLANG_CHECK(
        entryPoint->specialize(&arg, 1, specializedEntryPoint.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    slang::IComponentType* components[] = {module, specializedEntryPoint.get()};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK(
        session->createCompositeComponentType(
            components,
            2,
            program.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> codeBlob;
    SLANG_CHECK(
        linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK(
        linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    auto coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    SLANG_CHECK(coverage != nullptr);
    SLANG_CHECK(coverage->getCounterCount() > 0);
    SLANG_CHECK(coverage->getEntryCount() > 0);

    uint32_t genericHelperFunctionCount = 0;
    bool seenComputeMainFunction = false;
    bool seenTrueArm = false;
    bool seenFalseArm = false;
    for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
    {
        slang::CoverageEntryInfo entry;
        SLANG_CHECK(coverage->getEntryInfo(i, &entry) == SLANG_OK);
        SLANG_CHECK(entry.counterIndex != slang::kInvalidCoverageCounterIndex);
        SLANG_CHECK(entry.counterIndex < coverage->getCounterCount());

        if (entry.kind == slang::CoverageEntryKind::Function)
        {
            SLANG_CHECK(entry.functionName != nullptr);
            auto functionName = UnownedStringSlice(entry.functionName);
            if (functionName.indexOf(toSlice("computeMain")) != -1)
                seenComputeMainFunction = true;
            if (functionName.indexOf(toSlice("genericHelper")) != -1)
                genericHelperFunctionCount++;
        }
        else if (entry.kind == slang::CoverageEntryKind::Branch)
        {
            if (entry.branchArmKind == slang::CoverageBranchArmKind::TrueArm)
                seenTrueArm = true;
            if (entry.branchArmKind == slang::CoverageBranchArmKind::FalseArm)
                seenFalseArm = true;
        }
    }

    SLANG_CHECK(seenComputeMainFunction);
    // Generic specializations share one source-level function coverage
    // entry. Specialized code may contain multiple calls/clones, but
    // the metadata stays source-oriented to avoid per-specialization
    // counter-entry fanout.
    SLANG_CHECK(genericHelperFunctionCount == 1);
    SLANG_CHECK(seenTrueArm);
    SLANG_CHECK(seenFalseArm);
}

SLANG_UNIT_TEST(coverageTracingBranchSiteIDsAreMetadataLocal)
{
    const char* moduleASource = R"(
        [noinline]
        public uint helperA(uint value)
        {
            if ((value & 1u) != 0u)
                return value + 10u;
            return value + 20u;
        }
    )";

    const char* moduleBSource = R"(
        import coverageBranchA;

        RWStructuredBuffer<uint> outputBuffer;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            uint value = helperA(tid.x);
            if ((value & 1u) != 0u)
                value += 1u;
            else
                value += 2u;
            outputBuffer[tid.x] = value;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CPP_SOURCE;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    slang::CompilerOptionEntry coverageOption = {};
    coverageOption.name = slang::CompilerOptionName::TraceBranchCoverage;
    coverageOption.value.kind = slang::CompilerOptionValueKind::Int;
    coverageOption.value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &coverageOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto moduleA = session->loadModuleFromSourceString(
        "coverageBranchA",
        "coverageBranchA.slang",
        moduleASource,
        diagnostics.writeRef());
    if (!moduleA && diagnostics)
    {
        fprintf(
            stderr,
            "coverageTracingBranchSiteIDsAreMetadataLocal module A diagnostics:\n%s\n",
            (const char*)diagnostics->getBufferPointer());
    }
    SLANG_CHECK(moduleA != nullptr);

    diagnostics.setNull();
    auto moduleB = session->loadModuleFromSourceString(
        "coverageBranchB",
        "coverageBranchB.slang",
        moduleBSource,
        diagnostics.writeRef());
    if (!moduleB && diagnostics)
    {
        fprintf(
            stderr,
            "coverageTracingBranchSiteIDsAreMetadataLocal module B diagnostics:\n%s\n",
            (const char*)diagnostics->getBufferPointer());
    }
    SLANG_CHECK(moduleB != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    moduleB->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    slang::IComponentType* components[] = {moduleA, moduleB, entryPoint.get()};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK(
        session->createCompositeComponentType(
            components,
            3,
            program.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> codeBlob;
    SLANG_CHECK(
        linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK(
        linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    auto coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    SLANG_CHECK(coverage != nullptr);

    uint32_t moduleASiteID = 0;
    uint32_t moduleBSiteID = 0;
    bool seenModuleATrueArm = false;
    bool seenModuleAFalseArm = false;
    bool seenModuleBTrueArm = false;
    bool seenModuleBFalseArm = false;

    auto recordSite = [](uint32_t& siteID, uint32_t newSiteID)
    {
        SLANG_CHECK(newSiteID != 0);
        if (siteID == 0)
            siteID = newSiteID;
        SLANG_CHECK(siteID == newSiteID);
    };

    for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
    {
        slang::CoverageEntryInfo entry;
        SLANG_CHECK(coverage->getEntryInfo(i, &entry) == SLANG_OK);
        if (entry.kind != slang::CoverageEntryKind::Branch)
            continue;

        SLANG_CHECK(entry.file != nullptr);
        auto file = UnownedStringSlice(entry.file);
        if (file.indexOf(toSlice("coverageBranchA.slang")) != -1)
        {
            recordSite(moduleASiteID, entry.branchSiteID);
            if (entry.branchArmKind == slang::CoverageBranchArmKind::TrueArm)
                seenModuleATrueArm = true;
            if (entry.branchArmKind == slang::CoverageBranchArmKind::FalseArm)
                seenModuleAFalseArm = true;
        }
        else if (file.indexOf(toSlice("coverageBranchB.slang")) != -1)
        {
            recordSite(moduleBSiteID, entry.branchSiteID);
            if (entry.branchArmKind == slang::CoverageBranchArmKind::TrueArm)
                seenModuleBTrueArm = true;
            if (entry.branchArmKind == slang::CoverageBranchArmKind::FalseArm)
                seenModuleBFalseArm = true;
        }
    }

    SLANG_CHECK(moduleASiteID != 0);
    SLANG_CHECK(moduleBSiteID != 0);
    SLANG_CHECK(moduleASiteID != moduleBSiteID);
    SLANG_CHECK(seenModuleATrueArm);
    SLANG_CHECK(seenModuleAFalseArm);
    SLANG_CHECK(seenModuleBTrueArm);
    SLANG_CHECK(seenModuleBFalseArm);
}

SLANG_UNIT_TEST(coverageTracingBranchSiteIDsManyFunctions)
{
    const char* shaderSource = R"(
        RWStructuredBuffer<uint> outputBuffer;

        [noinline]
        uint helperA(uint value)
        {
            uint result = value;
            if ((value & 1u) != 0u)
                result += 1u;
            else
                result += 2u;
            if ((value & 2u) != 0u)
                result += 3u;
            else
                result += 4u;
            return result;
        }

        [noinline]
        uint helperB(uint value)
        {
            uint result = value + 10u;
            if ((value & 4u) != 0u)
                result += 5u;
            else
                result += 6u;
            if ((value & 8u) != 0u)
                result += 7u;
            else
                result += 8u;
            return result;
        }

        [noinline]
        uint helperC(uint value)
        {
            uint result = value + 20u;
            if ((value & 16u) != 0u)
                result += 9u;
            else
                result += 10u;
            if ((value & 32u) != 0u)
                result += 11u;
            else
                result += 12u;
            return result;
        }

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            outputBuffer[tid.x] = helperA(tid.x) + helperB(tid.x) + helperC(tid.x);
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CPP_SOURCE;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    slang::CompilerOptionEntry coverageOption = {};
    coverageOption.name = slang::CompilerOptionName::TraceBranchCoverage;
    coverageOption.value.kind = slang::CompilerOptionValueKind::Int;
    coverageOption.value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &coverageOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "coverageBranchManyFunctions",
        "coverageBranchManyFunctions.slang",
        shaderSource,
        diagnostics.writeRef());
    if (!module && diagnostics)
    {
        fprintf(
            stderr,
            "coverageTracingBranchSiteIDsManyFunctions module diagnostics:\n%s\n",
            (const char*)diagnostics->getBufferPointer());
    }
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    slang::IComponentType* components[] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK(
        session->createCompositeComponentType(
            components,
            2,
            program.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> codeBlob;
    SLANG_CHECK(
        linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK(
        linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    auto coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    SLANG_CHECK(coverage != nullptr);

    struct BranchLineRecord
    {
        uint32_t line = 0;
        uint32_t siteID = 0;
        bool seenTrueArm = false;
        bool seenFalseArm = false;
    };

    List<BranchLineRecord> records;
    auto getOrAddRecordForLine = [&](uint32_t line) -> BranchLineRecord*
    {
        for (auto& record : records)
        {
            if (record.line == line)
                return &record;
        }

        BranchLineRecord record;
        record.line = line;
        records.add(record);
        return &records[records.getCount() - 1];
    };

    for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
    {
        slang::CoverageEntryInfo entry;
        SLANG_CHECK(coverage->getEntryInfo(i, &entry) == SLANG_OK);
        if (entry.kind != slang::CoverageEntryKind::Branch)
            continue;

        SLANG_CHECK(entry.file != nullptr);
        auto file = UnownedStringSlice(entry.file);
        SLANG_CHECK(file.indexOf(toSlice("coverageBranchManyFunctions.slang")) != -1);
        SLANG_CHECK(entry.line > 0);
        SLANG_CHECK(entry.counterIndex != slang::kInvalidCoverageCounterIndex);
        SLANG_CHECK(entry.counterIndex < coverage->getCounterCount());
        SLANG_CHECK(entry.branchSiteID != 0);
        SLANG_CHECK(entry.branchArmID != 0);

        auto record = getOrAddRecordForLine(entry.line);
        if (record->siteID == 0)
            record->siteID = entry.branchSiteID;
        SLANG_CHECK(record->siteID == entry.branchSiteID);

        if (entry.branchArmKind == slang::CoverageBranchArmKind::TrueArm)
            record->seenTrueArm = true;
        else if (entry.branchArmKind == slang::CoverageBranchArmKind::FalseArm)
            record->seenFalseArm = true;
        else
            SLANG_CHECK_MSG(false, "expected only true/false branch arms");
    }

    SLANG_CHECK(records.getCount() == 6);
    for (Index i = 0; i < records.getCount(); ++i)
    {
        SLANG_CHECK(records[i].siteID != 0);
        SLANG_CHECK(records[i].seenTrueArm);
        SLANG_CHECK(records[i].seenFalseArm);
        for (Index j = i + 1; j < records.getCount(); ++j)
        {
            SLANG_CHECK(records[i].siteID != records[j].siteID);
        }
    }
}

SLANG_UNIT_TEST(coverageTracingFunctionEntryMetadataForMultipleReturns)
{
    const char* shaderSource = R"(
        RWStructuredBuffer<uint> outputBuffer;

        [noinline]
        uint branchyHelper(uint value)
        {
            if (value == 0u)
                return 10u;
            if (value == 1u)
                return 20u;
            return value + 30u;
        }

        [noinline]
        void voidHelper(inout uint value)
        {
            if ((value & 1u) != 0u)
            {
                value += 1u;
                return;
            }
            value += 2u;
        }

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            uint value = branchyHelper(tid.x);
            voidHelper(value);
            value += branchyHelper(tid.x + 1u);
            outputBuffer[tid.x] = value;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CPP_SOURCE;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    slang::CompilerOptionEntry coverageOption = {};
    coverageOption.name = slang::CompilerOptionName::TraceFunctionCoverage;
    coverageOption.value.kind = slang::CompilerOptionValueKind::Int;
    coverageOption.value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &coverageOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "coverageFunctionMultipleReturns",
        "coverageFunctionMultipleReturns.slang",
        shaderSource,
        diagnostics.writeRef());
    if (!module && diagnostics)
    {
        fprintf(
            stderr,
            "coverageTracingFunctionEntryMetadataForMultipleReturns module diagnostics:\n%s\n",
            (const char*)diagnostics->getBufferPointer());
    }
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    slang::IComponentType* components[] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK(
        session->createCompositeComponentType(
            components,
            2,
            program.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> codeBlob;
    SLANG_CHECK(
        linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK(
        linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    auto coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    SLANG_CHECK(coverage != nullptr);

    uint32_t branchyHelperCount = 0;
    uint32_t voidHelperCount = 0;
    uint32_t computeMainCount = 0;
    List<uint32_t> functionCounters;
    for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
    {
        slang::CoverageEntryInfo entry;
        SLANG_CHECK(coverage->getEntryInfo(i, &entry) == SLANG_OK);
        if (entry.kind != slang::CoverageEntryKind::Function)
            continue;

        SLANG_CHECK(entry.functionName != nullptr);
        SLANG_CHECK(entry.counterIndex != slang::kInvalidCoverageCounterIndex);
        SLANG_CHECK(entry.counterIndex < coverage->getCounterCount());
        for (auto counterIndex : functionCounters)
            SLANG_CHECK(counterIndex != entry.counterIndex);
        functionCounters.add(entry.counterIndex);

        auto functionName = UnownedStringSlice(entry.functionName);
        if (functionName.indexOf(toSlice("branchyHelper")) != -1)
            branchyHelperCount++;
        if (functionName.indexOf(toSlice("voidHelper")) != -1)
            voidHelperCount++;
        if (functionName.indexOf(toSlice("computeMain")) != -1)
            computeMainCount++;
    }

    // A function entry counter is attached to the function entry block,
    // not to each return block or call site.
    SLANG_CHECK(branchyHelperCount == 1);
    SLANG_CHECK(voidHelperCount == 1);
    SLANG_CHECK(computeMainCount == 1);
}

SLANG_UNIT_TEST(coverageTracingFunctionEntryCallableKindsMetadata)
{
    const char* shaderSource = R"(
        RWStructuredBuffer<uint> outputBuffer;

        struct CallableKinds
        {
            uint value;

            __init(uint v)
            {
                value = v;
            }

            uint instanceMethod(uint x)
            {
                return value + x;
            }

            static uint staticMethod(uint x)
            {
                return x + 3u;
            }
        };

        [ForceInline]
        uint forceInlineHelper(uint x)
        {
            return x + 5u;
        }

        uint freeHelper(uint x)
        {
            return forceInlineHelper(x) + forceInlineHelper(x + 1u);
        }

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            let lambda = (uint x) => x + 9u;
            CallableKinds callable = CallableKinds(tid.x);
            outputBuffer[0] =
                freeHelper(tid.x) + callable.instanceMethod(tid.y) +
                CallableKinds::staticMethod(tid.z) + lambda(tid.x);
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CPP_SOURCE;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    slang::CompilerOptionEntry coverageOption = {};
    coverageOption.name = slang::CompilerOptionName::TraceFunctionCoverage;
    coverageOption.value.kind = slang::CompilerOptionValueKind::Int;
    coverageOption.value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &coverageOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "coverageCallableKinds",
        "coverageCallableKinds.slang",
        shaderSource,
        diagnostics.writeRef());
    if (!module && diagnostics)
    {
        fprintf(
            stderr,
            "coverageTracingFunctionEntryCallableKindsMetadata module diagnostics:\n%s\n",
            (const char*)diagnostics->getBufferPointer());
    }
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    slang::IComponentType* components[] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK(
        session->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            program.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> codeBlob;
    SLANG_CHECK(
        linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK(
        linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    auto coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    SLANG_CHECK(coverage != nullptr);

    uint32_t functionEntryCount = 0;
    uint32_t constructorCount = 0;
    uint32_t forceInlineHelperCount = 0;
    uint32_t freeHelperCount = 0;
    uint32_t instanceMethodCount = 0;
    uint32_t staticMethodCount = 0;
    uint32_t lambdaInitCount = 0;
    uint32_t lambdaCallCount = 0;
    uint32_t computeMainCount = 0;
    List<uint32_t> functionCounters;

    for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
    {
        slang::CoverageEntryInfo entry;
        SLANG_CHECK(coverage->getEntryInfo(i, &entry) == SLANG_OK);
        if (entry.kind != slang::CoverageEntryKind::Function)
            continue;

        functionEntryCount++;
        SLANG_CHECK(entry.file != nullptr);
        SLANG_CHECK(
            UnownedStringSlice(entry.file).indexOf(toSlice("coverageCallableKinds.slang")) != -1);
        SLANG_CHECK(entry.line > 0);
        SLANG_CHECK(entry.counterIndex != slang::kInvalidCoverageCounterIndex);
        SLANG_CHECK(entry.counterIndex < coverage->getCounterCount());
        for (auto counterIndex : functionCounters)
            SLANG_CHECK(counterIndex != entry.counterIndex);
        functionCounters.add(entry.counterIndex);
        SLANG_CHECK(entry.functionName != nullptr);
        SLANG_CHECK(entry.functionMangledName != nullptr);

        auto functionName = UnownedStringSlice(entry.functionName);
        auto mangledName = UnownedStringSlice(entry.functionMangledName);

        if (functionName == toSlice("forceInlineHelper"))
            forceInlineHelperCount++;
        else if (functionName == toSlice("freeHelper"))
            freeHelperCount++;
        else if (functionName == toSlice("instanceMethod"))
            instanceMethodCount++;
        else if (functionName == toSlice("staticMethod"))
            staticMethodCount++;
        else if (functionName == toSlice("computeMain"))
            computeMainCount++;
        else if (
            functionName == toSlice("$init") &&
            mangledName.indexOf(toSlice("Lambda_computeMain")) != -1)
            lambdaInitCount++;
        else if (
            functionName == toSlice("()") &&
            mangledName.indexOf(toSlice("Lambda_computeMain")) != -1)
            lambdaCallCount++;
        else if (
            functionName == toSlice("$init") &&
            mangledName.indexOf(toSlice("13CallableKinds")) != -1)
            constructorCount++;
    }

    SLANG_CHECK(functionEntryCount == 8);
    SLANG_CHECK(constructorCount == 1);
    // The helper is called twice, but function coverage records the
    // source callable once rather than fanning out by call site.
    SLANG_CHECK(forceInlineHelperCount == 1);
    SLANG_CHECK(freeHelperCount == 1);
    SLANG_CHECK(instanceMethodCount == 1);
    SLANG_CHECK(staticMethodCount == 1);
    SLANG_CHECK(lambdaInitCount == 1);
    SLANG_CHECK(lambdaCallCount == 1);
    SLANG_CHECK(computeMainCount == 1);
}

SLANG_UNIT_TEST(coverageTracingSpecializedMultiEntryPointMetadata)
{
    const char* shaderSource = R"(
        RWStructuredBuffer<uint> outputBufferA;
        RWStructuredBuffer<uint> outputBufferB;

        [shader("compute")]
        [numthreads(groupSize, 1, 1)]
        void computeA<int groupSize>(uint3 tid : SV_DispatchThreadID)
        {
            uint value = tid.x;
            if ((value & 1u) != 0u)
                value += uint(groupSize);
            else
                value += uint(groupSize + 1);
            outputBufferA[tid.x] = value;
        }

        [shader("compute")]
        [numthreads(groupSize, 1, 1)]
        void computeB<int groupSize>(uint3 tid : SV_DispatchThreadID)
        {
            uint value = tid.x + 10u;
            if ((value & 2u) != 0u)
                value += uint(groupSize);
            else
                value += uint(groupSize + 1);
            outputBufferB[tid.x] = value;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CPP_SOURCE;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    slang::CompilerOptionEntry coverageOptions[2] = {};
    coverageOptions[0].name = slang::CompilerOptionName::TraceFunctionCoverage;
    coverageOptions[0].value.kind = slang::CompilerOptionValueKind::Int;
    coverageOptions[0].value.intValue0 = 1;
    coverageOptions[1].name = slang::CompilerOptionName::TraceBranchCoverage;
    coverageOptions[1].value.kind = slang::CompilerOptionValueKind::Int;
    coverageOptions[1].value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(coverageOptions);
    sessionDesc.compilerOptionEntries = coverageOptions;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "coverageSpecializedMultiEntryPoint",
        "coverageSpecializedMultiEntryPoint.slang",
        shaderSource,
        diagnostics.writeRef());
    if (!module && diagnostics)
    {
        fprintf(
            stderr,
            "coverageTracingSpecializedMultiEntryPointMetadata module diagnostics:\n%s\n",
            (const char*)diagnostics->getBufferPointer());
    }
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPointA;
    module->findAndCheckEntryPoint(
        "computeA",
        SLANG_STAGE_COMPUTE,
        entryPointA.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(entryPointA != nullptr);

    ComPtr<slang::IEntryPoint> entryPointB;
    module->findAndCheckEntryPoint(
        "computeB",
        SLANG_STAGE_COMPUTE,
        entryPointB.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(entryPointB != nullptr);

    slang::SpecializationArg argA = slang::SpecializationArg::fromExpr("4");
    ComPtr<slang::IComponentType> specializedEntryPointA;
    SLANG_CHECK(
        entryPointA
            ->specialize(&argA, 1, specializedEntryPointA.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    slang::SpecializationArg argB = slang::SpecializationArg::fromExpr("8");
    ComPtr<slang::IComponentType> specializedEntryPointB;
    SLANG_CHECK(
        entryPointB
            ->specialize(&argB, 1, specializedEntryPointB.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    slang::IComponentType* components[] = {
        module,
        specializedEntryPointA.get(),
        specializedEntryPointB.get(),
    };
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK(
        session->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            program.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    auto checkEntryPointCoverage = [&](uint32_t entryPointIndex, const char* expectedFunctionName)
    {
        ComPtr<slang::IBlob> codeBlob;
        SLANG_CHECK(
            linked->getEntryPointCode(
                entryPointIndex,
                0,
                codeBlob.writeRef(),
                diagnostics.writeRef()) == SLANG_OK);

        ComPtr<slang::IMetadata> metadata;
        SLANG_CHECK(
            linked->getEntryPointMetadata(
                entryPointIndex,
                0,
                metadata.writeRef(),
                diagnostics.writeRef()) == SLANG_OK);

        auto coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
            slang::ICoverageTracingMetadata::getTypeGuid());
        SLANG_CHECK(coverage != nullptr);
        SLANG_CHECK(coverage->getCounterCount() > 0);
        SLANG_CHECK(coverage->getEntryCount() > 0);

        bool seenExpectedFunction = false;
        bool seenTrueArm = false;
        bool seenFalseArm = false;
        for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
        {
            slang::CoverageEntryInfo entry;
            SLANG_CHECK(coverage->getEntryInfo(i, &entry) == SLANG_OK);
            SLANG_CHECK(entry.counterIndex != slang::kInvalidCoverageCounterIndex);
            SLANG_CHECK(entry.counterIndex < coverage->getCounterCount());

            if (entry.kind == slang::CoverageEntryKind::Function)
            {
                SLANG_CHECK(entry.functionName != nullptr);
                auto functionName = UnownedStringSlice(entry.functionName);
                if (functionName.indexOf(UnownedStringSlice(expectedFunctionName)) != -1)
                    seenExpectedFunction = true;
            }
            else if (entry.kind == slang::CoverageEntryKind::Branch)
            {
                SLANG_CHECK(entry.branchSiteID != 0);
                SLANG_CHECK(entry.branchArmID != 0);
                if (entry.branchArmKind == slang::CoverageBranchArmKind::TrueArm)
                    seenTrueArm = true;
                if (entry.branchArmKind == slang::CoverageBranchArmKind::FalseArm)
                    seenFalseArm = true;
            }
        }

        SLANG_CHECK(seenExpectedFunction);
        SLANG_CHECK(seenTrueArm);
        SLANG_CHECK(seenFalseArm);
    };

    checkEntryPointCoverage(0, "computeA");
    checkEntryPointCoverage(1, "computeB");
}

SLANG_UNIT_TEST(coverageTracingReservedNameFailsCodegen)
{
    const char* shaderSource = R"(
        RWStructuredBuffer<uint> __slang_coverage;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            if ((tid.x & 1u) != 0u)
                __slang_coverage[0] = 1;
            else
                __slang_coverage[0] = 2;
        }
    )";

    slang::CompilerOptionName coverageOptionNames[] = {
        slang::CompilerOptionName::TraceCoverage,
        slang::CompilerOptionName::TraceFunctionCoverage,
        slang::CompilerOptionName::TraceBranchCoverage,
    };

    for (auto optionName : coverageOptionNames)
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_HLSL;
        targetDesc.profile = globalSession->findProfile("cs_5_0");

        slang::CompilerOptionEntry coverageOption = {};
        coverageOption.name = optionName;
        coverageOption.value.kind = slang::CompilerOptionValueKind::Int;
        coverageOption.value.intValue0 = 1;

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        sessionDesc.compilerOptionEntryCount = 1;
        sessionDesc.compilerOptionEntries = &coverageOption;

        ComPtr<slang::ISession> session;
        SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnostics;
        auto module = session->loadModuleFromSourceString(
            "coverageReservedNameCodegenFail",
            "coverageReservedNameCodegenFail.slang",
            shaderSource,
            diagnostics.writeRef());
        SLANG_CHECK(module != nullptr);

        ComPtr<slang::IEntryPoint> entryPoint;
        module->findAndCheckEntryPoint(
            "computeMain",
            SLANG_STAGE_COMPUTE,
            entryPoint.writeRef(),
            diagnostics.writeRef());
        SLANG_CHECK(entryPoint != nullptr);

        slang::IComponentType* components[] = {module, entryPoint.get()};
        ComPtr<slang::IComponentType> program;
        SLANG_CHECK(
            session->createCompositeComponentType(
                components,
                SLANG_COUNT_OF(components),
                program.writeRef(),
                diagnostics.writeRef()) == SLANG_OK);

        ComPtr<slang::IComponentType> linked;
        SLANG_CHECK(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> codeBlob;
        diagnostics.setNull();
        SlangResult codeResult =
            linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef());
        SLANG_CHECK(SLANG_FAILED(codeResult));
        SLANG_CHECK(codeBlob == nullptr || codeBlob->getBufferSize() == 0);
        SLANG_CHECK(diagnostics != nullptr);
        String diagnosticText(UnownedStringSlice(
            (const char*)diagnostics->getBufferPointer(),
            diagnostics->getBufferSize()));
        SLANG_CHECK(diagnosticText.indexOf(toSlice("E45100")) != -1);
        SLANG_CHECK(diagnosticText.indexOf(toSlice("__slang_coverage")) != -1);

        // If metadata is still materialized after the failed codegen
        // query, it must not expose a synthesized coverage resource for
        // the conflicting user declaration.
        ComPtr<slang::IMetadata> metadata;
        diagnostics.setNull();
        SlangResult metadataResult =
            linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef());
        if (SLANG_SUCCEEDED(metadataResult) && metadata)
        {
            auto syntheticResources = (slang::ISyntheticResourceMetadata*)metadata->castAs(
                slang::ISyntheticResourceMetadata::getTypeGuid());
            if (syntheticResources)
                SLANG_CHECK(syntheticResources->getResourceCount() == 0);
        }
    }
}

SLANG_UNIT_TEST(coverageTracingNestedBranchSiteIDs)
{
    const char* shaderSource = R"(
        RWStructuredBuffer<uint> outputBuffer;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            uint value = tid.x;
            if (tid.x != 0u)
            {
                if (tid.y != 0u)
                    value += 1u;
                else
                    value += 2u;
            }
            else
            {
                switch (int(tid.z & 1u))
                {
                case 0:
                    value += 3u;
                    break;
                default:
                    value += 4u;
                    break;
                }
            }
            outputBuffer[0] = value;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CPP_SOURCE;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    slang::CompilerOptionEntry coverageOption = {};
    coverageOption.name = slang::CompilerOptionName::TraceBranchCoverage;
    coverageOption.value.kind = slang::CompilerOptionValueKind::Int;
    coverageOption.value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &coverageOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "coverageNestedBranchSiteIDs",
        "coverageNestedBranchSiteIDs.slang",
        shaderSource,
        diagnostics.writeRef());
    if (!module && diagnostics)
    {
        fprintf(
            stderr,
            "coverageTracingNestedBranchSiteIDs module diagnostics:\n%s\n",
            (const char*)diagnostics->getBufferPointer());
    }
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    slang::IComponentType* components[] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK(
        session->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            program.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK(program->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> codeBlob;
    SLANG_CHECK(
        linked->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK(
        linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    auto coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    SLANG_CHECK(coverage != nullptr);

    struct BranchSiteRecord
    {
        uint32_t siteID = 0;
        bool seenTrueArm = false;
        bool seenFalseArm = false;
        bool seenCaseArm = false;
        bool seenDefaultArm = false;
    };

    List<BranchSiteRecord> records;
    auto getOrAddRecordForSite = [&](uint32_t siteID) -> BranchSiteRecord*
    {
        for (auto& record : records)
        {
            if (record.siteID == siteID)
                return &record;
        }

        BranchSiteRecord record;
        record.siteID = siteID;
        records.add(record);
        return &records[records.getCount() - 1];
    };

    for (uint32_t i = 0; i < coverage->getEntryCount(); ++i)
    {
        slang::CoverageEntryInfo entry;
        SLANG_CHECK(coverage->getEntryInfo(i, &entry) == SLANG_OK);
        if (entry.kind != slang::CoverageEntryKind::Branch)
            continue;

        SLANG_CHECK(entry.file != nullptr);
        auto file = UnownedStringSlice(entry.file);
        SLANG_CHECK(file.indexOf(toSlice("coverageNestedBranchSiteIDs.slang")) != -1);
        SLANG_CHECK(entry.counterIndex != slang::kInvalidCoverageCounterIndex);
        SLANG_CHECK(entry.counterIndex < coverage->getCounterCount());
        SLANG_CHECK(entry.branchSiteID != 0);
        SLANG_CHECK(entry.branchArmID != 0);

        auto record = getOrAddRecordForSite(entry.branchSiteID);
        if (entry.branchArmKind == slang::CoverageBranchArmKind::TrueArm)
            record->seenTrueArm = true;
        else if (entry.branchArmKind == slang::CoverageBranchArmKind::FalseArm)
            record->seenFalseArm = true;
        else if (entry.branchArmKind == slang::CoverageBranchArmKind::CaseArm)
            record->seenCaseArm = true;
        else if (entry.branchArmKind == slang::CoverageBranchArmKind::DefaultArm)
            record->seenDefaultArm = true;
        else
            SLANG_CHECK_MSG(false, "unexpected branch arm kind");
    }

    uint32_t ifSiteCount = 0;
    uint32_t switchSiteCount = 0;
    SLANG_CHECK(records.getCount() == 3);
    for (auto& record : records)
    {
        SLANG_CHECK(record.siteID != 0);
        if (record.seenTrueArm || record.seenFalseArm)
        {
            SLANG_CHECK(record.seenTrueArm);
            SLANG_CHECK(record.seenFalseArm);
            SLANG_CHECK(!record.seenCaseArm);
            SLANG_CHECK(!record.seenDefaultArm);
            ifSiteCount++;
        }
        else
        {
            SLANG_CHECK(record.seenCaseArm);
            SLANG_CHECK(record.seenDefaultArm);
            switchSiteCount++;
        }
    }
    SLANG_CHECK(ifSiteCount == 2);
    SLANG_CHECK(switchSiteCount == 1);
}
