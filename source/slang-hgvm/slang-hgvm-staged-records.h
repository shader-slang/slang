#pragma once

#include "core/slang-array-view.h"
#include "slang-hgvm-types.h"

#include <slang-rhi.h>
#include <type_traits>

namespace Slang::HGVM::StagedRecord
{
static constexpr uint32_t kInternalOpcodeBit = 0x80000000u;
static constexpr uint32_t kComputeCallBit = 0x40000000u;
static constexpr uint32_t kGraphicsCallBit = 0x20000000u;
static constexpr uint32_t kBreakpointBit = 0x10000000u;
static constexpr uint32_t kDebugRecordBit = 0x08000000u;
// Records after the first lowering one HGBC instruction cannot be stopped independently:
// a copy and its ownership updates must finish before debugger cancellation is observed.
static constexpr uint32_t kInstructionContinuationBit = 0x04000000u;
static constexpr uint32_t kOpcodeMask = 0x0000ffffu;

/// Every complete record starts and ends at this alignment. Tail padding belongs to its
/// record extent, so advancing a PC never lands in padding before the following instruction.
inline constexpr size_t kRecordAlignment = alignof(uint64_t);

/// Returns the byte offset of a typed tail within its owning record.
template<typename Record, typename Element>
constexpr size_t getTailByteOffset()
{
    static_assert(alignof(Record) <= kRecordAlignment);
    static_assert(alignof(Element) <= kRecordAlignment);
    return (sizeof(Record) + alignof(Element) - 1) & ~(alignof(Element) - 1);
}

/// Returns the complete aligned extent, including the tail and final padding.
template<typename Record, typename Element = uint8_t>
constexpr uint64_t getByteSize(uint64_t count = 0)
{
    SLANG_RELEASE_ASSERT(
        (count <= (UINT64_MAX - getTailByteOffset<Record, Element>() - (kRecordAlignment - 1)) /
                      sizeof(Element)));
    return (getTailByteOffset<Record, Element>() + count * sizeof(Element) + kRecordAlignment - 1) &
           ~uint64_t(kRecordAlignment - 1);
}

/// Borrows the typed entries following a record in finalized instruction storage.
///
/// Consider a format operation with two arguments. The builder copies its prefix and argument
/// array into one allocation using this same tail offset. This accessor must be called on that
/// stored prefix, never on a standalone copy whose trailing storage does not exist.
template<typename Element, typename Record>
ConstArrayView<Element> getTail(const Record* record, uint32_t count)
{
    const auto* bytes = reinterpret_cast<const unsigned char*>(record);
    // The builder bounds the entire record's byte size by the maximum Index, so its tail count fits
    // signed Count even on 32-bit hosts.
    SLANG_RELEASE_ASSERT(uint64_t(count) <= uint64_t(std::numeric_limits<Count>::max()));
    return {
        reinterpret_cast<const Element*>(bytes + getTailByteOffset<Record, Element>()),
        Count(count)};
}

/// Describes one member copy trailing a `MakeAggregate`.
///
/// A struct field and an array element both use a direct source address, a destination byte
/// offset, and a byte size. No field, index, or type ID survives into execution.
struct AggregateElementCopy
{
    Address sourceAddress;
    uint32_t destinationByteOffset = 0;
    uint32_t byteSize = 0;
};

/// Describes one already classified value consumed by `FormatString`.
struct FormatArgument
{
    Address valueAddress;
    ValueKind kind = ValueKind::Scalar;
    ScalarType scalarType = ScalarType::UInt32;
};

/// Pairs one argument value with its host-to-shader layout converter.
struct DispatchArgument
{
    Address valueAddress;
    Address converterAddress;
};

/// Keeps the source type of a deferred shader argument until the native program is assembled.
/// Unlike compute dispatch arguments, a registered ray shader has no destination layout yet.
struct RayShaderArgument
{
    Address valueAddress;
    Address typeInfoAddress;
    uint32_t parameterIndex = 0;
    HGBCRayStageRole stage = HGBCRayStageRole::ClosestHit;
    HGBCId typeId = 0;
};

/// Describes one generic argument trailing a specialized entry-point descriptor.
/// For example, `fill<float, 4>` retains `(Type, "float")` and `(Expr, "4")` as immutable strings.
struct SpecializationArgument
{
    HGBCSpecializationArgKind kind = HGBCSpecializationArgKind::Type;
    Address valueAddress;
    uint32_t valueByteSize = 0;
};

/// Describes the common prefix of every executable record in the staged instruction stack.
/// The execution loop advances by recordWordCount, including the operand tail and padding.
struct Header
{
    Address resultAddress;
    uint32_t opcodeAndFlags = 0;
    uint16_t operandCount = 0;
    uint16_t recordWordCount = 0;
};

static_assert(sizeof(Header) == 4 * sizeof(uint32_t));

struct Parameter;

/// Updates debugger context using a scope index and inline depth resolved during staging.
struct DebugScope
{
    Header header;
    uint32_t scopeIndex = 0;
    /// Maintains logical stack depth without consulting the debug map during execution.
    uint32_t inlineSiteCount = 0;
};

/// Evaluates an element selector when the source assignment executes.
struct DebugElementOffset
{
    Address index;
    uint32_t stride = 0;
    uint32_t count = 0;
};

/// Copies a whole variable or an inline subobject into non-owning debug storage.
struct DebugValue
{
    Header header;
    Address source;
    uint32_t byteSize = 0;
    uint32_t destinationSize = 0;
    uint32_t staticOffset = 0;
    uint32_t elementCount = 0;
    ConstArrayView<DebugElementOffset> getElements() const
    {
        return getTail<DebugElementOffset>(this, elementCount);
    }
};

/// Records immutable metadata for one callable function in the instruction stack.
///
/// For example, a staged `main` points `entryProgramCounter` at its first `OpPrint` and
/// `exitProgramCounter` at the first record in its generated cleanup block. For a function with no
/// managed values, that record is `FinishFrameExit`.
struct Function
{
    Header header;
    uint32_t entryProgramCounter = 0;
    uint32_t exitProgramCounter = 0;
    uint32_t frameSize = 0;
    uint32_t frameAlignment = 1;
    int64_t firstParameterByteOffset = 0;
    uint32_t parameterCount = 0;

    /// Returns the first descriptor in this function's variable-sized parameter sequence.
    const Parameter* getFirstParameter() const
    {
        return parameterCount
                   ? reinterpret_cast<const Parameter*>(
                         reinterpret_cast<const unsigned char*>(this) + firstParameterByteOffset)
                   : nullptr;
    }
};

/// Describes one positional raw argument transfer into a callee frame.
///
/// For example, a `float` parameter transfers four value bytes, while an `inout float` parameter
/// transfers the eight bytes of its absolute `Address`. Call execution performs the same copy in
/// both cases and never inspects a source-language parameter kind.
struct Parameter
{
    Header header;
    uint32_t transferByteSize = 0;
    uint32_t managedLeafCount = 0;
    /// Returns the managedLeafOffsets stored immediately after this record.
    ConstArrayView<uint32_t> getManagedLeafOffsets() const
    {
        return getTail<uint32_t>(this, managedLeafCount);
    }

    /// Returns one managedLeafOffset by its logical index.
    const uint32_t* getManagedLeafOffset(uint32_t index) const
    {
        SLANG_RELEASE_ASSERT(index < managedLeafCount);
        return &getManagedLeafOffsets()[index];
    }

    /// Returns the following parameter after this record's aligned ownership tail.
    const Parameter* getNext() const
    {
        return reinterpret_cast<const Parameter*>(
            reinterpret_cast<const unsigned char*>(this) +
            size_t(header.recordWordCount) * sizeof(uint32_t));
    }
};

/// Begins one variable-size direct function-call record.
///
/// Consider `add(a, b)`: the prefix names `add`'s staged function record and is followed by the
/// direct stack addresses of `a` and `b`. `header.resultAddress` remains caller-owned storage.
struct Call
{
    Header header;
    Address functionAddress;
    /// Returns the arguments stored immediately after this record.
    ConstArrayView<Address> getArguments() const
    {
        return getTail<Address>(this, header.operandCount);
    }

    /// Returns one argument by its logical index.
    const Address* getArgument(uint32_t index) const
    {
        SLANG_RELEASE_ASSERT(index < header.operandCount);
        return &getArguments()[index];
    }
};

/// Describes one struct field in the compact host representation.
///
/// Consider `struct S { bool enabled; float weight; }`: the first entry names the staged `bool`
/// descriptor, shared field name, and offset zero, while the second names `float`, `weight`, and
/// offset four. Layout conversion resolves the names against reflection because optimization can
/// remove empty fields from the host type while the shader layout retains them.
struct StructFieldInfo
{
    Address typeInfoAddress;
    Address fieldNameAddress;
    uint32_t fieldNameByteCount = 0;
    uint32_t sourceByteOffset = 0;
};

/// Stores the immutable size and alignment chosen for one staged HGBC type.
///
/// Consider `Array<Sample, 3>` where `Sample` has size eight and alignment four: its descriptor
/// records size 24, alignment four, element stride eight, and the `Sample` descriptor address.
/// `Ptr<Sample>` instead points `valueTypeInfoAddress` at `Sample`. Ordinary loads and stores only
/// copy the recorded size and never visit aggregate members.
struct TypeInfo
{
    TypeKind kind = TypeKind::Scalar;
    HeapObjectKind heapObjectKind = HeapObjectKind::StructuredBuffer;
    DescriptorAccess descriptorAccess = DescriptorAccess::None;
    NumericLayout numericLayout;
    Address valueTypeInfoAddress;
    Address elementTypeInfoAddress;
    /// Points to the canonical managed offsets used when values outlive their caller frame.
    Address managedLeafOffsetsAddress;
    uint32_t byteSize = 0;
    uint32_t byteAlignment = 1;
    uint32_t elementByteStride = 0;
    uint32_t elementCount = 0;
    uint32_t matrixLayout = 0;
    uint32_t fieldCount = 0;
    uint32_t managedLeafCount = 0;

    /// Returns the field descriptors stored with this type, without resolving another address.
    ConstArrayView<StructFieldInfo> getFields() const
    {
        return getTail<StructFieldInfo>(this, fieldCount);
    }
};

/// Creates one canonical absolute stack address for an `OpVar` result.
///
/// For example, if the current frame starts at byte 64 and `storageByteOffset` is 24, execution
/// stores absolute-frame address 88 in `header.resultAddress`. Every later load, store, or call can
/// consume that address without knowing which frame originally allocated the variable.
struct Var
{
    Header header;
    uint32_t storageByteOffset = 0;
};

/// Copies bytes from the storage named by an address value into an inline result slot.
///
/// For example, loading a 16-byte struct reads an `Address` from `pointerAddress` and performs one
/// 16-byte copy into `header.resultAddress`; it does not inspect any struct field.
struct Load
{
    Header header;
    Address pointerAddress;
    uint32_t byteSize = 0;
};

/// Copies an owning graph resource reference into its explicitly typed descriptor-handle value.
struct ResourceToDescriptorHandle
{
    Header header;
    Address resourceAddress;
};

/// Allocates one mutable shader registry with a runtime-selected number of ray slots.
struct MakeRaytracingPipeline
{
    Header header;
    Address rayTypeCountAddress;
};

/// Keeps dynamic ray-generation arguments typed until the complete pipeline is assembled.
struct DispatchRays
{
    Header header;
    Address entryPointAddress;
    Address extentAddress;
    uint32_t maxRecursion = 1;
    uint32_t argumentCount = 0;

    ConstArrayView<RayShaderArgument> getArguments() const
    {
        return getTail<RayShaderArgument>(this, argumentCount);
    }
};

struct AllocRayAccelerationStructure
{
    Header header;
    bool topLevel = false;
};

/// Resolves the public descriptor's named fields once, before interpreting build operations.
/// Triangle offsets name vertices, indices, vertex offset/count, index offset/count, and flags.
/// Instance offsets name BLAS, transform, instance ID, mask, and flags.
struct BuildRayAccelerationStructure
{
    Header header;
    HGBCAccelerationStructureInputKind inputKind = HGBCAccelerationStructureInputKind::Triangles;
    Address structureAddress;
    Address inputsAddress;
    Address buildFlagsAddress;
    Address buildModeAddress;
    Address inputsTypeAddress;
    bool topLevel = false;
    uint32_t fieldOffsets[7] = {};
};

/// Replaces one registration, retaining checked stage roles and deferred argument types.
/// A miss uses entryPoints[0]; a hit group uses the explicit closest-hit/any-hit/intersection
/// roles.
struct SetRayShader
{
    Header header;
    Address pipelineAddress;
    Address geometrySlotAddress;
    Address raySlotAddress;
    Address entryPoints[3];
    HGBCId payloadType = 0;
    HGBCId attributeType = 0;
    HGBCRayGeometryKind geometryKind = HGBCRayGeometryKind::Triangles;
    uint32_t argumentCount = 0;

    ConstArrayView<RayShaderArgument> getArguments() const
    {
        return getTail<RayShaderArgument>(this, argumentCount);
    }
};

/// Copies the graph-object address stored by a typed handle back into a resource value.
struct DescriptorHandleToResource
{
    Header header;
    Address handleAddress;
};

/// Copies one inline value into storage named by an address value.
///
/// For example, storing an eight-byte struct reads the destination `Address` from `pointerAddress`
/// and copies eight bytes from `valueAddress`, including the struct's layout padding.
struct Store
{
    Header header;
    Address pointerAddress;
    Address valueAddress;
    uint32_t byteSize = 0;
};

/// Produces a field address using the offset calculated once during staging.
///
/// Consider `&sample.weight` where `weight` begins at byte four: execution reads the absolute base
/// address, adds four, and stores the resulting absolute address in `header.resultAddress`.
struct FieldAddress
{
    Header header;
    Address baseAddress;
    uint32_t fieldByteOffset = 0;
    Address classTypeAddress;
};

/// Copies one field out of an inline struct value without a runtime field lookup.
///
/// For example, extracting an eight-byte nested struct at offset four copies from
/// `baseAddress + 4` into the result. The explicit source extent lets address resolution remain
/// unaware of the aggregate's type or total size.
struct FieldExtract
{
    Header header;
    Address baseAddress;
    uint32_t fieldByteOffset = 0;
    uint32_t byteSize = 0;
    Address classTypeAddress;
};

/// Produces one fixed-array element address using precomputed stride and bounds metadata.
///
/// Consider `&values[index]` for `Array<float, 4>`: execution reads `index`, multiplies it by the
/// staged four-byte stride, and adds that byte offset to the canonical address read from
/// `baseAddress`. No array type descriptor is consulted on the hot path.
struct ElementAddress
{
    Header header;
    Address baseAddress;
    Address indexAddress;
    uint32_t elementByteStride = 0;
    uint32_t elementCount = 0;
    Address runtimeArrayTypeAddress;
    ScalarType indexScalarType = ScalarType::UInt32;
};

/// Copies one fixed-array element from an inline array value using its precomputed layout.
///
/// Consider `values[index]` for `Array<Pair, 3>`: execution adds `index * elementByteStride` to
/// `baseAddress`, then copies `elementByteSize` bytes into the result. `Pair` remains opaque.
struct ElementExtract
{
    Header header;
    Address baseAddress;
    Address indexAddress;
    uint32_t elementByteStride = 0;
    uint32_t elementByteSize = 0;
    uint32_t elementCount = 0;
};

/// Begins one variable-size aggregate-construction record.
///
/// Consider both `Pair { x, y }` and `Array<uint, 2>(x, y)`: this prefix is followed by two
/// `AggregateElementCopy` entries. The result slot is part of an already-zeroed frame, so
/// the runtime only applies those precomputed copies without distinguishing structs from arrays.
struct MakeAggregate
{
    Header header;
    uint32_t byteSize = 0;
    uint32_t elementCount = 0;
    /// Returns the elementCopies stored immediately after this record.
    ConstArrayView<AggregateElementCopy> getElementCopies() const
    {
        return getTail<AggregateElementCopy>(this, elementCount);
    }

    /// Returns one elementCopy by its logical index.
    const AggregateElementCopy* getElementCopy(uint32_t index) const
    {
        SLANG_RELEASE_ASSERT(index < elementCount);
        return &getElementCopies()[index];
    }
};

/// Stores a fixed descriptor for any literal without making its payload part of record traversal.
///
/// Consider a multi-megabyte `OpBinaryLit`: this record remains fixed-size and `dataAddress` points
/// at separately appended bytes, so the payload is not constrained by 16-bit `recordWordCount`.
struct Literal
{
    Header header;
    ValueKind kind = ValueKind::Scalar;
    ScalarType scalarType = ScalarType::UInt32;
    Address dataAddress;
    uint32_t byteSize = 0;
};

/// Stores the execution-ready representation of one scalar or string print.
///
/// For example, `print(3u)` embeds `UInt32` and an eight-byte value address, while `print("hi")`
/// embeds the address of its fixed-width `StringValue`. Execution follows only `kind` and, for a
/// scalar, `scalarType`; it never resolves an HGBC type.
struct Print
{
    Header header;
    ValueKind kind = ValueKind::Scalar;
    ScalarType scalarType = ScalarType::UInt32;
    Address valueAddress;
};

/// Begins one variable-size string-format record followed by typed argument descriptors.
///
/// Consider `format("{}: {:.1f}", name, rate)`: `patternAddress` names one `StringValue`, while
/// two trailing `FormatArgument`s classify `name` as a String and `rate` as Float32.
/// The runtime parses only the pattern text and never resolves an HGBC type or value ID.
struct FormatString
{
    Header header;
    Address patternAddress;
    uint32_t argumentCount = 0;
    /// Returns the arguments stored immediately after this record.
    ConstArrayView<FormatArgument> getArguments() const
    {
        return getTail<FormatArgument>(this, argumentCount);
    }

    /// Returns one argument by its logical index.
    const FormatArgument* getArgument(uint32_t index) const
    {
        SLANG_RELEASE_ASSERT(index < argumentCount);
        return &getArguments()[index];
    }
};

/// Stores one String operation whose only source is a fixed-width String value.
///
/// For example, `OpStringGetByteCount` writes the string's staged byte count, while `OpFail`
/// copies the same bytes into the invocation failure before unwinding through function exit blocks.
struct StringOperation
{
    Header header;
    Address stringAddress;
};

/// Stores the direct String and index addresses used by `OpStringReadByte`.
///
/// For example, reading byte three performs a bounds check against the embedded byte count and
/// writes one zero-extended byte to the UInt32 result slot.
struct StringReadByte
{
    Header header;
    Address stringAddress;
    Address indexAddress;
};

/// Creates, queries, or resets one managed wall-clock timer through a direct handle address.
///
/// `OpCreateTimer` writes a new handle through `header.resultAddress`. `OpTimerElapsed`
/// additionally writes a Float32 result there, while `OpResetTimer` only consumes `timerAddress`.
struct TimerOperation
{
    Header header;
    Address timerAddress;
};

/// Stores the common execution-ready shape of unary numeric operations.
///
/// For example, staged `OpSin float3` contains `Float32`, lane count three, and the direct address
/// of its packed input. Execution dispatches `Float32` once and evaluates all three lanes.
struct Unary
{
    Header header;
    ScalarType scalarType = ScalarType::UInt32;
    uint32_t laneCount = 1;
    Address operandAddress;
};

/// Stores one scalar cast after both HGBC type IDs have become direct execution codes.
///
/// Consider `float(i)`: staging records `Int32`, `Float32`, and `i`'s address. Execution performs
/// two compact enum dispatches and never resolves either source type record or the runtime value's
/// type. `header.resultAddress` already has the destination type's size and alignment.
struct ScalarTypeCast
{
    Header header;
    ScalarType sourceType = ScalarType::UInt32;
    ScalarType destinationType = ScalarType::UInt32;
    Address sourceAddress;
};

/// Stores the common execution-ready shape of binary numeric and comparison operations.
///
/// For example, `OpLess float4` carries `Float32`, lane count four, and two direct packed input
/// addresses, while `header.resultAddress` names four one-byte Boolean result lanes.
struct Binary
{
    Header header;
    ScalarType scalarType = ScalarType::UInt32;
    uint32_t laneCount = 1;
    Address leftAddress;
    Address rightAddress;
};

/// Begins a variable-size vector or matrix construction record.
///
/// Consider `float3(x, y, z)`: this prefix is followed by three source addresses. The destination
/// lanes are packed, so execution derives every destination offset from the scalar representation
/// and needs no per-lane copy descriptor.
struct MakeNumeric
{
    Header header;
    ScalarType scalarType = ScalarType::UInt32;
    uint32_t laneCount = 0;
    /// Returns the lanes stored immediately after this record.
    ConstArrayView<Address> getLanes() const { return getTail<Address>(this, laneCount); }

    /// Returns one lane by its logical index.
    const Address* getLane(uint32_t index) const
    {
        SLANG_RELEASE_ASSERT(index < laneCount);
        return &getLanes()[index];
    }
};

/// Redirects a conditional branch to one of two direct staged program counters.
///
/// For example, `if (%condition)` reads one canonical boolean byte and assigns either the already
/// patched true or false PC; no block-ID lookup occurs in the execution loop.
struct BranchConditional
{
    Header header;
    Address conditionAddress;
    uint32_t trueProgramCounter = 0;
    uint32_t falseProgramCounter = 0;
};

/// Redirects an unconditional branch to one direct staged program counter.
///
/// For example, non-SSA HGBC `OpBranch %merge` becomes this fixed record. Phi elimination has
/// already converted any value transfer into explicit `OpStore` and `OpLoad` instructions.
struct Branch
{
    Header header;
    uint32_t targetProgramCounter = 0;
};

/// Describes one class allocation followed by its object-relative managed-leaf offsets.
///
/// Consider `new Holder()` where `Holder` contains a `Counter` and a resource: the fixed prefix
/// records the complete allocation layout and two trailing offsets. Execution copies those offsets
/// after the object's fields so its reference-counted destruction is independent of the staged
/// program.
struct AllocateObject
{
    Header header;
    Address classTypeAddress;
    uint32_t allocationByteSize = 0;
    uint32_t allocationByteAlignment = 1;
    uint32_t managedLeafOffsetsByteOffset = 0;
    uint32_t managedLeafCount = 0;
    /// Returns the managedLeafOffsets stored immediately after this record.
    ConstArrayView<uint32_t> getManagedLeafOffsets() const
    {
        return getTail<uint32_t>(this, managedLeafCount);
    }

    /// Returns one managedLeafOffset by its logical index.
    const uint32_t* getManagedLeafOffset(uint32_t index) const
    {
        SLANG_RELEASE_ASSERT(index < managedLeafCount);
        return &getManagedLeafOffsets()[index];
    }
};

/// Boxes a concrete value using its staged layout and trailing managed-leaf offsets.
struct Box
{
    Header header;
    Address valueAddress;
    Address payloadTypeAddress;
    uint32_t payloadByteSize = 0;
    uint32_t payloadByteAlignment = 1;
    uint32_t managedLeafCount = 0;

    ConstArrayView<uint32_t> getManagedLeafOffsets() const
    {
        return getTail<uint32_t>(this, managedLeafCount);
    }
};

/// Extracts an owned concrete copy after checking its actual type and, when present, union case.
struct Unbox
{
    Header header;
    Address sourceAddress;
    Address payloadTypeAddress;
    uint32_t payloadByteSize = 0;
    uint32_t caseTag = 0;
    uint32_t managedLeafCount = 0;

    ConstArrayView<uint32_t> getManagedLeafOffsets() const
    {
        return getTail<uint32_t>(this, managedLeafCount);
    }
};

/// Defines one legal case at a tagged-value construction site; an invalid type means no payload.
struct TaggedUnionCase
{
    uint32_t caseTag = 0;
    Address payloadTypeAddress;
};

struct MakeTaggedUnion
{
    Header header;
    Address caseAddress;
    Address payloadAddress;
    uint32_t caseCount = 0;

    ConstArrayView<TaggedUnionCase> getCases() const
    {
        return getTail<TaggedUnionCase>(this, caseCount);
    }
};

/// Reads the case or owning payload from one initialized tagged value.
struct TaggedUnionAccess
{
    Header header;
    Address sourceAddress;
};

/// Maps a validated source case to its destination case without copying the boxed payload.
struct TaggedUnionCaseMapping
{
    uint32_t sourceCaseTag = 0;
    uint32_t destinationCaseTag = 0;
    uint32_t destinationCaseIndexPlusOne = 0;
};

struct ConvertTaggedUnion
{
    Header header;
    Address sourceAddress;
    uint32_t mappingCount = 0;

    ConstArrayView<TaggedUnionCaseMapping> getMappings() const
    {
        return getTail<TaggedUnionCaseMapping>(this, mappingCount);
    }
};

/// Describes one runtime-array allocation followed by the managed leaves within one element.
struct AllocateRuntimeArray
{
    Header header;
    Address elementCountAddress;
    Address runtimeArrayTypeAddress;
    uint32_t elementByteStride = 0;
    uint32_t elementByteAlignment = 1;
    uint32_t elementManagedLeafCount = 0;
    ScalarType countScalarType = ScalarType::UInt32;
    /// Returns the managedLeafOffsets stored immediately after this record.
    ConstArrayView<uint32_t> getManagedLeafOffsets() const
    {
        return getTail<uint32_t>(this, elementManagedLeafCount);
    }

    /// Returns one managedLeafOffset by its logical index.
    const uint32_t* getManagedLeafOffset(uint32_t index) const
    {
        SLANG_RELEASE_ASSERT(index < elementManagedLeafCount);
        return &getManagedLeafOffsets()[index];
    }
};

/// Reads the immutable element count stored in one runtime-array object.
struct RuntimeArrayGetCount
{
    Header header;
    Address arrayAddress;
    Address runtimeArrayTypeAddress;
};

/// Copies one element range between host runtime arrays and/or device structured buffers.
struct CopyArrayRange
{
    Header header;
    Address converterAddress;
    Address destinationAddress;
    Address destinationOffsetAddress;
    Address sourceAddress;
    Address sourceOffsetAddress;
    Address elementCountAddress;
    Address destinationRuntimeArrayTypeAddress;
    Address sourceRuntimeArrayTypeAddress;
    HeapObjectKind destinationKind = HeapObjectKind::RuntimeArray;
    HeapObjectKind sourceKind = HeapObjectKind::RuntimeArray;
};

/// Consider `RWStructuredBuffer<uint>::alloc(4)`: HGBC identifies a device-local buffer, its
/// initial state and usages, the `uint` element type, and the three logical extents `{4, 1, 1}`.
/// Staging resolves only addresses and RHI enums; execution follows `kind` and never asks what
/// type produced `header.resultAddress`.
struct AllocateResource
{
    Header header;
    Address widthOrElementCountAddress;
    Address heightAddress;
    Address depthAddress;
    Address layoutAddress;
    HGBCResourceKind kind = HGBCResourceKind::Buffer;
    HGBCResourceDimension dimension = HGBCResourceDimension::Buffer;
    HGBCTextureAccess textureAccess = HGBCTextureAccess::ReadWrite;
    rhi::Format format = rhi::Format::Undefined;
    rhi::MemoryType memoryType = rhi::MemoryType::DeviceLocal;
    rhi::ResourceState initialState = rhi::ResourceState::Undefined;
    rhi::BufferUsage bufferUsage = rhi::BufferUsage::None;
    rhi::TextureUsage textureUsage = rhi::TextureUsage::None;
    uint32_t mipCount = 1;
    uint32_t sampleCount = 1;
    uint32_t sampleQuality = 0;
    uint32_t textureElementByteSize = 0;
};

/// Reads one element from a structured buffer synchronously or starts a deferred transfer.
///
/// Consider `readBack(buffer, 0)`: `resultByteSize` is the inline result extent. For
/// `asyncReadBack(buffer, 0)`, it is the eventual `T` extent while `header.resultAddress` names the
/// `Defer<T>` heap-handle slot. The opcode selects those two paths without any type lookup.
struct ReadBackBufferElement
{
    Header header;
    Address converterAddress;
    Address bufferAddress;
    Address indexAddress;
    uint32_t resultByteSize = 0;
};

/// Resolves one staged deferred readback into an inline result slot.
///
/// For example, `pending.wait()` embeds only the `Defer<T>` handle address and the exact size of
/// `T`. The deferred object already owns the staging buffer and layout converter selected by the
/// originating async operation.
struct WaitDefer
{
    Header header;
    Address deferredAddress;
    uint32_t resultByteSize = 0;
};

/// Returns the cached element count and byte stride of one structured buffer.
///
/// For example, a four-element `RWStructuredBuffer<uint>` writes `{4, 4}` directly into its
/// staged `uint2` result without reflecting the resource type during execution.
struct StructuredBufferDimensions
{
    Header header;
    Address bufferAddress;
};

/// Returns the width and height from one texture wrapper's RHI descriptor.
///
/// For example, querying a 16-by-9 texture writes `{16, 9}` directly into its staged `uint2`
/// result without looking up either the resource or result HGBC types.
struct TextureDimensions
{
    Header header;
    Address textureAddress;
};

/// Creates a platform window from one String and two scalar frame locations.
///
/// Consider `createWindow("Demo", 1280, 720)`: staging embeds the three direct addresses here, so
/// execution reads their fixed representations without resolving HGBC IDs or operand types.
struct CreateWindow
{
    Header header;
    Address titleAddress;
    Address widthAddress;
    Address heightAddress;
};

/// Executes a window operation that needs only the managed window handle.
///
/// `OpProcessWindowEvents` writes its Boolean result through `header.resultAddress`;
/// `OpCloseWindow` leaves that address invalid. Both otherwise have exactly the same staged operand
/// shape.
struct WindowOperation
{
    Header header;
    Address windowAddress;
};

/// Changes one window title using direct window and String locations.
struct SetWindowTitle
{
    Header header;
    Address windowAddress;
    Address titleAddress;
};

/// Writes the six validated fields of the public input-event struct at precomputed offsets.
///
/// Consider `{ type, key, button, pos, scroll, size }`: the host returns one canonical C++ event,
/// while these offsets adapt it to the interpreter's deterministic struct layout without runtime
/// field names or reflection.
struct PollInputEvent
{
    Header header;
    Address windowAddress;
    uint32_t fieldByteOffsets[6] = {};
};

/// Creates a host surface with a statically selected logical format.
///
/// For example, `Surface<RGBA16Float>` becomes `rhi::Format::RGBA16Float` during staging; execution
/// reads only the window handle and requested image count before calling the host service.
struct CreateSurface
{
    Header header;
    Address windowAddress;
    Address imageCountAddress;
    rhi::Format requestedFormat = rhi::Format::Undefined;
};

/// Queries, waits for, acquires, or presents a surface through its direct managed-handle location.
///
/// Acquisition transfers a new texture wrapper into `header.resultAddress`. Presentation first
/// submits the DeviceState batch containing the corresponding blit, then calls the host service.
struct SurfaceOperation
{
    Header header;
    Address surfaceAddress;
};

/// Writes an acquired texture through an owning destination slot and reports availability.
struct TryAcquireSurfaceTexture
{
    Header header;
    Address surfaceAddress;
    Address destinationPointerAddress;
};

/// Reconfigures a surface at an explicit application-selected synchronization boundary.
struct ResizeSurface
{
    Header header;
    Address surfaceAddress;
    Address widthAddress;
    Address heightAddress;
};

/// Blits one complete typed texture into another using direct managed-handle locations.
///
/// The endpoint HGBC types were consumed during staging. RHI resources carry their concrete
/// backend formats, so execution records one format-converting fullscreen pass without type lookup.
struct BlitTexture
{
    Header header;
    Address destinationAddress;
    Address sourceAddress;
    TextureScalarClass destinationScalarClass = TextureScalarClass::Float;
    TextureScalarClass sourceScalarClass = TextureScalarClass::Float;
};

/// Records a clear separately from any pipeline, retaining its target, clear values, and
/// subresource bounds.
struct ClearTexture
{
    Header header;
    Address textureAddress;
    Address valueAddress;
    Address stencilAddress = {};
    Address mipAddress;
    Address mipCountAddress;
    Address layerAddress;
    Address layerCountAddress;
    HGBCTextureAspect aspect;
    HGBCResourceFormat format;
    uint32_t valueByteSize;
    TextureScalarClass scalarClass = TextureScalarClass::Float;
};

/// Opens one file-backed sequential byte stream using a staged access mode and String address.
///
/// For example, `OpByteStreamFromFile Read %path` resolves `%path` once and transfers the newly
/// created stream object's initial program reference into `header.resultAddress`.
struct ByteStreamFromFile
{
    Header header;
    HGBCByteStreamAccess access = HGBCByteStreamAccess::Read;
    Address pathAddress;
};

/// Names one stream object for explicit close without releasing its program reference.
struct CloseByteStream
{
    Header header;
    Address streamAddress;
};

/// Describes one exact raw transfer between a stream and a statically selected endpoint kind.
///
/// Consider a host `out uint`: `endpointAddress` names the slot containing its absolute `Address`,
/// and `hostByteExtent` is four. Device buffers and runtime arrays instead name slots containing
/// heap handles; the enum selects their representation without a type lookup during execution.
struct ByteStreamTransfer
{
    Header header;
    HGBCByteStreamEndpointKind endpointKind = HGBCByteStreamEndpointKind::HostAddress;
    Address streamAddress;
    Address endpointAddress;
    Address endpointByteOffsetAddress;
    Address byteCountAddress;
    uint32_t hostByteExtent = 0;
    Address runtimeArrayTypeAddress;
};

/// Describes one codec transformation between an encoded endpoint and a pixel endpoint.
///
/// For example, EXR decode stages `(EXR, ByteStream, DeviceTexture2D)` plus two direct value
/// addresses. Execution switches only on those enums and never reopens either operand's type.
struct ImageOperation
{
    Header header;
    HGBCImageCodec codec = HGBCImageCodec::EXR;
    HGBCEncodedImageEndpointKind encodedEndpointKind = HGBCEncodedImageEndpointKind::ByteStream;
    HGBCPixelImageEndpointKind pixelEndpointKind = HGBCPixelImageEndpointKind::DeviceTexture2D;
    Address encodedEndpointAddress;
    Address pixelEndpointAddress;
};

/// Stores the immutable RHI state and entry points used to create a graphics pipeline.
struct GraphicsPipeline
{
    Header header;
    LazyObjectKind kind = LazyObjectKind::GraphicsPipeline;
    Address vertexEntryPoint;
    Address fragmentEntryPoint;
    rhi::PrimitiveTopology topology = rhi::PrimitiveTopology::TriangleList;
    rhi::RasterizerDesc rasterizer;
    rhi::DepthStencilDesc depthStencil;
    rhi::MultisampleDesc multisample;
    rhi::ColorTargetDesc colorTarget;
};

struct GraphicsCall
{
    Header header;
    Address pipelineAddress;
    Address colorAddress;
    Address depthAddress;
    Address drawAddress;
    Address parametersAddress;
    uint32_t argumentCount = 0;
    /// Returns the arguments stored immediately after this record.
    ConstArrayView<DispatchArgument> getArguments() const
    {
        return getTail<DispatchArgument>(this, argumentCount);
    }

    /// Returns one argument by its logical index.
    const DispatchArgument* getArgument(uint32_t index) const
    {
        SLANG_RELEASE_ASSERT(index < argumentCount);
        return &getArguments()[index];
    }
};

struct MakeGraphicsDraw
{
    Header header;
    Address count;
    Address first;
    Address instanceCount;
    Address firstInstance;
    Address baseVertex;
    Address indices;
};

struct MakeGraphicsDrawParameters
{
    Header header;
    Address operands[13];
};

/// Begins a staged compute-dispatch call followed by typed binding descriptors.
/// For example, `dispatch(fill, uint3(1))(buffer, 42)` embeds the dispatcher, group count,
/// and two DispatchArgument entries while retaining the OpCall opcode.
struct ComputeCall
{
    Header header;
    Address dispatcherAddress;
    Address groupCountAddress;
    uint32_t argumentCount = 0;
    /// Returns the arguments stored immediately after this record.
    ConstArrayView<DispatchArgument> getArguments() const
    {
        return getTail<DispatchArgument>(this, argumentCount);
    }

    /// Returns one argument by its logical index.
    const DispatchArgument* getArgument(uint32_t index) const
    {
        SLANG_RELEASE_ASSERT(index < argumentCount);
        return &getArguments()[index];
    }
};

/// Describes one domain-specific type layout before its const object is first resolved.
///
/// For `kernel(uint3 id : SV_DispatchThreadID, uniform Empty empty, uniform uint value)`, the
/// layout for `value` has `contextIndex` two even if the host's empty argument was erased.
/// `contextAddress` names the dispatcher; the index identifies the original reflected parameter.
/// A host layout instead uses `hostTypeInfoAddress` and leaves the context empty.
struct TypeLayoutObject
{
    Header header;
    LazyObjectKind kind = LazyObjectKind::TypeLayout;
    HGBCId logicalTypeId = 0;
    HGBCLayoutDomain domain = HGBCLayoutDomain::Host;
    HGBCLayoutUsage usage = HGBCLayoutUsage::Value;
    Address hostTypeInfoAddress;
    Address contextAddress;
    uint32_t contextIndex = 0;
    uint32_t entryPointIndex = 0;
    bool graphicsContext = false;
};

/// Describes two const layouts whose conversion rules are resolved together on first use.
///
/// For example, a readback descriptor names `(device Particle, host Particle)`; it does not point
/// at a buffer, so the same immutable converter can be reused by every `Particle` buffer.
struct LayoutConverterObject
{
    Header header;
    LazyObjectKind kind = LazyObjectKind::LayoutConverter;
    Address sourceLayoutAddress;
    Address destinationLayoutAddress;
};

/// Describes one lazily evaluated EMBD value in immutable instruction storage.
///
/// For example, a module descriptor stores addresses for its name and binary plus trailing const
/// addresses for imported modules. The corresponding in-place const object points back to this
/// descriptor on first read.
struct LazyObject
{
    Header header;
    LazyObjectKind kind = LazyObjectKind::SlangModule;
    Address firstOperandAddress;
    Address secondOperandAddress;
    uint32_t firstOperandByteSize = 0;
    uint32_t secondOperandByteSize = 0;
    uint32_t itemCount = 0;
    /// Returns the imported modules of a module descriptor.
    ConstArrayView<Address> getDependencies() const
    {
        SLANG_RELEASE_ASSERT(
            (header.opcodeAndFlags & kOpcodeMask) == uint32_t(HGBCOpcode::SlangModule));
        return getTail<Address>(this, itemCount);
    }

    /// Returns the typed arguments of a specialized entry point.
    ConstArrayView<SpecializationArgument> getSpecializationArguments() const
    {
        SLANG_RELEASE_ASSERT(
            (header.opcodeAndFlags & kOpcodeMask) ==
            uint32_t(HGBCOpcode::SpecializeSlangEntryPoint));
        return getTail<SpecializationArgument>(this, itemCount);
    }
};

/// Names the small set of bookkeeping records that have no HGBC semantic opcode.
///
/// For example, source `OpReturn` jumps to the current function's cleanup block. Its final
/// `FinishFrameExit` record either restores caller traversal or completes the root invocation.
enum class InternalOpcode : uint16_t
{
    IncRef,
    MaybeDecRefAndRelease,
    FinishFrameExit,
};

/// Applies one explicit reference operation to a heap-handle leaf.
///
/// A fixed frame cleanup embeds the leaf address directly. An assignment through `Ptr<T>` instead
/// sets `isIndirect`, causing execution to read the destination `Address` from the named slot
/// before adding `leafByteOffset`. This dual form lets fixed cleanup and indirect assignment share
/// one execution record until generated support functions can be staged as ordinary calls.
struct Reference
{
    Header header;
    Address targetAddress;
    uint32_t leafByteOffset = 0;
    uint32_t isIndirect = 0;
};

/// Marks the end of the mandatory function exit block.
///
/// For example, hello world's void `main` has no resources to release, so its exit block consists
/// only of this record and transitions the interpreter from `Running` to `Completed`.
struct FinishFrameExit
{
    Header header;
};

/// Returns the fixed word count encoded in a staged C++ record type.
///
/// For example, `getRecordWordCount<Header>()` returns four and is checked before the
/// value is narrowed into the 16-bit record header.
template<typename T>
constexpr uint16_t getRecordWordCount()
{
    static_assert(getByteSize<T>() / sizeof(uint32_t) <= std::numeric_limits<uint16_t>::max());
    return uint16_t(getByteSize<T>() / sizeof(uint32_t));
}

} // namespace Slang::HGVM::StagedRecord
