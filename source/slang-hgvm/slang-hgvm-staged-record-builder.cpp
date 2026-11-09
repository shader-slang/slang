#include "slang-hgvm-staged-record-builder.h"

#include "core/slang-exception.h"

#include <algorithm>

namespace Slang::HGVM
{

StagedRecordBuilder::StagedRecordBuilder(StagedRecordStorage& storage)
    : m_storage(storage), m_startByteSize(storage.m_committedByteSize)
{
    SLANG_RELEASE_ASSERT(!storage.m_activeBuilder);
    SLANG_RELEASE_ASSERT(storage.m_bytes.getCount() == m_startByteSize);
    storage.m_activeBuilder = this;
}

StagedRecordBuilder::~StagedRecordBuilder()
{
    if (m_storage.m_activeBuilder == this)
        rollback();
}

StagedRecord::Header StagedRecordBuilder::makeHeader(
    uint32_t opcode,
    uint64_t operandCount,
    uint64_t byteSize)
{
    if (operandCount > UINT16_MAX || byteSize / sizeof(uint32_t) > UINT16_MAX)
        throw Exception("HGVM record exceeds its staged operand or size limit");
    StagedRecord::Header header;
    header.opcodeAndFlags = opcode;
    header.operandCount = uint16_t(operandCount);
    header.recordWordCount = uint16_t(byteSize / sizeof(uint32_t));
    return header;
}

uint32_t StagedRecordBuilder::getWordOffset() const
{
    SLANG_RELEASE_ASSERT(m_storage.m_activeBuilder == this);
    SLANG_RELEASE_ASSERT(m_storage.m_bytes.getCount() % StagedRecord::kRecordAlignment == 0);
    return uint32_t(m_storage.m_bytes.getCount() / sizeof(uint32_t));
}

void StagedRecordBuilder::markInstructionContinuations(uint32_t firstWordOffset)
{
    const uint32_t end = getWordOffset();
    if (firstWordOffset == end)
        return;
    validatePatchAddress(Address::makeAbsoluteInstruction(firstWordOffset));
    uint32_t offset = firstWordOffset;
    while (offset < end)
    {
        auto* header = const_cast<StagedRecord::Header*>(getRecord<StagedRecord::Header>(offset));
        SLANG_RELEASE_ASSERT(header->recordWordCount && header->recordWordCount <= end - offset);
        if (offset != firstWordOffset)
            header->opcodeAndFlags |= StagedRecord::kInstructionContinuationBit;
        offset += header->recordWordCount;
    }
    SLANG_RELEASE_ASSERT(offset == end);
}

uint32_t StagedRecordBuilder::allocate(uint64_t byteSize, uint32_t alignment)
{
    SLANG_RELEASE_ASSERT(m_storage.m_activeBuilder == this);
    SLANG_RELEASE_ASSERT(alignment && !(alignment & (alignment - 1)));
    SLANG_RELEASE_ASSERT(alignment <= kStorageBaseAlignment);
    if (byteSize >= kInstructionPayloadLimit * sizeof(uint32_t))
        throw Exception("HGVM staged instruction stack exceeds its address limit");
    alignment = std::max(alignment, uint32_t(StagedRecord::kRecordAlignment));
    uint64_t oldSize = uint64_t(m_storage.m_bytes.getCount());
    uint64_t start = (oldSize + alignment - 1) & ~uint64_t(alignment - 1);
    uint64_t end = (start + byteSize + StagedRecord::kRecordAlignment - 1) &
                   ~uint64_t(StagedRecord::kRecordAlignment - 1);
    if (end / sizeof(uint32_t) >= kInstructionPayloadLimit ||
        end > uint64_t(std::numeric_limits<Index>::max()))
        throw Exception("HGVM staged instruction stack exceeds its address limit");
    m_storage.m_bytes.growToCount(Index(end));
    if (end != oldSize)
        std::memset(m_storage.m_bytes.getBuffer() + oldSize, 0, size_t(end - oldSize));
    return uint32_t(start / sizeof(uint32_t));
}

uint32_t StagedRecordBuilder::appendAlignedBytes(
    const void* bytes,
    uint32_t byteCount,
    uint32_t alignment)
{
    uint32_t start = allocate(byteCount, alignment);
    if (byteCount)
        std::memcpy(
            m_storage.m_bytes.getBuffer() + size_t(start) * sizeof(uint32_t),
            bytes,
            byteCount);
    return start;
}

void StagedRecordBuilder::finalize()
{
    SLANG_RELEASE_ASSERT(m_storage.m_activeBuilder == this && m_pendingFixupCount == 0);
    m_storage.m_committedByteSize = m_storage.m_bytes.getCount();
    m_storage.m_activeBuilder = nullptr;
}

void StagedRecordBuilder::rollback()
{
    SLANG_RELEASE_ASSERT(m_storage.m_activeBuilder == this);
    m_storage.m_bytes.setCount(m_startByteSize);
    m_storage.m_activeBuilder = nullptr;
}

void StagedRecordBuilder::validatePatchAddress(Address address) const
{
    SLANG_RELEASE_ASSERT(m_storage.m_activeBuilder == this);
    SLANG_RELEASE_ASSERT(address.getKind() == AddressKind::InstructionAbsolute);
    if (address.getUnsignedPayload() < uint64_t(m_startByteSize) / sizeof(uint32_t))
        throw Exception("HGVM cannot patch a committed staged record");
}

Address StagedRecordBuilder::emitDebugScope(uint32_t scopeIndex, uint32_t inlineSiteCount)
{
    StagedRecord::DebugScope record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::DebugScope) | StagedRecord::kDebugRecordBit,
        0,
        StagedRecord::getByteSize<StagedRecord::DebugScope>());
    record.scopeIndex = scopeIndex;
    record.inlineSiteCount = inlineSiteCount;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitDebugValue(
    Address destination,
    Address source,
    uint32_t byteSize,
    uint32_t destinationSize,
    uint32_t staticOffset,
    ConstArrayView<StagedRecord::DebugElementOffset> elements)
{
    StagedRecord::DebugValue record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::DebugValue) | StagedRecord::kDebugRecordBit,
        elements.getCount(),
        StagedRecord::getByteSize<StagedRecord::DebugValue, StagedRecord::DebugElementOffset>(
            elements.getCount()));
    record.header.resultAddress = destination;
    record.source = source;
    record.byteSize = byteSize;
    record.destinationSize = destinationSize;
    record.staticOffset = staticOffset;
    record.elementCount = uint32_t(elements.getCount());
    return emitRecord(record, elements);
}

Address StagedRecordBuilder::emitLiteral(
    HGBCOpcode opcode,
    Address resultAddress,
    ValueKind kind,
    ScalarType scalarType,
    Address dataAddress,
    uint32_t byteSize)
{
    StagedRecord::Literal record;
    record.header =
        makeHeader(uint32_t(opcode), 0, StagedRecord::getByteSize<StagedRecord::Literal>());
    record.header.resultAddress = resultAddress;
    record.kind = kind;
    record.scalarType = scalarType;
    record.dataAddress = dataAddress;
    record.byteSize = byteSize;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitGraphicsPipeline(
    Address vertexEntryPoint,
    Address fragmentEntryPoint,
    rhi::PrimitiveTopology topology,
    const rhi::RasterizerDesc& rasterizer,
    const rhi::DepthStencilDesc& depthStencil,
    const rhi::MultisampleDesc& multisample,
    const rhi::ColorTargetDesc& colorTarget)
{
    StagedRecord::GraphicsPipeline record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::MakeGraphicsPipeline),
        0,
        StagedRecord::getByteSize<StagedRecord::GraphicsPipeline>());
    record.vertexEntryPoint = vertexEntryPoint;
    record.fragmentEntryPoint = fragmentEntryPoint;
    record.topology = topology;
    record.rasterizer = rasterizer;
    record.depthStencil = depthStencil;
    record.multisample = multisample;
    record.colorTarget = colorTarget;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitTypeLayoutObject(
    HGBCId logicalTypeId,
    HGBCLayoutDomain domain,
    HGBCLayoutUsage usage,
    Address hostTypeInfoAddress,
    Address contextAddress,
    uint32_t contextIndex,
    uint32_t entryPointIndex,
    bool graphicsContext)
{
    StagedRecord::TypeLayoutObject record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::TypeLayout),
        0,
        StagedRecord::getByteSize<StagedRecord::TypeLayoutObject>());
    record.logicalTypeId = logicalTypeId;
    record.domain = domain;
    record.usage = usage;
    record.hostTypeInfoAddress = hostTypeInfoAddress;
    record.contextAddress = contextAddress;
    record.contextIndex = contextIndex;
    record.entryPointIndex = entryPointIndex;
    record.graphicsContext = graphicsContext;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitLayoutConverterObject(
    Address sourceLayoutAddress,
    Address destinationLayoutAddress)
{
    StagedRecord::LayoutConverterObject record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::LayoutConverter),
        0,
        StagedRecord::getByteSize<StagedRecord::LayoutConverterObject>());
    record.sourceLayoutAddress = sourceLayoutAddress;
    record.destinationLayoutAddress = destinationLayoutAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitPrint(ValueKind kind, ScalarType scalarType, Address valueAddress)
{
    StagedRecord::Print record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::Print),
        1,
        StagedRecord::getByteSize<StagedRecord::Print>());
    record.kind = kind;
    record.scalarType = scalarType;
    record.valueAddress = valueAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitStringOperation(
    HGBCOpcode opcode,
    Address resultAddress,
    Address stringAddress)
{
    StagedRecord::StringOperation record;
    record.header =
        makeHeader(uint32_t(opcode), 1, StagedRecord::getByteSize<StagedRecord::StringOperation>());
    record.header.resultAddress = resultAddress;
    record.stringAddress = stringAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitStringReadByte(
    Address resultAddress,
    Address stringAddress,
    Address indexAddress)
{
    StagedRecord::StringReadByte record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::StringReadByte),
        2,
        StagedRecord::getByteSize<StagedRecord::StringReadByte>());
    record.header.resultAddress = resultAddress;
    record.stringAddress = stringAddress;
    record.indexAddress = indexAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitTimerOperation(
    HGBCOpcode opcode,
    Address resultAddress,
    Address timerAddress)
{
    StagedRecord::TimerOperation record;
    record.header = makeHeader(
        uint32_t(opcode),
        opcode == HGBCOpcode::CreateTimer ? 0 : 1,
        StagedRecord::getByteSize<StagedRecord::TimerOperation>());
    record.header.resultAddress = resultAddress;
    record.timerAddress = timerAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitMakeRaytracingPipeline(
    Address resultAddress,
    Address rayTypeCountAddress)
{
    StagedRecord::MakeRaytracingPipeline record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::MakeRaytracingPipeline),
        1,
        StagedRecord::getByteSize<StagedRecord::MakeRaytracingPipeline>());
    record.header.resultAddress = resultAddress;
    record.rayTypeCountAddress = rayTypeCountAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitDispatchRays(
    const StagedRecord::DispatchRays& dispatch,
    ConstArrayView<StagedRecord::RayShaderArgument> arguments)
{
    auto record = dispatch;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::DispatchRays),
        arguments.getCount(),
        StagedRecord::getByteSize<StagedRecord::DispatchRays, StagedRecord::RayShaderArgument>(
            arguments.getCount()));
    record.argumentCount = uint32_t(arguments.getCount());
    return emitRecord(record, arguments);
}

Address StagedRecordBuilder::emitAllocRayAccelerationStructure(Address result, bool topLevel)
{
    StagedRecord::AllocRayAccelerationStructure record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::AllocAccelerationStructure),
        0,
        StagedRecord::getByteSize<StagedRecord::AllocRayAccelerationStructure>());
    record.header.resultAddress = result;
    record.topLevel = topLevel;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitBuildRayAccelerationStructure(
    const StagedRecord::BuildRayAccelerationStructure& build)
{
    auto record = build;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::BuildAccelerationStructure),
        0,
        StagedRecord::getByteSize<StagedRecord::BuildRayAccelerationStructure>());
    return emitRecord(record);
}

Address StagedRecordBuilder::emitSetRayShader(
    HGBCOpcode opcode,
    const StagedRecord::SetRayShader& registration,
    ConstArrayView<StagedRecord::RayShaderArgument> arguments)
{
    StagedRecord::SetRayShader record = registration;
    record.header = makeHeader(
        uint32_t(opcode),
        arguments.getCount(),
        StagedRecord::getByteSize<StagedRecord::SetRayShader, StagedRecord::RayShaderArgument>(
            arguments.getCount()));
    record.argumentCount = uint32_t(arguments.getCount());
    return emitRecord(record, arguments);
}

Address StagedRecordBuilder::emitUnary(
    HGBCOpcode opcode,
    Address resultAddress,
    ScalarType scalarType,
    uint32_t laneCount,
    Address operandAddress)
{
    StagedRecord::Unary record;
    record.header =
        makeHeader(uint32_t(opcode), 1, StagedRecord::getByteSize<StagedRecord::Unary>());
    record.header.resultAddress = resultAddress;
    record.scalarType = scalarType;
    record.laneCount = laneCount;
    record.operandAddress = operandAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitScalarTypeCast(
    Address resultAddress,
    ScalarType sourceType,
    ScalarType destinationType,
    Address sourceAddress)
{
    StagedRecord::ScalarTypeCast record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::ScalarTypeCast),
        1,
        StagedRecord::getByteSize<StagedRecord::ScalarTypeCast>());
    record.header.resultAddress = resultAddress;
    record.sourceType = sourceType;
    record.destinationType = destinationType;
    record.sourceAddress = sourceAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitBinary(
    HGBCOpcode opcode,
    Address resultAddress,
    ScalarType scalarType,
    uint32_t laneCount,
    Address leftAddress,
    Address rightAddress)
{
    StagedRecord::Binary record;
    record.header =
        makeHeader(uint32_t(opcode), 2, StagedRecord::getByteSize<StagedRecord::Binary>());
    record.header.resultAddress = resultAddress;
    record.scalarType = scalarType;
    record.laneCount = laneCount;
    record.leftAddress = leftAddress;
    record.rightAddress = rightAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitVar(Address resultAddress, uint32_t storageByteOffset)
{
    StagedRecord::Var record;
    record.header =
        makeHeader(uint32_t(HGBCOpcode::Var), 0, StagedRecord::getByteSize<StagedRecord::Var>());
    record.header.resultAddress = resultAddress;
    record.storageByteOffset = storageByteOffset;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitReference(
    StagedRecord::InternalOpcode opcode,
    Address targetAddress,
    uint32_t leafByteOffset,
    uint32_t isIndirect)
{
    StagedRecord::Reference record;
    record.header = makeHeader(
        StagedRecord::kInternalOpcodeBit | uint32_t(opcode),
        0,
        StagedRecord::getByteSize<StagedRecord::Reference>());
    record.targetAddress = targetAddress;
    record.leafByteOffset = leafByteOffset;
    record.isIndirect = isIndirect;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitLoad(
    Address resultAddress,
    Address pointerAddress,
    uint32_t byteSize)
{
    StagedRecord::Load record;
    record.header =
        makeHeader(uint32_t(HGBCOpcode::Load), 1, StagedRecord::getByteSize<StagedRecord::Load>());
    record.header.resultAddress = resultAddress;
    record.pointerAddress = pointerAddress;
    record.byteSize = byteSize;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitResourceToDescriptorHandle(
    Address resultAddress,
    Address resourceAddress)
{
    StagedRecord::ResourceToDescriptorHandle record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::ResourceToDescriptorHandle),
        1,
        StagedRecord::getByteSize<StagedRecord::ResourceToDescriptorHandle>());
    record.header.resultAddress = resultAddress;
    record.resourceAddress = resourceAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitDescriptorHandleToResource(
    Address resultAddress,
    Address handleAddress)
{
    StagedRecord::DescriptorHandleToResource record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::DescriptorHandleToResource),
        1,
        StagedRecord::getByteSize<StagedRecord::DescriptorHandleToResource>());
    record.header.resultAddress = resultAddress;
    record.handleAddress = handleAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitStore(
    Address pointerAddress,
    Address valueAddress,
    uint32_t byteSize)
{
    StagedRecord::Store record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::Store),
        2,
        StagedRecord::getByteSize<StagedRecord::Store>());
    record.pointerAddress = pointerAddress;
    record.valueAddress = valueAddress;
    record.byteSize = byteSize;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitFieldAddress(
    Address resultAddress,
    Address baseAddress,
    uint32_t fieldByteOffset,
    Address classTypeAddress)
{
    StagedRecord::FieldAddress record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::FieldAddress),
        1,
        StagedRecord::getByteSize<StagedRecord::FieldAddress>());
    record.header.resultAddress = resultAddress;
    record.baseAddress = baseAddress;
    record.fieldByteOffset = fieldByteOffset;
    record.classTypeAddress = classTypeAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitFieldExtract(
    Address resultAddress,
    Address baseAddress,
    uint32_t fieldByteOffset,
    uint32_t byteSize,
    Address classTypeAddress)
{
    StagedRecord::FieldExtract record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::FieldExtract),
        1,
        StagedRecord::getByteSize<StagedRecord::FieldExtract>());
    record.header.resultAddress = resultAddress;
    record.baseAddress = baseAddress;
    record.fieldByteOffset = fieldByteOffset;
    record.byteSize = byteSize;
    record.classTypeAddress = classTypeAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitElementAddress(
    Address resultAddress,
    Address baseAddress,
    Address indexAddress,
    uint32_t elementByteStride,
    uint32_t elementCount,
    Address runtimeArrayTypeAddress,
    ScalarType indexScalarType)
{
    StagedRecord::ElementAddress record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::ElementAddress),
        2,
        StagedRecord::getByteSize<StagedRecord::ElementAddress>());
    record.header.resultAddress = resultAddress;
    record.baseAddress = baseAddress;
    record.indexAddress = indexAddress;
    record.elementByteStride = elementByteStride;
    record.elementCount = elementCount;
    record.runtimeArrayTypeAddress = runtimeArrayTypeAddress;
    record.indexScalarType = indexScalarType;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitElementExtract(
    Address resultAddress,
    Address baseAddress,
    Address indexAddress,
    uint32_t elementByteStride,
    uint32_t elementByteSize,
    uint32_t elementCount)
{
    StagedRecord::ElementExtract record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::ExtractElement),
        2,
        StagedRecord::getByteSize<StagedRecord::ElementExtract>());
    record.header.resultAddress = resultAddress;
    record.baseAddress = baseAddress;
    record.indexAddress = indexAddress;
    record.elementByteStride = elementByteStride;
    record.elementByteSize = elementByteSize;
    record.elementCount = elementCount;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitRuntimeArrayGetCount(
    Address resultAddress,
    Address arrayAddress,
    Address runtimeArrayTypeAddress)
{
    StagedRecord::RuntimeArrayGetCount record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::RuntimeArrayGetCount),
        2,
        StagedRecord::getByteSize<StagedRecord::RuntimeArrayGetCount>());
    record.header.resultAddress = resultAddress;
    record.arrayAddress = arrayAddress;
    record.runtimeArrayTypeAddress = runtimeArrayTypeAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitCopyArrayRange(
    Address converterAddress,
    Address destinationAddress,
    Address destinationOffsetAddress,
    Address sourceAddress,
    Address sourceOffsetAddress,
    Address elementCountAddress,
    Address destinationRuntimeArrayTypeAddress,
    Address sourceRuntimeArrayTypeAddress,
    HeapObjectKind destinationKind,
    HeapObjectKind sourceKind)
{
    StagedRecord::CopyArrayRange record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::CopyArrayRange),
        0,
        StagedRecord::getByteSize<StagedRecord::CopyArrayRange>());
    record.converterAddress = converterAddress;
    record.destinationAddress = destinationAddress;
    record.destinationOffsetAddress = destinationOffsetAddress;
    record.sourceAddress = sourceAddress;
    record.sourceOffsetAddress = sourceOffsetAddress;
    record.elementCountAddress = elementCountAddress;
    record.destinationRuntimeArrayTypeAddress = destinationRuntimeArrayTypeAddress;
    record.sourceRuntimeArrayTypeAddress = sourceRuntimeArrayTypeAddress;
    record.destinationKind = destinationKind;
    record.sourceKind = sourceKind;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitAllocateResource(
    Address resultAddress,
    Address widthOrElementCountAddress,
    Address heightAddress,
    Address depthAddress,
    Address layoutAddress,
    HGBCResourceKind kind,
    HGBCResourceDimension dimension,
    HGBCTextureAccess textureAccess,
    rhi::Format format,
    rhi::MemoryType memoryType,
    rhi::ResourceState initialState,
    rhi::BufferUsage bufferUsage,
    rhi::TextureUsage textureUsage,
    uint32_t mipCount,
    uint32_t sampleCount,
    uint32_t sampleQuality,
    uint32_t textureElementByteSize)
{
    StagedRecord::AllocateResource record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::AllocateResource),
        3,
        StagedRecord::getByteSize<StagedRecord::AllocateResource>());
    record.header.resultAddress = resultAddress;
    record.widthOrElementCountAddress = widthOrElementCountAddress;
    record.heightAddress = heightAddress;
    record.depthAddress = depthAddress;
    record.layoutAddress = layoutAddress;
    record.kind = kind;
    record.dimension = dimension;
    record.textureAccess = textureAccess;
    record.format = format;
    record.memoryType = memoryType;
    record.initialState = initialState;
    record.bufferUsage = bufferUsage;
    record.textureUsage = textureUsage;
    record.mipCount = mipCount;
    record.sampleCount = sampleCount;
    record.sampleQuality = sampleQuality;
    record.textureElementByteSize = textureElementByteSize;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitReadBackBufferElement(
    HGBCOpcode opcode,
    Address resultAddress,
    Address converterAddress,
    Address bufferAddress,
    Address indexAddress,
    uint32_t resultByteSize)
{
    StagedRecord::ReadBackBufferElement record;
    record.header = makeHeader(
        uint32_t(opcode),
        2,
        StagedRecord::getByteSize<StagedRecord::ReadBackBufferElement>());
    record.header.resultAddress = resultAddress;
    record.converterAddress = converterAddress;
    record.bufferAddress = bufferAddress;
    record.indexAddress = indexAddress;
    record.resultByteSize = resultByteSize;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitWaitDefer(
    Address resultAddress,
    Address deferredAddress,
    uint32_t resultByteSize)
{
    StagedRecord::WaitDefer record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::WaitDefer),
        1,
        StagedRecord::getByteSize<StagedRecord::WaitDefer>());
    record.header.resultAddress = resultAddress;
    record.deferredAddress = deferredAddress;
    record.resultByteSize = resultByteSize;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitStructuredBufferDimensions(
    Address resultAddress,
    Address bufferAddress)
{
    StagedRecord::StructuredBufferDimensions record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::StructuredBufferGetDimensions),
        1,
        StagedRecord::getByteSize<StagedRecord::StructuredBufferDimensions>());
    record.header.resultAddress = resultAddress;
    record.bufferAddress = bufferAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitTextureDimensions(Address resultAddress, Address textureAddress)
{
    StagedRecord::TextureDimensions record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::TextureGetDimensions),
        1,
        StagedRecord::getByteSize<StagedRecord::TextureDimensions>());
    record.header.resultAddress = resultAddress;
    record.textureAddress = textureAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitCreateWindow(
    Address resultAddress,
    Address titleAddress,
    Address widthAddress,
    Address heightAddress)
{
    StagedRecord::CreateWindow record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::CreateWindow),
        3,
        StagedRecord::getByteSize<StagedRecord::CreateWindow>());
    record.header.resultAddress = resultAddress;
    record.titleAddress = titleAddress;
    record.widthAddress = widthAddress;
    record.heightAddress = heightAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitWindowOperation(
    HGBCOpcode opcode,
    Address resultAddress,
    Address windowAddress)
{
    StagedRecord::WindowOperation record;
    record.header =
        makeHeader(uint32_t(opcode), 1, StagedRecord::getByteSize<StagedRecord::WindowOperation>());
    record.header.resultAddress = resultAddress;
    record.windowAddress = windowAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitSetWindowTitle(Address windowAddress, Address titleAddress)
{
    StagedRecord::SetWindowTitle record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::SetWindowTitle),
        2,
        StagedRecord::getByteSize<StagedRecord::SetWindowTitle>());
    record.windowAddress = windowAddress;
    record.titleAddress = titleAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitPollInputEvent(
    Address resultAddress,
    Address windowAddress,
    ConstArrayView<uint32_t> fieldByteOffsets)
{
    StagedRecord::PollInputEvent record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::PollInputEvent),
        1,
        StagedRecord::getByteSize<StagedRecord::PollInputEvent>());
    record.header.resultAddress = resultAddress;
    record.windowAddress = windowAddress;
    SLANG_RELEASE_ASSERT(fieldByteOffsets.getCount() == 6);
    std::memcpy(
        record.fieldByteOffsets,
        fieldByteOffsets.getBuffer(),
        sizeof(record.fieldByteOffsets));
    return emitRecord(record);
}

Address StagedRecordBuilder::emitCreateSurface(
    Address resultAddress,
    Address windowAddress,
    Address imageCountAddress,
    rhi::Format requestedFormat)
{
    StagedRecord::CreateSurface record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::CreateSurface),
        2,
        StagedRecord::getByteSize<StagedRecord::CreateSurface>());
    record.header.resultAddress = resultAddress;
    record.windowAddress = windowAddress;
    record.imageCountAddress = imageCountAddress;
    record.requestedFormat = requestedFormat;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitSurfaceOperation(
    HGBCOpcode opcode,
    Address resultAddress,
    Address surfaceAddress)
{
    StagedRecord::SurfaceOperation record;
    record.header = makeHeader(
        uint32_t(opcode),
        1,
        StagedRecord::getByteSize<StagedRecord::SurfaceOperation>());
    record.header.resultAddress = resultAddress;
    record.surfaceAddress = surfaceAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitTryAcquireSurfaceTexture(
    Address resultAddress,
    Address surfaceAddress,
    Address destinationPointerAddress)
{
    StagedRecord::TryAcquireSurfaceTexture record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::TryAcquireSurfaceTexture),
        2,
        StagedRecord::getByteSize<StagedRecord::TryAcquireSurfaceTexture>());
    record.header.resultAddress = resultAddress;
    record.surfaceAddress = surfaceAddress;
    record.destinationPointerAddress = destinationPointerAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitResizeSurface(
    Address resultAddress,
    Address surfaceAddress,
    Address widthAddress,
    Address heightAddress)
{
    StagedRecord::ResizeSurface record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::ResizeSurface),
        3,
        StagedRecord::getByteSize<StagedRecord::ResizeSurface>());
    record.header.resultAddress = resultAddress;
    record.surfaceAddress = surfaceAddress;
    record.widthAddress = widthAddress;
    record.heightAddress = heightAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitBlitTexture(
    Address destinationAddress,
    Address sourceAddress,
    TextureScalarClass destinationScalarClass,
    TextureScalarClass sourceScalarClass)
{
    StagedRecord::BlitTexture record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::BlitTexture2D),
        2,
        StagedRecord::getByteSize<StagedRecord::BlitTexture>());
    record.destinationAddress = destinationAddress;
    record.sourceAddress = sourceAddress;
    record.destinationScalarClass = destinationScalarClass;
    record.sourceScalarClass = sourceScalarClass;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitClearTexture(
    Address textureAddress,
    Address valueAddress,
    Address stencilAddress,
    Address mipAddress,
    Address mipCountAddress,
    Address layerAddress,
    Address layerCountAddress,
    HGBCTextureAspect aspect,
    HGBCResourceFormat format,
    uint32_t valueByteSize,
    TextureScalarClass scalarClass)
{
    StagedRecord::ClearTexture record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::ClearTexture),
        stencilAddress ? 7 : 6,
        StagedRecord::getByteSize<StagedRecord::ClearTexture>());
    record.textureAddress = textureAddress;
    record.valueAddress = valueAddress;
    record.stencilAddress = stencilAddress;
    record.mipAddress = mipAddress;
    record.mipCountAddress = mipCountAddress;
    record.layerAddress = layerAddress;
    record.layerCountAddress = layerCountAddress;
    record.aspect = aspect;
    record.format = format;
    record.valueByteSize = valueByteSize;
    record.scalarClass = scalarClass;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitByteStreamFromFile(
    Address resultAddress,
    HGBCByteStreamAccess access,
    Address pathAddress)
{
    StagedRecord::ByteStreamFromFile record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::ByteStreamFromFile),
        1,
        StagedRecord::getByteSize<StagedRecord::ByteStreamFromFile>());
    record.header.resultAddress = resultAddress;
    record.access = access;
    record.pathAddress = pathAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitCloseByteStream(
    HGBCOpcode opcode,
    Address resultAddress,
    Address streamAddress)
{
    StagedRecord::CloseByteStream record;
    record.header =
        makeHeader(uint32_t(opcode), 1, StagedRecord::getByteSize<StagedRecord::CloseByteStream>());
    record.header.resultAddress = resultAddress;
    record.streamAddress = streamAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitByteStreamTransfer(
    HGBCOpcode opcode,
    Address resultAddress,
    HGBCByteStreamEndpointKind endpointKind,
    Address streamAddress,
    Address endpointAddress,
    Address endpointByteOffsetAddress,
    Address byteCountAddress,
    uint32_t hostByteExtent,
    Address runtimeArrayTypeAddress)
{
    StagedRecord::ByteStreamTransfer record;
    record.header = makeHeader(
        uint32_t(opcode),
        4,
        StagedRecord::getByteSize<StagedRecord::ByteStreamTransfer>());
    record.header.resultAddress = resultAddress;
    record.endpointKind = endpointKind;
    record.streamAddress = streamAddress;
    record.endpointAddress = endpointAddress;
    record.endpointByteOffsetAddress = endpointByteOffsetAddress;
    record.byteCountAddress = byteCountAddress;
    record.hostByteExtent = hostByteExtent;
    record.runtimeArrayTypeAddress = runtimeArrayTypeAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitImageOperation(
    HGBCOpcode opcode,
    Address resultAddress,
    HGBCImageCodec codec,
    HGBCEncodedImageEndpointKind encodedEndpointKind,
    HGBCPixelImageEndpointKind pixelEndpointKind,
    Address encodedEndpointAddress,
    Address pixelEndpointAddress)
{
    StagedRecord::ImageOperation record;
    record.header =
        makeHeader(uint32_t(opcode), 2, StagedRecord::getByteSize<StagedRecord::ImageOperation>());
    record.header.resultAddress = resultAddress;
    record.codec = codec;
    record.encodedEndpointKind = encodedEndpointKind;
    record.pixelEndpointKind = pixelEndpointKind;
    record.encodedEndpointAddress = encodedEndpointAddress;
    record.pixelEndpointAddress = pixelEndpointAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitMakeGraphicsDraw(
    Address resultAddress,
    Address count,
    Address first,
    Address instanceCount,
    Address firstInstance,
    Address baseVertex,
    Address indices)
{
    StagedRecord::MakeGraphicsDraw record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::MakeGraphicsDraw),
        0,
        StagedRecord::getByteSize<StagedRecord::MakeGraphicsDraw>());
    record.header.resultAddress = resultAddress;
    record.count = count;
    record.first = first;
    record.instanceCount = instanceCount;
    record.firstInstance = firstInstance;
    record.baseVertex = baseVertex;
    record.indices = indices;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitMakeGraphicsDrawParameters(
    Address resultAddress,
    ConstArrayView<Address> operands)
{
    StagedRecord::MakeGraphicsDrawParameters record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::MakeGraphicsDrawParameters),
        0,
        StagedRecord::getByteSize<StagedRecord::MakeGraphicsDrawParameters>());
    record.header.resultAddress = resultAddress;
    SLANG_RELEASE_ASSERT(operands.getCount() == 13);
    std::memcpy(record.operands, operands.getBuffer(), sizeof(record.operands));
    return emitRecord(record);
}

Address StagedRecordBuilder::emitParameter(
    Address resultAddress,
    uint32_t transferByteSize,
    ConstArrayView<uint32_t> elements)
{
    StagedRecord::Parameter record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::Param),
        0,
        StagedRecord::getByteSize<StagedRecord::Parameter, uint32_t>(elements.getCount()));
    record.managedLeafCount = uint32_t(elements.getCount());
    record.header.resultAddress = resultAddress;
    record.transferByteSize = transferByteSize;
    return emitRecord(record, elements);
}

Address StagedRecordBuilder::emitBox(
    Address resultAddress,
    Address valueAddress,
    Address payloadTypeAddress,
    uint32_t payloadByteSize,
    uint32_t payloadByteAlignment,
    ConstArrayView<uint32_t> managedLeafOffsets)
{
    StagedRecord::Box record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::Box),
        1,
        StagedRecord::getByteSize<StagedRecord::Box, uint32_t>(managedLeafOffsets.getCount()));
    record.header.resultAddress = resultAddress;
    record.valueAddress = valueAddress;
    record.payloadTypeAddress = payloadTypeAddress;
    record.payloadByteSize = payloadByteSize;
    record.payloadByteAlignment = payloadByteAlignment;
    record.managedLeafCount = uint32_t(managedLeafOffsets.getCount());
    return emitRecord(record, managedLeafOffsets);
}

Address StagedRecordBuilder::emitUnbox(
    HGBCOpcode opcode,
    Address resultAddress,
    Address sourceAddress,
    Address payloadTypeAddress,
    uint32_t payloadByteSize,
    uint32_t caseTag,
    ConstArrayView<uint32_t> managedLeafOffsets)
{
    StagedRecord::Unbox record;
    record.header = makeHeader(
        uint32_t(opcode),
        1,
        StagedRecord::getByteSize<StagedRecord::Unbox, uint32_t>(managedLeafOffsets.getCount()));
    record.header.resultAddress = resultAddress;
    record.sourceAddress = sourceAddress;
    record.payloadTypeAddress = payloadTypeAddress;
    record.payloadByteSize = payloadByteSize;
    record.caseTag = caseTag;
    record.managedLeafCount = uint32_t(managedLeafOffsets.getCount());
    return emitRecord(record, managedLeafOffsets);
}

Address StagedRecordBuilder::emitMakeTaggedUnion(
    Address resultAddress,
    Address caseAddress,
    Address payloadAddress,
    ConstArrayView<StagedRecord::TaggedUnionCase> cases)
{
    StagedRecord::MakeTaggedUnion record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::MakeTaggedUnion),
        2,
        StagedRecord::getByteSize<StagedRecord::MakeTaggedUnion, StagedRecord::TaggedUnionCase>(
            cases.getCount()));
    record.header.resultAddress = resultAddress;
    record.caseAddress = caseAddress;
    record.payloadAddress = payloadAddress;
    record.caseCount = uint32_t(cases.getCount());
    return emitRecord(record, cases);
}

Address StagedRecordBuilder::emitTaggedUnionAccess(
    HGBCOpcode opcode,
    Address resultAddress,
    Address sourceAddress)
{
    StagedRecord::TaggedUnionAccess record;
    record.header = makeHeader(uint32_t(opcode), 1, sizeof(record));
    record.header.resultAddress = resultAddress;
    record.sourceAddress = sourceAddress;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitConvertTaggedUnion(
    Address resultAddress,
    Address sourceAddress,
    ConstArrayView<StagedRecord::TaggedUnionCaseMapping> mappings)
{
    StagedRecord::ConvertTaggedUnion record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::ConvertTaggedUnion),
        1,
        StagedRecord::
            getByteSize<StagedRecord::ConvertTaggedUnion, StagedRecord::TaggedUnionCaseMapping>(
                mappings.getCount()));
    record.header.resultAddress = resultAddress;
    record.sourceAddress = sourceAddress;
    record.mappingCount = uint32_t(mappings.getCount());
    return emitRecord(record, mappings);
}

Address StagedRecordBuilder::emitAllocateObject(
    Address resultAddress,
    Address classTypeAddress,
    uint32_t allocationByteSize,
    uint32_t allocationByteAlignment,
    uint32_t managedLeafOffsetsByteOffset,
    ConstArrayView<uint32_t> elements)
{
    StagedRecord::AllocateObject record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::AllocateObject),
        0,
        StagedRecord::getByteSize<StagedRecord::AllocateObject, uint32_t>(elements.getCount()));
    record.managedLeafCount = uint32_t(elements.getCount());
    record.classTypeAddress = classTypeAddress;
    record.allocationByteSize = allocationByteSize;
    record.allocationByteAlignment = allocationByteAlignment;
    record.managedLeafOffsetsByteOffset = managedLeafOffsetsByteOffset;
    record.header.resultAddress = resultAddress;
    return emitRecord(record, elements);
}

Address StagedRecordBuilder::emitAllocateRuntimeArray(
    Address resultAddress,
    Address elementCountAddress,
    Address runtimeArrayTypeAddress,
    uint32_t elementByteStride,
    uint32_t elementByteAlignment,
    ScalarType countScalarType,
    ConstArrayView<uint32_t> elements)
{
    StagedRecord::AllocateRuntimeArray record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::AllocateRuntimeArray),
        1,
        StagedRecord::getByteSize<StagedRecord::AllocateRuntimeArray, uint32_t>(
            elements.getCount()));
    record.elementManagedLeafCount = uint32_t(elements.getCount());
    record.elementCountAddress = elementCountAddress;
    record.runtimeArrayTypeAddress = runtimeArrayTypeAddress;
    record.elementByteStride = elementByteStride;
    record.elementByteAlignment = elementByteAlignment;
    record.countScalarType = countScalarType;
    record.header.resultAddress = resultAddress;
    return emitRecord(record, elements);
}

Address StagedRecordBuilder::emitMakeAggregate(
    HGBCOpcode opcode,
    Address resultAddress,
    uint32_t byteSize,
    ConstArrayView<StagedRecord::AggregateElementCopy> elements)
{
    StagedRecord::MakeAggregate record;
    record.header = makeHeader(
        uint32_t(opcode),
        elements.getCount(),
        StagedRecord::getByteSize<StagedRecord::MakeAggregate, StagedRecord::AggregateElementCopy>(
            elements.getCount()));
    record.elementCount = uint32_t(elements.getCount());
    record.header.resultAddress = resultAddress;
    record.byteSize = byteSize;
    return emitRecord(record, elements);
}

Address StagedRecordBuilder::emitMakeNumeric(
    HGBCOpcode opcode,
    Address resultAddress,
    ScalarType scalarType,
    ConstArrayView<Address> elements)
{
    StagedRecord::MakeNumeric record;
    record.header = makeHeader(
        uint32_t(opcode),
        elements.getCount(),
        StagedRecord::getByteSize<StagedRecord::MakeNumeric, Address>(elements.getCount()));
    record.laneCount = uint32_t(elements.getCount());
    record.header.resultAddress = resultAddress;
    record.scalarType = scalarType;
    return emitRecord(record, elements);
}

Address StagedRecordBuilder::emitFormatString(
    Address resultAddress,
    Address patternAddress,
    ConstArrayView<StagedRecord::FormatArgument> elements)
{
    StagedRecord::FormatString record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::FormatString),
        uint64_t(elements.getCount()) + 1,
        StagedRecord::getByteSize<StagedRecord::FormatString, StagedRecord::FormatArgument>(
            elements.getCount()));
    record.argumentCount = uint32_t(elements.getCount());
    record.header.resultAddress = resultAddress;
    record.patternAddress = patternAddress;
    return emitRecord(record, elements);
}

Address StagedRecordBuilder::emitComputeCall(
    Address dispatcherAddress,
    Address groupCountAddress,
    ConstArrayView<StagedRecord::DispatchArgument> elements)
{
    StagedRecord::ComputeCall record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::Call) | StagedRecord::kComputeCallBit,
        elements.getCount(),
        StagedRecord::getByteSize<StagedRecord::ComputeCall, StagedRecord::DispatchArgument>(
            elements.getCount()));
    record.argumentCount = uint32_t(elements.getCount());
    record.dispatcherAddress = dispatcherAddress;
    record.groupCountAddress = groupCountAddress;
    return emitRecord(record, elements);
}

Address StagedRecordBuilder::emitGraphicsCall(
    Address pipelineAddress,
    Address colorAddress,
    Address depthAddress,
    Address drawAddress,
    Address parametersAddress,
    ConstArrayView<StagedRecord::DispatchArgument> elements)
{
    StagedRecord::GraphicsCall record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::Call) | StagedRecord::kGraphicsCallBit,
        elements.getCount(),
        StagedRecord::getByteSize<StagedRecord::GraphicsCall, StagedRecord::DispatchArgument>(
            elements.getCount()));
    record.argumentCount = uint32_t(elements.getCount());
    record.pipelineAddress = pipelineAddress;
    record.colorAddress = colorAddress;
    record.depthAddress = depthAddress;
    record.drawAddress = drawAddress;
    record.parametersAddress = parametersAddress;
    return emitRecord(record, elements);
}

Address StagedRecordBuilder::emitCall(Address resultAddress, ConstArrayView<Address> arguments)
{
    StagedRecord::Call record;
    record.functionAddress = Address{}; // Invalid until patchCallTarget resolves the callee.
    record.header = makeHeader(
        uint32_t(HGBCOpcode::Call),
        arguments.getCount(),
        StagedRecord::getByteSize<StagedRecord::Call, Address>(arguments.getCount()));
    record.header.resultAddress = resultAddress;
    Address address = emitRecord(record, arguments);
    ++m_pendingFixupCount;
    return address;
}

Address StagedRecordBuilder::emitBranch()
{
    StagedRecord::Branch record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::Branch),
        0,
        StagedRecord::getByteSize<StagedRecord::Branch>());
    record.targetProgramCounter = kInvalidInstructionWordOffset;
    Address address = emitRecord(record);
    ++m_pendingFixupCount;
    return address;
}

Address StagedRecordBuilder::emitBranchConditional(Address conditionAddress)
{
    StagedRecord::BranchConditional record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::BranchConditional),
        1,
        StagedRecord::getByteSize<StagedRecord::BranchConditional>());
    record.conditionAddress = conditionAddress;
    record.trueProgramCounter = record.falseProgramCounter = kInvalidInstructionWordOffset;
    Address address = emitRecord(record);
    ++m_pendingFixupCount;
    return address;
}

Address StagedRecordBuilder::emitFunction(uint32_t frameSize, uint32_t frameAlignment)
{
    StagedRecord::Function record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::Func),
        0,
        StagedRecord::getByteSize<StagedRecord::Function>());
    record.frameSize = frameSize;
    record.frameAlignment = frameAlignment;
    record.entryProgramCounter = kInvalidInstructionWordOffset;
    Address address = emitRecord(record);
    ++m_pendingFixupCount;
    return address;
}

Address StagedRecordBuilder::emitReturn()
{
    auto header = makeHeader(
        uint32_t(HGBCOpcode::Return),
        0,
        StagedRecord::getByteSize<StagedRecord::Header>());
    return emitRecord(header);
}

Address StagedRecordBuilder::emitFinishFrameExit()
{
    StagedRecord::FinishFrameExit record;
    record.header = makeHeader(
        StagedRecord::kInternalOpcodeBit | uint32_t(StagedRecord::InternalOpcode::FinishFrameExit),
        0,
        StagedRecord::getByteSize<StagedRecord::FinishFrameExit>());
    return emitRecord(record);
}

void StagedRecordBuilder::patchCallTarget(Address address, Address target)
{
    auto* record = getMutableRecord<StagedRecord::Call>(address);
    SLANG_RELEASE_ASSERT(record->header.opcodeAndFlags == uint32_t(HGBCOpcode::Call));
    SLANG_RELEASE_ASSERT(target.getKind() == AddressKind::InstructionAbsolute);
    SLANG_RELEASE_ASSERT(
        getRecord<StagedRecord::Function>(uint32_t(target.getUnsignedPayload()))
            ->header.opcodeAndFlags == uint32_t(HGBCOpcode::Func));
    SLANG_RELEASE_ASSERT(!record->functionAddress && m_pendingFixupCount);
    record->functionAddress = target;
    --m_pendingFixupCount;
}

void StagedRecordBuilder::patchBranchTarget(Address address, uint32_t target)
{
    auto* record = getMutableRecord<StagedRecord::Branch>(address);
    SLANG_RELEASE_ASSERT(record->header.opcodeAndFlags == uint32_t(HGBCOpcode::Branch));
    SLANG_RELEASE_ASSERT(
        target < getWordOffset() &&
        (size_t(target) * sizeof(uint32_t)) % StagedRecord::kRecordAlignment == 0);
    SLANG_RELEASE_ASSERT(
        record->targetProgramCounter == kInvalidInstructionWordOffset && m_pendingFixupCount);
    record->targetProgramCounter = target;
    --m_pendingFixupCount;
}

void StagedRecordBuilder::patchConditionalTargets(
    Address address,
    uint32_t trueTarget,
    uint32_t falseTarget)
{
    auto* record = getMutableRecord<StagedRecord::BranchConditional>(address);
    SLANG_RELEASE_ASSERT(record->header.opcodeAndFlags == uint32_t(HGBCOpcode::BranchConditional));
    SLANG_RELEASE_ASSERT(trueTarget < getWordOffset() && falseTarget < getWordOffset());
    SLANG_RELEASE_ASSERT(
        (size_t(trueTarget) * sizeof(uint32_t)) % StagedRecord::kRecordAlignment == 0 &&
        (size_t(falseTarget) * sizeof(uint32_t)) % StagedRecord::kRecordAlignment == 0);
    SLANG_RELEASE_ASSERT(
        record->trueProgramCounter == kInvalidInstructionWordOffset && m_pendingFixupCount);
    record->trueProgramCounter = trueTarget;
    record->falseProgramCounter = falseTarget;
    --m_pendingFixupCount;
}

void StagedRecordBuilder::patchFunction(
    Address address,
    uint32_t entry,
    uint32_t exit,
    uint32_t firstParameter,
    uint32_t parameterCount)
{
    auto* record = getMutableRecord<StagedRecord::Function>(address);
    SLANG_RELEASE_ASSERT(record->header.opcodeAndFlags == uint32_t(HGBCOpcode::Func));
    SLANG_RELEASE_ASSERT(entry < getWordOffset() && exit < getWordOffset());
    SLANG_RELEASE_ASSERT(
        record->entryProgramCounter == kInvalidInstructionWordOffset && m_pendingFixupCount);
    record->entryProgramCounter = entry;
    record->exitProgramCounter = exit;
    record->firstParameterByteOffset =
        (int64_t(firstParameter) - int64_t(address.getUnsignedPayload())) * sizeof(uint32_t);
    record->parameterCount = parameterCount;
    --m_pendingFixupCount;
}

Address StagedRecordBuilder::emitTypeInfo(
    const StagedRecord::TypeInfo& layout,
    ConstArrayView<StagedRecord::StructFieldInfo> fields,
    ConstArrayView<uint32_t> managedLeafOffsets)
{
    StagedRecord::TypeInfo record = layout;
    record.fieldCount = uint32_t(fields.getCount());
    record.managedLeafCount = uint32_t(managedLeafOffsets.getCount());
    if (record.managedLeafCount)
    {
        uint32_t offset = appendAlignedBytes(
            managedLeafOffsets.getBuffer(),
            record.managedLeafCount * uint32_t(sizeof(uint32_t)),
            alignof(uint32_t));
        record.managedLeafOffsetsAddress = Address::makeAbsoluteInstruction(offset);
    }
    return emitRecord(record, fields);
}

Address StagedRecordBuilder::emitSlangModule(
    Address name,
    uint32_t nameSize,
    Address binary,
    uint32_t binarySize,
    ConstArrayView<Address> entries)
{
    StagedRecord::LazyObject record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::SlangModule),
        0,
        StagedRecord::getByteSize<StagedRecord::LazyObject, Address>(entries.getCount()));
    record.kind = LazyObjectKind::SlangModule;
    record.firstOperandAddress = name;
    record.firstOperandByteSize = nameSize;
    record.secondOperandAddress = binary;
    record.secondOperandByteSize = binarySize;
    record.itemCount = uint32_t(entries.getCount());
    return emitRecord(record, entries);
}

Address StagedRecordBuilder::emitSlangEntryPoint(Address module, Address name, uint32_t nameSize)
{
    StagedRecord::LazyObject record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::SlangEntryPoint),
        0,
        StagedRecord::getByteSize<StagedRecord::LazyObject>());
    record.kind = LazyObjectKind::SlangEntryPoint;
    record.firstOperandAddress = module;
    record.secondOperandAddress = name;
    record.secondOperandByteSize = nameSize;
    return emitRecord(record);
}

Address StagedRecordBuilder::emitSpecializeSlangEntryPoint(
    Address base,
    ConstArrayView<StagedRecord::SpecializationArgument> entries)
{
    StagedRecord::LazyObject record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::SpecializeSlangEntryPoint),
        0,
        StagedRecord::getByteSize<StagedRecord::LazyObject, StagedRecord::SpecializationArgument>(
            entries.getCount()));
    record.kind = LazyObjectKind::SlangEntryPoint;
    record.firstOperandAddress = base;
    record.itemCount = uint32_t(entries.getCount());
    return emitRecord(record, entries);
}

Address StagedRecordBuilder::emitMakeDispatcher(Address entryPoint)
{
    StagedRecord::LazyObject record;
    record.header = makeHeader(
        uint32_t(HGBCOpcode::MakeDispatcher),
        0,
        StagedRecord::getByteSize<StagedRecord::LazyObject>());
    record.kind = LazyObjectKind::ComputeDispatcher;
    record.firstOperandAddress = entryPoint;
    return emitRecord(record);
}


} // namespace Slang::HGVM
