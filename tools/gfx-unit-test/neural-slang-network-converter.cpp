#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "core/slang-string.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <vector>

using namespace rhi;

namespace gfx_test
{

static const int kElementCountLogical = 321;
static const int kElementCountPhysical = 1088;
static const int kBytesLogical = kElementCountLogical * int(sizeof(float));
static const int kBytesPhysical = kElementCountPhysical * int(sizeof(float));
static const uint32_t kConverterThreadGroupSizeX = 8;
static const uint32_t kConverterThreadGroupSizeY = 4;
static const uint32_t kConverterThreadGroupSizeZ = 2;
static const uint32_t kConverterThreadsPerGroup =
    kConverterThreadGroupSizeX * kConverterThreadGroupSizeY * kConverterThreadGroupSizeZ;
static const uint32_t kConverterDispatchGroupY = 2;
static const uint32_t kConverterDispatchGroupZ = 3;

struct LayerDesc
{
    int inputSize;
    int outputSize;
    bool hasBias;
};

static const LayerDesc kLayers[] = {
    {5, 13, true},
    {13, 7, false},
    {7, 19, true},
};

enum class ConverterTarget
{
    CUDA,
    SPIRV,
    Metal,
};

struct TargetCase
{
    const char* name;
    ConverterTarget converterTarget;
    DeviceType deviceType;
};

struct ReflectedNetworkSize
{
    size_t elementCountLogical;
    size_t elementCountPhysical;
    size_t bytesLogical;
    size_t bytesPhysical;
};

struct DispatchSize
{
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

static int align16(int value)
{
    return ((value + 15) / 16) * 16;
}

static int rowMajorToCudaNativeLayout(int row, int col)
{
    int rowGroup = row < 8 ? 0 : 1;
    int rowInGroup = row - 8 * rowGroup;
    int tidLocal = rowInGroup / 2;
    int pair = rowInGroup % 2;
    int sub = col < 8 ? 0 : 1;
    int gid = col - 8 * sub;
    int lane = gid * 4 + tidLocal;
    int localPos = sub * 4 + rowGroup * 2 + pair;
    return lane * 8 + localPos;
}

static int getPhysicalWeightOffset(
    ConverterTarget target,
    const LayerDesc& layer,
    int outputIndex,
    int inputIndex)
{
    static const int kTileSize = 16;

    int paddedInputSize = align16(layer.inputSize);
    int paddedOutputSize = align16(layer.outputSize);
    int outputTile = outputIndex / kTileSize;
    int inputTile = inputIndex / kTileSize;
    int outputInTile = outputIndex - outputTile * kTileSize;
    int inputInTile = inputIndex - inputTile * kTileSize;

    switch (target)
    {
    case ConverterTarget::CUDA:
        {
            int tileCols = paddedOutputSize / kTileSize;
            int tileBase = (inputTile * tileCols + outputTile) * (kTileSize * kTileSize);
            return tileBase + rowMajorToCudaNativeLayout(inputInTile, outputInTile);
        }
    case ConverterTarget::SPIRV:
        {
            int tileCols = paddedInputSize / kTileSize;
            int tileBase = (outputTile * tileCols + inputTile) * (kTileSize * kTileSize);
            return tileBase + outputInTile * kTileSize + inputInTile;
        }
    case ConverterTarget::Metal:
        return outputIndex * paddedInputSize + inputIndex;
    }

    return 0;
}

static int getLayerLogicalElementCount(const LayerDesc& layer)
{
    return layer.inputSize * layer.outputSize + (layer.hasBias ? layer.outputSize : 0);
}

static int getLayerPhysicalElementCount(const LayerDesc& layer)
{
    return align16(layer.inputSize) * align16(layer.outputSize) + align16(layer.outputSize);
}

static DispatchSize getConverterDispatchSize(size_t elementCount)
{
    uint32_t elementsPerDispatchX =
        kConverterThreadsPerGroup * kConverterDispatchGroupY * kConverterDispatchGroupZ;
    uint32_t dispatchGroupX =
        uint32_t((elementCount + elementsPerDispatchX - 1) / elementsPerDispatchX);
    return {dispatchGroupX, kConverterDispatchGroupY, kConverterDispatchGroupZ};
}

static float makePortableValue(int index)
{
    int randomish = (index * 37 + 19) % 257;
    return float(randomish - 128) * 0.125f;
}

static std::vector<float> makePortableParameters(size_t elementCountLogical)
{
    std::vector<float> result(elementCountLogical);
    for (size_t i = 0; i < elementCountLogical; ++i)
        result[i] = makePortableValue(int(i));
    return result;
}

static std::vector<float> makeExpectedOptimalParameters(
    ConverterTarget target,
    size_t elementCountPhysical)
{
    std::vector<float> result(elementCountPhysical, 0.0f);

    int logicalLayerOffset = 0;
    int physicalLayerOffset = 0;
    for (const auto& layer : kLayers)
    {
        int physicalWeightCount = align16(layer.inputSize) * align16(layer.outputSize);
        int logicalWeightCount = layer.inputSize * layer.outputSize;
        int logicalLayerCount = getLayerLogicalElementCount(layer);

        for (int outputIndex = 0; outputIndex < layer.outputSize; ++outputIndex)
        {
            for (int inputIndex = 0; inputIndex < layer.inputSize; ++inputIndex)
            {
                int sourceOffset = logicalLayerOffset + outputIndex * layer.inputSize + inputIndex;
                int targetOffset = physicalLayerOffset +
                                   getPhysicalWeightOffset(target, layer, outputIndex, inputIndex);
                result[targetOffset] = makePortableValue(sourceOffset);
            }

            if (layer.hasBias)
            {
                int sourceOffset = logicalLayerOffset + logicalWeightCount + outputIndex;
                int targetOffset = physicalLayerOffset + physicalWeightCount + outputIndex;
                result[targetOffset] = makePortableValue(sourceOffset);
            }
        }

        logicalLayerOffset += logicalLayerCount;
        physicalLayerOffset += getLayerPhysicalElementCount(layer);
    }

    SLANG_CHECK_ABORT(logicalLayerOffset == kElementCountLogical);
    SLANG_CHECK_ABORT(size_t(physicalLayerOffset) == elementCountPhysical);
    return result;
}

static Slang::String makeConverterTypeName(const TargetCase& target)
{
    Slang::StringBuilder builder;
    SLANG_UNUSED(target);
    builder << "NetworkParameterLayoutConverter<float, 5, 5, 13, 7, 19>";
    return builder.produceString();
}

static Slang::String makeShaderSource(const TargetCase& target)
{
    SLANG_UNUSED(target);

    return R"(
        import slang.neural;

        static const uint ConverterThreadGroupSizeX = 8;
        static const uint ConverterThreadGroupSizeY = 4;
        static const uint ConverterThreadGroupSizeZ = 2;
        static const uint ConverterDispatchGroupY = 2;
        static const uint ConverterDispatchGroupZ = 3;
        static const uint ConverterThreadsPerGroup =
            ConverterThreadGroupSizeX * ConverterThreadGroupSizeY * ConverterThreadGroupSizeZ;

        uniform RWStructuredBuffer<float>.Handle portableBuffer;
        uniform RWStructuredBuffer<float>.Handle optimalBuffer;
        uniform RWStructuredBuffer<float>.Handle roundTripBuffer;

        uint getConverterDispatchGroupX(uint elementCount)
        {
            uint elementsPerDispatchX =
                ConverterThreadsPerGroup * ConverterDispatchGroupY * ConverterDispatchGroupZ;
            return (elementCount + elementsPerDispatchX - 1) / elementsPerDispatchX;
        }

        uint getConverterThreadIndex(uint3 dispatchThreadId, uint elementCount)
        {
            uint dispatchGroupX = getConverterDispatchGroupX(elementCount);
            uint dispatchSizeX = dispatchGroupX * ConverterThreadGroupSizeX;
            uint dispatchSizeY = ConverterDispatchGroupY * ConverterThreadGroupSizeY;
            return dispatchThreadId.x +
                dispatchThreadId.y * dispatchSizeX +
                dispatchThreadId.z * dispatchSizeX * dispatchSizeY;
        }

        uint getConverterThreadCount(uint elementCount)
        {
            return getConverterDispatchGroupX(elementCount) *
                ConverterDispatchGroupY *
                ConverterDispatchGroupZ *
                ConverterThreadsPerGroup;
        }

        void toOptimalLayout(uint3 dispatchThreadId)
        {
            let portable = BindlessAddress<float>(portableBuffer);
            let optimal = BindlessAddress<float>(optimalBuffer);
            uint elementCount =
                uint(NetworkParameterLayoutConverter<float, 5, 5, 13, 7, 19>.ElementCountPhysical);

            NetworkParameterLayoutConverter<float, 5, 5, 13, 7, 19>
                .toOptimalLayout(
                    portable,
                    optimal,
                    getConverterThreadIndex(dispatchThreadId, elementCount),
                    getConverterThreadCount(elementCount));
        }

        void toPortableLayout(uint3 dispatchThreadId)
        {
            let optimal = BindlessAddress<float>(optimalBuffer);
            let roundTrip = BindlessAddress<float>(roundTripBuffer);
            uint elementCount =
                uint(NetworkParameterLayoutConverter<float, 5, 5, 13, 7, 19>.ElementCountLogical);

            NetworkParameterLayoutConverter<float, 5, 5, 13, 7, 19>
                .toPortableLayout(
                    optimal,
                    roundTrip,
                    getConverterThreadIndex(dispatchThreadId, elementCount),
                    getConverterThreadCount(elementCount));
        }

        [shader("compute")]
        [numthreads(8, 4, 2)]
        void toOptimalMain(uint3 dispatchThreadId : SV_DispatchThreadID)
        {
            toOptimalLayout(dispatchThreadId);
        }

        [shader("compute")]
        [numthreads(8, 4, 2)]
        void toPortableMain(uint3 dispatchThreadId : SV_DispatchThreadID)
        {
            toPortableLayout(dispatchThreadId);
        }
    )";
}

static Slang::ComPtr<IDevice> createDeviceWithExperimentalFeature(
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

    Slang::List<const char*> searchPaths = getSlangSearchPaths();
    deviceDesc.slang.searchPaths = searchPaths.getBuffer();
    deviceDesc.slang.searchPathCount = searchPaths.getCount();

    std::vector<slang::CompilerOptionEntry> compilerOptions;

    slang::CompilerOptionEntry emitSpirvDirectly = {};
    emitSpirvDirectly.name = slang::CompilerOptionName::EmitSpirvDirectly;
    emitSpirvDirectly.value.kind = slang::CompilerOptionValueKind::Int;
    emitSpirvDirectly.value.intValue0 = 1;
    compilerOptions.push_back(emitSpirvDirectly);

    slang::CompilerOptionEntry experimentalFeature = {};
    experimentalFeature.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalFeature.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalFeature.value.intValue0 = 1;
    compilerOptions.push_back(experimentalFeature);

    deviceDesc.slang.compilerOptionEntries = compilerOptions.data();
    deviceDesc.slang.compilerOptionEntryCount = compilerOptions.size();

    if (context->enableDebugLayers)
    {
        deviceDesc.enableValidation = context->enableDebugLayers;
        deviceDesc.debugCallback = context->debugCallback;
        getRHI()->enableDebugLayers();
    }

    Slang::ComPtr<IDevice> device;
    if (SLANG_FAILED(getRHI()->createDevice(deviceDesc, device.writeRef())))
    {
        SLANG_IGNORE_TEST;
    }

    return device;
}

static Slang::Result loadConverterProgram(
    IDevice* device,
    const TargetCase& target,
    const char* entryPointName,
    Slang::ComPtr<IShaderProgram>& outShaderProgram,
    slang::ProgramLayout*& outLayout)
{
    Slang::ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_ON_FAIL(device->getSlangSession(slangSession.writeRef()));

    Slang::String source = makeShaderSource(target);
    Slang::String moduleName =
        Slang::String("neuralNetworkConverter_") + target.name + "_" + entryPointName;
    Slang::String fileName = moduleName + ".slang";

    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = slangSession->loadModuleFromSourceString(
        moduleName.getBuffer(),
        fileName.getBuffer(),
        source.getBuffer(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if (!module)
        return SLANG_FAIL;

    Slang::ComPtr<slang::IEntryPoint> computeEntryPoint;
    SLANG_RETURN_ON_FAIL(
        module->findEntryPointByName(entryPointName, computeEntryPoint.writeRef()));

    slang::IComponentType* componentTypes[] = {module, computeEntryPoint.get()};
    Slang::ComPtr<slang::IComponentType> composedProgram;
    Slang::Result result = slangSession->createCompositeComponentType(
        componentTypes,
        SLANG_COUNT_OF(componentTypes),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    outLayout = linkedProgram->getLayout();

    ShaderProgramDesc programDesc = {};
    programDesc.slangGlobalScope = linkedProgram.get();
    outShaderProgram = device->createShaderProgram(programDesc, diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    return outShaderProgram ? SLANG_OK : SLANG_FAIL;
}

static int64_t getStaticInt(slang::ProgramLayout* layout, const char* typeName, const char* varName)
{
    auto type = layout->findTypeByName(typeName);
    SLANG_CHECK_ABORT(type != nullptr);

    auto var = layout->findVarByNameInType(type, varName);
    SLANG_CHECK_ABORT(var != nullptr);

    int64_t value = 0;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(var->getDefaultValueInt(&value)));
    return value;
}

static ReflectedNetworkSize queryAndCheckNetworkSize(
    slang::ProgramLayout* layout,
    const TargetCase& target)
{
    Slang::String converterTypeName = makeConverterTypeName(target);
    const char* typeName = converterTypeName.getBuffer();

    ReflectedNetworkSize result = {};
    result.elementCountLogical = size_t(getStaticInt(layout, typeName, "ElementCountLogical"));
    result.elementCountPhysical = size_t(getStaticInt(layout, typeName, "ElementCountPhysical"));
    result.bytesLogical = size_t(getStaticInt(layout, typeName, "BytesLogical"));
    result.bytesPhysical = size_t(getStaticInt(layout, typeName, "BytesPhysical"));

    SLANG_CHECK_ABORT(result.elementCountLogical == kElementCountLogical);
    SLANG_CHECK_ABORT(result.elementCountPhysical == kElementCountPhysical);
    SLANG_CHECK_ABORT(result.bytesLogical == kBytesLogical);
    SLANG_CHECK_ABORT(result.bytesPhysical == kBytesPhysical);
    return result;
}

static bool isDescriptorHandleType(const ShaderCursor& cursor)
{
    auto typeLayout = cursor.getTypeLayout();
    auto uniformSize = typeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);
    auto descriptorSlotSize = typeLayout->getSize(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);

    if (uniformSize != 8 || descriptorSlotSize != 0)
        return false;

    auto type = typeLayout->getType();
    auto kind = type->getKind();

    if (kind == slang::TypeReflection::Kind::Vector && type->getElementCount() == 2 &&
        type->getElementType()->getScalarType() == slang::TypeReflection::ScalarType::UInt32)
        return true;

    if (kind == slang::TypeReflection::Kind::Scalar &&
        type->getScalarType() == slang::TypeReflection::ScalarType::UInt64)
    {
        auto elementType = type->getElementType();
        return elementType && (elementType->getKind() == slang::TypeReflection::Kind::Resource ||
                               elementType->getKind() == slang::TypeReflection::Kind::SamplerState);
    }

    return false;
}

static Slang::Result bindRWStructuredBufferHandle(
    const ShaderCursor& cursor,
    IBuffer* buffer,
    size_t byteSize)
{
    auto type = cursor.getTypeLayout()->getType();
    bool isPointerLike = (type->getKind() == slang::TypeReflection::Kind::Scalar &&
                          type->getScalarType() == slang::TypeReflection::ScalarType::UInt64) ||
                         type->getKind() == slang::TypeReflection::Kind::Pointer;

    // Vulkan stores bindless buffer handles as descriptor-table indices in uniform data.
    // CUDA/Metal do not currently expose RHI descriptor handles for buffers, so if this
    // attempt fails we fall back to the pointer representation used by those targets.
    if (isDescriptorHandleType(cursor))
    {
        DescriptorHandle handle = {};
        auto result = buffer->getDescriptorHandle(
            DescriptorHandleAccess::ReadWrite,
            Format::Undefined,
            BufferRange{0, byteSize},
            &handle);
        if (SLANG_SUCCEEDED(result))
            return cursor.setDescriptorHandle(handle);
    }

    if (isPointerLike)
    {
        uint64_t address = buffer->getDeviceAddress();
        return cursor.setData(&address, sizeof(address));
    }

    return cursor.setBinding(Binding(buffer));
}

static Slang::ComPtr<IBuffer> createFloatBuffer(IDevice* device, const float* data, size_t count)
{
    BufferDesc desc = {};
    desc.size = count * sizeof(float);
    desc.elementSize = sizeof(float);
    desc.format = Format::Undefined;
    desc.memoryType = MemoryType::DeviceLocal;
    desc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                 BufferUsage::CopyDestination | BufferUsage::CopySource;
    desc.defaultState = ResourceState::UnorderedAccess;

    Slang::ComPtr<IBuffer> buffer;
    GFX_CHECK_CALL_ABORT(device->createBuffer(desc, (void*)data, buffer.writeRef()));
    return buffer;
}

static void checkFloatBuffer(
    IDevice* device,
    IBuffer* buffer,
    const char* bufferName,
    const std::vector<float>& expected)
{
    Slang::ComPtr<ISlangBlob> blob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        device->readBuffer(buffer, 0, expected.size() * sizeof(float), blob.writeRef())));
    SLANG_CHECK_ABORT(blob->getBufferSize() == expected.size() * sizeof(float));

    const float* actual = reinterpret_cast<const float*>(blob->getBufferPointer());
    int printedMismatchCount = 0;
    for (size_t i = 0; i < expected.size(); ++i)
    {
        if (actual[i] != expected[i] && printedMismatchCount < 8)
        {
            fprintf(
                stderr,
                "%s[%zu] = %.6g, expected %.6g\n",
                bufferName,
                i,
                actual[i],
                expected[i]);
            printedMismatchCount++;
        }
        SLANG_CHECK(actual[i] == expected[i]);
    }
}

static void bindConverterBuffers(
    IShaderObject* rootObject,
    IBuffer* portableBuffer,
    size_t portableByteSize,
    IBuffer* optimalBuffer,
    size_t optimalByteSize,
    IBuffer* roundTripBuffer,
    size_t roundTripByteSize)
{
    ShaderCursor rootCursor(rootObject);
    GFX_CHECK_CALL_ABORT(bindRWStructuredBufferHandle(
        rootCursor.getPath("portableBuffer"),
        portableBuffer,
        portableByteSize));
    GFX_CHECK_CALL_ABORT(bindRWStructuredBufferHandle(
        rootCursor.getPath("optimalBuffer"),
        optimalBuffer,
        optimalByteSize));
    GFX_CHECK_CALL_ABORT(bindRWStructuredBufferHandle(
        rootCursor.getPath("roundTripBuffer"),
        roundTripBuffer,
        roundTripByteSize));
}

static void dispatchConverter(
    IDevice* device,
    IComputePipeline* pipeline,
    IBuffer* portableBuffer,
    size_t portableByteSize,
    IBuffer* optimalBuffer,
    size_t optimalByteSize,
    IBuffer* roundTripBuffer,
    size_t roundTripByteSize,
    DispatchSize dispatchSize)
{
    auto queue = device->getQueue(QueueType::Graphics);
    auto commandEncoder = queue->createCommandEncoder();
    auto encoder = commandEncoder->beginComputePass();

    auto rootObject = encoder->bindPipeline(pipeline);
    bindConverterBuffers(
        rootObject,
        portableBuffer,
        portableByteSize,
        optimalBuffer,
        optimalByteSize,
        roundTripBuffer,
        roundTripByteSize);

    encoder->dispatchCompute(dispatchSize.x, dispatchSize.y, dispatchSize.z);
    encoder->end();

    queue->submit(commandEncoder->finish());
    queue->waitOnHost();
}

static void neuralNetworkConverterTestImpl(IDevice* device, const TargetCase& target)
{
    // 1. Compile the two shader entry points used by the workflow. We keep the
    // conversions in separate dispatches so the test exercises the same
    // portable -> optimal -> portable sequence a client would run. Each
    // dispatch launches a 3-D, warp-aligned set of threads and the converter
    // maps flattened dispatch thread IDs to physical/logical element streams.
    Slang::ComPtr<IShaderProgram> toOptimalProgram;
    slang::ProgramLayout* slangReflection = nullptr;
    GFX_CHECK_CALL_ABORT(
        loadConverterProgram(device, target, "toOptimalMain", toOptimalProgram, slangReflection));

    // 2. Query the public converter type directly through reflection. The
    // allocations below deliberately use these reflected values, which is the
    // same pattern we expect end-user host code to follow.
    ReflectedNetworkSize reflectedSize = queryAndCheckNetworkSize(slangReflection, target);

    Slang::ComPtr<IShaderProgram> toPortableProgram;
    slang::ProgramLayout* unusedReflection = nullptr;
    GFX_CHECK_CALL_ABORT(loadConverterProgram(
        device,
        target,
        "toPortableMain",
        toPortableProgram,
        unusedReflection));

    ComputePipelineDesc toOptimalPipelineDesc = {};
    toOptimalPipelineDesc.program = toOptimalProgram.get();
    Slang::ComPtr<IComputePipeline> toOptimalPipeline;
    GFX_CHECK_CALL_ABORT(
        device->createComputePipeline(toOptimalPipelineDesc, toOptimalPipeline.writeRef()));

    ComputePipelineDesc toPortablePipelineDesc = {};
    toPortablePipelineDesc.program = toPortableProgram.get();
    Slang::ComPtr<IComputePipeline> toPortablePipeline;
    GFX_CHECK_CALL_ABORT(
        device->createComputePipeline(toPortablePipelineDesc, toPortablePipeline.writeRef()));

    std::vector<float> portable = makePortableParameters(reflectedSize.elementCountLogical);

    // These sentinel values make missed writes visible when buffers are read
    // back: any untouched optimal or round-trip element will fail comparison.
    std::vector<float> optimalInitial(reflectedSize.elementCountPhysical, -9123.25f);
    std::vector<float> roundTripInitial(reflectedSize.elementCountLogical, 7777.0f);

    auto portableBuffer = createFloatBuffer(device, portable.data(), portable.size());
    auto optimalBuffer = createFloatBuffer(device, optimalInitial.data(), optimalInitial.size());
    auto roundTripBuffer =
        createFloatBuffer(device, roundTripInitial.data(), roundTripInitial.size());

    size_t portableByteSize = reflectedSize.bytesLogical;
    size_t optimalByteSize = reflectedSize.bytesPhysical;
    size_t roundTripByteSize = reflectedSize.bytesLogical;
    SLANG_CHECK_ABORT(portableByteSize == portable.size() * sizeof(float));
    SLANG_CHECK_ABORT(optimalByteSize == optimalInitial.size() * sizeof(float));
    SLANG_CHECK_ABORT(roundTripByteSize == roundTripInitial.size() * sizeof(float));

    // 3. Dispatch the utility shader that converts the user-facing portable
    // row-major network parameter block into the target-specific optimal block,
    // then compare the result against a CPU reference for the selected target.
    dispatchConverter(
        device,
        toOptimalPipeline,
        portableBuffer,
        portableByteSize,
        optimalBuffer,
        optimalByteSize,
        roundTripBuffer,
        roundTripByteSize,
        getConverterDispatchSize(reflectedSize.elementCountPhysical));

    checkFloatBuffer(
        device,
        optimalBuffer,
        "optimal",
        makeExpectedOptimalParameters(target.converterTarget, reflectedSize.elementCountPhysical));

    // 4. Dispatch the reverse utility shader and verify the recovered portable
    // block is identical to the original input.
    dispatchConverter(
        device,
        toPortablePipeline,
        portableBuffer,
        portableByteSize,
        optimalBuffer,
        optimalByteSize,
        roundTripBuffer,
        roundTripByteSize,
        getConverterDispatchSize(reflectedSize.elementCountLogical));

    checkFloatBuffer(device, roundTripBuffer, "roundTrip", portable);
}

static void runNeuralNetworkConverterTest(UnitTestContext* context, const TargetCase& target)
{
    auto device = createDeviceWithExperimentalFeature(context, target.deviceType);
    if (!device)
    {
        SLANG_IGNORE_TEST;
    }

    neuralNetworkConverterTestImpl(device, target);
}

SLANG_UNIT_TEST(neuralSlangNetworkConverterCUDA)
{
    TargetCase target = {"cuda", ConverterTarget::CUDA, DeviceType::CUDA};
    runNeuralNetworkConverterTest(unitTestContext, target);
}

SLANG_UNIT_TEST(neuralSlangNetworkConverterVulkan)
{
    TargetCase target = {"vulkan", ConverterTarget::SPIRV, DeviceType::Vulkan};
    runNeuralNetworkConverterTest(unitTestContext, target);
}

SLANG_UNIT_TEST(neuralSlangNetworkConverterMetal)
{
    TargetCase target = {"metal", ConverterTarget::Metal, DeviceType::Metal};
    runNeuralNetworkConverterTest(unitTestContext, target);
}

} // namespace gfx_test
