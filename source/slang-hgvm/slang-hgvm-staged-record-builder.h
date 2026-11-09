#pragma once

#include "core/slang-allocator.h"
#include "core/slang-list.h"
#include "slang-hgvm-staged-records.h"

#include <cstring>

namespace Slang::HGVM
{

class StagedRecordBuilder;

/// Owns committed staged records and at most one unpublished append transaction.
/// Word addresses survive growth; native pointers are borrowed only until the next append.
class StagedRecordStorage
{
public:
    StagedRecordStorage() = default;
    StagedRecordStorage(const StagedRecordStorage&) = delete;
    StagedRecordStorage& operator=(const StagedRecordStorage&) = delete;

    Index getWordCount() const { return m_committedByteSize / sizeof(uint32_t); }
    const unsigned char* getBuffer() const { return m_bytes.getBuffer(); }
    size_t getByteSize() const { return size_t(m_committedByteSize); }

    /// Changes only a debugger break bit. The caller supplies an executable record boundary and
    /// serializes this operation with execution; all operands and record extents remain frozen.
    void setBreakpoint(uint32_t wordOffset, bool enabled)
    {
        auto* header =
            const_cast<StagedRecord::Header*>(getRecord<StagedRecord::Header>(wordOffset));
        if (enabled)
            header->opcodeAndFlags |= StagedRecord::kBreakpointBit;
        else
            header->opcodeAndFlags &= ~StagedRecord::kBreakpointBit;
    }

    /// Borrows a committed record, including while another program is being appended.
    /// The pointer must not be retained across an operation that can grow the storage.
    template<typename T>
    const T* getRecord(uint32_t wordOffset) const
    {
        return getRecordWithinExtent<T>(wordOffset, getByteSize());
    }

private:
    friend class StagedRecordBuilder;

    template<typename T>
    const T* getRecordDuringConstruction(uint32_t wordOffset) const
    {
        return getRecordWithinExtent<T>(wordOffset, size_t(m_bytes.getCount()));
    }

    template<typename T>
    const T* getRecordWithinExtent(uint32_t wordOffset, size_t byteSize) const
    {
        static_assert(std::is_trivially_copyable_v<T>);
        size_t offset = size_t(wordOffset) * sizeof(uint32_t);
        SLANG_RELEASE_ASSERT(offset <= byteSize && sizeof(T) <= byteSize - offset);
        const auto* bytes = m_bytes.getBuffer() + offset;
        SLANG_RELEASE_ASSERT(reinterpret_cast<uintptr_t>(bytes) % alignof(T) == 0);
        return reinterpret_cast<const T*>(bytes);
    }

    List<unsigned char, AlignedAllocator<kStorageBaseAlignment>> m_bytes;
    Index m_committedByteSize = 0;
    StagedRecordBuilder* m_activeBuilder = nullptr;
};

/// Assembles execution-ready records from already resolved operands and metadata.
///
/// Consider format("{}", value): the stager classifies value, then emitFormatString writes
/// its prefix, argument array, and padding together. All references retained during staging
/// are word addresses, so reallocating the byte buffer cannot invalidate a pending fixup.
class StagedRecordBuilder
{
public:
    /// Begins an append transaction after the existing committed records.
    explicit StagedRecordBuilder(StagedRecordStorage& storage);

    /// Discards this transaction's unpublished records unless finalize() committed them.
    ~StagedRecordBuilder();

    StagedRecordBuilder(const StagedRecordBuilder&) = delete;
    StagedRecordBuilder& operator=(const StagedRecordBuilder&) = delete;

    /// Marks the next aligned record location, including a block containing no source operations.
    uint32_t getWordOffset() const;

    /// Marks the remaining records of one source instruction as indivisible debugger steps.
    /// For example, a store and its following retains cannot be interrupted independently.
    void markInstructionContinuations(uint32_t firstWordOffset);

    /// Borrows a committed or pending record for staging and const-object initialization.
    /// For example, a new layout can be inspected before its program is published. The borrow
    /// ends before the next emission, rollback, or destruction of this transaction.
    template<typename T>
    const T* getRecord(uint32_t wordOffset) const
    {
        SLANG_RELEASE_ASSERT(m_storage.m_activeBuilder == this);
        return m_storage.getRecordDuringConstruction<T>(wordOffset);
    }

    /// Copies a separately addressed literal payload, with padding outside its logical extent.
    uint32_t appendAlignedBytes(
        const void* bytes,
        uint32_t byteCount,
        uint32_t alignment = sizeof(uint32_t));

    /// Publishes this append after every function and branch target has been patched.
    void finalize();

    /// Discards the unpublished suffix without changing any committed record or address.
    void rollback();

    Address emitDebugScope(uint32_t scopeIndex, uint32_t inlineSiteCount);
    Address emitDebugValue(
        Address destination,
        Address source,
        uint32_t byteSize,
        uint32_t destinationSize,
        uint32_t staticOffset,
        ConstArrayView<StagedRecord::DebugElementOffset> elements);

    /// Emits one complete Literal record from resolved operands.
    Address emitLiteral(
        HGBCOpcode opcode,
        Address resultAddress,
        ValueKind kind,
        ScalarType scalarType,
        Address dataAddress,
        uint32_t byteSize);

    /// Emits one complete GraphicsPipeline record from resolved operands.
    Address emitGraphicsPipeline(
        Address vertexEntryPoint,
        Address fragmentEntryPoint,
        rhi::PrimitiveTopology topology,
        const rhi::RasterizerDesc& rasterizer,
        const rhi::DepthStencilDesc& depthStencil,
        const rhi::MultisampleDesc& multisample,
        const rhi::ColorTargetDesc& colorTarget);

    /// Emits one complete TypeLayoutObject record from resolved operands.
    Address emitTypeLayoutObject(
        HGBCId logicalTypeId,
        HGBCLayoutDomain domain,
        HGBCLayoutUsage usage,
        Address hostTypeInfoAddress,
        Address contextAddress,
        uint32_t contextIndex,
        uint32_t entryPointIndex,
        bool graphicsContext);

    /// Emits one complete LayoutConverterObject record from resolved operands.
    Address emitLayoutConverterObject(
        Address sourceLayoutAddress,
        Address destinationLayoutAddress);

    /// Emits one complete Print record from resolved operands.
    Address emitPrint(ValueKind kind, ScalarType scalarType, Address valueAddress);

    /// Emits one complete StringOperation record from resolved operands.
    Address emitStringOperation(HGBCOpcode opcode, Address resultAddress, Address stringAddress);

    /// Emits one complete StringReadByte record from resolved operands.
    Address emitStringReadByte(Address resultAddress, Address stringAddress, Address indexAddress);

    /// Emits one complete TimerOperation record from resolved operands.
    Address emitTimerOperation(HGBCOpcode opcode, Address resultAddress, Address timerAddress);

    Address emitMakeRaytracingPipeline(Address resultAddress, Address rayTypeCountAddress);
    Address emitDispatchRays(
        const StagedRecord::DispatchRays& dispatch,
        ConstArrayView<StagedRecord::RayShaderArgument> arguments);
    Address emitAllocRayAccelerationStructure(Address result, bool topLevel);
    Address emitBuildRayAccelerationStructure(
        const StagedRecord::BuildRayAccelerationStructure& build);
    Address emitSetRayShader(
        HGBCOpcode opcode,
        const StagedRecord::SetRayShader& registration,
        ConstArrayView<StagedRecord::RayShaderArgument> arguments);

    /// Emits one complete Unary record from resolved operands.
    Address emitUnary(
        HGBCOpcode opcode,
        Address resultAddress,
        ScalarType scalarType,
        uint32_t laneCount,
        Address operandAddress);

    /// Emits one complete ScalarTypeCast record from resolved operands.
    Address emitScalarTypeCast(
        Address resultAddress,
        ScalarType sourceType,
        ScalarType destinationType,
        Address sourceAddress);

    /// Emits one complete Binary record from resolved operands.
    Address emitBinary(
        HGBCOpcode opcode,
        Address resultAddress,
        ScalarType scalarType,
        uint32_t laneCount,
        Address leftAddress,
        Address rightAddress);

    /// Emits one complete Var record from resolved operands.
    Address emitVar(Address resultAddress, uint32_t storageByteOffset);

    /// Emits one complete Reference record from resolved operands.
    Address emitReference(
        StagedRecord::InternalOpcode opcode,
        Address targetAddress,
        uint32_t leafByteOffset,
        uint32_t isIndirect);

    /// Emits one complete Load record from resolved operands.
    Address emitLoad(Address resultAddress, Address pointerAddress, uint32_t byteSize);

    /// Emits one complete ResourceToDescriptorHandle record from resolved operands.
    Address emitResourceToDescriptorHandle(Address resultAddress, Address resourceAddress);

    /// Emits one complete DescriptorHandleToResource record from resolved operands.
    Address emitDescriptorHandleToResource(Address resultAddress, Address handleAddress);

    /// Emits one complete Store record from resolved operands.
    Address emitStore(Address pointerAddress, Address valueAddress, uint32_t byteSize);

    /// Emits one complete FieldAddress record from resolved operands.
    Address emitFieldAddress(
        Address resultAddress,
        Address baseAddress,
        uint32_t fieldByteOffset,
        Address classTypeAddress);

    /// Emits one complete FieldExtract record from resolved operands.
    Address emitFieldExtract(
        Address resultAddress,
        Address baseAddress,
        uint32_t fieldByteOffset,
        uint32_t byteSize,
        Address classTypeAddress);

    /// Emits one complete ElementAddress record from resolved operands.
    Address emitElementAddress(
        Address resultAddress,
        Address baseAddress,
        Address indexAddress,
        uint32_t elementByteStride,
        uint32_t elementCount,
        Address runtimeArrayTypeAddress,
        ScalarType indexScalarType);

    /// Emits one complete ElementExtract record from resolved operands.
    Address emitElementExtract(
        Address resultAddress,
        Address baseAddress,
        Address indexAddress,
        uint32_t elementByteStride,
        uint32_t elementByteSize,
        uint32_t elementCount);

    /// Emits one complete RuntimeArrayGetCount record from resolved operands.
    Address emitRuntimeArrayGetCount(
        Address resultAddress,
        Address arrayAddress,
        Address runtimeArrayTypeAddress);

    /// Emits one complete CopyArrayRange record from resolved operands.
    Address emitCopyArrayRange(
        Address converterAddress,
        Address destinationAddress,
        Address destinationOffsetAddress,
        Address sourceAddress,
        Address sourceOffsetAddress,
        Address elementCountAddress,
        Address destinationRuntimeArrayTypeAddress,
        Address sourceRuntimeArrayTypeAddress,
        HeapObjectKind destinationKind,
        HeapObjectKind sourceKind);

    /// Emits one complete AllocateResource record from resolved operands.
    Address emitAllocateResource(
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
        uint32_t textureElementByteSize);

    /// Emits one complete ReadBackBufferElement record from resolved operands.
    Address emitReadBackBufferElement(
        HGBCOpcode opcode,
        Address resultAddress,
        Address converterAddress,
        Address bufferAddress,
        Address indexAddress,
        uint32_t resultByteSize);

    /// Emits one complete WaitDefer record from resolved operands.
    Address emitWaitDefer(Address resultAddress, Address deferredAddress, uint32_t resultByteSize);

    /// Emits one complete StructuredBufferDimensions record from resolved operands.
    Address emitStructuredBufferDimensions(Address resultAddress, Address bufferAddress);

    /// Emits one complete TextureDimensions record from resolved operands.
    Address emitTextureDimensions(Address resultAddress, Address textureAddress);

    /// Emits one complete CreateWindow record from resolved operands.
    Address emitCreateWindow(
        Address resultAddress,
        Address titleAddress,
        Address widthAddress,
        Address heightAddress);

    /// Emits one complete WindowOperation record from resolved operands.
    Address emitWindowOperation(HGBCOpcode opcode, Address resultAddress, Address windowAddress);

    /// Emits one complete SetWindowTitle record from resolved operands.
    Address emitSetWindowTitle(Address windowAddress, Address titleAddress);

    /// Emits one complete PollInputEvent record from resolved operands.
    Address emitPollInputEvent(
        Address resultAddress,
        Address windowAddress,
        ConstArrayView<uint32_t> fieldByteOffsets);

    /// Emits one complete CreateSurface record from resolved operands.
    Address emitCreateSurface(
        Address resultAddress,
        Address windowAddress,
        Address imageCountAddress,
        rhi::Format requestedFormat);

    /// Emits one complete SurfaceOperation record from resolved operands.
    Address emitSurfaceOperation(HGBCOpcode opcode, Address resultAddress, Address surfaceAddress);

    /// Emits a recoverable acquisition into the addressed texture slot.
    Address emitTryAcquireSurfaceTexture(
        Address resultAddress,
        Address surfaceAddress,
        Address destinationPointerAddress);

    /// Emits an explicit surface resize with runtime width and height.
    Address emitResizeSurface(
        Address resultAddress,
        Address surfaceAddress,
        Address widthAddress,
        Address heightAddress);

    /// Emits one complete BlitTexture record from resolved operands.
    Address emitBlitTexture(
        Address destinationAddress,
        Address sourceAddress,
        TextureScalarClass destinationScalarClass,
        TextureScalarClass sourceScalarClass);

    /// Emits one complete ClearTexture record from resolved operands.
    Address emitClearTexture(
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
        TextureScalarClass scalarClass);

    /// Emits one complete ByteStreamFromFile record from resolved operands.
    Address emitByteStreamFromFile(
        Address resultAddress,
        HGBCByteStreamAccess access,
        Address pathAddress);

    /// Emits one complete CloseByteStream record from resolved operands.
    Address emitCloseByteStream(HGBCOpcode opcode, Address resultAddress, Address streamAddress);

    /// Emits one complete ByteStreamTransfer record from resolved operands.
    Address emitByteStreamTransfer(
        HGBCOpcode opcode,
        Address resultAddress,
        HGBCByteStreamEndpointKind endpointKind,
        Address streamAddress,
        Address endpointAddress,
        Address endpointByteOffsetAddress,
        Address byteCountAddress,
        uint32_t hostByteExtent,
        Address runtimeArrayTypeAddress);

    /// Emits one complete ImageOperation record from resolved operands.
    Address emitImageOperation(
        HGBCOpcode opcode,
        Address resultAddress,
        HGBCImageCodec codec,
        HGBCEncodedImageEndpointKind encodedEndpointKind,
        HGBCPixelImageEndpointKind pixelEndpointKind,
        Address encodedEndpointAddress,
        Address pixelEndpointAddress);

    /// Emits one complete MakeGraphicsDraw record from resolved operands.
    Address emitMakeGraphicsDraw(
        Address resultAddress,
        Address count,
        Address first,
        Address instanceCount,
        Address firstInstance,
        Address baseVertex,
        Address indices);

    /// Emits one complete MakeGraphicsDrawParameters record from resolved operands.
    Address emitMakeGraphicsDrawParameters(Address resultAddress, ConstArrayView<Address> operands);

    /// Emits one complete Parameter record from resolved operands.
    Address emitParameter(
        Address resultAddress,
        uint32_t transferByteSize,
        ConstArrayView<uint32_t> elements);

    /// Emits an owned-erasure operation after its concrete layouts and case maps are resolved.
    Address emitBox(
        Address resultAddress,
        Address valueAddress,
        Address payloadTypeAddress,
        uint32_t payloadByteSize,
        uint32_t payloadByteAlignment,
        ConstArrayView<uint32_t> managedLeafOffsets);
    Address emitUnbox(
        HGBCOpcode opcode,
        Address resultAddress,
        Address sourceAddress,
        Address payloadTypeAddress,
        uint32_t payloadByteSize,
        uint32_t caseTag,
        ConstArrayView<uint32_t> managedLeafOffsets);
    Address emitMakeTaggedUnion(
        Address resultAddress,
        Address caseAddress,
        Address payloadAddress,
        ConstArrayView<StagedRecord::TaggedUnionCase> cases);
    Address emitTaggedUnionAccess(HGBCOpcode opcode, Address resultAddress, Address sourceAddress);
    Address emitConvertTaggedUnion(
        Address resultAddress,
        Address sourceAddress,
        ConstArrayView<StagedRecord::TaggedUnionCaseMapping> mappings);

    /// Emits one complete AllocateObject record from resolved operands.
    Address emitAllocateObject(
        Address resultAddress,
        Address classTypeAddress,
        uint32_t allocationByteSize,
        uint32_t allocationByteAlignment,
        uint32_t managedLeafOffsetsByteOffset,
        ConstArrayView<uint32_t> elements);

    /// Emits one complete AllocateRuntimeArray record from resolved operands.
    Address emitAllocateRuntimeArray(
        Address resultAddress,
        Address elementCountAddress,
        Address runtimeArrayTypeAddress,
        uint32_t elementByteStride,
        uint32_t elementByteAlignment,
        ScalarType countScalarType,
        ConstArrayView<uint32_t> elements);

    /// Emits one complete MakeAggregate record from resolved operands.
    Address emitMakeAggregate(
        HGBCOpcode opcode,
        Address resultAddress,
        uint32_t byteSize,
        ConstArrayView<StagedRecord::AggregateElementCopy> elements);

    /// Emits one complete MakeNumeric record from resolved operands.
    Address emitMakeNumeric(
        HGBCOpcode opcode,
        Address resultAddress,
        ScalarType scalarType,
        ConstArrayView<Address> elements);

    /// Emits one complete FormatString record from resolved operands.
    Address emitFormatString(
        Address resultAddress,
        Address patternAddress,
        ConstArrayView<StagedRecord::FormatArgument> elements);

    /// Emits one complete ComputeCall record from resolved operands.
    Address emitComputeCall(
        Address dispatcherAddress,
        Address groupCountAddress,
        ConstArrayView<StagedRecord::DispatchArgument> elements);

    /// Emits one complete GraphicsCall record from resolved operands.
    Address emitGraphicsCall(
        Address pipelineAddress,
        Address colorAddress,
        Address depthAddress,
        Address drawAddress,
        Address parametersAddress,
        ConstArrayView<StagedRecord::DispatchArgument> elements);

    /// Emits one complete Call record from resolved operands.
    Address emitCall(Address resultAddress, ConstArrayView<Address> arguments);

    /// Emits one complete Branch record from resolved operands.
    Address emitBranch();

    /// Emits one complete BranchConditional record from resolved operands.
    Address emitBranchConditional(Address conditionAddress);

    /// Emits one complete Function record from resolved operands.
    Address emitFunction(uint32_t frameSize, uint32_t frameAlignment);

    /// Emits one complete Return record from resolved operands.
    Address emitReturn();

    /// Emits one complete FinishFrameExit record from resolved operands.
    Address emitFinishFrameExit();

    /// Resolves the pending targets of one previously emitted Call record.
    void patchCallTarget(Address address, Address target);

    /// Resolves the pending targets of one previously emitted Branch record.
    void patchBranchTarget(Address address, uint32_t target);

    /// Resolves the pending targets of one previously emitted BranchConditional record.
    void patchConditionalTargets(Address address, uint32_t trueTarget, uint32_t falseTarget);

    /// Resolves the pending targets of one previously emitted Function record.
    void patchFunction(
        Address address,
        uint32_t entry,
        uint32_t exit,
        uint32_t firstParameter,
        uint32_t parameterCount);

    /// Emits an immutable TypeInfo descriptor and its owned data.
    Address emitTypeInfo(
        const StagedRecord::TypeInfo& layout,
        ConstArrayView<StagedRecord::StructFieldInfo> fields,
        ConstArrayView<uint32_t> managedLeafOffsets);

    /// Emits an immutable SlangModule descriptor and its owned data.
    Address emitSlangModule(
        Address name,
        uint32_t nameSize,
        Address binary,
        uint32_t binarySize,
        ConstArrayView<Address> entries);

    /// Emits an immutable SlangEntryPoint descriptor and its owned data.
    Address emitSlangEntryPoint(Address module, Address name, uint32_t nameSize);

    /// Emits an immutable SpecializeSlangEntryPoint descriptor and its owned data.
    Address emitSpecializeSlangEntryPoint(
        Address base,
        ConstArrayView<StagedRecord::SpecializationArgument> entries);

    /// Emits an immutable MakeDispatcher descriptor and its owned data.
    Address emitMakeDispatcher(Address entryPoint);


private:
    static StagedRecord::Header makeHeader(
        uint32_t opcode,
        uint64_t operandCount,
        uint64_t byteSize);
    uint32_t allocate(uint64_t byteSize, uint32_t alignment = StagedRecord::kRecordAlignment);
    void validatePatchAddress(Address address) const;

    /// Copies a complete fixed record into aligned byte storage. memcpy establishes the
    /// lifetime of these implicit-lifetime objects without exposing typed mutable storage.
    template<typename T>
    Address emitRecord(const T& record)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(alignof(T) <= StagedRecord::kRecordAlignment);
        uint32_t offset = allocate(StagedRecord::getByteSize<T>());
        std::memcpy(
            m_storage.m_bytes.getBuffer() + size_t(offset) * sizeof(uint32_t),
            &record,
            sizeof(T));
        return Address::makeAbsoluteInstruction(offset);
    }

    /// Writes one prefix and its typed operand array as a single complete record.
    template<typename T, typename Element>
    Address emitRecord(const T& record, ConstArrayView<Element> elements)
    {
        static_assert(std::is_trivially_copyable_v<T> && std::is_trivially_copyable_v<Element>);
        SLANG_RELEASE_ASSERT(
            elements.getCount() >= 0 && uint64_t(elements.getCount()) <= UINT32_MAX);
        uint32_t offset = allocate(StagedRecord::getByteSize<T, Element>(elements.getCount()));
        auto* bytes = m_storage.m_bytes.getBuffer() + size_t(offset) * sizeof(uint32_t);
        std::memcpy(bytes, &record, sizeof(T));
        if (elements.getCount())
            std::memcpy(
                bytes + StagedRecord::getTailByteOffset<T, Element>(),
                elements.getBuffer(),
                size_t(elements.getCount()) * sizeof(Element));
        return Address::makeAbsoluteInstruction(offset);
    }

    template<typename T>
    T* getMutableRecord(Address address)
    {
        validatePatchAddress(address);
        return const_cast<T*>(
            m_storage.getRecordDuringConstruction<T>(uint32_t(address.getUnsignedPayload())));
    }

    StagedRecordStorage& m_storage;
    Index m_startByteSize = 0;
    uint32_t m_pendingFixupCount = 0;
};

} // namespace Slang::HGVM
