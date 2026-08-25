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
    const char* name;
    SlangStage stage;
};

Result loadProgram(
    IDevice* device,
    const char* moduleName,
    const EntryDesc* entries,
    Index entryCount,
    IShaderProgram** outProgram)
{
    auto slangSession = device->getSlangSession();
    ComPtr<slang::IBlob> diagnostics;
    auto module = slangSession->loadModule(moduleName, diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics);
    if (!module)
        return SLANG_FAIL;

    std::vector<ComPtr<slang::IEntryPoint>> entryPoints;
    std::vector<slang::IComponentType*> components;
    components.push_back(module);
    for (Index i = 0; i < entryCount; ++i)
    {
        auto& entryDesc = entries[i];
        ComPtr<slang::IEntryPoint> entryPoint;
        auto result = module->findAndCheckEntryPoint(
            entryDesc.name,
            entryDesc.stage,
            entryPoint.writeRef(),
            diagnostics.writeRef());
        diagnoseIfNeeded(diagnostics);
        SLANG_RETURN_ON_FAIL(result);
        entryPoints.push_back(entryPoint);
        components.push_back(entryPoint);
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
    GFX_CHECK_CALL_ABORT(root["scene"].setBinding(Binding(scene.topLevel)));
    GFX_CHECK_CALL_ABORT(root["results"].setBinding(Binding(results)));
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
    resultDesc.size = sizeof(StructuralRayTracingProceduralResult) * 2;
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
    passEncoder->dispatchRays(0, 2, 1, 1);
    passEncoder->end();
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    ComPtr<ISlangBlob> resultBlob;
    GFX_CHECK_CALL_ABORT(device->readBuffer(results, 0, resultDesc.size, resultBlob.writeRef()));
    auto actual =
        static_cast<const StructuralRayTracingProceduralResult*>(resultBlob->getBufferPointer());
    static const StructuralRayTracingProceduralResult kExpected[] = {
        {3, 9, 2, 2},
        {2, 0, 0, 2},
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

void runStructuralRayTracingMultipleSlots(IDevice* device)
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
        {"ClosestHit0", SLANG_STAGE_CLOSEST_HIT},
        {"ClosestHit1", SLANG_STAGE_CLOSEST_HIT},
        {"Miss0", SLANG_STAGE_MISS},
        {"Miss1", SLANG_STAGE_MISS},
    };
    ComPtr<IShaderProgram> program;
    GFX_CHECK_CALL_ABORT(loadProgram(
        device,
        "multiple-slots",
        kEntries,
        SLANG_COUNT_OF(kEntries),
        program.writeRef()));

    static const char* kHitGroupNames[] = {"hitGroup0", "hitGroup1"};
    HitGroupDesc hitGroups[2] = {};
    hitGroups[0].hitGroupName = kHitGroupNames[0];
    hitGroups[0].closestHitEntryPoint = "ClosestHit0";
    hitGroups[1].hitGroupName = kHitGroupNames[1];
    hitGroups[1].closestHitEntryPoint = "ClosestHit1";

    RayTracingPipelineDesc pipelineDesc = {};
    pipelineDesc.program = program;
    pipelineDesc.hitGroups = hitGroups;
    pipelineDesc.hitGroupCount = SLANG_COUNT_OF(hitGroups);
    pipelineDesc.maxRecursion = 1;
    pipelineDesc.maxRayPayloadSize = sizeof(uint32_t) * 2;
    pipelineDesc.maxAttributeSizeInBytes = sizeof(float) * 2;

    ComPtr<IRayTracingPipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createRayTracingPipeline(pipelineDesc, pipeline.writeRef()));

    static const char* kRayGenerationNames[] = {"main"};
    static const char* kMissNames[] = {"Miss0", "Miss1"};
    ShaderRecordOverwrite hitRecords[2] = {};
    ShaderRecordOverwrite missRecords[2] = {};
    for (Index i = 0; i < 2; ++i)
    {
        uint32_t hitRecordValue = uint32_t((i + 1) * 100);
        uint32_t missRecordValue = uint32_t((i + 3) * 100);
        hitRecords[i].offset = 32;
        hitRecords[i].size = sizeof(uint32_t);
        std::memcpy(hitRecords[i].data, &hitRecordValue, sizeof(hitRecordValue));
        missRecords[i].offset = 32;
        missRecords[i].size = sizeof(uint32_t);
        std::memcpy(missRecords[i].data, &missRecordValue, sizeof(missRecordValue));
    }

    ShaderTableDesc shaderTableDesc = {};
    shaderTableDesc.program = program;
    shaderTableDesc.rayGenShaderCount = SLANG_COUNT_OF(kRayGenerationNames);
    shaderTableDesc.rayGenShaderEntryPointNames = kRayGenerationNames;
    shaderTableDesc.missShaderCount = SLANG_COUNT_OF(kMissNames);
    shaderTableDesc.missShaderEntryPointNames = kMissNames;
    shaderTableDesc.missShaderRecordOverwrites = missRecords;
    shaderTableDesc.hitGroupCount = SLANG_COUNT_OF(kHitGroupNames);
    shaderTableDesc.hitGroupNames = kHitGroupNames;
    shaderTableDesc.hitGroupRecordOverwrites = hitRecords;

    ComPtr<IShaderTable> shaderTable;
    GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, shaderTable.writeRef()));

    BufferDesc resultDesc = {};
    resultDesc.size = sizeof(StructuralRayTracingMultipleSlotsResult) * 4;
    resultDesc.elementSize = sizeof(StructuralRayTracingMultipleSlotsResult);
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
    auto actual =
        static_cast<const StructuralRayTracingMultipleSlotsResult*>(resultBlob->getBufferPointer());
    static const StructuralRayTracingMultipleSlotsResult kExpected[] = {
        {10, 100, 4},
        {11, 200, 4},
        {20, 300, 4},
        {21, 400, 4},
    };
    for (Index i = 0; i < SLANG_COUNT_OF(kExpected); ++i)
    {
        SLANG_CHECK(actual[i].stage == kExpected[i].stage);
        SLANG_CHECK(actual[i].recordValue == kExpected[i].recordValue);
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

} // namespace gfx_test
