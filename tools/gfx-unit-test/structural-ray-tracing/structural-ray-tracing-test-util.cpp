#include "structural-ray-tracing-test-util.h"

#include "structural-ray-tracing-scenes.h"

#include <cstring>
#include <slang-rhi/shader-cursor.h>
#include <vector>

using namespace rhi;
using namespace Slang;

namespace gfx_test
{

namespace
{

struct EntryDesc
{
    // The module uses this name to find the source declaration.
    const char* sourceName;
    SlangStage stage;
    // When present, this is the target-safe name used after linking and in the native SBT.
    const char* linkedName = nullptr;
};

// Creates a linked RHI program from an already loaded module. Keeping module loading separate lets
// callers inspect structural reflection and select the synthesized entry points before composition.
Result loadProgram(
    IDevice* device,
    slang::IModule* module,
    const EntryDesc* entries,
    Index entryCount,
    IShaderProgram** outProgram)
{
    auto slangSession = device->getSlangSession();
    ComPtr<slang::IBlob> diagnostics;
    std::vector<ComPtr<slang::IComponentType>> entryPointComponents;
    std::vector<slang::IComponentType*> components;
    components.push_back(module);
    for (Index i = 0; i < entryCount; ++i)
    {
        auto& entryDesc = entries[i];
        ComPtr<slang::IEntryPoint> entryPoint;
        auto result = module->findAndCheckEntryPoint(
            entryDesc.sourceName,
            entryDesc.stage,
            entryPoint.writeRef(),
            diagnostics.writeRef());
        diagnoseIfNeeded(diagnostics);
        SLANG_RETURN_ON_FAIL(result);

        ComPtr<slang::IComponentType> entryPointComponent;
        if (entryDesc.linkedName)
        {
            SLANG_RETURN_ON_FAIL(
                entryPoint->renameEntryPoint(entryDesc.linkedName, entryPointComponent.writeRef()));
        }
        else
        {
            entryPointComponent = entryPoint;
        }
        entryPointComponents.push_back(entryPointComponent);
        components.push_back(entryPointComponent);
    }

    ComPtr<slang::IComponentType> composedProgram;
    auto result = slangSession->createCompositeComponentType(
        components.data(),
        components.size(),
        composedProgram.writeRef(),
        diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics);
    SLANG_RETURN_ON_FAIL(result);

    ComPtr<slang::IComponentType> linkedProgram;
    result = composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics);
    SLANG_RETURN_ON_FAIL(result);

    ShaderProgramDesc programDesc = {};
    programDesc.slangGlobalScope = linkedProgram;
    result = device->createShaderProgram(programDesc, outProgram, diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics);
    return result;
}

// Loads a module and creates a linked RHI program for tests that use fixed entry-point names.
Result loadProgram(
    IDevice* device,
    const char* moduleName,
    const EntryDesc* entries,
    Index entryCount,
    IShaderProgram** outProgram)
{
    auto slangSession = device->getSlangSession();
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(slangSession->loadModule(moduleName, diagnostics.writeRef()));
    diagnoseIfNeeded(diagnostics);
    if (!module)
        return SLANG_FAIL;

    return loadProgram(device, module, entries, entryCount, outProgram);
}

// Finds the schema partition by its stable, qualified payload type name. Function indices are only
// meaningful within this partition, so callers must resolve the partition before interpreting a
// hit or miss index.
slang::RayTracingPayloadReflection* findPayloadPartition(
    slang::TraceProgramSchemaReflection* schema,
    const char* payloadTypeName)
{
    for (SlangUInt i = 0; i < schema->getPayloadCount(); ++i)
    {
        auto payload = schema->getPayload(i);
        auto payloadType = payload ? payload->getType() : nullptr;
        ComPtr<ISlangBlob> reflectedName;
        if (payloadType && SLANG_SUCCEEDED(payloadType->getFullName(reflectedName.writeRef())) &&
            reflectedName &&
            std::strcmp(
                static_cast<const char*>(reflectedName->getBufferPointer()),
                payloadTypeName) == 0)
        {
            return payload;
        }
    }
    return nullptr;
}

} // namespace

ComPtr<IDevice> createStructuralRayTracingTestDevice(
    UnitTestContext* context,
    DeviceType deviceType)
{
    if (!deviceTypeInEnabledApis(deviceType, context->enabledApis))
    {
        SLANG_IGNORE_TEST;
    }

    DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = deviceType;
    deviceDesc.slang.slangGlobalSession = context->slangGlobalSession;

    auto searchPaths = getSlangSearchPaths();
    searchPaths.add("../../tests/ray-tracing-2/runtime/shaders");
    searchPaths.add("tests/ray-tracing-2/runtime/shaders");
    deviceDesc.slang.searchPaths = searchPaths.getBuffer();
    deviceDesc.slang.searchPathCount = searchPaths.getCount();

    slang::CompilerOptionEntry options[2] = {};
    options[0].name = slang::CompilerOptionName::EmitSpirvDirectly;
    options[0].value.kind = slang::CompilerOptionValueKind::Int;
    options[0].value.intValue0 = 1;
    options[1].name = slang::CompilerOptionName::ExperimentalFeature;
    options[1].value.kind = slang::CompilerOptionValueKind::Int;
    options[1].value.intValue0 = 1;
    deviceDesc.slang.compilerOptionEntries = options;
    deviceDesc.slang.compilerOptionEntryCount = SLANG_COUNT_OF(options);

    if (context->enableDebugLayers)
    {
        deviceDesc.enableValidation = true;
        deviceDesc.debugCallback = context->debugCallback;
        getRHI()->enableDebugLayers();
    }

    ComPtr<IDevice> device;
    if (SLANG_FAILED(getRHI()->createDevice(deviceDesc, device.writeRef())))
    {
        SLANG_IGNORE_TEST;
    }
    return device;
}

void runStructuralRayTracingTriangleHitMiss(IDevice* device)
{
    if (!device->hasFeature(Feature::RayTracing))
    {
        SLANG_IGNORE_TEST;
    }

    auto queue = device->getQueue(QueueType::Graphics);
    SLANG_CHECK_ABORT(queue != nullptr);
    StructuralRayTracingTriangleScene scene(device, queue);

    ComPtr<IShaderProgram> program;
    static const EntryDesc kEntries[] = {
        {"main", SLANG_STAGE_RAY_GENERATION},
        {"RuntimeClosestHit", SLANG_STAGE_CLOSEST_HIT},
        {"RuntimeMiss", SLANG_STAGE_MISS},
    };
    GFX_CHECK_CALL_ABORT(loadProgram(
        device,
        "triangle-hit-miss",
        kEntries,
        SLANG_COUNT_OF(kEntries),
        program.writeRef()));

    static const char* kHitGroupNames[] = {"hitGroup0"};
    HitGroupDesc hitGroup = {};
    hitGroup.hitGroupName = kHitGroupNames[0];
    hitGroup.closestHitEntryPoint = "RuntimeClosestHit";

    RayTracingPipelineDesc pipelineDesc = {};
    pipelineDesc.program = program;
    pipelineDesc.hitGroups = &hitGroup;
    pipelineDesc.hitGroupCount = 1;
    pipelineDesc.maxRecursion = 1;
    pipelineDesc.maxRayPayloadSize = sizeof(uint32_t) * 2;
    pipelineDesc.maxAttributeSizeInBytes = sizeof(float) * 2;

    ComPtr<IRayTracingPipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createRayTracingPipeline(pipelineDesc, pipeline.writeRef()));

    static const char* kRayGenerationNames[] = {"main"};
    static const char* kMissNames[] = {"RuntimeMiss"};
    ShaderTableDesc shaderTableDesc = {};
    shaderTableDesc.program = program;
    shaderTableDesc.rayGenShaderCount = SLANG_COUNT_OF(kRayGenerationNames);
    shaderTableDesc.rayGenShaderEntryPointNames = kRayGenerationNames;
    shaderTableDesc.missShaderCount = SLANG_COUNT_OF(kMissNames);
    shaderTableDesc.missShaderEntryPointNames = kMissNames;
    shaderTableDesc.hitGroupCount = SLANG_COUNT_OF(kHitGroupNames);
    shaderTableDesc.hitGroupNames = kHitGroupNames;

    ComPtr<IShaderTable> shaderTable;
    GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, shaderTable.writeRef()));

    BufferDesc resultDesc = {};
    resultDesc.size = sizeof(StructuralRayTracingRuntimeResult) * 2;
    resultDesc.elementSize = sizeof(StructuralRayTracingRuntimeResult);
    resultDesc.usage = BufferUsage::UnorderedAccess | BufferUsage::CopySource;
    resultDesc.defaultState = ResourceState::UnorderedAccess;
    auto results = device->createBuffer(resultDesc);
    SLANG_CHECK_ABORT(results != nullptr);

    auto commandEncoder = queue->createCommandEncoder();
    auto passEncoder = commandEncoder->beginRayTracingPass();
    auto rootObject = passEncoder->bindPipeline(pipeline, shaderTable);
    ShaderCursor root(rootObject);
    GFX_CHECK_CALL_ABORT(root["frame"]["scene"].setBinding(Binding(scene.topLevel)));
    GFX_CHECK_CALL_ABORT(root["frame"]["results"].setBinding(Binding(results)));
    passEncoder->dispatchRays(0, 2, 1, 1);
    passEncoder->end();
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    ComPtr<ISlangBlob> resultBlob;
    GFX_CHECK_CALL_ABORT(device->readBuffer(results, 0, resultDesc.size, resultBlob.writeRef()));
    auto actual =
        static_cast<const StructuralRayTracingRuntimeResult*>(resultBlob->getBufferPointer());
    static const StructuralRayTracingRuntimeResult kExpected[] = {
        {1, 0, 2},
        {2, 0xffffffff, 2},
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kExpected); ++i)
    {
        SLANG_CHECK(actual[i].stage == kExpected[i].stage);
        SLANG_CHECK(actual[i].primitiveIndex == kExpected[i].primitiveIndex);
        SLANG_CHECK(actual[i].dispatchWidth == kExpected[i].dispatchWidth);
    }
}

void runStructuralRayTracingProceduralHitFilter(IDevice* device)
{
    if (!device->hasFeature(Feature::RayTracing))
    {
        SLANG_IGNORE_TEST;
    }

    auto queue = device->getQueue(QueueType::Graphics);
    SLANG_CHECK_ABORT(queue != nullptr);
    StructuralRayTracingProceduralScene scene(device, queue);

    static const EntryDesc kEntries[] = {
        {"main", SLANG_STAGE_RAY_GENERATION},
        {"RuntimeIntersection", SLANG_STAGE_INTERSECTION},
        {"RuntimeAnyHit", SLANG_STAGE_ANY_HIT},
        {"RuntimeClosestHit", SLANG_STAGE_CLOSEST_HIT},
        {"RuntimeMiss", SLANG_STAGE_MISS},
    };
    ComPtr<IShaderProgram> program;
    GFX_CHECK_CALL_ABORT(loadProgram(
        device,
        "procedural-hit-filter",
        kEntries,
        SLANG_COUNT_OF(kEntries),
        program.writeRef()));

    static const char* kHitGroupNames[] = {"proceduralHitGroup"};
    HitGroupDesc hitGroup = {};
    hitGroup.hitGroupName = kHitGroupNames[0];
    hitGroup.intersectionEntryPoint = "RuntimeIntersection";
    hitGroup.anyHitEntryPoint = "RuntimeAnyHit";
    hitGroup.closestHitEntryPoint = "RuntimeClosestHit";

    RayTracingPipelineDesc pipelineDesc = {};
    pipelineDesc.program = program;
    pipelineDesc.hitGroups = &hitGroup;
    pipelineDesc.hitGroupCount = 1;
    pipelineDesc.maxRecursion = 1;
    pipelineDesc.maxRayPayloadSize = sizeof(uint32_t) * 3;
    pipelineDesc.maxAttributeSizeInBytes = sizeof(uint32_t);

    ComPtr<IRayTracingPipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createRayTracingPipeline(pipelineDesc, pipeline.writeRef()));

    static const char* kRayGenerationNames[] = {"main"};
    static const char* kMissNames[] = {"RuntimeMiss"};
    ShaderTableDesc shaderTableDesc = {};
    shaderTableDesc.program = program;
    shaderTableDesc.rayGenShaderCount = SLANG_COUNT_OF(kRayGenerationNames);
    shaderTableDesc.rayGenShaderEntryPointNames = kRayGenerationNames;
    shaderTableDesc.missShaderCount = SLANG_COUNT_OF(kMissNames);
    shaderTableDesc.missShaderEntryPointNames = kMissNames;
    shaderTableDesc.hitGroupCount = SLANG_COUNT_OF(kHitGroupNames);
    shaderTableDesc.hitGroupNames = kHitGroupNames;

    ComPtr<IShaderTable> shaderTable;
    GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, shaderTable.writeRef()));

    BufferDesc resultDesc = {};
    resultDesc.size = sizeof(StructuralRayTracingProceduralResult) * 5;
    resultDesc.elementSize = sizeof(StructuralRayTracingProceduralResult);
    resultDesc.usage = BufferUsage::UnorderedAccess | BufferUsage::CopySource;
    resultDesc.defaultState = ResourceState::UnorderedAccess;
    auto results = device->createBuffer(resultDesc);
    SLANG_CHECK_ABORT(results != nullptr);

    auto commandEncoder = queue->createCommandEncoder();
    auto passEncoder = commandEncoder->beginRayTracingPass();
    auto rootObject = passEncoder->bindPipeline(pipeline, shaderTable);
    ShaderCursor root(rootObject);
    GFX_CHECK_CALL_ABORT(root["scene"].setBinding(Binding(scene.topLevel)));
    GFX_CHECK_CALL_ABORT(root["results"].setBinding(Binding(results)));
    passEncoder->dispatchRays(0, 5, 1, 1);
    passEncoder->end();
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    ComPtr<ISlangBlob> resultBlob;
    GFX_CHECK_CALL_ABORT(device->readBuffer(results, 0, resultDesc.size, resultBlob.writeRef()));
    auto actual =
        static_cast<const StructuralRayTracingProceduralResult*>(resultBlob->getBufferPointer());
    static const StructuralRayTracingProceduralResult kExpected[] = {
        {3, 9, 2, 5},
        {2, 0, 0, 5},
        {3, 7, 1, 5},
        {3, 9, 1, 5},
        {2, 0, 0, 5},
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kExpected); ++i)
    {
        SLANG_CHECK(actual[i].stage == kExpected[i].stage);
        SLANG_CHECK(actual[i].candidate == kExpected[i].candidate);
        SLANG_CHECK(actual[i].anyHitCount == kExpected[i].anyHitCount);
        SLANG_CHECK(actual[i].dispatchWidth == kExpected[i].dispatchWidth);
    }
}

void runStructuralRayTracingCallableRecord(IDevice* device)
{
    if (!device->hasFeature(Feature::RayTracing))
    {
        SLANG_IGNORE_TEST;
    }

    auto queue = device->getQueue(QueueType::Graphics);
    SLANG_CHECK_ABORT(queue != nullptr);

    static const EntryDesc kEntries[] = {
        {"main", SLANG_STAGE_RAY_GENERATION},
        {"RuntimeCallable", SLANG_STAGE_CALLABLE},
    };
    ComPtr<IShaderProgram> program;
    GFX_CHECK_CALL_ABORT(loadProgram(
        device,
        "callable-record",
        kEntries,
        SLANG_COUNT_OF(kEntries),
        program.writeRef()));

    RayTracingPipelineDesc pipelineDesc = {};
    pipelineDesc.program = program;
    pipelineDesc.maxRecursion = 1;
    pipelineDesc.maxRayPayloadSize = sizeof(uint32_t);

    ComPtr<IRayTracingPipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createRayTracingPipeline(pipelineDesc, pipeline.writeRef()));

    static const char* kRayGenerationNames[] = {"main"};
    static const char* kCallableNames[] = {"RuntimeCallable"};
    ShaderRecordOverwrite callableRecord = {};
    callableRecord.offset = 32;
    callableRecord.size = sizeof(uint32_t);
    callableRecord.data[0] = 7;

    ShaderTableDesc shaderTableDesc = {};
    shaderTableDesc.program = program;
    shaderTableDesc.rayGenShaderCount = SLANG_COUNT_OF(kRayGenerationNames);
    shaderTableDesc.rayGenShaderEntryPointNames = kRayGenerationNames;
    shaderTableDesc.callableShaderCount = SLANG_COUNT_OF(kCallableNames);
    shaderTableDesc.callableShaderEntryPointNames = kCallableNames;
    shaderTableDesc.callableShaderRecordOverwrites = &callableRecord;

    ComPtr<IShaderTable> shaderTable;
    GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, shaderTable.writeRef()));

    BufferDesc resultDesc = {};
    resultDesc.size = sizeof(StructuralRayTracingCallableResult);
    resultDesc.elementSize = sizeof(StructuralRayTracingCallableResult);
    resultDesc.usage = BufferUsage::UnorderedAccess | BufferUsage::CopySource;
    resultDesc.defaultState = ResourceState::UnorderedAccess;
    auto results = device->createBuffer(resultDesc);
    SLANG_CHECK_ABORT(results != nullptr);

    auto commandEncoder = queue->createCommandEncoder();
    auto passEncoder = commandEncoder->beginRayTracingPass();
    auto rootObject = passEncoder->bindPipeline(pipeline, shaderTable);
    ShaderCursor root(rootObject);
    GFX_CHECK_CALL_ABORT(root["results"].setBinding(Binding(results)));
    passEncoder->dispatchRays(0, 1, 1, 1);
    passEncoder->end();
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    ComPtr<ISlangBlob> resultBlob;
    GFX_CHECK_CALL_ABORT(device->readBuffer(results, 0, resultDesc.size, resultBlob.writeRef()));
    auto actual =
        static_cast<const StructuralRayTracingCallableResult*>(resultBlob->getBufferPointer());
    SLANG_CHECK(actual->value == 22);
    SLANG_CHECK(actual->dispatchWidth == 1);
}

void runStructuralRayTracingRecursiveTrace(IDevice* device)
{
    if (!device->hasFeature(Feature::RayTracing))
    {
        SLANG_IGNORE_TEST;
    }

    auto queue = device->getQueue(QueueType::Graphics);
    SLANG_CHECK_ABORT(queue != nullptr);
    StructuralRayTracingTriangleScene scene(device, queue);

    static const EntryDesc kEntries[] = {
        {"main", SLANG_STAGE_RAY_GENERATION},
        {"RuntimeClosestHit", SLANG_STAGE_CLOSEST_HIT},
        {"RuntimeMiss", SLANG_STAGE_MISS},
    };
    ComPtr<IShaderProgram> program;
    GFX_CHECK_CALL_ABORT(loadProgram(
        device,
        "recursive-trace",
        kEntries,
        SLANG_COUNT_OF(kEntries),
        program.writeRef()));

    static const char* kHitGroupNames[] = {"hitGroup0"};
    HitGroupDesc hitGroup = {};
    hitGroup.hitGroupName = kHitGroupNames[0];
    hitGroup.closestHitEntryPoint = "RuntimeClosestHit";

    RayTracingPipelineDesc pipelineDesc = {};
    pipelineDesc.program = program;
    pipelineDesc.hitGroups = &hitGroup;
    pipelineDesc.hitGroupCount = 1;
    pipelineDesc.maxRecursion = 2;
    pipelineDesc.maxRayPayloadSize = sizeof(uint32_t) * 2;
    pipelineDesc.maxAttributeSizeInBytes = sizeof(float) * 2;

    ComPtr<IRayTracingPipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createRayTracingPipeline(pipelineDesc, pipeline.writeRef()));

    static const char* kRayGenerationNames[] = {"main"};
    static const char* kMissNames[] = {"RuntimeMiss"};
    ShaderTableDesc shaderTableDesc = {};
    shaderTableDesc.program = program;
    shaderTableDesc.rayGenShaderCount = SLANG_COUNT_OF(kRayGenerationNames);
    shaderTableDesc.rayGenShaderEntryPointNames = kRayGenerationNames;
    shaderTableDesc.missShaderCount = SLANG_COUNT_OF(kMissNames);
    shaderTableDesc.missShaderEntryPointNames = kMissNames;
    shaderTableDesc.hitGroupCount = SLANG_COUNT_OF(kHitGroupNames);
    shaderTableDesc.hitGroupNames = kHitGroupNames;

    ComPtr<IShaderTable> shaderTable;
    GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, shaderTable.writeRef()));

    BufferDesc resultDesc = {};
    resultDesc.size = sizeof(StructuralRayTracingRecursiveResult) * 2;
    resultDesc.elementSize = sizeof(StructuralRayTracingRecursiveResult);
    resultDesc.usage = BufferUsage::UnorderedAccess | BufferUsage::CopySource;
    resultDesc.defaultState = ResourceState::UnorderedAccess;
    auto results = device->createBuffer(resultDesc);
    SLANG_CHECK_ABORT(results != nullptr);

    auto commandEncoder = queue->createCommandEncoder();
    auto passEncoder = commandEncoder->beginRayTracingPass();
    auto rootObject = passEncoder->bindPipeline(pipeline, shaderTable);
    ShaderCursor root(rootObject);
    GFX_CHECK_CALL_ABORT(root["scene"].setBinding(Binding(scene.topLevel)));
    GFX_CHECK_CALL_ABORT(root["results"].setBinding(Binding(results)));
    passEncoder->dispatchRays(0, 2, 1, 1);
    passEncoder->end();
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    ComPtr<ISlangBlob> resultBlob;
    GFX_CHECK_CALL_ABORT(device->readBuffer(results, 0, resultDesc.size, resultBlob.writeRef()));
    auto actual =
        static_cast<const StructuralRayTracingRecursiveResult*>(resultBlob->getBufferPointer());
    static const StructuralRayTracingRecursiveResult kExpected[] = {
        {21, 1, 2},
        {20, 0, 2},
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kExpected); ++i)
    {
        SLANG_CHECK(actual[i].stage == kExpected[i].stage);
        SLANG_CHECK(actual[i].depth == kExpected[i].depth);
        SLANG_CHECK(actual[i].dispatchWidth == kExpected[i].dispatchWidth);
    }
}

void runStructuralRayTracingRepeatedRecords(IDevice* device)
{
    if (!device->hasFeature(Feature::RayTracing))
    {
        SLANG_IGNORE_TEST;
    }

    auto queue = device->getQueue(QueueType::Graphics);
    SLANG_CHECK_ABORT(queue != nullptr);
    StructuralRayTracingTriangleScene scene(device, queue);

    auto slangSession = device->getSlangSession();
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(
        slangSession->loadModule("repeated-records", diagnostics.writeRef()));
    diagnoseIfNeeded(diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);

    auto schema = module->getLayout()->findTraceProgramSchema("Schema");
    SLANG_CHECK_ABORT(schema != nullptr);
    SLANG_CHECK_ABORT(schema->getPayloadCount() == 1);
    auto payload = schema->getPayload(0);
    SLANG_CHECK_ABORT(payload != nullptr);
    SLANG_CHECK_ABORT(payload->getHitGroupCount() == 1);
    SLANG_CHECK_ABORT(payload->getMissShaderCount() == 1);

    auto reflectedHitGroup = payload->getHitGroup(0);
    auto reflectedMissShader = payload->getMissShader(0);
    SLANG_CHECK_ABORT(reflectedHitGroup != nullptr);
    SLANG_CHECK_ABORT(reflectedMissShader != nullptr);
    auto reflectedClosestHit = reflectedHitGroup->getClosestHit();
    auto reflectedMiss = reflectedMissShader->getMiss();
    SLANG_CHECK_ABORT(reflectedClosestHit != nullptr);
    SLANG_CHECK_ABORT(reflectedMiss != nullptr);

    // Function indices identify structural functions, not physical SBT records. The host maps each
    // reflected function to its native pipeline name and may then repeat that name in any number of
    // physical records.
    static const char* kReflectedHitGroupNames[] = {"hitFunction0"};
    auto hitFunctionIndex = reflectedHitGroup->getFunctionIndex();
    auto missFunctionIndex = reflectedMissShader->getFunctionIndex();
    SLANG_CHECK_ABORT(hitFunctionIndex == 0);
    SLANG_CHECK_ABORT(missFunctionIndex == 0);
    const char* closestHitEntryPointName = reflectedClosestHit->getEntryPointName();
    const char* missEntryPointName = reflectedMiss->getEntryPointName();
    SLANG_CHECK_ABORT(closestHitEntryPointName != nullptr);
    SLANG_CHECK_ABORT(missEntryPointName != nullptr);
    const char* reflectedMissEntryPointNames[] = {missEntryPointName};

    const EntryDesc kEntries[] = {
        {"main", SLANG_STAGE_RAY_GENERATION},
        {closestHitEntryPointName, SLANG_STAGE_CLOSEST_HIT},
        {missEntryPointName, SLANG_STAGE_MISS},
    };
    ComPtr<IShaderProgram> program;
    GFX_CHECK_CALL_ABORT(
        loadProgram(device, module, kEntries, SLANG_COUNT_OF(kEntries), program.writeRef()));

    HitGroupDesc hitGroup = {};
    hitGroup.hitGroupName = kReflectedHitGroupNames[hitFunctionIndex];
    hitGroup.closestHitEntryPoint = closestHitEntryPointName;

    RayTracingPipelineDesc pipelineDesc = {};
    pipelineDesc.program = program;
    pipelineDesc.hitGroups = &hitGroup;
    pipelineDesc.hitGroupCount = 1;
    pipelineDesc.maxRecursion = 1;
    pipelineDesc.maxRayPayloadSize = sizeof(uint32_t) * 2;
    pipelineDesc.maxAttributeSizeInBytes = sizeof(float) * 2;

    ComPtr<IRayTracingPipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createRayTracingPipeline(pipelineDesc, pipeline.writeRef()));

    static const char* kRayGenerationNames[] = {"main"};
    const char* kHitGroupNames[] = {
        kReflectedHitGroupNames[hitFunctionIndex],
        kReflectedHitGroupNames[hitFunctionIndex],
    };
    const char* kMissNames[] = {
        reflectedMissEntryPointNames[missFunctionIndex],
        reflectedMissEntryPointNames[missFunctionIndex],
    };
    uint32_t hitRecordValues[2] = {};
    uint32_t missRecordValues[2] = {};
    ShaderRecordData hitRecords[2] = {};
    ShaderRecordData missRecords[2] = {};
    for (Index i = 0; i < 2; ++i)
    {
        hitRecordValues[i] = uint32_t((i + 1) * 100);
        missRecordValues[i] = uint32_t((i + 3) * 100);
        hitRecords[i] = {&hitRecordValues[i], sizeof(hitRecordValues[i])};
        missRecords[i] = {&missRecordValues[i], sizeof(missRecordValues[i])};
    }

    ShaderTableDesc shaderTableDesc = {};
    shaderTableDesc.program = program;
    shaderTableDesc.rayGenShaderCount = SLANG_COUNT_OF(kRayGenerationNames);
    shaderTableDesc.rayGenShaderEntryPointNames = kRayGenerationNames;
    shaderTableDesc.missShaderCount = SLANG_COUNT_OF(kMissNames);
    shaderTableDesc.missShaderEntryPointNames = kMissNames;
    shaderTableDesc.missShaderRecordData = missRecords;
    shaderTableDesc.hitGroupCount = SLANG_COUNT_OF(kHitGroupNames);
    shaderTableDesc.hitGroupNames = kHitGroupNames;
    shaderTableDesc.hitGroupRecordData = hitRecords;

    ComPtr<IShaderTable> shaderTable;
    GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, shaderTable.writeRef()));

    BufferDesc resultDesc = {};
    resultDesc.size = sizeof(StructuralRayTracingRepeatedRecordResult) * 4;
    resultDesc.elementSize = sizeof(StructuralRayTracingRepeatedRecordResult);
    resultDesc.usage = BufferUsage::UnorderedAccess | BufferUsage::CopySource;
    resultDesc.defaultState = ResourceState::UnorderedAccess;
    auto results = device->createBuffer(resultDesc);
    SLANG_CHECK_ABORT(results != nullptr);

    auto commandEncoder = queue->createCommandEncoder();
    auto passEncoder = commandEncoder->beginRayTracingPass();
    auto rootObject = passEncoder->bindPipeline(pipeline, shaderTable);
    ShaderCursor root(rootObject);
    GFX_CHECK_CALL_ABORT(root["scene"].setBinding(Binding(scene.topLevel)));
    GFX_CHECK_CALL_ABORT(root["results"].setBinding(Binding(results)));
    passEncoder->dispatchRays(0, 4, 1, 1);
    passEncoder->end();
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    ComPtr<ISlangBlob> resultBlob;
    GFX_CHECK_CALL_ABORT(device->readBuffer(results, 0, resultDesc.size, resultBlob.writeRef()));
    auto actual = static_cast<const StructuralRayTracingRepeatedRecordResult*>(
        resultBlob->getBufferPointer());
    static const StructuralRayTracingRepeatedRecordResult kExpected[] = {
        {10, 100, 4},
        {10, 200, 4},
        {20, 300, 4},
        {20, 400, 4},
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kExpected); ++i)
    {
        SLANG_CHECK(actual[i].stage == kExpected[i].stage);
        SLANG_CHECK(actual[i].recordValue == kExpected[i].recordValue);
        SLANG_CHECK(actual[i].dispatchWidth == kExpected[i].dispatchWidth);
    }
}

void runStructuralRayTracingMultiplePayloads(IDevice* device)
{
    if (!device->hasFeature(Feature::RayTracing))
    {
        SLANG_IGNORE_TEST;
    }

    auto queue = device->getQueue(QueueType::Graphics);
    SLANG_CHECK_ABORT(queue != nullptr);
    StructuralRayTracingTriangleScene scene(device, queue);

    auto slangSession = device->getSlangSession();
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(
        slangSession->loadModule("multiple-payloads", diagnostics.writeRef()));
    diagnoseIfNeeded(diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);

    auto schema = module->getLayout()->findTraceProgramSchema("Schema");
    SLANG_CHECK_ABORT(schema != nullptr);
    SLANG_CHECK_ABORT(schema->getPayloadCount() == 2);

    auto radiancePartition = findPayloadPartition(schema, "RadiancePayload");
    auto shadowPartition = findPayloadPartition(schema, "ShadowPayload");
    SLANG_CHECK_ABORT(radiancePartition != nullptr);
    SLANG_CHECK_ABORT(shadowPartition != nullptr);
    SLANG_CHECK_ABORT(radiancePartition->getHitGroupCount() == 1);
    SLANG_CHECK_ABORT(radiancePartition->getMissShaderCount() == 1);
    SLANG_CHECK_ABORT(shadowPartition->getHitGroupCount() == 1);
    SLANG_CHECK_ABORT(shadowPartition->getMissShaderCount() == 1);

    auto radianceHit = radiancePartition->getHitGroup(0);
    auto radianceMiss = radiancePartition->getMissShader(0);
    auto shadowHit = shadowPartition->getHitGroup(0);
    auto shadowMiss = shadowPartition->getMissShader(0);
    SLANG_CHECK_ABORT(radianceHit != nullptr);
    SLANG_CHECK_ABORT(radianceMiss != nullptr);
    SLANG_CHECK_ABORT(shadowHit != nullptr);
    SLANG_CHECK_ABORT(shadowMiss != nullptr);

    auto radianceClosestHit = radianceHit->getClosestHit();
    auto radianceMissStage = radianceMiss->getMiss();
    auto shadowClosestHit = shadowHit->getClosestHit();
    auto shadowMissStage = shadowMiss->getMiss();
    SLANG_CHECK_ABORT(radianceClosestHit != nullptr);
    SLANG_CHECK_ABORT(radianceMissStage != nullptr);
    SLANG_CHECK_ABORT(shadowClosestHit != nullptr);
    SLANG_CHECK_ABORT(shadowMissStage != nullptr);

    auto radianceHitFunctionIndex = radianceHit->getFunctionIndex();
    auto radianceMissFunctionIndex = radianceMiss->getFunctionIndex();
    auto shadowHitFunctionIndex = shadowHit->getFunctionIndex();
    auto shadowMissFunctionIndex = shadowMiss->getFunctionIndex();
    SLANG_CHECK_ABORT(radianceHitFunctionIndex == 0);
    SLANG_CHECK_ABORT(radianceMissFunctionIndex == 0);
    SLANG_CHECK_ABORT(shadowHitFunctionIndex == 0);
    SLANG_CHECK_ABORT(shadowMissFunctionIndex == 0);

    const char* radianceClosestHitLinkedName = radianceClosestHit->getEntryPointName();
    const char* radianceMissLinkedName = radianceMissStage->getEntryPointName();
    const char* shadowClosestHitLinkedName = shadowClosestHit->getEntryPointName();
    const char* shadowMissLinkedName = shadowMissStage->getEntryPointName();
    SLANG_CHECK_ABORT(radianceClosestHitLinkedName != nullptr);
    SLANG_CHECK_ABORT(radianceMissLinkedName != nullptr);
    SLANG_CHECK_ABORT(shadowClosestHitLinkedName != nullptr);
    SLANG_CHECK_ABORT(shadowMissLinkedName != nullptr);

    // Stage lookup consumes the qualified source type name. The linked pipeline and SBT consume
    // the distinct, target-safe entry-point name reported by structural stage reflection.
    ComPtr<ISlangBlob> radianceClosestHitSourceName;
    ComPtr<ISlangBlob> radianceMissSourceName;
    ComPtr<ISlangBlob> shadowClosestHitSourceName;
    ComPtr<ISlangBlob> shadowMissSourceName;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        radianceClosestHit->getType()->getFullName(radianceClosestHitSourceName.writeRef())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        radianceMissStage->getType()->getFullName(radianceMissSourceName.writeRef())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        shadowClosestHit->getType()->getFullName(shadowClosestHitSourceName.writeRef())));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(shadowMissStage->getType()->getFullName(shadowMissSourceName.writeRef())));
    SLANG_CHECK_ABORT(
        std::strcmp(
            static_cast<const char*>(radianceClosestHitSourceName->getBufferPointer()),
            radianceClosestHitLinkedName) != 0);

    const EntryDesc kEntries[] = {
        {"rayGenerationMain", SLANG_STAGE_RAY_GENERATION},
        {static_cast<const char*>(radianceClosestHitSourceName->getBufferPointer()),
         SLANG_STAGE_CLOSEST_HIT,
         radianceClosestHitLinkedName},
        {static_cast<const char*>(radianceMissSourceName->getBufferPointer()),
         SLANG_STAGE_MISS,
         radianceMissLinkedName},
        {static_cast<const char*>(shadowClosestHitSourceName->getBufferPointer()),
         SLANG_STAGE_CLOSEST_HIT,
         shadowClosestHitLinkedName},
        {static_cast<const char*>(shadowMissSourceName->getBufferPointer()),
         SLANG_STAGE_MISS,
         shadowMissLinkedName},
    };
    ComPtr<IShaderProgram> program;
    GFX_CHECK_CALL_ABORT(
        loadProgram(device, module, kEntries, SLANG_COUNT_OF(kEntries), program.writeRef()));

    // Both payload partitions start their function indices at zero. Indexing separate host-side
    // name tables preserves that partition boundary before their hit and miss functions enter the
    // respective sections of one native SBT.
    const char* radianceHitGroupNamesByFunctionIndex[1] = {};
    const char* shadowHitGroupNamesByFunctionIndex[1] = {};
    const char* radianceMissNamesByFunctionIndex[1] = {};
    const char* shadowMissNamesByFunctionIndex[1] = {};
    radianceHitGroupNamesByFunctionIndex[radianceHitFunctionIndex] = "radianceHitFunction0";
    shadowHitGroupNamesByFunctionIndex[shadowHitFunctionIndex] = "shadowHitFunction0";
    radianceMissNamesByFunctionIndex[radianceMissFunctionIndex] = radianceMissLinkedName;
    shadowMissNamesByFunctionIndex[shadowMissFunctionIndex] = shadowMissLinkedName;

    HitGroupDesc hitGroups[2] = {};
    hitGroups[0].hitGroupName = radianceHitGroupNamesByFunctionIndex[radianceHitFunctionIndex];
    hitGroups[0].closestHitEntryPoint = radianceClosestHitLinkedName;
    hitGroups[1].hitGroupName = shadowHitGroupNamesByFunctionIndex[shadowHitFunctionIndex];
    hitGroups[1].closestHitEntryPoint = shadowClosestHitLinkedName;

    RayTracingPipelineDesc pipelineDesc = {};
    pipelineDesc.program = program;
    pipelineDesc.hitGroups = hitGroups;
    pipelineDesc.hitGroupCount = SLANG_COUNT_OF(hitGroups);
    pipelineDesc.maxRecursion = 1;
    pipelineDesc.maxRayPayloadSize = sizeof(uint32_t) * 3;
    pipelineDesc.maxAttributeSizeInBytes = sizeof(float) * 2;

    ComPtr<IRayTracingPipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createRayTracingPipeline(pipelineDesc, pipeline.writeRef()));

    static const char* kRayGenerationNames[] = {"rayGenerationMain"};
    // Deliberately reverse the payload order in the physical SBT. The shader's runtime selectors
    // use record zero for shadow rays and record one for radiance rays.
    const char* kHitGroupNames[] = {
        shadowHitGroupNamesByFunctionIndex[shadowHitFunctionIndex],
        radianceHitGroupNamesByFunctionIndex[radianceHitFunctionIndex],
    };
    const char* kMissNames[] = {
        shadowMissNamesByFunctionIndex[shadowMissFunctionIndex],
        radianceMissNamesByFunctionIndex[radianceMissFunctionIndex],
    };

    ShaderTableDesc shaderTableDesc = {};
    shaderTableDesc.program = program;
    shaderTableDesc.rayGenShaderCount = SLANG_COUNT_OF(kRayGenerationNames);
    shaderTableDesc.rayGenShaderEntryPointNames = kRayGenerationNames;
    shaderTableDesc.missShaderCount = SLANG_COUNT_OF(kMissNames);
    shaderTableDesc.missShaderEntryPointNames = kMissNames;
    shaderTableDesc.hitGroupCount = SLANG_COUNT_OF(kHitGroupNames);
    shaderTableDesc.hitGroupNames = kHitGroupNames;

    ComPtr<IShaderTable> shaderTable;
    GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, shaderTable.writeRef()));

    BufferDesc resultDesc = {};
    resultDesc.size = sizeof(StructuralRayTracingMultiplePayloadResult) * 4;
    resultDesc.elementSize = sizeof(StructuralRayTracingMultiplePayloadResult);
    resultDesc.usage = BufferUsage::UnorderedAccess | BufferUsage::CopySource;
    resultDesc.defaultState = ResourceState::UnorderedAccess;
    auto results = device->createBuffer(resultDesc);
    SLANG_CHECK_ABORT(results != nullptr);

    auto commandEncoder = queue->createCommandEncoder();
    auto passEncoder = commandEncoder->beginRayTracingPass();
    auto rootObject = passEncoder->bindPipeline(pipeline, shaderTable);
    ShaderCursor root(rootObject);
    GFX_CHECK_CALL_ABORT(root["scene"].setBinding(Binding(scene.topLevel)));
    GFX_CHECK_CALL_ABORT(root["results"].setBinding(Binding(results)));
    passEncoder->dispatchRays(0, 4, 1, 1);
    passEncoder->end();
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    ComPtr<ISlangBlob> resultBlob;
    GFX_CHECK_CALL_ABORT(device->readBuffer(results, 0, resultDesc.size, resultBlob.writeRef()));
    auto actual = static_cast<const StructuralRayTracingMultiplePayloadResult*>(
        resultBlob->getBufferPointer());
    static const StructuralRayTracingMultiplePayloadResult kExpected[] = {
        {10, 1101, 4},
        {20, 1, 4},
        {11, 202, 4},
        {21, 0, 4},
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kExpected); ++i)
    {
        SLANG_CHECK(actual[i].stage == kExpected[i].stage);
        SLANG_CHECK(actual[i].value == kExpected[i].value);
        SLANG_CHECK(actual[i].dispatchWidth == kExpected[i].dispatchWidth);
    }
}

void runStructuralRayTracingTriangleAttributesFlags(IDevice* device)
{
    if (!device->hasFeature(Feature::RayTracing))
    {
        SLANG_IGNORE_TEST;
    }

    auto queue = device->getQueue(QueueType::Graphics);
    SLANG_CHECK_ABORT(queue != nullptr);
    StructuralRayTracingTriangleScene scene(
        device,
        queue,
        AccelerationStructureInstanceFlags::None);

    static const EntryDesc kEntries[] = {
        {"main", SLANG_STAGE_RAY_GENERATION},
        {"RuntimeClosestHit", SLANG_STAGE_CLOSEST_HIT},
        {"RuntimeAnyHit", SLANG_STAGE_ANY_HIT},
        {"RuntimeMiss", SLANG_STAGE_MISS},
    };
    ComPtr<IShaderProgram> program;
    GFX_CHECK_CALL_ABORT(loadProgram(
        device,
        "triangle-attributes-flags",
        kEntries,
        SLANG_COUNT_OF(kEntries),
        program.writeRef()));

    HitGroupDesc hitGroup = {};
    hitGroup.hitGroupName = "hitGroup0";
    hitGroup.closestHitEntryPoint = "RuntimeClosestHit";
    hitGroup.anyHitEntryPoint = "RuntimeAnyHit";

    RayTracingPipelineDesc pipelineDesc = {};
    pipelineDesc.program = program;
    pipelineDesc.hitGroups = &hitGroup;
    pipelineDesc.hitGroupCount = 1;
    pipelineDesc.maxRecursion = 1;
    pipelineDesc.maxRayPayloadSize = sizeof(uint32_t) * 8;
    pipelineDesc.maxAttributeSizeInBytes = sizeof(float) * 2;

    ComPtr<IRayTracingPipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createRayTracingPipeline(pipelineDesc, pipeline.writeRef()));

    static const char* kRayGenerationNames[] = {"main"};
    static const char* kMissNames[] = {"RuntimeMiss"};
    static const char* kHitGroupNames[] = {"hitGroup0"};
    ShaderTableDesc shaderTableDesc = {};
    shaderTableDesc.program = program;
    shaderTableDesc.rayGenShaderCount = SLANG_COUNT_OF(kRayGenerationNames);
    shaderTableDesc.rayGenShaderEntryPointNames = kRayGenerationNames;
    shaderTableDesc.missShaderCount = SLANG_COUNT_OF(kMissNames);
    shaderTableDesc.missShaderEntryPointNames = kMissNames;
    shaderTableDesc.hitGroupCount = SLANG_COUNT_OF(kHitGroupNames);
    shaderTableDesc.hitGroupNames = kHitGroupNames;

    ComPtr<IShaderTable> shaderTable;
    GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, shaderTable.writeRef()));

    BufferDesc resultDesc = {};
    resultDesc.size = sizeof(StructuralRayTracingTriangleAttributesFlagsResult) * 10;
    resultDesc.elementSize = sizeof(StructuralRayTracingTriangleAttributesFlagsResult);
    resultDesc.usage = BufferUsage::UnorderedAccess | BufferUsage::CopySource;
    resultDesc.defaultState = ResourceState::UnorderedAccess;
    auto results = device->createBuffer(resultDesc);
    SLANG_CHECK_ABORT(results != nullptr);

    auto commandEncoder = queue->createCommandEncoder();
    auto passEncoder = commandEncoder->beginRayTracingPass();
    auto rootObject = passEncoder->bindPipeline(pipeline, shaderTable);
    ShaderCursor root(rootObject);
    GFX_CHECK_CALL_ABORT(root["scene"].setBinding(Binding(scene.topLevel)));
    GFX_CHECK_CALL_ABORT(root["results"].setBinding(Binding(results)));
    passEncoder->dispatchRays(0, 10, 1, 1);
    passEncoder->end();
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    ComPtr<ISlangBlob> resultBlob;
    GFX_CHECK_CALL_ABORT(device->readBuffer(results, 0, resultDesc.size, resultBlob.writeRef()));
    auto actual = static_cast<const StructuralRayTracingTriangleAttributesFlagsResult*>(
        resultBlob->getBufferPointer());
    static const StructuralRayTracingTriangleAttributesFlagsResult kExpected[] = {
        {3, 0, 25, 25, 0, 1, 10},
        {2, 0, 0, 0, 0, 1, 10},
        {2, 1, 25, 25, 0, 1, 10},
        {3, 1, 25, 25, 0, 1, 10},
        {40, 0, 0, 0, 0, 1, 10},
        {2, 0, 0, 0, 0, 1, 10},
        {3, 0, 25, 25, 0, 1, 10},
        {2, 0, 0, 0, 0, 1, 10},
        {3, 1, 25, 25, 0, 1, 10},
        {3, 0, 25, 25, 0, 1, 10},
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kExpected); ++i)
    {
        SLANG_CHECK(actual[i].stage == kExpected[i].stage);
        SLANG_CHECK(actual[i].anyHitCount == kExpected[i].anyHitCount);
        SLANG_CHECK(actual[i].barycentricX == kExpected[i].barycentricX);
        SLANG_CHECK(actual[i].barycentricY == kExpected[i].barycentricY);
        SLANG_CHECK(actual[i].frontFacing == kExpected[i].frontFacing);
        SLANG_CHECK(actual[i].flagsMatch == kExpected[i].flagsMatch);
        SLANG_CHECK(actual[i].dispatchWidth == kExpected[i].dispatchWidth);
    }
}

void runStructuralRayTracingStageInputState(IDevice* device)
{
    if (!device->hasFeature(Feature::RayTracing))
    {
        SLANG_IGNORE_TEST;
    }

    auto queue = device->getQueue(QueueType::Graphics);
    SLANG_CHECK_ABORT(queue != nullptr);
    static const float kScaleXTransform[12] = {
        2.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
    };
    StructuralRayTracingTriangleScene scene(
        device,
        queue,
        AccelerationStructureInstanceFlags::TriangleFacingCullDisable,
        17,
        kScaleXTransform);

    static const EntryDesc kEntries[] = {
        {"main", SLANG_STAGE_RAY_GENERATION},
        {"RuntimeClosestHit", SLANG_STAGE_CLOSEST_HIT},
        {"RuntimeMiss", SLANG_STAGE_MISS},
    };
    ComPtr<IShaderProgram> program;
    GFX_CHECK_CALL_ABORT(loadProgram(
        device,
        "stage-input-state",
        kEntries,
        SLANG_COUNT_OF(kEntries),
        program.writeRef()));

    HitGroupDesc hitGroup = {};
    hitGroup.hitGroupName = "hitGroup0";
    hitGroup.closestHitEntryPoint = "RuntimeClosestHit";

    RayTracingPipelineDesc pipelineDesc = {};
    pipelineDesc.program = program;
    pipelineDesc.hitGroups = &hitGroup;
    pipelineDesc.hitGroupCount = 1;
    pipelineDesc.maxRecursion = 1;
    pipelineDesc.maxRayPayloadSize = sizeof(StructuralRayTracingStageInputStateResult);
    pipelineDesc.maxAttributeSizeInBytes = sizeof(float) * 2;

    ComPtr<IRayTracingPipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createRayTracingPipeline(pipelineDesc, pipeline.writeRef()));

    static const char* kRayGenerationNames[] = {"main"};
    static const char* kMissNames[] = {"RuntimeMiss"};
    static const char* kHitGroupNames[] = {"hitGroup0"};
    ShaderTableDesc shaderTableDesc = {};
    shaderTableDesc.program = program;
    shaderTableDesc.rayGenShaderCount = SLANG_COUNT_OF(kRayGenerationNames);
    shaderTableDesc.rayGenShaderEntryPointNames = kRayGenerationNames;
    shaderTableDesc.missShaderCount = SLANG_COUNT_OF(kMissNames);
    shaderTableDesc.missShaderEntryPointNames = kMissNames;
    shaderTableDesc.hitGroupCount = SLANG_COUNT_OF(kHitGroupNames);
    shaderTableDesc.hitGroupNames = kHitGroupNames;

    ComPtr<IShaderTable> shaderTable;
    GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, shaderTable.writeRef()));

    BufferDesc resultDesc = {};
    resultDesc.size = sizeof(StructuralRayTracingStageInputStateResult) * 2;
    resultDesc.elementSize = sizeof(StructuralRayTracingStageInputStateResult);
    resultDesc.usage = BufferUsage::UnorderedAccess | BufferUsage::CopySource;
    resultDesc.defaultState = ResourceState::UnorderedAccess;
    auto results = device->createBuffer(resultDesc);
    SLANG_CHECK_ABORT(results != nullptr);

    auto commandEncoder = queue->createCommandEncoder();
    auto passEncoder = commandEncoder->beginRayTracingPass();
    auto rootObject = passEncoder->bindPipeline(pipeline, shaderTable);
    ShaderCursor root(rootObject);
    GFX_CHECK_CALL_ABORT(root["scene"].setBinding(Binding(scene.topLevel)));
    GFX_CHECK_CALL_ABORT(root["results"].setBinding(Binding(results)));
    passEncoder->dispatchRays(0, 2, 1, 1);
    passEncoder->end();
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    ComPtr<ISlangBlob> resultBlob;
    GFX_CHECK_CALL_ABORT(device->readBuffer(results, 0, resultDesc.size, resultBlob.writeRef()));
    auto actual = static_cast<const StructuralRayTracingStageInputStateResult*>(
        resultBlob->getBufferPointer());
    static const StructuralRayTracingStageInputStateResult kExpected[] = {
        {1, 1, 1000, 50, 100, 25, 100, 200, 50, 0, 0, 0, 17, 1, 0, 2},
        {2, 1, 100000, 300, 100, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2},
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kExpected); ++i)
    {
        const uint32_t* actualWords = reinterpret_cast<const uint32_t*>(&actual[i]);
        const uint32_t* expectedWords = reinterpret_cast<const uint32_t*>(&kExpected[i]);
        for (Index word = 0; word < sizeof(kExpected[i]) / sizeof(uint32_t); ++word)
            SLANG_CHECK(actualWords[word] == expectedWords[word]);
    }
}

} // namespace gfx_test
