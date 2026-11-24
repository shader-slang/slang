#pragma once

#include "core/slang-basic.h"
#include "slang-hgbc-format.h"
#include "slang-hgvm-events.h"
#include "slang-hgvm-invocation.h"

#include <cstddef>
#include <limits>
#include <type_traits>

namespace Slang::HGVM
{
static constexpr uint64_t kAddressTagShift = 56;
static constexpr uint64_t kAddressPayloadMask = (uint64_t(1) << kAddressTagShift) - 1;
// Staged offsets remain 32-bit even though the uniform address payload leaves 56 bits available.
static constexpr uint64_t kInstructionPayloadLimit = uint64_t(UINT32_MAX) + 1;
static constexpr uint64_t kFramePayloadLimit = uint64_t(UINT32_MAX) + 1;
static constexpr uint32_t kStorageBaseAlignment = 64;
static constexpr uint32_t kInvalidInstructionWordOffset = 0xffffffffu;

struct HeapObject;

/// Identifies how the payload of an `Address` locates its bytes.
///
/// `Native` is the only kind a program value of an address type (`Ptr`, `Out`, `InOut`, `Ref`,
/// `BorrowIn`) may have: its payload is a raw host address that stays valid for the lifetime of
/// the storage it names. The other kinds appear only as staged operands and internal VM handles.
/// Consider `absoluteInstruction(12)`: its kind is `InstructionAbsolute`, and its payload names
/// word 12 in the VM's shared staging storage rather than a host pointer.
enum class AddressKind : uint8_t
{
    Native = 0x00,
    FrameRelative = 0x01,
    ConstHeap = 0x02,
    InstructionRelative = 0x03,
    InstructionAbsolute = 0x04,
    Invalid = 0xfe,
    Forbidden = 0xff,
};

/// Stores one logical interpreter address with its address space in the high byte.
///
/// For example, `Address::makeAbsoluteInstruction(12)` stores the instruction tag in bits 63:56
/// and word offset 12 in bits 55:0. A native address instead has a zero tag and stores the
/// complete host pointer in the payload, so its eight bytes equal the pointer value itself.
struct Address
{
    uint64_t value = 0;

    enum class Tag : uint8_t
    {
        Native = 0x00,
        FrameRelative = 0x01,
        ConstHeap = 0x02,
        InstructionRelative = 0x03,
        InstructionAbsolute = 0x04,
        Forbidden = 0xff,
    };

    static Address makeTagged(Tag tag, uint64_t payload)
    {
        SLANG_RELEASE_ASSERT(payload <= kAddressPayloadMask);
        Address result;
        result.value = (uint64_t(tag) << kAddressTagShift) | payload;
        return result;
    }

    /// Encodes an absolute instruction-stack word address when it fits the staged 32-bit offset.
    ///
    /// For example, word 7 stores 7 below the `InstructionAbsolute` tag.
    static Address makeAbsoluteInstruction(uint32_t wordOffset)
    {
        SLANG_ASSERT(wordOffset < kInstructionPayloadLimit);
        return makeTagged(Tag::InstructionAbsolute, wordOffset);
    }

    /// Encodes a byte offset relative to the current frame when it fits the staged 32-bit offset.
    ///
    /// For example, a scalar slot at byte offset 24 stores 24 below the `FrameRelative` tag; a
    /// callee can use the same embedded value because resolution adds that callee's frame base.
    /// Only staged operands use this form; a program value that names frame storage is the
    /// `Native` address of the frame base plus the offset.
    static Address makeRelativeFrame(uint32_t byteOffset)
    {
        SLANG_ASSERT(byteOffset < kFramePayloadLimit);
        return makeTagged(Tag::FrameRelative, byteOffset);
    }

    /// Encodes the byte offset of one object in the interpreter-local const heap.
    ///
    /// For example, a staged `OpMakeDispatcher` laid out at const offset 16 stores 16 below the
    /// `ConstHeap` tag. Reading it resolves directly to the in-place dispatcher object rather than
    /// first loading a separate heap handle.
    static Address makeConstHeap(uint32_t byteOffset)
    {
        SLANG_ASSERT(byteOffset < kFramePayloadLimit);
        return makeTagged(Tag::ConstHeap, byteOffset);
    }

    /// Encodes one raw host address after proving that its high byte is available for the tag.
    ///
    /// The check is deliberately performed before masking. Architectures or allocator modes that
    /// return tagged pointers must fail here instead of silently changing the host address.
    static Address fromNativePointer(const void* pointer)
    {
        static_assert(sizeof(uintptr_t) <= sizeof(uint64_t));
        uint64_t rawPointer = uint64_t(reinterpret_cast<uintptr_t>(pointer));
        SLANG_RELEASE_ASSERT((rawPointer & ~kAddressPayloadMask) == 0);
        return makeTagged(Tag::Native, rawPointer);
    }

    static Address fromObject(HeapObject* object) { return fromNativePointer(object); }

    /// Decodes the address-space tag without inspecting the addressed value.
    ///
    /// For example, a staged `FrameRelative` operand needs the current frame base added, while a
    /// `Native` address already is the host pointer.
    SLANG_FORCE_INLINE AddressKind getKind() const
    {
        if (!value)
            return AddressKind::Invalid;
        uint8_t kind = uint8_t(value >> kAddressTagShift);
        SLANG_ASSERT(
            kind != uint8_t(Tag::Forbidden) && "HGVM rejected a forbidden high-half address");
        SLANG_ASSERT(
            kind <= uint8_t(Tag::InstructionAbsolute) && "HGVM encountered an unknown address tag");
        return AddressKind(kind);
    }

    /// Returns the unsigned payload used by frame, const, and absolute-instruction forms.
    ///
    /// For example, an instruction address with payload 12 returns instruction word offset 12.
    uint64_t getUnsignedPayload() const { return value & kAddressPayloadMask; }

    /// Returns the sign-extended 56-bit displacement from a relative instruction address.
    ///
    /// For example, a displacement of `-1` resolves to the word immediately before the current
    /// function record.
    int32_t getInstructionDisplacement() const
    {
        uint64_t payload = getUnsignedPayload();
        if (payload & (uint64_t(1) << 55))
            payload |= ~kAddressPayloadMask;
        SLANG_RELEASE_ASSERT(
            int64_t(payload) >= std::numeric_limits<int32_t>::min() &&
            int64_t(payload) <= std::numeric_limits<int32_t>::max());
        return int32_t(int64_t(payload));
    }

    /// Returns the raw host pointer without changing the ownership of what it names.
    void* getNativePointer() const
    {
        SLANG_ASSERT(getKind() == AddressKind::Native);
        return reinterpret_cast<void*>(uintptr_t(getUnsignedPayload()));
    }

    HeapObject* getObject() const { return static_cast<HeapObject*>(getNativePointer()); }

    explicit operator bool() const { return value != 0; }

    /// Adds a byte displacement without changing the address kind.
    ///
    /// Consider a native address for `value` and a staged field offset of eight bytes: this
    /// returns the native address for `value.field`, whether `value` lives in a frame, a class
    /// object, or any other host storage. A staged frame-relative cleanup operand stays
    /// frame-relative. Instruction-stack addresses remain word-addressed, so their byte
    /// displacements are passed separately to `resolveAddressAt`.
    Address addByteOffset(uint32_t byteOffset) const
    {
        AddressKind kind = getKind();
        if (kind == AddressKind::Native)
        {
            uint8_t* pointer = static_cast<uint8_t*>(getNativePointer());
            return fromNativePointer(pointer + byteOffset);
        }
        SLANG_ASSERT(kind == AddressKind::FrameRelative);
        uint64_t payload = getUnsignedPayload();
        SLANG_RELEASE_ASSERT(byteOffset <= kAddressPayloadMask - payload);
        return makeTagged(Tag::FrameRelative, payload + byteOffset);
    }
};

static_assert(sizeof(Address) == sizeof(uint64_t));
static_assert(uint8_t(AddressKind::Native) == uint8_t(Address::Tag::Native));
static_assert(uint8_t(AddressKind::FrameRelative) == uint8_t(Address::Tag::FrameRelative));
static_assert(uint8_t(AddressKind::ConstHeap) == uint8_t(Address::Tag::ConstHeap));
static_assert(
    uint8_t(AddressKind::InstructionRelative) == uint8_t(Address::Tag::InstructionRelative));
static_assert(
    uint8_t(AddressKind::InstructionAbsolute) == uint8_t(Address::Tag::InstructionAbsolute));
static_assert(uint8_t(AddressKind::Forbidden) == uint8_t(Address::Tag::Forbidden));

/// Stores the fixed-width interpreter representation of every source-language `String`.
///
/// Consider the literal `"hello"`: `literalBytes` is the native address of five NUL-terminated
/// bytes in the program's literal segment, `byteCount` is five, and `owner` is null. The result
/// of `format("{}", value)` instead leaves `literalBytes` invalid and owns a `StringObject`.
/// Keeping both cases in the same trivial value lets calls, returns, aggregate construction,
/// loads, and stores copy exactly 24 bytes.
struct StringValue
{
    Address literalBytes;
    uint32_t byteCount = 0;
    Address owner;
};

static_assert(sizeof(StringValue) == 24);
static_assert(offsetof(StringValue, owner) == 16);
static_assert(std::is_trivially_copyable_v<StringValue>);

/// Identifies the concrete interpreter object behind a heap `Address` without C++ RTTI.
///
/// Consider a staged buffer-dimensions operation: its record already says `StructuredBuffer`, so
/// execution checks this compact tag in debug builds and casts directly to the corresponding C++
/// type instead of looking up an HGBC type.
enum class HeapObjectKind : uint32_t
{
    StructuredBuffer,
    Texture2D,
    String,
    ByteStream,
    DeferredReadback,
    Timer,
    Window,
    Surface,
    Class,
    RuntimeArray,
    Box,
    RaytracingPipeline,
    RayAccelerationStructure,
};

/// Provides the common tagged base for program-visible managed HGVM heap objects.
///
/// For example, a runtime-created structured buffer and texture both use `RefObject`'s atomic
/// reference count, while `kind` lets trusted staged operations assert that they selected the
/// matching concrete representation. Interpreter-owned const objects use a separate inline base.
struct HeapObject : RefObject
{
    explicit HeapObject(HeapObjectKind objectKind)
        : kind(objectKind)
    {
    }

    HeapObjectKind kind;
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    ~HeapObject() override;
    RefPtr<EventTracker> eventTracker;
    uint64_t trackedByteSize = 0;
    HGBCId trackedTypeId = 0;
    EventOrigin allocationOrigin;
#endif
};

/// Distinguishes fixed-width scalar and aggregate data from variable-sized binary literal bytes.
///
/// For example, `OpFloatLit` is `Scalar`, a fixed-size array is `Array`, and every source `String`
/// is a fixed-width `StringValue`. Only `Binary` names raw variable-sized bytes directly.
enum class ValueKind : uint32_t
{
    Scalar,
    Vector,
    Matrix,
    Struct,
    Array,
    Address,
    HeapObject,
    ConstObject,
    String,
    Binary,
};

/// Identifies the execution layout stored in a staged type descriptor.
///
/// For example, `uint` is `Scalar`, `struct Pair { uint x; float y; }` is `Struct`,
/// `Array<Pair, 3>` is `Array`, and `Ptr<Pair>` is `Address`. Resources, classes, streams, and
/// deferred values use `HeapObject`; their ownership is expressed by managed-leaf offsets rather
/// than additional type kinds.
enum class TypeKind : uint32_t
{
    Scalar,
    Vector,
    Matrix,
    Struct,
    Array,
    Address,
    HeapObject,
    String,
    TaggedUnion,
};

/// Stores an initialized case and one owning payload handle without overlapping payload bytes.
/// Zeroed frame storage has no active case, including when zero is a declared case tag.
struct TaggedUnionValue
{
    uint32_t caseIndexPlusOne = 0;
    uint32_t caseTag = 0;
    Address payload;
};

/// Records whether a managed buffer reference is exposed to a shader through a descriptor heap.
enum class DescriptorAccess : uint32_t
{
    None,
    Read,
    ReadWrite,
};

/// Describes the packed scalar lanes used by scalar, vector, and matrix values.
///
/// Consider `float3x2`: it has three rows, two columns, and six `Float32` lanes in row-major
/// logical order. The source matrix-layout decoration is retained separately because it affects
/// GPU marshalling, not arithmetic on the interpreter's canonical value representation.
struct NumericLayout
{
    ScalarType scalarType = ScalarType::UInt32;
    uint32_t rowCount = 1;
    uint32_t columnCount = 1;
    uint32_t laneCount = 1;
    uint32_t laneByteStride = 0;
};

/// Identifies which embedded HGBC operation realizes one const-heap object.
/// The lazy path uses this kind once on first access; later dispatches borrow the resolved object.
enum class LazyObjectKind : uint32_t
{
    SlangModule,
    SlangEntryPoint,
    ComputeDispatcher,
    GraphicsPipeline,
    TypeLayout,
    LayoutConverter,
};

/// Selects the shader resource/result scalar family used by a staged texture blit.
///
/// For example, RGBA32Uint to RGBA16Float uses `UInt` source loads and `Float` fragment output;
/// this decision is made from the explicit endpoint types once during staging.
enum class TextureScalarClass : uint32_t
{
    Float,
    Int,
    UInt,
};


} // namespace Slang::HGVM
