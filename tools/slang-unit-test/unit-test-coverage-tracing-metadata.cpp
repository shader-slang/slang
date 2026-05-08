// unit-test-coverage-tracing-metadata.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

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
//     and buffer binding is reported
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

    auto createMetadataBundle = [&](SlangCompileTarget format, const char* profileName)
    {
        MetadataBundle bundle;

        slang::TargetDesc targetDesc = {};
        targetDesc.format = format;
        targetDesc.profile = globalSession->findProfile(profileName);

        slang::CompilerOptionEntry covOption = {};
        covOption.name = slang::CompilerOptionName::TraceCoverage;
        covOption.value.kind = slang::CompilerOptionValueKind::Int;
        covOption.value.intValue0 = 1;

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        sessionDesc.compilerOptionEntries = &covOption;
        sessionDesc.compilerOptionEntryCount = 1;

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
            linked->getEntryPointMetadata(0, 0, bundle.metadata.writeRef(), diagnostics.writeRef()) ==
            SLANG_OK);

        bundle.coverage = (slang::ICoverageTracingMetadata*)bundle.metadata->castAs(
            slang::ICoverageTracingMetadata::getTypeGuid());
        SLANG_CHECK(bundle.coverage != nullptr);
        bundle.syntheticResources = (slang::ISyntheticResourceMetadata*)bundle.metadata->castAs(
            slang::ISyntheticResourceMetadata::getTypeGuid());
        SLANG_CHECK(bundle.syntheticResources != nullptr);

        return bundle;
    };

    auto cpuBundle = createMetadataBundle(SLANG_CPP_SOURCE, "sm_5_0");
    auto* coverage = cpuBundle.coverage;
    auto* syntheticResources = cpuBundle.syntheticResources;

    // The shader has multiple instrumented statements (for loop, if,
    // else, assignments, writeback) — expect at least several slots.
    uint32_t counterCount = coverage->getCounterCount();
    SLANG_CHECK(counterCount > 0);

    // Walk every slot and exercise the per-entry accessor. File
    // string and line number must both be populated for every slot;
    // the synthesizer gives every counter op a real source location.
    for (uint32_t i = 0; i < counterCount; ++i)
    {
        slang::CoverageEntryInfo entry;
        SLANG_CHECK(coverage->getEntryInfo(i, &entry) == SLANG_OK);
        SLANG_CHECK(entry.file != nullptr);
        SLANG_CHECK(entry.line > 0);
    }

    // Out-of-range query must return SLANG_E_INVALID_ARG.
    {
        slang::CoverageEntryInfo entry;
        SLANG_CHECK(coverage->getEntryInfo(counterCount, &entry) == SLANG_E_INVALID_ARG);
    }

    // Null pointer must return SLANG_E_INVALID_ARG, not crash.
    SLANG_CHECK(coverage->getEntryInfo(0, nullptr) == SLANG_E_INVALID_ARG);

    // Wrong structSize must return SLANG_E_INVALID_ARG.
    {
        slang::CoverageEntryInfo entry;
        entry.structSize = 0;
        SLANG_CHECK(coverage->getEntryInfo(0, &entry) == SLANG_E_INVALID_ARG);
    }

    // Buffer binding info: for a CPU target, space/binding values may
    // be -1, but the call must succeed.
    {
        slang::CoverageBufferInfo bufferInfo;
        SLANG_CHECK(coverage->getBufferInfo(&bufferInfo) == SLANG_OK);
    }
    SLANG_CHECK(coverage->getBufferInfo(nullptr) == SLANG_E_INVALID_ARG);

    // Synthetic resource metadata: coverage should surface one hidden
    // mutable structured buffer resource that hosts can use as a
    // binding helper bridge.
    {
        const uint32_t resourceCount = syntheticResources->getResourceCount();
        SLANG_CHECK(resourceCount > 0);

        bool foundCoverageResource = false;
        for (uint32_t i = 0; i < resourceCount; ++i)
        {
            slang::SyntheticResourceInfo info;
            SLANG_CHECK(syntheticResources->getResourceInfo(i, &info) == SLANG_OK);
            if (info.featureTag && UnownedStringSlice(info.featureTag) == toSlice("coverage"))
            {
                foundCoverageResource = true;
                SLANG_CHECK(info.id != 0);
                SLANG_CHECK(info.bindingType == slang::BindingType::MutableRawBuffer);
                SLANG_CHECK(info.arraySize == 1);
                SLANG_CHECK(info.scope == slang::SyntheticResourceScope::Global);
                SLANG_CHECK(info.access == slang::SyntheticResourceAccess::ReadWrite);
                SLANG_CHECK(info.entryPointIndex == -1);
                SLANG_CHECK(info.debugName != nullptr);
                SLANG_CHECK(UnownedStringSlice(info.debugName) == toSlice("__slang_coverage"));

                // Descriptor-facing binding info should be available
                // even when the resource is hidden from ordinary
                // reflection. For the current CPU source target, the
                // metadata should also expose the concrete
                // marshaling location in the generated global params
                // payload.
                SLANG_CHECK(info.binding >= 0);
                SLANG_CHECK(info.space >= -1);
                SLANG_CHECK(info.uniformOffset >= 0);
                SLANG_CHECK(info.uniformStride > 0);

                uint32_t lookedUpIndex = ~0u;
                SLANG_CHECK(syntheticResources->findResourceIndexByID(info.id, &lookedUpIndex) == SLANG_OK);
                SLANG_CHECK(lookedUpIndex == i);

                slang::SyntheticResourceDescriptorBindingInfo descriptorInfo;
                SLANG_CHECK(
                    syntheticResources->getResourceDescriptorBindingInfo(i, &descriptorInfo) ==
                    SLANG_OK);
                SLANG_CHECK(descriptorInfo.binding == info.binding);
                SLANG_CHECK(descriptorInfo.space == info.space);

                slang::SyntheticResourceDescriptorClass descriptorClass =
                    slang::SyntheticResourceDescriptorClass::Unsupported;
                SLANG_CHECK(slang::getSyntheticResourceDescriptorClass(info.bindingType, &descriptorClass));
                SLANG_CHECK(descriptorClass == slang::SyntheticResourceDescriptorClass::StorageBuffer);

                slang::SyntheticResourceDescriptorRange descriptorRange = {};
                SLANG_CHECK(
                    slang::getSyntheticResourceDescriptorRange(syntheticResources, i, &descriptorRange) ==
                    SLANG_OK);
                SLANG_CHECK(descriptorRange.id == info.id);
                SLANG_CHECK(descriptorRange.bindingType == info.bindingType);
                SLANG_CHECK(descriptorRange.descriptorClass == descriptorClass);
                SLANG_CHECK(descriptorRange.space == info.space);
                SLANG_CHECK(descriptorRange.binding == info.binding);
                SLANG_CHECK(descriptorRange.arraySize == info.arraySize);
                SLANG_CHECK(descriptorRange.scope == info.scope);
                SLANG_CHECK(descriptorRange.access == info.access);
                SLANG_CHECK(descriptorRange.entryPointIndex == info.entryPointIndex);
                SLANG_CHECK(descriptorRange.debugName != nullptr);
                SLANG_CHECK(UnownedStringSlice(descriptorRange.debugName) == toSlice("__slang_coverage"));

                slang::SyntheticResourceDescriptorRange foundDescriptorRange = {};
                SLANG_CHECK(
                    slang::findSyntheticResourceDescriptorRangeByID(
                        syntheticResources,
                        info.id,
                        &foundDescriptorRange) == SLANG_OK);
                SLANG_CHECK(foundDescriptorRange.space == descriptorRange.space);
                SLANG_CHECK(foundDescriptorRange.binding == descriptorRange.binding);

                slang::SyntheticResourceDescriptorSpaceSpan spaceSpan = {};
                SLANG_CHECK(
                    slang::getSyntheticResourceDescriptorSpaceSpan(syntheticResources, &spaceSpan) ==
                    SLANG_OK);
                SLANG_CHECK(spaceSpan.descriptorResourceCount > 0);
                SLANG_CHECK(spaceSpan.minSpace == info.space);
                SLANG_CHECK(spaceSpan.maxSpace == info.space);

                uint32_t descriptorRangeCount = 0;
                SLANG_CHECK(
                    slang::getSyntheticResourceDescriptorRangeCountForSpace(
                        syntheticResources,
                        uint32_t(info.space),
                        &descriptorRangeCount) == SLANG_OK);
                SLANG_CHECK(descriptorRangeCount == 1);

                descriptorRangeCount = 0;
                SLANG_CHECK(
                    slang::getSyntheticResourceDescriptorRangesForSpace(
                        syntheticResources,
                        uint32_t(info.space),
                        &descriptorRangeCount,
                        nullptr) == SLANG_OK);
                SLANG_CHECK(descriptorRangeCount == 1);

                descriptorRangeCount = 0;
                SLANG_CHECK(
                    slang::getSyntheticResourceDescriptorRangesForSpace(
                        syntheticResources,
                        uint32_t(info.space),
                        &descriptorRangeCount,
                        &descriptorRange) == SLANG_E_BUFFER_TOO_SMALL);
                SLANG_CHECK(descriptorRangeCount == 1);

                descriptorRangeCount = 1;
                slang::SyntheticResourceDescriptorRange descriptorRanges[1] = {};
                SLANG_CHECK(
                    slang::getSyntheticResourceDescriptorRangesForSpace(
                        syntheticResources,
                        uint32_t(info.space),
                        &descriptorRangeCount,
                        descriptorRanges) == SLANG_OK);
                SLANG_CHECK(descriptorRangeCount == 1);
                SLANG_CHECK(descriptorRanges[0].binding == descriptorRange.binding);
                SLANG_CHECK(descriptorRanges[0].space == descriptorRange.space);

                slang::SyntheticResourceUniformBindingInfo uniformInfo;
                SLANG_CHECK(
                    syntheticResources->getResourceUniformBindingInfo(i, &uniformInfo) ==
                    SLANG_OK);
                SLANG_CHECK(uniformInfo.uniformOffset == info.uniformOffset);
                SLANG_CHECK(uniformInfo.uniformStride == info.uniformStride);
            }
        }
        SLANG_CHECK(foundCoverageResource);
    }

    // Descriptor-backed targets should expose descriptor-facing binding
    // information while leaving CPU/CUDA uniform marshaling fields at their
    // documented sentinel values.
    {
        auto spirvBundle = createMetadataBundle(SLANG_SPIRV, "spirv_1_5");
        auto* spirvSyntheticResources = spirvBundle.syntheticResources;

        const uint32_t resourceCount = spirvSyntheticResources->getResourceCount();
        SLANG_CHECK(resourceCount > 0);

        bool foundCoverageResource = false;
        for (uint32_t i = 0; i < resourceCount; ++i)
        {
            slang::SyntheticResourceInfo info;
            SLANG_CHECK(spirvSyntheticResources->getResourceInfo(i, &info) == SLANG_OK);
            if (info.featureTag && UnownedStringSlice(info.featureTag) == toSlice("coverage"))
            {
                foundCoverageResource = true;
                SLANG_CHECK(info.binding >= 0);
                SLANG_CHECK(info.space >= 0);
                SLANG_CHECK(info.uniformOffset == -1);
                SLANG_CHECK(info.uniformStride == 0);

                slang::SyntheticResourceDescriptorBindingInfo descriptorInfo;
                SLANG_CHECK(
                    spirvSyntheticResources->getResourceDescriptorBindingInfo(i, &descriptorInfo) ==
                    SLANG_OK);
                SLANG_CHECK(descriptorInfo.binding == info.binding);
                SLANG_CHECK(descriptorInfo.space == info.space);

                slang::SyntheticResourceUniformBindingInfo uniformInfo;
                SLANG_CHECK(
                    spirvSyntheticResources->getResourceUniformBindingInfo(i, &uniformInfo) ==
                    SLANG_OK);
                SLANG_CHECK(uniformInfo.uniformOffset == -1);
                SLANG_CHECK(uniformInfo.uniformStride == 0);
            }
        }
        SLANG_CHECK(foundCoverageResource);
    }

    {
        uint32_t index = 0;
        SLANG_CHECK(syntheticResources->findResourceIndexByID(0, &index) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(syntheticResources->findResourceIndexByID(1, nullptr) == SLANG_E_INVALID_ARG);
    }
    {
        slang::SyntheticResourceDescriptorBindingInfo info;
        SLANG_CHECK(
            syntheticResources->getResourceDescriptorBindingInfo(
                syntheticResources->getResourceCount(),
                &info) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(
            syntheticResources->getResourceDescriptorBindingInfo(0, nullptr) ==
            SLANG_E_INVALID_ARG);
        info.structSize = 0;
        SLANG_CHECK(
            syntheticResources->getResourceDescriptorBindingInfo(0, &info) ==
            SLANG_E_INVALID_ARG);
    }
    {
        slang::SyntheticResourceUniformBindingInfo info;
        SLANG_CHECK(
            syntheticResources->getResourceUniformBindingInfo(
                syntheticResources->getResourceCount(),
                &info) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(
            syntheticResources->getResourceUniformBindingInfo(0, nullptr) ==
            SLANG_E_INVALID_ARG);
        info.structSize = 0;
        SLANG_CHECK(
            syntheticResources->getResourceUniformBindingInfo(0, &info) ==
            SLANG_E_INVALID_ARG);
    }
    {
        slang::SyntheticResourceDescriptorRange descriptorRange = {};
        SLANG_CHECK(
            slang::getSyntheticResourceDescriptorRange(nullptr, 0, &descriptorRange) ==
            SLANG_E_INVALID_ARG);
        descriptorRange.structSize = 0;
        SLANG_CHECK(
            slang::getSyntheticResourceDescriptorRange(syntheticResources, 0, &descriptorRange) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(
            slang::findSyntheticResourceDescriptorRangeByID(nullptr, 1, &descriptorRange) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(
            slang::findSyntheticResourceDescriptorRangeByID(syntheticResources, 0, &descriptorRange) ==
            SLANG_E_INVALID_ARG);
    }
    {
        slang::SyntheticResourceDescriptorSpaceSpan spaceSpan = {};
        SLANG_CHECK(
            slang::getSyntheticResourceDescriptorSpaceSpan(nullptr, &spaceSpan) ==
            SLANG_E_INVALID_ARG);
        spaceSpan.structSize = 0;
        SLANG_CHECK(
            slang::getSyntheticResourceDescriptorSpaceSpan(syntheticResources, &spaceSpan) ==
            SLANG_E_INVALID_ARG);
    }
    {
        uint32_t count = 0;
        SLANG_CHECK(
            slang::getSyntheticResourceDescriptorRangeCountForSpace(nullptr, 0, &count) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(
            slang::getSyntheticResourceDescriptorRangeCountForSpace(syntheticResources, 0, nullptr) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(
            slang::getSyntheticResourceDescriptorRangesForSpace(nullptr, 0, &count, nullptr) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(
            slang::getSyntheticResourceDescriptorRangesForSpace(syntheticResources, 0, nullptr, nullptr) ==
            SLANG_E_INVALID_ARG);
    }

    {
        slang::SyntheticResourceInfo info;
        SLANG_CHECK(
            syntheticResources->getResourceInfo(
                syntheticResources->getResourceCount(),
                &info) == SLANG_E_INVALID_ARG);
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
    // structurally valid manifest with the version-1 contract.
    {
        ComPtr<ISlangBlob> manifest;
        SLANG_CHECK(slang_writeCoverageManifestJson(coverage, manifest.writeRef()) == SLANG_OK);
        SLANG_CHECK(manifest != nullptr);
        SLANG_CHECK(manifest->getBufferSize() > 0);
        UnownedStringSlice json(
            (const char*)manifest->getBufferPointer(),
            manifest->getBufferSize());
        SLANG_CHECK(json.indexOf(toSlice("\"version\": 1")) != -1);
        SLANG_CHECK(json.indexOf(toSlice("\"counters\"")) != -1);
        SLANG_CHECK(json.indexOf(toSlice("__slang_coverage")) != -1);
        SLANG_CHECK(json.indexOf(toSlice("\"entries\"")) != -1);
    }

    // Argument validation on the serializer.
    {
        ComPtr<ISlangBlob> dummy;
        SLANG_CHECK(
            slang_writeCoverageManifestJson(nullptr, dummy.writeRef()) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(slang_writeCoverageManifestJson(coverage, nullptr) == SLANG_E_INVALID_ARG);
    }
}
