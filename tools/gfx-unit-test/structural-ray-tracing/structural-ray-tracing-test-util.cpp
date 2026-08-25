#include "structural-ray-tracing-test-util.h"

#include "structural-ray-tracing-scenes.h"

#include <slang-rhi/shader-cursor.h>
#include <vector>

using namespace rhi;
using namespace Slang;

namespace gfx_test
{

namespace
{

Result loadProgram(IDevice* device, IShaderProgram** outProgram)
{
    auto slangSession = device->getSlangSession();
    ComPtr<slang::IBlob> diagnostics;
    auto module = slangSession->loadModule("triangle-hit-miss", diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics);
    if (!module)
        return SLANG_FAIL;

    struct EntryDesc
    {
        const char* name;
        SlangStage stage;
    };
    static const EntryDesc kEntries[] = {
        {"main", SLANG_STAGE_RAY_GENERATION},
        {"RuntimeClosestHit", SLANG_STAGE_CLOSEST_HIT},
        {"RuntimeMiss", SLANG_STAGE_MISS},
    };

    std::vector<ComPtr<slang::IEntryPoint>> entryPoints;
    std::vector<slang::IComponentType*> components;
    components.push_back(module);
    for (const auto& entryDesc : kEntries)
    {
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
    GFX_CHECK_CALL_ABORT(loadProgram(device, program.writeRef()));

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

} // namespace gfx_test
