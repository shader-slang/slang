#include "metal-test-host.h"

#include "metal-test-scenes.h"
#include "slang-com-ptr.h"
#include "slang.h"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

using Slang::ComPtr;

namespace
{

struct NativePayloadPartition
{
    id<MTLIntersectionFunctionTable> intersectionTable;
    id<MTLVisibleFunctionTable> missTable;
    id<MTLVisibleFunctionTable> closestHitTable;
};

struct FrameParameters
{
    uint64_t scene;
    uint64_t programResources;
    uint64_t results;
};

struct NativeProgram
{
    // Keep the Slang objects alive because every reflection pointer below is owned by the linked
    // program. The emitted Metal source and the reflected SBT ABI therefore always describe the
    // same compilation.
    ComPtr<slang::ISession> slangSession;
    ComPtr<slang::IModule> slangModule;
    ComPtr<slang::IComponentType> slangProgram;
    ComPtr<slang::IBlob> metalSource;
    ComPtr<slang::IMetadata> targetMetadata;
    slang::IStructuralRayTracingMetadata* structuralMetadata = nullptr;
    slang::TraceProgramSchemaReflection* schema = nullptr;

    id<MTLComputePipelineState> pipeline;
    std::vector<NativePayloadPartition> payloads;
    id<MTLVisibleFunctionTable> callableTable;
};

struct ProgramDescription
{
    const char* sourceRelativePath;
    const char* schemaName;
    const char* entryPointName = "main";
};

enum class RecordSection : uint32_t
{
    Hit,
    Miss,
    Callable,
    Count,
};

// A physical record chooses a reflected shader-table function and optionally appends the source
// record data consumed by that function. `physicalIndex` is deliberately independent from
// `shaderIndex`: two physical records can select the same shader while carrying different data.
struct RecordInitializer
{
    RecordSection section;
    uint32_t physicalIndex;
    uint32_t payloadIndex;
    uint32_t shaderIndex;
    const void* data;
    size_t dataSize;
};

struct RecordBufferDescription
{
    const uint32_t* instanceHitGroupOffsets;
    uint32_t instanceHitGroupOffsetCount;
    const RecordInitializer* records;
    uint32_t recordCount;
};

bool fail(NSString* message)
{
    std::fprintf(stderr, "metal-structural-raytracing-test: %s\n", message.UTF8String);
    return false;
}

bool failDiagnostics(const char* operation, slang::IBlob* diagnostics)
{
    std::fprintf(stderr, "metal-structural-raytracing-test: %s failed", operation);
    if (diagnostics && diagnostics->getBufferSize())
    {
        std::fprintf(
            stderr,
            ":\n%.*s",
            int(diagnostics->getBufferSize()),
            static_cast<const char*>(diagnostics->getBufferPointer()));
    }
    else
    {
        std::fprintf(stderr, "\n");
    }
    return false;
}

NSString* sourcePath(NSString* repositoryRoot, const char* relativePath)
{
    return [repositoryRoot
        stringByAppendingPathComponent:[NSString stringWithUTF8String:relativePath]];
}

bool compileProgram(
    slang::IGlobalSession* globalSession,
    NSString* repositoryRoot,
    const ProgramDescription& description,
    NativeProgram& outProgram)
{
    slang::CompilerOptionEntry experimentalOption = {};
    experimentalOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalOption.value.intValue0 = 1;

    slang::TargetDesc target = {};
    target.format = SLANG_METAL;
    target.profile = globalSession->findProfile("metal_3_1");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalOption;
    if (SLANG_FAILED(globalSession->createSession(sessionDesc, outProgram.slangSession.writeRef())))
        return failDiagnostics("creating the Slang session", nullptr);

    auto path = sourcePath(repositoryRoot, description.sourceRelativePath);
    ComPtr<slang::IBlob> diagnostics;
    outProgram.slangModule = outProgram.slangSession->loadModuleFromSource(
        "metalStructuralRayTracingTest",
        path.fileSystemRepresentation,
        nullptr,
        diagnostics.writeRef());
    if (!outProgram.slangModule)
        return failDiagnostics("loading the Slang module", diagnostics);

    ComPtr<slang::IEntryPoint> entryPoint;
    if (SLANG_FAILED(outProgram.slangModule->findEntryPointByName(
            description.entryPointName,
            entryPoint.writeRef())))
    {
        return failDiagnostics("finding the ray-generation entry point", nullptr);
    }

    slang::IComponentType* components[] = {outProgram.slangModule, entryPoint};
    ComPtr<slang::IComponentType> composite;
    if (SLANG_FAILED(outProgram.slangSession->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            composite.writeRef(),
            diagnostics.writeRef())))
    {
        return failDiagnostics("composing the Slang program", diagnostics);
    }
    if (SLANG_FAILED(composite->link(outProgram.slangProgram.writeRef(), diagnostics.writeRef())))
        return failDiagnostics("linking the Slang program", diagnostics);

    if (SLANG_FAILED(outProgram.slangProgram->getEntryPointCode(
            0,
            0,
            outProgram.metalSource.writeRef(),
            diagnostics.writeRef())))
        return failDiagnostics("generating Metal source", diagnostics);

    outProgram.schema =
        outProgram.slangProgram->getLayout()->findTraceProgramSchema(description.schemaName);
    if (!outProgram.schema)
        return failDiagnostics("reflecting the structural ray-tracing schema", nullptr);

    if (SLANG_FAILED(outProgram.slangProgram->getTargetMetadata(
            0,
            outProgram.targetMetadata.writeRef(),
            diagnostics.writeRef())))
    {
        return failDiagnostics("reflecting the finalized Metal ABI", diagnostics);
    }
    outProgram.structuralMetadata = static_cast<slang::IStructuralRayTracingMetadata*>(
        outProgram.targetMetadata->castAs(slang::IStructuralRayTracingMetadata::getTypeGuid()));
    if (!outProgram.structuralMetadata)
        return failDiagnostics("finding structural ray-tracing target metadata", nullptr);
    return true;
}

id<MTLLibrary> loadLibrary(id<MTLDevice> device, slang::IBlob* sourceBlob)
{
    NSString* source = [[NSString alloc] initWithBytes:sourceBlob->getBufferPointer()
                                                length:sourceBlob->getBufferSize()
                                              encoding:NSUTF8StringEncoding];
    if (!source)
        return fail(@"the generated Metal source is not valid UTF-8"), nil;

    NSError* error = nil;
    MTLCompileOptions* options = [MTLCompileOptions new];
    options.languageVersion = MTLLanguageVersion3_1;
    id<MTLLibrary> library = [device newLibraryWithSource:source options:options error:&error];
    if (!library)
        fail(error.localizedDescription);
    return library;
}

id<MTLFunction> loadIntersectionFunction(
    id<MTLLibrary> library,
    const char* name,
    NSError** outError)
{
    auto descriptor = [MTLIntersectionFunctionDescriptor new];
    descriptor.name = [NSString stringWithUTF8String:name];
    return [library newIntersectionFunctionWithDescriptor:descriptor error:outError];
}

id<MTLVisibleFunctionTable> createVisibleFunctionTable(
    id<MTLComputePipelineState> pipeline,
    const std::vector<id<MTLFunction>>& functions)
{
    auto descriptor = [MTLVisibleFunctionTableDescriptor new];
    descriptor.functionCount = functions.empty() ? 1 : functions.size();
    id<MTLVisibleFunctionTable> table = [pipeline newVisibleFunctionTableWithDescriptor:descriptor];
    for (NSUInteger i = 0; i < functions.size(); ++i)
    {
        if (functions[i])
            [table setFunction:[pipeline functionHandleWithFunction:functions[i]] atIndex:i];
    }
    return table;
}

bool createProgram(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    NSString* repositoryRoot,
    const ProgramDescription& description,
    NativeProgram& outProgram)
{
    if (!compileProgram(globalSession, repositoryRoot, description, outProgram))
        return false;

    id<MTLLibrary> library = loadLibrary(device, outProgram.metalSource);
    if (!library)
        return false;

    // Metal reserves the C-family spelling `main`, so Slang emits that one source name as
    // `main_0`. Other entry-point names remain exact; the multi-payload source deliberately uses a
    // non-reserved spelling so the same shader can also compile for OptiX.
    const char* emittedEntryPointName = std::strcmp(description.entryPointName, "main") == 0
                                            ? "main_0"
                                            : description.entryPointName;
    id<MTLFunction> kernel =
        [library newFunctionWithName:[NSString stringWithUTF8String:emittedEntryPointName]];
    if (!kernel)
        return fail(@"the generated Metal library is missing the ray-generation entry point");

    NSMutableArray<id<MTLFunction>>* allFunctions = [NSMutableArray array];

    struct PayloadFunctionObjects
    {
        std::vector<id<MTLFunction>> intersectionFunctions;
        std::vector<SlangStructuralRayTracingIntersectionFunctionImplementationKind>
            intersectionKinds;
        std::vector<id<MTLFunction>> missFunctions;
        std::vector<id<MTLFunction>> closestHitFunctions;
        slang::MetalIntersectionFunctionSignature intersectionSignature =
            slang::MetalIntersectionFunctionSignature::None;
    };
    const auto payloadCount = outProgram.schema->getPayloadCount();
    std::vector<PayloadFunctionObjects> payloadFunctions(payloadCount);
    std::vector<id<MTLFunction>> callableFunctions(outProgram.schema->getCallableShaderCount());

    NSError* error = nil;
    auto loadVisibleFunction = [&](const char* name, id<MTLFunction>& outFunction)
    {
        if (!name)
            return false;
        outFunction = [library newFunctionWithName:[NSString stringWithUTF8String:name]];
        if (!outFunction)
            return false;
        [allFunctions addObject:outFunction];
        return true;
    };

    for (uint32_t payloadIndex = 0; payloadIndex < payloadCount; ++payloadIndex)
    {
        auto reflectedPayload = outProgram.schema->getPayload(payloadIndex);
        if (!reflectedPayload)
            return fail(@"the trace-program schema has a missing payload partition");

        auto& functions = payloadFunctions[payloadIndex];
        functions.intersectionFunctions.resize(
            reflectedPayload->getIntersectionFunctionTableSize());
        functions.intersectionKinds.resize(
            reflectedPayload->getIntersectionFunctionTableSize(),
            SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_IMPLEMENTATION_UNKNOWN);
        functions.missFunctions.resize(reflectedPayload->getMissShaderCount());
        functions.closestHitFunctions.resize(reflectedPayload->getHitGroupCount());

        bool foundTargetInfo = false;
        for (uint32_t metadataIndex = 0;
             metadataIndex < outProgram.structuralMetadata->getMetalPayloadInfoCount();
             ++metadataIndex)
        {
            slang::StructuralRayTracingMetalPayloadInfo info = {};
            if (SLANG_FAILED(
                    outProgram.structuralMetadata->getMetalPayloadInfo(metadataIndex, &info)))
            {
                return fail(@"structural ray-tracing target metadata is malformed");
            }
            if (info.payloadIndex == payloadIndex && info.schemaName &&
                std::strcmp(info.schemaName, outProgram.schema->getName()) == 0)
            {
                if (foundTargetInfo)
                    return fail(@"structural ray-tracing target metadata has a duplicate payload");
                functions.intersectionSignature = info.intersectionFunctionSignature;
                foundTargetInfo = true;
            }
        }
        if (!foundTargetInfo)
            return fail(@"structural ray-tracing target metadata is missing a payload");

        for (SlangUInt i = 0; i < reflectedPayload->getIntersectionFunctionCount(); ++i)
        {
            auto reflectedFunction = reflectedPayload->getIntersectionFunction(i);
            if (!reflectedFunction)
                return fail(@"schema reflection returned a missing intersection function");
            auto tableIndex = reflectedFunction->getIntersectionFunctionTableIndex();
            if (tableIndex < 0 || size_t(tableIndex) >= functions.intersectionKinds.size() ||
                functions.intersectionKinds[size_t(tableIndex)] !=
                    SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_IMPLEMENTATION_UNKNOWN)
            {
                return fail(@"schema reflection returned an invalid Metal IFT index");
            }
            auto implementation = reflectedFunction->getImplementationKind();
            functions.intersectionKinds[size_t(tableIndex)] = implementation;
            if (implementation ==
                SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_EXPORTED_FUNCTION)
            {
                auto name = reflectedFunction->getEntryPointName();
                if (!name)
                    return fail(@"an exported Metal intersection function has no reflected name");
                id<MTLFunction> function = loadIntersectionFunction(library, name, &error);
                if (!function)
                    return fail(error.localizedDescription);
                functions.intersectionFunctions[size_t(tableIndex)] = function;
                [allFunctions addObject:function];
            }
            else if (reflectedFunction->getEntryPointName())
            {
                return fail(@"a built-in opaque intersection function has an exported name");
            }
        }

        for (SlangUInt i = 0; i < reflectedPayload->getMissShaderCount(); ++i)
        {
            auto shader = reflectedPayload->getMissShader(i);
            auto functionIndex = shader ? shader->getFunctionIndex() : -1;
            auto stage = shader ? shader->getMiss() : nullptr;
            if (functionIndex < 0 || size_t(functionIndex) >= functions.missFunctions.size() ||
                !stage ||
                !loadVisibleFunction(
                    stage->getEntryPointName(),
                    functions.missFunctions[size_t(functionIndex)]))
            {
                return fail(@"the generated Metal library is missing a reflected miss function");
            }
        }

        bool hasClosestHitFunction = false;
        for (SlangUInt i = 0; i < reflectedPayload->getHitGroupCount(); ++i)
        {
            auto group = reflectedPayload->getHitGroup(i);
            auto functionIndex = group ? group->getFunctionIndex() : -1;
            if (functionIndex < 0 || size_t(functionIndex) >= functions.closestHitFunctions.size())
            {
                return fail(@"schema reflection returned an invalid closest-hit function index");
            }
            auto name = group->getClosestHitEntryPointName();
            if (!name)
                continue;
            if (!loadVisibleFunction(name, functions.closestHitFunctions[size_t(functionIndex)]))
            {
                return fail(
                    @"the generated Metal library is missing a reflected closest-hit function");
            }
            hasClosestHitFunction = true;
        }
        if (hasClosestHitFunction)
        {
            for (auto function : functions.closestHitFunctions)
            {
                if (!function)
                    return fail(@"the reflected Metal closest-hit VFT contains a hole");
            }
        }
    }

    for (SlangUInt i = 0; i < outProgram.schema->getCallableShaderCount(); ++i)
    {
        auto shader = outProgram.schema->getCallableShader(i);
        auto functionIndex = shader ? shader->getFunctionIndex() : -1;
        auto stage = shader ? shader->getCallable() : nullptr;
        if (functionIndex < 0 || size_t(functionIndex) >= callableFunctions.size() || !stage ||
            !loadVisibleFunction(
                stage->getEntryPointName(),
                callableFunctions[size_t(functionIndex)]))
        {
            return fail(@"the generated Metal library is missing a reflected callable function");
        }
    }

    MTLLinkedFunctions* linkedFunctions = [MTLLinkedFunctions new];
    linkedFunctions.functions = allFunctions;
    MTLComputePipelineDescriptor* pipelineDescriptor = [MTLComputePipelineDescriptor new];
    pipelineDescriptor.computeFunction = kernel;
    pipelineDescriptor.linkedFunctions = linkedFunctions;
    outProgram.pipeline = [device newComputePipelineStateWithDescriptor:pipelineDescriptor
                                                                options:MTLPipelineOptionNone
                                                             reflection:nil
                                                                  error:&error];
    if (!outProgram.pipeline)
        return fail(error.localizedDescription);

    outProgram.payloads.resize(payloadCount);
    for (uint32_t payloadIndex = 0; payloadIndex < payloadCount; ++payloadIndex)
    {
        auto& payload = outProgram.payloads[payloadIndex];
        auto& functions = payloadFunctions[payloadIndex];
        payload.missTable =
            createVisibleFunctionTable(outProgram.pipeline, functions.missFunctions);
        payload.closestHitTable =
            createVisibleFunctionTable(outProgram.pipeline, functions.closestHitFunctions);

        auto intersectionTableDescriptor = [MTLIntersectionFunctionTableDescriptor new];
        intersectionTableDescriptor.functionCount =
            functions.intersectionFunctions.empty() ? 1 : functions.intersectionFunctions.size();
        payload.intersectionTable = [outProgram.pipeline
            newIntersectionFunctionTableWithDescriptor:intersectionTableDescriptor];
        auto signature = static_cast<MTLIntersectionFunctionSignature>(
            uint32_t(functions.intersectionSignature));
        for (NSUInteger i = 0; i < functions.intersectionKinds.size(); ++i)
        {
            switch (functions.intersectionKinds[i])
            {
            case SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_EXPORTED_FUNCTION:
                if (!functions.intersectionFunctions[i])
                    return fail(@"an exported Metal IFT entry has no linked function");
                [payload.intersectionTable
                    setFunction:[outProgram.pipeline
                                    functionHandleWithFunction:functions.intersectionFunctions[i]]
                        atIndex:i];
                break;
            case SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_OPAQUE_TRIANGLE:
                [payload.intersectionTable
                    setOpaqueTriangleIntersectionFunctionWithSignature:signature
                                                               atIndex:i];
                break;
            case SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_OPAQUE_CURVE:
                [payload.intersectionTable setOpaqueCurveIntersectionFunctionWithSignature:signature
                                                                                   atIndex:i];
                break;
            case SLANG_STRUCTURAL_RAY_TRACING_INTERSECTION_FUNCTION_IMPLEMENTATION_UNKNOWN:
                break;
            default:
                return fail(@"schema reflection returned an unknown Metal IFT implementation");
            }
        }
        if (!payload.missTable || !payload.closestHitTable || !payload.intersectionTable)
            return false;
    }
    outProgram.callableTable = createVisibleFunctionTable(outProgram.pipeline, callableFunctions);
    return outProgram.callableTable != nil;
}

size_t alignRecordSection(size_t value)
{
    constexpr size_t kAlignment = 16;
    return (value + kAlignment - 1) & ~(kAlignment - 1);
}

size_t getRecordStride(const NativeProgram& program, RecordSection section)
{
    switch (section)
    {
    case RecordSection::Hit:
        return program.schema->getHitRecordStride();
    case RecordSection::Miss:
        return program.schema->getMissRecordStride();
    case RecordSection::Callable:
        return program.schema->getCallableRecordStride();
    default:
        return 0;
    }
}

SlangInt getReflectedFunctionIndex(
    const NativeProgram& program,
    const RecordInitializer& initializer)
{
    if (initializer.section == RecordSection::Callable)
    {
        if (initializer.shaderIndex >= program.schema->getCallableShaderCount())
            return -1;
        return program.schema->getCallableShader(initializer.shaderIndex)->getFunctionIndex();
    }

    if (initializer.payloadIndex >= program.schema->getPayloadCount())
        return -1;
    auto payload = program.schema->getPayload(initializer.payloadIndex);
    if (initializer.section == RecordSection::Hit)
    {
        if (initializer.shaderIndex >= payload->getHitGroupCount())
            return -1;
        return payload->getHitGroup(initializer.shaderIndex)->getFunctionIndex();
    }
    if (initializer.section == RecordSection::Miss)
    {
        if (initializer.shaderIndex >= payload->getMissShaderCount())
            return -1;
        return payload->getMissShader(initializer.shaderIndex)->getFunctionIndex();
    }
    return -1;
}

void writeUInt32(std::vector<uint8_t>& bytes, size_t offset, uint32_t value)
{
    std::memcpy(bytes.data() + offset, &value, sizeof(value));
}

// Builds the native records buffer from the reflected schema ABI. The first four words contain
// byte offsets for the instance-contribution table and the three record sections. Every record is
// addressed with the corresponding reflected, schema-wide byte stride; the first 16 bytes are a
// compiler-owned header whose first word selects a function-table entry.
id<MTLBuffer> createRecords(
    id<MTLDevice> device,
    const NativeProgram& program,
    const RecordBufferDescription& description)
{
    uint32_t sectionRecordCounts[uint32_t(RecordSection::Count)] = {};
    for (uint32_t i = 0; i < description.recordCount; ++i)
    {
        auto sectionIndex = uint32_t(description.records[i].section);
        if (sectionIndex >= uint32_t(RecordSection::Count))
            return fail(@"a record initializer has an invalid section"), nil;
        auto requiredCount = description.records[i].physicalIndex + 1;
        if (requiredCount > sectionRecordCounts[sectionIndex])
            sectionRecordCounts[sectionIndex] = requiredCount;
    }

    size_t sectionOffsets[uint32_t(RecordSection::Count)] = {};
    constexpr size_t kHeaderSize = sizeof(uint32_t) * (uint32_t(RecordSection::Count) + 1);
    size_t byteCount = kHeaderSize + sizeof(uint32_t) * description.instanceHitGroupOffsetCount;
    for (uint32_t sectionIndex = 0; sectionIndex < uint32_t(RecordSection::Count); ++sectionIndex)
    {
        auto section = RecordSection(sectionIndex);
        auto stride = getRecordStride(program, section);
        if (stride < 16 || (stride & 15) != 0)
            return fail(@"schema reflection returned an invalid Metal record stride"), nil;
        byteCount = alignRecordSection(byteCount);
        sectionOffsets[sectionIndex] = byteCount;
        byteCount += stride * sectionRecordCounts[sectionIndex];
    }
    byteCount = alignRecordSection(byteCount);

    std::vector<uint8_t> bytes(byteCount);
    writeUInt32(bytes, 0, uint32_t(kHeaderSize));
    for (uint32_t sectionIndex = 0; sectionIndex < uint32_t(RecordSection::Count); ++sectionIndex)
    {
        writeUInt32(
            bytes,
            sizeof(uint32_t) * (sectionIndex + 1),
            uint32_t(sectionOffsets[sectionIndex]));
        auto stride = getRecordStride(program, RecordSection(sectionIndex));
        for (uint32_t recordIndex = 0; recordIndex < sectionRecordCounts[sectionIndex];
             ++recordIndex)
        {
            writeUInt32(bytes, sectionOffsets[sectionIndex] + recordIndex * stride, UINT32_MAX);
        }
    }
    if (description.instanceHitGroupOffsetCount)
    {
        if (!description.instanceHitGroupOffsets)
            return fail(@"instance hit-group offsets are missing"), nil;
        std::memcpy(
            bytes.data() + kHeaderSize,
            description.instanceHitGroupOffsets,
            sizeof(uint32_t) * description.instanceHitGroupOffsetCount);
    }

    for (uint32_t i = 0; i < description.recordCount; ++i)
    {
        const auto& initializer = description.records[i];
        auto functionIndex = getReflectedFunctionIndex(program, initializer);
        if (functionIndex < 0 || uint64_t(functionIndex) > UINT32_MAX)
            return fail(@"a record initializer does not identify a reflected shader"), nil;

        auto sectionIndex = uint32_t(initializer.section);
        auto stride = getRecordStride(program, initializer.section);
        if (initializer.dataSize > stride - 16 || (initializer.dataSize != 0 && !initializer.data))
        {
            return fail(@"record data does not fit the reflected Metal record stride"), nil;
        }
        auto recordOffset = sectionOffsets[sectionIndex] + initializer.physicalIndex * stride;
        writeUInt32(bytes, recordOffset, uint32_t(functionIndex));
        if (initializer.dataSize)
        {
            std::memcpy(bytes.data() + recordOffset + 16, initializer.data, initializer.dataSize);
        }
    }

    return [device newBufferWithBytes:bytes.data()
                               length:bytes.size()
                              options:MTLResourceStorageModeShared];
}

id<MTLBuffer> createDefaultTraceRecords(
    id<MTLDevice> device,
    const NativeProgram& program,
    uint32_t instanceCount = 1)
{
    // Every scene in this helper maps each native instance to physical hit record zero. Both
    // physical records then select logical shader zero through their reflected function indices.
    std::vector<uint32_t> instanceHitGroupOffsets(instanceCount);
    const RecordInitializer records[] = {
        {RecordSection::Hit, 0, 0, 0, nullptr, 0},
        {RecordSection::Miss, 0, 0, 0, nullptr, 0},
    };
    RecordBufferDescription description = {
        instanceHitGroupOffsets.data(),
        uint32_t(instanceHitGroupOffsets.size()),
        records,
        uint32_t(SLANG_COUNT_OF(records)),
    };
    return createRecords(device, program, description);
}

id<MTLBuffer> createProgramResourceBuffer(
    id<MTLDevice> device,
    const NativeProgram& program,
    id<MTLBuffer> records)
{
    std::vector<uint64_t> resources(program.schema->getDescriptorResourceCount());
    for (SlangUInt i = 0; i < program.schema->getDescriptorResourceCount(); ++i)
    {
        auto payloadIndex = program.schema->getDescriptorResourcePayloadIndex(i);
        switch (program.schema->getDescriptorResourceKind(i))
        {
        case SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_INTERSECTION_FUNCTION_TABLE:
            if (payloadIndex < 0 || size_t(payloadIndex) >= program.payloads.size())
                return nil;
            resources[i] =
                program.payloads[size_t(payloadIndex)].intersectionTable.gpuResourceID._impl;
            break;
        case SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_MISS_VISIBLE_FUNCTION_TABLE:
            if (payloadIndex < 0 || size_t(payloadIndex) >= program.payloads.size())
                return nil;
            resources[i] = program.payloads[size_t(payloadIndex)].missTable.gpuResourceID._impl;
            break;
        case SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_CLOSEST_HIT_VISIBLE_FUNCTION_TABLE:
            if (payloadIndex < 0 || size_t(payloadIndex) >= program.payloads.size())
                return nil;
            resources[i] =
                program.payloads[size_t(payloadIndex)].closestHitTable.gpuResourceID._impl;
            break;
        case SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_CALLABLE_VISIBLE_FUNCTION_TABLE:
            if (payloadIndex != -1)
                return nil;
            resources[i] = program.callableTable.gpuResourceID._impl;
            break;
        case SLANG_STRUCTURAL_RAY_TRACING_DESCRIPTOR_RECORDS:
            if (payloadIndex != -1)
                return nil;
            resources[i] = records.gpuAddress;
            break;
        default:
            return nil;
        }
    }
    return [device newBufferWithBytes:resources.data()
                               length:sizeof(uint64_t) * resources.size()
                              options:MTLResourceStorageModeShared];
}

bool dispatch(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    const NativeProgram& program,
    id<MTLAccelerationStructure> scene,
    id<MTLBuffer> programResources,
    id<MTLBuffer> records,
    id<MTLBuffer> results,
    uint32_t threadCount,
    bool nestedFrame,
    bool callableOnly)
{
    id<MTLBuffer> frameBuffer = nil;
    if (nestedFrame)
    {
        FrameParameters frame = {
            scene.gpuResourceID._impl,
            programResources.gpuAddress,
            results.gpuAddress,
        };
        frameBuffer = [device newBufferWithBytes:&frame
                                          length:sizeof(frame)
                                         options:MTLResourceStorageModeShared];
    }

    id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
    id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];
    [encoder setComputePipelineState:program.pipeline];
    if (nestedFrame)
    {
        [encoder setBuffer:frameBuffer offset:0 atIndex:0];
    }
    else if (callableOnly)
    {
        [encoder setBuffer:programResources offset:0 atIndex:0];
        [encoder setBuffer:results offset:0 atIndex:1];
    }
    else
    {
        [encoder setAccelerationStructure:scene atBufferIndex:0];
        [encoder setBuffer:programResources offset:0 atIndex:1];
        [encoder setBuffer:results offset:0 atIndex:2];
    }

    if (scene)
        [encoder useResource:scene usage:MTLResourceUsageRead];
    if (frameBuffer)
        [encoder useResource:frameBuffer usage:MTLResourceUsageRead];
    [encoder useResource:programResources usage:MTLResourceUsageRead];
    for (const auto& payload : program.payloads)
    {
        [encoder useResource:payload.intersectionTable usage:MTLResourceUsageRead];
        [encoder useResource:payload.missTable usage:MTLResourceUsageRead];
        [encoder useResource:payload.closestHitTable usage:MTLResourceUsageRead];
    }
    [encoder useResource:program.callableTable usage:MTLResourceUsageRead];
    [encoder useResource:records usage:MTLResourceUsageRead];
    [encoder useResource:results usage:MTLResourceUsageWrite];
    [encoder dispatchThreads:MTLSizeMake(threadCount, 1, 1)
        threadsPerThreadgroup:MTLSizeMake(threadCount, 1, 1)];
    [encoder endEncoding];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
    if (commandBuffer.status == MTLCommandBufferStatusError)
        return fail(commandBuffer.error.localizedDescription);
    return true;
}

bool validateResults(
    const char* testName,
    id<MTLBuffer> results,
    const uint32_t* expected,
    uint32_t wordCount)
{
    auto actual = static_cast<const uint32_t*>(results.contents);
    for (uint32_t i = 0; i < wordCount; ++i)
    {
        if (actual[i] != expected[i])
        {
            std::fprintf(
                stderr,
                "%s: result word %u is %u, expected %u\n",
                testName,
                i,
                actual[i],
                expected[i]);
            return false;
        }
    }
    std::printf("%s: passed\n", testName);
    return true;
}

bool runTriangleProgram(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot,
    const char* testName,
    const ProgramDescription& description,
    const uint32_t* expected,
    uint32_t expectedWordCount,
    uint32_t threadCount,
    MTLAccelerationStructureInstanceOptions instanceOptions =
        MTLAccelerationStructureInstanceOptionOpaque,
    uint32_t userInstanceID = 0,
    const MTLPackedFloat4x3* transform = nullptr,
    bool nestedFrame = false)
{
    NativeProgram program = {};
    if (!createProgram(globalSession, device, repositoryRoot, description, program))
        return false;

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalTriangleScene(
            device,
            queue,
            instanceOptions,
            userInstanceID,
            transform,
            scene,
            &sceneError))
        return fail(sceneError);

    id<MTLBuffer> records = createDefaultTraceRecords(device, program);
    if (!records)
        return false;
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    id<MTLBuffer> results = [device newBufferWithLength:expectedWordCount * sizeof(uint32_t)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            threadCount,
            nestedFrame,
            false))
        return false;
    return validateResults(testName, results, expected, expectedWordCount);
}

bool runTriangleHitMiss(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/shaders/triangle-hit-miss.slang",
        "Schema"};
    static const uint32_t kExpected[] = {1, 0, 2, 2, 0xffffffff, 2};
    return runTriangleProgram(
        globalSession,
        device,
        queue,
        repositoryRoot,
        "triangle-hit-miss",
        description,
        kExpected,
        6,
        2,
        MTLAccelerationStructureInstanceOptionOpaque,
        0,
        nullptr,
        true);
}

bool runRecursiveTrace(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/shaders/recursive-trace.slang",
        "Schema"};
    static const uint32_t kExpected[] = {21, 1, 2, 20, 0, 2};
    return runTriangleProgram(
        globalSession,
        device,
        queue,
        repositoryRoot,
        "recursive-trace",
        description,
        kExpected,
        6,
        2);
}

bool runRepeatedRecords(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/shaders/repeated-records.slang",
        "Schema"};
    NativeProgram program = {};
    if (!createProgram(globalSession, device, repositoryRoot, description, program))
        return false;

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalTriangleScene(
            device,
            queue,
            MTLAccelerationStructureInstanceOptionOpaque,
            0,
            nullptr,
            scene,
            &sceneError))
    {
        return fail(sceneError);
    }

    const uint32_t instanceHitGroupOffset = 0;
    const uint32_t recordValues[] = {100, 200, 300, 400};
    // Physical records zero and one deliberately choose the same reflected function index in each
    // section. Different record bytes must still reach the stage selected by each runtime index.
    const RecordInitializer recordsToWrite[] = {
        {RecordSection::Hit, 0, 0, 0, &recordValues[0], sizeof(uint32_t)},
        {RecordSection::Hit, 1, 0, 0, &recordValues[1], sizeof(uint32_t)},
        {RecordSection::Miss, 0, 0, 0, &recordValues[2], sizeof(uint32_t)},
        {RecordSection::Miss, 1, 0, 0, &recordValues[3], sizeof(uint32_t)},
    };
    const RecordBufferDescription recordDescription = {
        &instanceHitGroupOffset,
        1,
        recordsToWrite,
        uint32_t(SLANG_COUNT_OF(recordsToWrite)),
    };
    id<MTLBuffer> records = createRecords(device, program, recordDescription);
    if (!records)
        return false;
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    static const uint32_t kExpected[] = {
        10,
        100,
        4,
        10,
        200,
        4,
        20,
        300,
        4,
        20,
        400,
        4,
    };
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(kExpected)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            4,
            false,
            false))
    {
        return false;
    }
    return validateResults(
        "repeated-records",
        results,
        kExpected,
        uint32_t(SLANG_COUNT_OF(kExpected)));
}

bool runMultiplePayloads(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/shaders/multiple-payloads.slang",
        "Schema",
        "rayGenerationMain"};
    NativeProgram program = {};
    if (!createProgram(globalSession, device, repositoryRoot, description, program))
        return false;
    if (program.schema->getPayloadCount() != 2)
        return fail(@"multiple-payloads did not reflect two payload partitions");

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalTriangleScene(
            device,
            queue,
            MTLAccelerationStructureInstanceOptionOpaque,
            0,
            nullptr,
            scene,
            &sceneError))
    {
        return fail(sceneError);
    }

    const uint32_t instanceHitGroupOffset = 0;
    // The shader selects physical record zero for shadow rays and one for radiance rays. Reflection
    // keeps each record's function index scoped to its payload table, so both logical indices are
    // zero even though the physical record ordering is reversed.
    const RecordInitializer recordsToWrite[] = {
        {RecordSection::Hit, 0, 1, 0, nullptr, 0},
        {RecordSection::Hit, 1, 0, 0, nullptr, 0},
        {RecordSection::Miss, 0, 1, 0, nullptr, 0},
        {RecordSection::Miss, 1, 0, 0, nullptr, 0},
    };
    const RecordBufferDescription recordDescription = {
        &instanceHitGroupOffset,
        1,
        recordsToWrite,
        uint32_t(SLANG_COUNT_OF(recordsToWrite)),
    };
    id<MTLBuffer> records = createRecords(device, program, recordDescription);
    if (!records)
        return false;
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    static const uint32_t kExpected[] = {
        10,
        1101,
        4,
        20,
        1,
        4,
        11,
        202,
        4,
        21,
        0,
        4,
    };
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(kExpected)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            4,
            false,
            false))
    {
        return false;
    }
    return validateResults(
        "multiple-payloads",
        results,
        kExpected,
        uint32_t(SLANG_COUNT_OF(kExpected)));
}

bool runTriangleAttributesFlags(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/shaders/triangle-attributes-flags.slang",
        "Schema"};
    static const uint32_t kExpected[] = {
        3,  0, 25, 25, 0,  1, 10, 2,  0, 0, 0,  0,  1, 10, 2,  1, 25, 25, 0,  1, 10, 3,  1, 25,
        25, 0, 1,  10, 40, 0, 0,  0,  0, 1, 10, 2,  0, 0,  0,  0, 1,  10, 3,  0, 25, 25, 0, 1,
        10, 2, 0,  0,  0,  0, 1,  10, 3, 1, 25, 25, 0, 1,  10, 3, 0,  25, 25, 0, 1,  10,
    };
    return runTriangleProgram(
        globalSession,
        device,
        queue,
        repositoryRoot,
        "triangle-attributes-flags",
        description,
        kExpected,
        sizeof(kExpected) / sizeof(kExpected[0]),
        10,
        MTLAccelerationStructureInstanceOptionNone);
}

bool runStageInputState(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/shaders/stage-input-state.slang",
        "Schema"};
    static const uint32_t kExpected[] = {
        1, 1, 1000,   50,  100, 25, 100, 200, 50, 0, 0, 0, 17, 1, 0, 2,
        2, 1, 100000, 300, 100, 0,  0,   0,   0,  0, 0, 0, 0,  1, 1, 2,
    };
    MTLPackedFloat4x3 transform = {};
    transform.columns[0] = {2.0f, 0.0f, 0.0f};
    transform.columns[1] = {0.0f, 1.0f, 0.0f};
    transform.columns[2] = {0.0f, 0.0f, 1.0f};
    transform.columns[3] = {0.0f, 0.0f, 0.0f};
    return runTriangleProgram(
        globalSession,
        device,
        queue,
        repositoryRoot,
        "stage-input-state",
        description,
        kExpected,
        sizeof(kExpected) / sizeof(kExpected[0]),
        2,
        MTLAccelerationStructureInstanceOptionTriangleFrontFacingWindingCounterClockwise |
            MTLAccelerationStructureInstanceOptionDisableTriangleCulling,
        17,
        &transform);
}

bool runCallableRecord(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/shaders/callable-record.slang",
        "Schema"};
    NativeProgram program = {};
    if (!createProgram(globalSession, device, repositoryRoot, description, program))
        return false;

    const uint32_t factor = 7;
    const RecordInitializer record = {
        RecordSection::Callable,
        0,
        0,
        0,
        &factor,
        sizeof(factor),
    };
    const RecordBufferDescription recordDescription = {nullptr, 0, &record, 1};
    id<MTLBuffer> records = createRecords(device, program, recordDescription);
    if (!records)
        return false;
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(uint32_t) * 2
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(device, queue, program, nil, programResources, records, results, 1, false, true))
        return false;
    static const uint32_t kExpected[] = {22, 1};
    return validateResults("callable-record", results, kExpected, 2);
}

bool runProceduralHitFilter(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/shaders/procedural-hit-filter.slang",
        "Schema"};
    NativeProgram program = {};
    if (!createProgram(globalSession, device, repositoryRoot, description, program))
        return false;

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalBoundingBoxScene(device, queue, scene, &sceneError))
        return fail(sceneError);

    id<MTLBuffer> records = createDefaultTraceRecords(device, program);
    if (!records)
        return false;
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    static const uint32_t kExpected[] = {
        3, 9, 2, 5, 2, 0, 0, 5, 3, 7, 1, 5, 3, 9, 1, 5, 2, 0, 0, 5,
    };
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(kExpected)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            5,
            false,
            false))
        return false;
    return validateResults(
        "procedural-hit-filter",
        results,
        kExpected,
        sizeof(kExpected) / sizeof(kExpected[0]));
}

bool runOpaqueIntersectionFunctions(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/metal/opaque-intersection-functions.slang",
        "Schema"};
    NativeProgram program = {};
    if (!createProgram(globalSession, device, repositoryRoot, description, program))
        return false;

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalBoundingBoxScene(device, queue, scene, &sceneError))
        return fail(sceneError);

    id<MTLBuffer> records = createDefaultTraceRecords(device, program);
    if (!records)
        return false;
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(uint32_t)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            1,
            false,
            false))
    {
        return false;
    }
    static const uint32_t kExpected[] = {7};
    return validateResults("opaque-intersection-functions", results, kExpected, 1);
}

bool runCurveHitFilter(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/metal/curve-hit-filter.slang",
        "Schema"};
    NativeProgram program = {};
    if (!createProgram(globalSession, device, repositoryRoot, description, program))
        return false;

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalCurveScene(device, queue, scene, &sceneError))
        return fail(sceneError);

    id<MTLBuffer> records = createDefaultTraceRecords(device, program);
    if (!records)
        return false;
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    static const uint32_t kExpected[] = {1, 1, 1, 2, 0, 0};
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(kExpected)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            2,
            false,
            false))
        return false;
    return validateResults(
        "curve-hit-filter",
        results,
        kExpected,
        sizeof(kExpected) / sizeof(kExpected[0]));
}

bool runMultilevelHit(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/metal/multilevel-hit.slang",
        "Schema"};
    NativeProgram program = {};
    if (!createProgram(globalSession, device, repositoryRoot, description, program))
        return false;

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalMultilevelScene(device, queue, scene, &sceneError))
        return fail(sceneError);

    // The leaf instance id can be either zero or one, and both select physical hit record zero.
    id<MTLBuffer> records = createDefaultTraceRecords(device, program, 2);
    if (!records)
        return false;
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    static const uint32_t kExpected[] = {1, 2};
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(kExpected)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            2,
            false,
            false))
        return false;
    return validateResults("multilevel-hit", results, kExpected, 2);
}

bool runMotionTime(
    slang::IGlobalSession* globalSession,
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    NSString* repositoryRoot)
{
    ProgramDescription description = {
        "tests/ray-tracing-2/runtime/metal/motion-time.slang",
        "Schema"};
    NativeProgram program = {};
    if (!createProgram(globalSession, device, repositoryRoot, description, program))
        return false;

    MetalRayTracingScene scene = {};
    NSString* sceneError = nil;
    if (!buildMetalPrimitiveMotionScene(device, queue, scene, &sceneError))
        return fail(sceneError);

    id<MTLBuffer> records = createDefaultTraceRecords(device, program);
    if (!records)
        return false;
    id<MTLBuffer> programResources = createProgramResourceBuffer(device, program, records);
    static const uint32_t kExpected[] = {1, 0, 1, 1000, 2, 500};
    id<MTLBuffer> results = [device newBufferWithLength:sizeof(kExpected)
                                                options:MTLResourceStorageModeShared];
    if (!dispatch(
            device,
            queue,
            program,
            scene.instanceAccelerationStructure,
            programResources,
            records,
            results,
            3,
            false,
            false))
        return false;
    return validateResults("motion-time", results, kExpected, 6);
}

} // namespace

bool runMetalStructuralRayTracingTests(const char* repositoryRootPath)
{
    @autoreleasepool
    {
        ComPtr<slang::IGlobalSession> globalSession;
        if (SLANG_FAILED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())))
        {
            return failDiagnostics("creating the Slang global session", nullptr);
        }

        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (!device)
            return fail(@"no Metal device is available");
        if (![device supportsRaytracing])
            return fail(@"the Metal device does not support ray tracing");
        if (device.argumentBuffersSupport != MTLArgumentBuffersTier2)
            return fail(@"the Metal device does not support tier-2 argument buffers");

        id<MTLCommandQueue> queue = [device newCommandQueue];
        if (!queue)
            return fail(@"failed to create a Metal command queue");

        NSString* repositoryRoot = [NSString stringWithUTF8String:repositoryRootPath];
        return runTriangleHitMiss(globalSession, device, queue, repositoryRoot) &&
               runProceduralHitFilter(globalSession, device, queue, repositoryRoot) &&
               runOpaqueIntersectionFunctions(globalSession, device, queue, repositoryRoot) &&
               runCallableRecord(globalSession, device, queue, repositoryRoot) &&
               runRecursiveTrace(globalSession, device, queue, repositoryRoot) &&
               runRepeatedRecords(globalSession, device, queue, repositoryRoot) &&
               runMultiplePayloads(globalSession, device, queue, repositoryRoot) &&
               runTriangleAttributesFlags(globalSession, device, queue, repositoryRoot) &&
               runStageInputState(globalSession, device, queue, repositoryRoot) &&
               runCurveHitFilter(globalSession, device, queue, repositoryRoot) &&
               runMultilevelHit(globalSession, device, queue, repositoryRoot) &&
               runMotionTime(globalSession, device, queue, repositoryRoot);
    }
}
