// slang-hgvm-interpreter.cpp
#include "slang-hgvm-interpreter.h"

#include "core/slang-allocator.h"
#include "core/slang-exception.h"
#include "core/slang-file-system.h"
#include "core/slang-io.h"
#include "core/slang-list.h"
#include "core/slang-math.h"
#include "core/slang-stream.h"
#include "slang-hgvm-buffer-pool.h"
#include "slang-hgvm-debug-map.h"
#include "slang-hgvm-image-io.h"
#include "slang-hgvm-resource.h"
#include "slang-hgvm-staged-record-builder.h"
#include "slang-hgvm.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <limits>
#include <new>
#include <slang-rhi/acceleration-structure-utils.h>
#include <slang-rhi/shader-cursor.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Slang::HGVM
{
namespace
{

/// Allocates HGVM storage whose alignment is determined by staged type metadata.
///
/// `AlignedAllocator` accepts its alignment as a template argument, but class and runtime-array
/// layouts provide it at runtime. Keep this allocation paired with the allocator selected by the
/// core library so mimalloc-enabled builds do not cross allocator boundaries.
void* allocateAlignedStorage(size_t size, size_t alignment)
{
#if defined(SLANG_ENABLE_MIMALLOC)
    void* result = mi_malloc_aligned(size, alignment);
#elif SLANG_WINDOWS_FAMILY
    void* result = _aligned_malloc(size, alignment);
#elif defined(__CYGWIN__)
    void* result = aligned_alloc(alignment, size);
#else
    void* result = nullptr;
    if (posix_memalign(&result, alignment, size) != 0)
        return nullptr;
#endif
    return result;
}

void deallocateAlignedStorage(void* storage)
{
#if defined(SLANG_ENABLE_MIMALLOC)
    mi_free(storage);
#elif SLANG_WINDOWS_FAMILY
    _aligned_free(storage);
#else
    free(storage);
#endif
}

/// Tracks separately owned array storage independently of its managed object header.
void* allocateAlignedPayload(EventTracker* events, size_t size, size_t alignment)
{
    void* result = allocateAlignedStorage(size, alignment);
    if (result)
        SLANG_HGVM_TRACK(events, trackPayloadAlloc(result, size, uint32_t(alignment)));
    return result;
}

void deallocateAlignedPayload(EventTracker* events, void* payload)
{
    if (payload)
        SLANG_HGVM_TRACK(events, trackPayloadFree(payload));
    deallocateAlignedStorage(payload);
}

/// Releases each owning field and clears its slot before destruction can invoke host callbacks.
/// A repeated object address represents separate references, so every field is visited.
void releaseManagedFields(uint8_t* bytes, ConstArrayView<uint32_t> offsets)
{
    for (uint32_t offset : offsets)
    {
        Address child;
        std::memcpy(&child, bytes + offset, sizeof(child));
        if (child)
        {
            Address empty;
            std::memcpy(bytes + offset, &empty, sizeof(empty));
            child.getObject()->releaseReference();
        }
    }
}

/// Shared storage and ownership for boxed values and values retained by native callers.
/// Each leaf owns one reference even when several leaves refer to the same object.
struct OwnedPayload
{
    OwnedPayload() = default;
    OwnedPayload(const OwnedPayload&) = delete;
    OwnedPayload& operator=(const OwnedPayload&) = delete;
    ~OwnedPayload() { clear(); }

    void initialize(
        EventTracker* tracker,
        uint32_t size,
        uint32_t alignment,
        ConstArrayView<uint32_t> offsets)
    {
        SLANG_RELEASE_ASSERT(!payload);
        events = tracker;
        payloadByteSize = size;
        managedLeafOffsets.addRange(offsets.getBuffer(), offsets.getCount());
        payload = static_cast<uint8_t*>(allocateAlignedPayload(
            events,
            std::max(size, 1u),
            std::max(alignment, uint32_t(alignof(void*)))));
        if (!payload)
            throw Exception("HGVM could not allocate an owned value");
        std::memset(payload, 0, size);
    }

    void copyRetaining(const void* source)
    {
        SLANG_RELEASE_ASSERT(payload);
        std::memcpy(payload, source, payloadByteSize);
        for (uint32_t offset : managedLeafOffsets)
        {
            Address child;
            std::memcpy(&child, payload + offset, sizeof(child));
            if (child)
                child.getObject()->addReference();
        }
    }

    void clear()
    {
        if (!payload)
            return;
        releaseManagedFields(payload, managedLeafOffsets.getArrayView());
        deallocateAlignedPayload(events, payload);
        payload = nullptr;
        payloadByteSize = 0;
        managedLeafOffsets.clear();
        events = nullptr;
    }

    EventTracker* events = nullptr;
    uint32_t payloadByteSize = 0;
    uint8_t* payload = nullptr;
    List<uint32_t> managedLeafOffsets;
};

// Staging accepts only HGBC that has already passed `validateProgram`. A rule belongs in
// `slang-hgbc-validation.cpp` only when it is an HGBC-format invariant shared by every consumer.
// Consider `OpPrint %stringType %value`: the validator proves that the IDs exist and that `%value`
// has the declared printable type. HGVM may still reject another valid instruction because its
// runtime representation is not implemented. Keep such implementation capability checks here;
// never strengthen the HGBC validator merely to simplify HGVM. Conversely, do not repeat
// HGBC-format validation here because the staging path must stay lean.

struct DeviceState;


/// Owns one allocation containing the class header, aligned fields, and managed-leaf offsets.
///
/// Consider `class Holder { Counter child; uint count; }`: ordinary values contain only the
/// managed address of this object. Staged field offsets address the `Counter` handle and `count`
/// directly from that base. A trailing table lets destruction release the child without retaining
/// staging metadata or allocating a separate vector for each instance.
struct ClassObject final : HeapObject
{
    static constexpr HeapObjectKind kKind = HeapObjectKind::Class;

    /// Constructs a zeroed instance using the complete layout computed during staging.
    static ClassObject* create(EventTracker* events, const StagedRecord::AllocateObject& record)
    {
        SLANG_ASSERT(record.allocationByteAlignment >= alignof(ClassObject));
        SLANG_ASSERT(record.managedLeafOffsetsByteOffset >= sizeof(ClassObject));
        SLANG_ASSERT(
            uint64_t(record.managedLeafOffsetsByteOffset) +
                uint64_t(record.managedLeafCount) * sizeof(uint32_t) <=
            record.allocationByteSize);
        void* storage =
            allocateAlignedStorage(record.allocationByteSize, record.allocationByteAlignment);
        SLANG_RELEASE_ASSERT(storage);
        ClassObject* object = ::new (storage) ClassObject(
            record.classTypeAddress,
            record.managedLeafOffsetsByteOffset,
            record.managedLeafCount);
        auto* bytes = static_cast<uint8_t*>(storage);
        std::memset(
            bytes + sizeof(ClassObject),
            0,
            record.allocationByteSize - sizeof(ClassObject));
        if (record.managedLeafCount)
            std::memcpy(
                bytes + record.managedLeafOffsetsByteOffset,
                record.getManagedLeafOffsets().getBuffer(),
                size_t(record.managedLeafCount) * sizeof(uint32_t));
        object->addReference();
        SLANG_HGVM_TRACK(events, trackHeapAlloc(object, record.allocationByteSize));
        return object;
    }

    ~ClassObject() override
    {
        auto* bytes = reinterpret_cast<const uint8_t*>(this);
        for (uint32_t i = 0; i < managedLeafCount; ++i)
        {
            uint32_t leafOffset;
            std::memcpy(
                &leafOffset,
                bytes + managedLeafOffsetsByteOffset + size_t(i) * sizeof(uint32_t),
                sizeof(leafOffset));
            SLANG_ASSERT(leafOffset >= sizeof(ClassObject));
            SLANG_ASSERT(uint64_t(leafOffset) + sizeof(Address) <= managedLeafOffsetsByteOffset);
            Address handle;
            std::memcpy(&handle, bytes + leafOffset, sizeof(handle));
            if (handle)
                handle.getObject()->releaseReference();
        }
    }

    /// Pairs the factory's aligned allocation with deletion through RefObject's virtual destructor.
    static void operator delete(void* storage) noexcept { deallocateAlignedStorage(storage); }

    Address typeAddress;

private:
    ClassObject(
        Address inTypeAddress,
        uint32_t inManagedLeafOffsetsByteOffset,
        uint32_t inManagedLeafCount)
        : HeapObject(kKind)
        , typeAddress(inTypeAddress)
        , managedLeafOffsetsByteOffset(inManagedLeafOffsetsByteOffset)
        , managedLeafCount(inManagedLeafCount)
    {
    }

    uint32_t managedLeafOffsetsByteOffset = 0;
    uint32_t managedLeafCount = 0;
};

/// Owns the runtime-sized element payload behind one `RuntimeArray<T>` handle.
///
/// The element layout is fixed while the count is selected by `alloc`. Managed-leaf offsets are
/// relative to one element, so destruction needs metadata proportional to `T`, not to the runtime
/// element count.
struct RuntimeArrayObject : HeapObject
{
    static constexpr HeapObjectKind kKind = HeapObjectKind::RuntimeArray;

    RuntimeArrayObject(
        EventTracker* events,
        Address inTypeAddress,
        uint32_t inElementCount,
        uint32_t inElementByteStride,
        uint32_t elementByteAlignment,
        std::span<const uint32_t> inElementManagedLeafOffsets)
        : HeapObject(kKind)
        , typeAddress(inTypeAddress)
        , elementCount(inElementCount)
        , elementByteStride(inElementByteStride)
        , elementManagedLeafOffsets(
              inElementManagedLeafOffsets.begin(),
              inElementManagedLeafOffsets.end())
    {
        uint64_t byteSize = uint64_t(elementCount) * elementByteStride;
        SLANG_RELEASE_ASSERT(byteSize <= std::numeric_limits<uint32_t>::max());
        payloadByteSize = uint32_t(byteSize);
        uint32_t allocationByteSize = payloadByteSize ? payloadByteSize : 1;
        uint32_t allocationAlignment =
            elementByteAlignment < alignof(void*) ? uint32_t(alignof(void*)) : elementByteAlignment;
        payload = static_cast<uint8_t*>(
            allocateAlignedPayload(events, allocationByteSize, allocationAlignment));
        SLANG_RELEASE_ASSERT(payload);
        std::memset(payload, 0, allocationByteSize);
    }

    ~RuntimeArrayObject()
    {
        for (uint32_t elementIndex = 0; elementIndex < elementCount; ++elementIndex)
        {
            uint32_t elementOffset = elementIndex * elementByteStride;
            for (uint32_t leafOffset : elementManagedLeafOffsets)
            {
                SLANG_ASSERT(
                    uint64_t(elementOffset) + leafOffset + sizeof(Address) <= payloadByteSize);
                Address handle;
                std::memcpy(&handle, payload + elementOffset + leafOffset, sizeof(handle));
                if (handle)
                    handle.getObject()->releaseReference();
            }
        }
        deallocateAlignedPayload(SLANG_HGVM_TRACKER(eventTracker), payload);
    }

    Address typeAddress;
    uint8_t* payload = nullptr;
    uint32_t payloadByteSize = 0;
    uint32_t elementCount = 0;
    uint32_t elementByteStride = 0;
    std::vector<uint32_t> elementManagedLeafOffsets;
};

/// Owns one immutable concrete value after its static type has been erased from a stack slot.
///
/// Consider an interface containing `struct S { Texture2D<float> image; }`: the stack owns this
/// object, and this object owns the texture at S's concrete managed offset. Keeping the staged
/// program alive preserves literal string storage without retaining
/// a stack frame. RefObject avoids coupling the allocation header to StagedProgram's definition.
struct BoxObject final : HeapObject, OwnedPayload
{
    static constexpr HeapObjectKind kKind = HeapObjectKind::Box;

    BoxObject(
        EventTracker* events,
        RefObject* program,
        const StagedRecord::Box& record,
        const void* source)
        : HeapObject(kKind), typeScope(program), payloadTypeAddress(record.payloadTypeAddress)
    {
        initialize(
            events,
            record.payloadByteSize,
            record.payloadByteAlignment,
            record.getManagedLeafOffsets());
        copyRetaining(source);
    }

    ~BoxObject() override
    {
        // Release payload leaves while the originating program and event tracker remain alive.
        clear();
    }

    RefPtr<RefObject> typeScope;
    Address payloadTypeAddress;
};

/// Stores the fixed-width interpreter representation of every source-language `String`.
///
/// Consider the literal `"hello"`: `literalBytes` points into immutable instruction storage,
/// `byteCount` is five, and `owner` is null. The result of `format("{}", value)` instead leaves
/// `literalBytes` invalid and owns a `StringObject`. Keeping both cases in the same trivial value
/// lets calls, returns, aggregate construction, loads, and stores copy exactly 24 bytes.
struct StringValue
{
    Address literalBytes;
    uint32_t byteCount = 0;
    Address owner;
};

static_assert(sizeof(StringValue) == 24);
static_assert(offsetof(StringValue, owner) == 16);
static_assert(std::is_trivially_copyable_v<StringValue>);

/// Owns the variable-sized bytes produced by a runtime string operation.
///
/// For example, `format("item {}", 3)` creates one immutable object containing `"item 3"`; the
/// returned `StringValue` transfers the object's initial program reference into its result slot.
/// Literal strings do not allocate this object because their bytes already live in the program.
struct StringObject : HeapObject
{
    static constexpr HeapObjectKind kKind = HeapObjectKind::String;

    explicit StringObject(std::string text)
        : HeapObject(kKind), bytes(std::move(text))
    {
    }

    std::string bytes;
};

/// Owns the transport state behind a source-language `ByteStream` value.
///
/// Consider an input EXR followed by another image: the first `OpImageDecode` repeatedly calls
/// `read`, leaves `stream` at the first image boundary, and the second decode continues there.
/// Local paths use a true file stream so multi-gigabyte assets stay bounded in host memory. A
/// whole-file blob remains only as a fallback for virtual `ISlangFileSystem` implementations.
struct ByteStreamObject : HeapObject, SequentialByteReader, SequentialByteWriter
{
    static constexpr HeapObjectKind kKind = HeapObjectKind::ByteStream;

    enum class Access : uint32_t
    {
        Read,
        Write,
    };

    ByteStreamObject()
        : HeapObject(kKind)
    {
    }

    SlangResult read(void* destination, size_t capacity, size_t& outByteCount) override
    {
        outByteCount = 0;
        if (closed || access != Access::Read || !stream)
            return SLANG_E_NOT_AVAILABLE;
        return stream->read(destination, capacity, outByteCount);
    }

    SlangResult write(std::span<const uint8_t> bytes) override
    {
        if (closed || access != Access::Write || !stream)
            return SLANG_E_NOT_AVAILABLE;
        return stream->write(bytes.data(), bytes.size());
    }

    Access access = Access::Read;
    bool closed = false;
    std::string path;
    ComPtr<ISlangBlob> inputContents;
    RefPtr<Stream> stream;
    RefPtr<OwnedMemoryStream> output;
    ComPtr<ISlangMutableFileSystem> mutableFileSystem;
};

/// Owns the reset point behind one source-language `Timer` handle.
///
/// Consider `Timer timer = createTimer()`: ordinary HGVM copies share this managed object, so
/// `resetTimer(timer)` changes the origin observed by every alias without storing native clock
/// state in fixed-width frame slots.
struct TimerObject : HeapObject
{
    static constexpr HeapObjectKind kKind = HeapObjectKind::Timer;

    TimerObject()
        : HeapObject(kKind), resetTime(std::chrono::steady_clock::now())
    {
    }

    std::chrono::steady_clock::time_point resetTime;
};

/// Owns one host window reference behind the program's fixed-width managed handle.
///
/// Consider `Window window = createWindow(...)`: the result slot contains an `Address` to this
/// wrapper, while `object` independently retains the platform window supplied by the host service.
struct WindowObject : HeapObject
{
    static constexpr HeapObjectKind kKind = HeapObjectKind::Window;

    WindowObject()
        : HeapObject(kKind)
    {
    }

    RefPtr<HGVMHostObject> object;
};

/// Owns one configured presentation surface and its host-selected native representation.
///
/// For example, `Surface<RGBA8UnormSrgb>` can retain a native BGRA8 sRGB swap chain here. HGBC
/// keeps the logical format in the staged create record; channel byte order remains host-private.
struct SurfaceObject : HeapObject
{
    static constexpr HeapObjectKind kKind = HeapObjectKind::Surface;

    SurfaceObject()
        : HeapObject(kKind)
    {
    }

    RefPtr<HGVMHostObject> object;
};

/// Wraps the physical pointer produced by the interpreter's single address-resolution bottleneck.
///
/// For example, resolving a four-byte frame address returns `AddressRef<uint32_t>`; ordinary
/// evaluators may dereference it but never decode the logical address themselves.
template<typename T>
struct AddressRef
{
    T* pointer = nullptr;
    uint32_t byteSize = 0;

    /// Returns the resolved pointer for fixed-size reads and writes.
    ///
    /// For example, `ref.get()[0]` reads the first staged byte after `resolveAddress` has checked
    /// the address space and requested extent.
    T* get() const { return pointer; }

    /// Dereferences a fixed-size resolved value.
    ///
    /// For example, `*resolveAddress<uint32_t>(...)` conceptually reads one scalar stack slot.
    T& operator*() const { return *pointer; }
};


/// Stores one IEEE-754 binary16 value while evaluating host-graph operations.
///
/// C++ has no portable native binary16 arithmetic type. Converting binary16 operands to `float`
/// is exact, and converting each result back here applies the same binary16 rounding required by
/// a Slang `half` operation.
struct Float16Value
{
    uint16_t bits = 0;

    Float16Value() = default;

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    Float16Value(T value)
        : bits(FloatToHalf(float(value)))
    {
    }

    operator float() const { return HalfToFloat(bits); }

    friend Float16Value operator+(Float16Value left, Float16Value right)
    {
        return Float16Value(float(left) + float(right));
    }
    friend Float16Value operator-(Float16Value left, Float16Value right)
    {
        return Float16Value(float(left) - float(right));
    }
    friend Float16Value operator*(Float16Value left, Float16Value right)
    {
        return Float16Value(float(left) * float(right));
    }
    friend Float16Value operator/(Float16Value left, Float16Value right)
    {
        return Float16Value(float(left) / float(right));
    }
    friend Float16Value operator-(Float16Value value) { return Float16Value(-float(value)); }
    friend bool operator==(Float16Value left, Float16Value right)
    {
        return float(left) == float(right);
    }
    friend bool operator!=(Float16Value left, Float16Value right) { return !(left == right); }
    friend bool operator<(Float16Value left, Float16Value right)
    {
        return float(left) < float(right);
    }
    friend bool operator>(Float16Value left, Float16Value right) { return right < left; }
    friend bool operator<=(Float16Value left, Float16Value right)
    {
        return float(left) <= float(right);
    }
    friend bool operator>=(Float16Value left, Float16Value right)
    {
        return float(left) >= float(right);
    }
};

static_assert(sizeof(Float16Value) == sizeof(uint16_t));
static_assert(alignof(Float16Value) == alignof(uint16_t));
static_assert(std::is_trivially_copyable_v<Float16Value>);

template<typename T>
static constexpr bool kIsInterpreterFloatingPoint =
    std::is_floating_point_v<T> || std::is_same_v<T, Float16Value>;

/// Selects the native math type, unwrapping binary16 without narrowing binary64 to float.
template<typename T>
using NativeFloat = std::conditional_t<std::is_same_v<T, Float16Value>, float, T>;


/// Describes whether a staged embedded object still needs its expensive realization work.
///
/// For example, an `OpSlangModule` object starts `Unresolved`; the first compute dispatcher that
/// reads it loads the IR blob and changes it to `Resolved`, while later dispatches reuse it.
enum class LazyState : uint32_t
{
    Unresolved,
    Resolving,
    Resolved,
};

/// Stores the common state for one interpreter-owned object laid out in the const heap.
///
/// For example, the const object for `OpMakeDispatcher` starts unresolved and retains the staged
/// descriptor word offset that names the entry-point dependency used to construct its RHI pipeline.
/// The surrounding `ProgramInstance` owns this storage; program reference-counting operations never
/// visit it.
struct LazyObject
{
    LazyObject(LazyObjectKind objectKind, uint32_t descriptorWordOffset)
        : kind(objectKind), descriptorWordOffset(descriptorWordOffset)
    {
    }

    LazyState state = LazyState::Unresolved;
    LazyObjectKind kind;
    uint32_t descriptorWordOffset = 0;
};

/// Stores a lazily loaded embedded Slang module and borrows its const-heap dependencies.
///
/// Consider module `A` importing `B`: realizing `A` first reads `B`'s const address, then records
/// the resulting pointer here. Both objects have the same `ProgramInstance` lifetime, while each
/// module payload owns its external Slang interface through `ComPtr`.
struct SlangModuleObject : LazyObject
{
    static constexpr LazyObjectKind kKind = LazyObjectKind::SlangModule;

    explicit SlangModuleObject(uint32_t descriptorWordOffset)
        : LazyObject(kKind, descriptorWordOffset)
    {
    }

    ComPtr<slang::IModule> module;
    List<SlangModuleObject*> dependencies;
    String name;
};

/// Stores one embedded entry-point declaration and borrows its const-heap module.
///
/// For example, the staged entry point named `writeValue` retains its resolved module and supplies
/// both values when a compute dispatcher lazily checks and links that shader.
struct SlangEntryPointObject : LazyObject
{
    static constexpr LazyObjectKind kKind = LazyObjectKind::SlangEntryPoint;

    explicit SlangEntryPointObject(uint32_t descriptorWordOffset)
        : LazyObject(kKind, descriptorWordOffset)
    {
    }

    SlangModuleObject* module = nullptr;
    String name;
    slang::IComponentType* typeLookupComponent = nullptr;

    /// Retains one generic argument in the textual form serialized by HGBC.
    ///
    /// Consider `fill<float, 4>`: the type argument stores `(Type, "float")`, while the value
    /// argument stores `(Expr, "4")`. Pipeline creation resolves both strings through Slang once
    /// and caches the resulting specialized pipeline in the enclosing dispatcher object.
    struct SpecializationArgument
    {
        HGBCSpecializationArgKind kind = HGBCSpecializationArgKind::Type;
        String value;
    };

    List<SpecializationArgument> specializationArguments;
};

/// Caches the linked Slang and RHI objects used by repeated compute calls.
///
/// Consider two calls to `dispatch(writeValue, size)`: the first call populates these fields and
/// the second binds the same compute pipeline without loading, checking, or linking the module.
struct ComputeDispatcherObject : LazyObject
{
    static constexpr LazyObjectKind kKind = LazyObjectKind::ComputeDispatcher;

    explicit ComputeDispatcherObject(uint32_t descriptorWordOffset)
        : LazyObject(kKind, descriptorWordOffset)
    {
    }

    SlangEntryPointObject* entryPoint = nullptr;
    ComPtr<slang::IComponentType> linkedComponent;
    ComPtr<rhi::IShaderProgram> program;
    ComPtr<rhi::IComputePipeline> pipeline;
    String label;
};

/// Owns one graphics pipeline prepared before execution of the host entry point. Draw calls
/// bind new resource values and dynamic parameters without selecting another pipeline variant.
struct GraphicsPipelineObject : LazyObject
{
    static constexpr LazyObjectKind kKind = LazyObjectKind::GraphicsPipeline;
    explicit GraphicsPipelineObject(uint32_t offset)
        : LazyObject(kKind, offset)
    {
    }
    ComPtr<slang::IComponentType> linkedComponent;
    ComPtr<rhi::IShaderProgram> program;
    ComPtr<rhi::IRenderPipeline> pipeline;
    rhi::Format colorFormat = rhi::Format::Undefined;
    rhi::Format depthFormat = rhi::Format::Undefined;
    bool scissorEnable = false;
};

/// The assembly value owns only an optional index-buffer reference. Shader resources are passed
/// separately and therefore cannot accidentally acquire a vertex-input role.
struct GraphicsDrawValue
{
    Address indices;
    uint32_t count = 0;
    uint32_t first = 0;
    uint32_t instanceCount = 1;
    uint32_t firstInstance = 0;
    int32_t baseVertex = 0;
};

struct GraphicsDrawParametersValue
{
    bool useDefaultViewport = true;
    bool useDefaultScissor = true;
    rhi::Viewport viewport;
    rhi::ScissorRect scissor;
    uint32_t stencilRef = 0;
};

/// Describes one scalar leaf copied between a reflected device layout and staged host storage.
///
/// Consider `struct S { bool enabled; uint value; }`: a device-to-host converter may contain one
/// rule mapping a four-byte boolean at device offset zero to a one-byte boolean at host offset
/// zero, followed by a direct four-byte `uint` rule at offset four.
struct LayoutConversionOperation
{
    ScalarType scalarType = ScalarType::UInt32;
    uint32_t sourceByteOffset = 0;
    uint32_t destinationByteOffset = 0;
    uint32_t sourceByteSize = 0;
    uint32_t destinationByteSize = 0;
};

/// Selects how one flattened host leaf crosses the RHI shader-object boundary.
///
/// A `StructuredBuffer<T>.Handle` is reflected as a buffer resource on CPU/CUDA, but as ordinary
/// descriptor-index data on Vulkan/D3D12. Recording the reflected representation here keeps that
/// target decision out of HGBC and lets later dispatches execute the cached operation directly.
enum class ShaderBindingMode : uint8_t
{
    Data,
    Resource,
    DescriptorHandle,
};

enum class LayoutConversionDirection : uint8_t
{
    DeviceToHost,
    HostToDevice,
};

/// Describes one leaf of a cached host-to-shader binding conversion.
///
/// Consider `struct Args { Array<float, 2> weights; Texture2D tex; }`. The converter's first lazy
/// resolution creates three operations whose cursor paths are `{0,0}`, `{0,1}`, and `{1}`. Each
/// operation also stores the host leaf offset and type-info address, so subsequent dispatches never
/// walk either aggregate type tree.
struct ShaderBindingOperation
{
    Address typeInfoAddress;
    ShaderBindingMode mode = ShaderBindingMode::Data;
    uint32_t sourceByteOffset = 0;
    uint32_t sourceDereferenceOffset = 0;
    uint32_t sourceDereferenceCount = 0;
    uint32_t cursorPathOffset = 0;
    uint32_t cursorPathCount = 0;
};

/// Caches one logical type's physical representation in a named external domain.
///
/// For example, the host layout of `Particle` points at its staged type-info record, while its
/// device-memory layout points at Slang reflection created with structured-buffer rules. Both
/// objects name the same HGBC logical type and are resolved at most once.
struct TypeLayoutObject : LazyObject
{
    static constexpr LazyObjectKind kKind = LazyObjectKind::TypeLayout;

    explicit TypeLayoutObject(uint32_t descriptorWordOffset)
        : LazyObject(kKind, descriptorWordOffset)
    {
    }

    HGBCId logicalTypeId = 0;
    HGBCLayoutDomain domain = HGBCLayoutDomain::Host;
    HGBCLayoutUsage usage = HGBCLayoutUsage::Value;
    Address hostTypeInfoAddress;
    Address contextAddress;
    uint32_t contextIndex = 0;
    uint32_t entryPointIndex = 0;
    bool graphicsContext = false;
    slang::TypeLayoutReflection* reflectedLayout = nullptr;
    uint32_t byteSize = 0;
    uint32_t byteAlignment = 1;
    uint32_t byteStride = 0;
};

/// Caches the rules needed to move one logical value between two physical layouts.
///
/// Consider a structured-buffer readback of `Array<bool, 4>`. The first use resolves the source
/// device layout and destination host layout, then flattens them into four scalar operations.
/// Later readbacks execute only those cached operations and never inspect a shader binding.
struct LayoutConverterObject : LazyObject
{
    static constexpr LazyObjectKind kKind = LazyObjectKind::LayoutConverter;

    explicit LayoutConverterObject(uint32_t descriptorWordOffset)
        : LazyObject(kKind, descriptorWordOffset)
    {
    }

    TypeLayoutObject* sourceLayout = nullptr;
    TypeLayoutObject* destinationLayout = nullptr;
    List<LayoutConversionOperation> operations;
    List<ShaderBindingOperation> shaderBindingOperations;
    List<uint32_t> sourceDereferenceOffsets;
    List<uint32_t> cursorPathIndices;
};

/// Sets the temporary device-layout capacity shared by every asynchronous readback pool slot.
///
/// For example, reading a `float4x4` copies exactly 64 bytes. Larger values are rejected until the
/// pool grows support for size classes instead of allocating an exceptional one-off buffer.
static constexpr uint32_t kAsyncReadbackBufferByteSize = 64;

/// Owns the device-to-host transfer behind one program-visible `Defer<T>` value.
///
/// Consider `Defer<uint> pending = asyncReadBack(buffer, 3)`: the readback operation records and
/// submits a copy into one slot of `DeviceState`'s readback pool, but does not wait for the GPU.
/// `pending.wait()` maps that slot once into `deviceBytes`, returns the slot to the pool, and
/// applies the already resolved converter. Copies of `pending` share this object through ordinary
/// program reference-counting, so a later wait reuses the captured bytes without another device
/// transfer.
struct DeferredReadbackObject : HeapObject
{
    static constexpr HeapObjectKind kKind = HeapObjectKind::DeferredReadback;

    DeferredReadbackObject()
        : HeapObject(kKind)
    {
    }

    ~DeferredReadbackObject();

    DeviceState* deviceState = nullptr;
    Index readbackPoolSlotIndex = -1;
    LayoutConverterObject* converter = nullptr;
    uint8_t deviceBytes[kAsyncReadbackBufferByteSize];
    uint32_t deviceByteSize = 0;
    uint32_t resultByteSize = 0;
    bool resolved = false;
};

/// Allocates fixed-size interpreter heap objects and transfers their initial owning reference.
///
/// Consider an `RWStructuredBuffer<uint>` allocation and a byte-stream allocation: both reach this
/// helper even though their payloads differ. Keeping that choice here makes a later allocator
/// change independent of their evaluators. Variable-size classes use `ClassObject::create`, while
/// const-heap objects are placement-constructed in their precomputed `ProgramInstance` storage.
template<typename T, typename... TArgs>
T* allocateHeapObject(EventTracker* events, TArgs&&... args)
{
    static_assert(std::is_base_of_v<HeapObject, T>);
    // TODO(HGVM workaround): `RefObject::releaseReference` destroys objects with `delete`, so the
    // initial implementation uses the matching ordinary allocation. Add paired allocation and
    // destruction hooks here before routing these small wrappers through mimalloc explicitly.
    T* object = new T(std::forward<TArgs>(args)...);
    // `RefObject` starts at zero. This increment creates the owning program reference that the raw
    // pointer transfers to a stack or aggregate leaf containing a heap `Address`.
    object->addReference();
    SLANG_HGVM_TRACK(events, trackHeapAlloc(object, sizeof(T)));
    return object;
}

/// Describes one staged value while HGBC IDs are being replaced with direct logical addresses.
///
/// For example, `%three` may map to a four-byte `UInt32` in immutable instruction storage, while
/// an arithmetic result maps to a relative-frame address allocated for the current function. For
/// a string literal, `address` names its fixed-width `StringValue`, while the payload fields retain
/// the raw bytes needed by embedded module descriptors. Runtime values leave those staging-only
/// payload fields empty.
struct StagedValueInfo
{
    ValueKind kind = ValueKind::Scalar;
    ScalarType scalarType = ScalarType::UInt32;
    HGBCId typeId = 0;
    Address address;
    uint32_t byteSize = 0;
    Address literalPayloadAddress;
    uint32_t literalPayloadByteSize = 0;
};


/// Describes one concrete object placement in a program instance's const-heap arena.
///
/// Consider a module followed by a dispatcher: staging may assign offsets zero and 64 after
/// applying their respective C++ alignments. Instance creation uses `kind` to placement-construct
/// each object and supplies its immutable lazy descriptor location.
struct ConstObjectLayout
{
    LazyObjectKind kind = LazyObjectKind::SlangModule;
    uint32_t byteOffset = 0;
    uint32_t descriptorWordOffset = kInvalidInstructionWordOffset;
};


/// Keeps reusable runtime layout facts independent of any program's HGBC IDs.
/// Field selectors and source type IDs live in each stager's local binding instead.
struct CanonicalLayout
{
    struct Field
    {
        uint32_t byteOffset;
        uint32_t byteSize;
    };
    TypeKind kind = TypeKind::Scalar;
    HeapObjectKind heapObjectKind = HeapObjectKind::StructuredBuffer;
    DescriptorAccess descriptorAccess = DescriptorAccess::None;
    NumericLayout numericLayout;
    Address typeInfoAddress;
    uint32_t byteSize = 0;
    uint32_t byteAlignment = 1;
    uint32_t elementByteStride = 0;
    uint32_t elementCount = 0;
    uint32_t matrixLayout = 0;
    std::vector<Field> fieldLayouts;
    std::vector<uint32_t> managedLeafOffsets;
    uint32_t allocationByteSize = 0;
    uint32_t allocationByteAlignment = 1;
    uint32_t managedLeafOffsetsByteOffset = 0;
    std::vector<uint32_t> objectManagedLeafOffsets;
    uint32_t elementByteAlignment = 1;
    std::vector<uint32_t> elementManagedLeafOffsets;
    HostValueType hostType;
    bool hostValueSupported = false;
    bool complete = false;
    std::string definition;
};

/// Owns the VM's append-only instruction, literal, and canonical type address space.
/// Native record pointers are borrowed until the next load; word addresses remain stable.
struct SharedStagingContext : RefObject
{
    StagedRecordStorage storage;
    std::unordered_map<std::string, std::shared_ptr<CanonicalLayout>> types;
    std::unordered_map<uint64_t, std::shared_ptr<CanonicalLayout>> typesByAddress;

    const CanonicalLayout& findType(Address address) const
    {
        auto found = typesByAddress.find(address.value);
        SLANG_RELEASE_ASSERT(found != typesByAddress.end());
        return *found->second;
    }
};

/// Owns one program's indexes into the VM's shared immutable staged records.
///
/// Consider two calls to the same HGBC `main`: both use the same words in shared storage, while
/// each call creates fresh mutable `InterpreterState` traversal fields.
class StagedProgram : public RefObject
{
public:
    RefPtr<HGVM::Program> sourceProgram;
    RefPtr<DebugMap> debugMap;
    RefPtr<SharedStagingContext> staging;
    std::unordered_map<HGBCId, Address> typeBindings;
    uint32_t firstWordOffset = 0;
    uint32_t endWordOffset = 0;
    std::unordered_map<HGBCId, Address> functionAddresses;
    List<ConstObjectLayout> constObjectLayouts;
    uint32_t constHeapByteSize = 0;
    Dictionary<uint32_t, List<Address>> debugOwnedSlots;
};

/// Owns the device-dependent const heap associated with one immutable staged program.
///
/// Consider two invocations using the same compute dispatcher: both share `program` and read the
/// same in-place const object, while this instance retains the lazily loaded module, Slang session,
/// and RHI
/// pipeline until the source program is released from this interpreter.
struct ProgramInstance : RefObject
{
    ~ProgramInstance() { destroyConstObjects(); }

    /// Allocates the final const arena and constructs every inexpensive lazy wrapper in place.
    ///
    /// Consider a staged module -> entry point -> dispatcher chain: this performs three placement
    /// constructions but does not load, check, link, or create any Slang/RHI objects. Those costly
    /// actions remain deferred until the corresponding wrapper is first read.
    void initializeConstObjects(StagedRecordBuilder& builder)
    {
        SLANG_ASSERT(program && constructedConstObjectCount == 0 && constHeap.getCount() == 0);
        constHeap.setCount(program->constHeapByteSize);
        for (const ConstObjectLayout& layout : program->constObjectLayouts)
        {
            SLANG_ASSERT(layout.byteOffset < uint32_t(constHeap.getCount()));
            void* storage = constHeap.getBuffer() + layout.byteOffset;
            switch (layout.kind)
            {
            case LazyObjectKind::SlangModule:
                SLANG_ASSERT(
                    (reinterpret_cast<uintptr_t>(storage) % alignof(SlangModuleObject)) == 0);
                new (storage) SlangModuleObject(layout.descriptorWordOffset);
                break;
            case LazyObjectKind::SlangEntryPoint:
                SLANG_ASSERT(
                    (reinterpret_cast<uintptr_t>(storage) % alignof(SlangEntryPointObject)) == 0);
                new (storage) SlangEntryPointObject(layout.descriptorWordOffset);
                break;
            case LazyObjectKind::ComputeDispatcher:
                SLANG_ASSERT(
                    (reinterpret_cast<uintptr_t>(storage) % alignof(ComputeDispatcherObject)) == 0);
                new (storage) ComputeDispatcherObject(layout.descriptorWordOffset);
                break;
            case LazyObjectKind::GraphicsPipeline:
                new (storage) GraphicsPipelineObject(layout.descriptorWordOffset);
                break;
            case LazyObjectKind::TypeLayout:
                SLANG_ASSERT(
                    (reinterpret_cast<uintptr_t>(storage) % alignof(TypeLayoutObject)) == 0);
                {
                    TypeLayoutObject* object =
                        new (storage) TypeLayoutObject(layout.descriptorWordOffset);
                    const auto& descriptor = *builder.getRecord<StagedRecord::TypeLayoutObject>(
                        layout.descriptorWordOffset);
                    object->logicalTypeId = descriptor.logicalTypeId;
                    object->domain = descriptor.domain;
                    object->usage = descriptor.usage;
                    object->hostTypeInfoAddress = descriptor.hostTypeInfoAddress;
                    object->contextAddress = descriptor.contextAddress;
                    object->contextIndex = descriptor.contextIndex;
                    object->entryPointIndex = descriptor.entryPointIndex;
                    object->graphicsContext = descriptor.graphicsContext;

                    if (descriptor.domain == HGBCLayoutDomain::Host)
                    {
                        // Host layout is already a staged literal. For `struct S { bool b; }`, the
                        // descriptor points directly at S's one-byte host type-info record, so no
                        // first-use transition or Slang reflection is required.
                        SLANG_ASSERT(
                            descriptor.hostTypeInfoAddress.getKind() ==
                            AddressKind::InstructionAbsolute);
                        const uint32_t typeInfoWordOffset =
                            uint32_t(descriptor.hostTypeInfoAddress.getUnsignedPayload());
                        const auto& typeInfo =
                            *builder.getRecord<StagedRecord::TypeInfo>(typeInfoWordOffset);
                        object->byteSize = typeInfo.byteSize;
                        object->byteAlignment = typeInfo.byteAlignment;
                        uint64_t byteStride =
                            (uint64_t(typeInfo.byteSize) + typeInfo.byteAlignment - 1) &
                            ~uint64_t(typeInfo.byteAlignment - 1);
                        SLANG_ASSERT(byteStride <= std::numeric_limits<uint32_t>::max());
                        object->byteStride = uint32_t(byteStride);
                        object->state = LazyState::Resolved;
                    }
                }
                break;
            case LazyObjectKind::LayoutConverter:
                SLANG_ASSERT(
                    (reinterpret_cast<uintptr_t>(storage) % alignof(LayoutConverterObject)) == 0);
                new (storage) LayoutConverterObject(layout.descriptorWordOffset);
                break;
            }
            ++constructedConstObjectCount;
        }
    }

    RefPtr<StagedProgram> program;
    List<uint8_t, AlignedAllocator<kStorageBaseAlignment>> constHeap;
    ComPtr<slang::ISession> slangSession;
    ComPtr<slang::IComponentType> typeLookupComponent;

private:
    /// Destroys exactly the in-place objects whose constructors completed, in reverse layout order.
    ///
    /// For example, if the third wrapper's construction throws, instance teardown destroys the
    /// first two wrappers without interpreting padding bytes or invoking managed-heap refcounts.
    void destroyConstObjects()
    {
        while (constructedConstObjectCount)
        {
            const ConstObjectLayout& layout =
                program->constObjectLayouts[--constructedConstObjectCount];
            uint8_t* storage = constHeap.getBuffer() + layout.byteOffset;
            switch (layout.kind)
            {
            case LazyObjectKind::SlangModule:
                reinterpret_cast<SlangModuleObject*>(storage)->~SlangModuleObject();
                break;
            case LazyObjectKind::SlangEntryPoint:
                reinterpret_cast<SlangEntryPointObject*>(storage)->~SlangEntryPointObject();
                break;
            case LazyObjectKind::ComputeDispatcher:
                reinterpret_cast<ComputeDispatcherObject*>(storage)->~ComputeDispatcherObject();
                break;
            case LazyObjectKind::GraphicsPipeline:
                reinterpret_cast<GraphicsPipelineObject*>(storage)->~GraphicsPipelineObject();
                break;
            case LazyObjectKind::TypeLayout:
                reinterpret_cast<TypeLayoutObject*>(storage)->~TypeLayoutObject();
                break;
            case LazyObjectKind::LayoutConverter:
                reinterpret_cast<LayoutConverterObject*>(storage)->~LayoutConverterObject();
                break;
            }
        }
    }

    uint32_t constructedConstObjectCount = 0;
};

/// Owns one deferred shader argument using the same managed leaves as an ordinary HGVM value.
/// Registration copies this storage before its caller returns; shader binding never borrows
/// the caller's frame. Self references to the owning single tracker are non-owning because
/// this value cannot outlive the registration that contains it.
struct RayShaderArgumentValue : RefObject
{
    ~RayShaderArgumentValue() override { deallocateAlignedStorage(bytes); }

    void* bytes = nullptr;
    Address typeInfoAddress;
    uint32_t parameterIndex = 0;
    HGBCRayStageRole stage = HGBCRayStageRole::ClosestHit;
    HGBCId typeId = 0;
    List<RefPtr<HeapObject>> resources;
};

/// Keeps code identity separate from the owned values supplied to one table registration.
struct RayShaderRegistration : RefObject
{
    /// Deferred addresses are relative to this program's instruction and const storage.
    RefPtr<ProgramInstance> programInstance;
    Address entryPoints[3];
    HGBCId payloadType = 0;
    HGBCId attributeType = 0;
    HGBCRayGeometryKind geometryKind = HGBCRayGeometryKind::Triangles;
    List<RefPtr<RayShaderArgumentValue>> arguments;
};

/// Stores one application's mutable registry, independently of any native pipeline or scene.
struct RaytracingPipelineObject : HeapObject
{
    static constexpr HeapObjectKind kKind = HeapObjectKind::RaytracingPipeline;

    explicit RaytracingPipelineObject(uint32_t count)
        : HeapObject(kKind), rayTypeCount(count)
    {
        misses.setCount(count);
    }

    uint32_t rayTypeCount = 0;
    uint64_t version = 0;
    List<RefPtr<RayShaderRegistration>> misses;
    Dictionary<uint64_t, RefPtr<RayShaderRegistration>> hits;
};

/// Holds the immutable code identity of one native ray pipeline, separately from SBT values.
/// Entry-point pointers are canonical objects in the retained program instance's const arena.
struct RayPipelineObject : RefObject
{
    struct HitGroup
    {
        int32_t entryPoints[3] = {-1, -1, -1};
        HGBCRayGeometryKind geometryKind = HGBCRayGeometryKind::Triangles;
        HGBCId attributeType = 0;
    };
    RefPtr<ProgramInstance> programInstance;
    List<SlangEntryPointObject*> entryPoints;
    List<SlangStage> stages;
    List<HitGroup> hitGroups;
    List<String> exportNames;
    List<String> groupNames;
    uint32_t maxRecursion = 1;
    ComPtr<slang::IComponentType> linkedComponent;
    ComPtr<rhi::IShaderProgram> program;
    ComPtr<rhi::IRayTracingPipeline> pipeline;
};

/// Aborts the exceptional HGVM path without adding branches to ordinary execution.
///
/// For example, a call that exceeds the logical stack limit throws here and the
/// `InterpreterCore::execute` boundary converts the message to `SLANG_FAIL`; successful
/// instructions do no error plumbing.
[[noreturn]] void fail(const std::string& message)
{
    throw Exception(message.c_str());
}

static constexpr uint32_t kInvalidStackOffset = 0xffffffffu;
static constexpr uint32_t kInvalidProgramCounter = 0xffffffffu;

/// Stores execution-only traversal data at the start of every byte-stack frame.
///
/// Consider the root `main`: its caller and return PCs are invalid. A nested call stores `main`'s
/// frame offset, return PC, function address, and result destination in the same fixed header.
struct FrameHeader
{
    uint32_t callerFrameOffset = kInvalidStackOffset;
    uint32_t previousStackTop = 0;
    uint32_t returnProgramCounter = kInvalidProgramCounter;
    Address callerFunctionAddress;
    Address returnDestinationAddress;
};

/// Assigns aligned relative-frame addresses without depending on a particular scalar width.
///
/// For example, allocating `UInt8` followed by `Float64` consumes one byte, alignment padding, and
/// eight bytes; both addresses remain valid if the stack backing allocation later relocates.
class FrameLayoutBuilder
{
public:
    FrameLayoutBuilder()
        : m_size(uint32_t(sizeof(FrameHeader))), m_alignment(uint32_t(alignof(FrameHeader)))
    {
    }

    /// Reserves one slot and returns its relative-frame logical address.
    ///
    /// For example, a `Float16` result requests size two and alignment two rather than inheriting
    /// the four-byte layout used by `Float32` values.
    Address allocate(uint32_t byteSize, uint32_t byteAlignment)
    {
        if (!byteSize || !byteAlignment || (byteAlignment & (byteAlignment - 1)))
            fail("HGVM frame slot has an invalid size or alignment");
        uint64_t alignedOffset =
            (uint64_t(m_size) + byteAlignment - 1) & ~uint64_t(byteAlignment - 1);
        uint64_t endOffset = alignedOffset + byteSize;
        if (endOffset >= kFramePayloadLimit || alignedOffset > std::numeric_limits<uint32_t>::max())
            fail("HGVM function frame exceeds its address limits");
        Address result = Address::makeRelativeFrame(uint32_t(alignedOffset));
        m_size = uint32_t(endOffset);
        if (byteAlignment > m_alignment)
            m_alignment = byteAlignment;
        return result;
    }

    /// Returns the final frame extent rounded up to its strongest slot alignment.
    ///
    /// For example, a frame ending after a one-byte boolean is rounded so its complete extent is a
    /// multiple of the strongest alignment required by its header or slots.
    uint32_t getFrameSize() const
    {
        uint64_t result = (uint64_t(m_size) + m_alignment - 1) & ~uint64_t(m_alignment - 1);
        SLANG_RELEASE_ASSERT(result < kFramePayloadLimit);
        return uint32_t(result);
    }

    /// Returns the strongest alignment requested by the frame header or any value slot.
    ///
    /// `FrameHeader` establishes the initial requirement; a slot with stronger alignment raises
    /// the returned value without changing the address representation used by its instructions.
    uint32_t getFrameAlignment() const { return m_alignment; }

private:
    uint32_t m_size;
    uint32_t m_alignment;
};

/// Translates a validated HGBC program into immutable execution-ready records.
///
/// Consider `main { print("hello"); return; }`: the stager emits a string record, a function
/// record, `StagedRecord::Print`, ordinary `OpReturn`, and a generated `FinishFrameExit` record in
/// that order. The return redirects control to the function's mandatory exit block.
class Stager
{
public:
    /// Stages one source program or reports the first unsupported executable construct.
    ///
    /// For example, literal IDs become immutable addresses before function analysis, while debug
    /// scope instructions are retained only for debug handlers, and unsupported operations report
    /// errors without adding restrictions to the HGBC validator.
    RefPtr<StagedProgram> stage(
        HGVM::Program* sourceProgram,
        bool preserveDebugScopes,
        SharedStagingContext* staging)
    {
        m_preserveDebugScopes = preserveDebugScopes;
        if (!sourceProgram)
            fail("HGVM staging requires a validated HGBC program");

        RefPtr<StagedProgram> program = new StagedProgram;
        program->sourceProgram = sourceProgram;
        program->staging = staging;
        program->firstWordOffset = uint32_t(staging->storage.getWordCount());
        m_sourceProgram = sourceProgram;
        m_program = program;
        m_builder = std::make_unique<StagedRecordBuilder>(program->staging->storage);
        m_debugMapBuilder = std::make_unique<DebugMapBuilder>(sourceProgram);
        program->debugMap = m_debugMapBuilder->getDebugMap();

        stageTypes();
        stageLiterals();
        stageEmbeddedObjects();
        for (const HGBC::Function& function : sourceProgram->getDecodedProgram().functions)
            stageFunction(function);
        patchCalls();
        // Function staging can discover additional layouts, such as timers and deferred values.
        // Finalize ownership only after every type that can cross an invocation is present.
        finalizeHostValueSupport();
        if (m_preserveDebugScopes)
        {
            for (const auto& pair : m_typeLayouts)
            {
                const auto& source = pair.second;
                DebugTypeLayout layout;
                layout.kind = source.kind;
                layout.heapKind = source.heapObjectKind;
                layout.numeric = source.numericLayout;
                layout.valueType = source.valueTypeId;
                layout.elementType = source.elementTypeId;
                layout.byteSize = source.byteSize;
                layout.elementStride = source.elementByteStride;
                layout.elementCount = source.elementCount;
                layout.allocationSize = source.allocationByteSize;
                for (const auto& field : source.fields)
                    layout.fields.add(
                        {String(m_sourceProgram->findString(field.nameId)->c_str()),
                         field.typeId,
                         field.byteOffset});
                for (auto offset : source.objectManagedLeafOffsets)
                    layout.objectManagedOffsets.add(offset);
                m_debugMapBuilder->setTypeLayout(pair.first, std::move(layout));
            }
        }
        program->endWordOffset = m_builder->getWordOffset();
        for (const auto& pair : m_typeLayouts)
            program->typeBindings.emplace(pair.first, pair.second.typeInfoAddress);
        if (program->endWordOffset >= kInstructionPayloadLimit)
            fail("HGVM staged instruction stack exceeds its 32-bit offset space");

        return program;
    }

    StagedRecordBuilder& getBuilder() { return *m_builder; }

    /// Publishes canonical entries only after the program and its const objects are ready.
    /// Reserve before merging nodes so an allocation failure leaves the committed maps intact.
    void commit()
    {
        SLANG_RELEASE_ASSERT(m_pendingTypeAliases.empty());
        auto& shared = *m_program->staging;
        shared.types.reserve(shared.types.size() + m_pendingTypes.size());
        shared.typesByAddress.reserve(
            shared.typesByAddress.size() + m_pendingTypesByAddress.size());
        shared.types.merge(m_pendingTypes);
        shared.typesByAddress.merge(m_pendingTypesByAddress);
        m_builder->finalize();
    }

private:
    /// Retains a branch prefix until all target block PCs have been emitted.
    ///
    /// For example, a forward branch records `%merge` here, then patching replaces it with the
    /// direct PC assigned when `%merge` is emitted later in the same function.
    struct BranchFixup
    {
        uint32_t programCounter = 0;
        HGBCId targetBlockId = 0;
    };

    /// Retains both conditional targets until their direct staged PCs are known.
    ///
    /// For example, an entry block can target two later blocks without reserving placeholder body
    /// sizes; only this fixed record prefix is patched after all blocks are emitted.
    struct ConditionalFixup
    {
        uint32_t programCounter = 0;
        HGBCId trueBlockId = 0;
        HGBCId falseBlockId = 0;
    };

    /// Retains a direct call target until every staged function address is known.
    ///
    /// For example, `main` can call a helper emitted later in HGBC. Its call arguments are emitted
    /// immediately, then this fixed prefix is patched with the helper's instruction-stack address.
    struct CallFixup
    {
        uint32_t programCounter = 0;
        HGBCId targetFunctionId = 0;
    };

    /// Stores one field's VM layout while HGBC field names are being replaced by byte offsets.
    ///
    /// For example, the `float weight` in `{ bool enabled; float weight; }` records name ID,
    /// type ID, offset four, and size four. `OpFieldAddress` embeds the offset and discards the
    /// IDs.
    struct FieldLayout
    {
        HGBCId nameId = 0;
        HGBCId typeId = 0;
        uint32_t byteOffset = 0;
        uint32_t byteSize = 0;
    };

    /// Retains the staging-time view of one immutable type descriptor.
    ///
    /// Consider `Array<Inner, 3>`: `byteSize` and `byteAlignment` plan its inline stack slot, while
    /// `elementByteStride` makes indexed operations simple offset calculations. Struct `fields`
    /// and array element metadata are consulted only during staging. For `Ptr<Inner>`,
    /// `valueTypeId` preserves `Inner` for load/store staging.
    struct TypeLayout : CanonicalLayout
    {
        HGBCId valueTypeId = 0;
        HGBCId elementTypeId = 0;
        std::vector<FieldLayout> fields;
    };

    /// Names one frame slot whose heap-handle leaves must be released on every function exit.
    ///
    /// For example, an allocation result contributes its result address and buffer type here;
    /// staging later emits one cleanup record for each flattened managed leaf of that type.
    struct OwningSlot
    {
        Address address;
        HGBCId typeId = 0;
    };

    /// Stores the concrete C++ extent selected for one kind of inline const object.
    ///
    /// For example, a `SlangModuleObject` contributes its complete size and alignment, including
    /// `String`, dependency-list, and `ComPtr` fields. The next object can therefore be placed at a
    /// directly addressable aligned offset without a pointer cell between it and the const address.
    struct ConstObjectStorageLayout
    {
        uint32_t byteSize = 0;
        uint32_t byteAlignment = 1;
    };

    HGVM::Program* m_sourceProgram = nullptr;
    RefPtr<StagedProgram> m_program;
    bool m_preserveDebugScopes = false;
    std::unique_ptr<DebugMapBuilder> m_debugMapBuilder;
    std::unique_ptr<StagedRecordBuilder> m_builder;
    FrameLayoutBuilder m_frameLayout;
    std::unordered_map<HGBCId, StagedValueInfo> m_values;
    std::unordered_map<HGBCId, StagedValueInfo> m_debugVariables;
    std::unordered_map<HGBCId, TypeLayout> m_typeLayouts;
    std::unordered_map<HGBCId, bool> m_typesBeingStaged;
    std::unordered_map<HGBCId, std::string> m_typeKeys;
    std::unordered_map<std::string, std::shared_ptr<CanonicalLayout>> m_pendingTypes;
    std::unordered_map<uint64_t, std::shared_ptr<CanonicalLayout>> m_pendingTypesByAddress;
    std::unordered_map<uint64_t, std::vector<HGBCId>> m_pendingTypeAliases;
    std::unordered_map<HGBCId, Address> m_variableStorageAddresses;
    std::unordered_map<HGBCId, uint32_t> m_blockProgramCounters;
    std::vector<BranchFixup> m_branchFixups;
    std::vector<ConditionalFixup> m_conditionalFixups;
    std::vector<CallFixup> m_callFixups;
    std::vector<OwningSlot> m_owningSlots;
    std::unordered_map<HGBCId, uint32_t> m_constObjectLayoutIndices;

    /// Returns the in-place C++ layout associated with one staged embedded-object kind.
    ///
    /// Consider an embedded compute dispatcher: staging uses the full
    /// `sizeof(ComputeDispatcherObject)` and `alignof(ComputeDispatcherObject)`, rather than the
    /// eight-byte managed handle used by a stack-resident texture or buffer.
    static ConstObjectStorageLayout getConstObjectStorageLayout(LazyObjectKind kind)
    {
        switch (kind)
        {
        case LazyObjectKind::SlangModule:
            return {uint32_t(sizeof(SlangModuleObject)), uint32_t(alignof(SlangModuleObject))};
        case LazyObjectKind::SlangEntryPoint:
            return {
                uint32_t(sizeof(SlangEntryPointObject)),
                uint32_t(alignof(SlangEntryPointObject))};
        case LazyObjectKind::ComputeDispatcher:
            return {
                uint32_t(sizeof(ComputeDispatcherObject)),
                uint32_t(alignof(ComputeDispatcherObject))};
        case LazyObjectKind::GraphicsPipeline:
            return {
                uint32_t(sizeof(GraphicsPipelineObject)),
                uint32_t(alignof(GraphicsPipelineObject))};
        case LazyObjectKind::TypeLayout:
            return {uint32_t(sizeof(TypeLayoutObject)), uint32_t(alignof(TypeLayoutObject))};
        case LazyObjectKind::LayoutConverter:
            return {
                uint32_t(sizeof(LayoutConverterObject)),
                uint32_t(alignof(LayoutConverterObject))};
        }
        SLANG_UNREACHABLE("unknown HGVM lazy-object kind");
    }

    /// Maps an embedded HGBC producer to the concrete lazy-object category staged for its result.
    ///
    /// For example, `OpMakeDispatcher` selects `ComputeDispatcher` in both the address-layout and
    /// descriptor-emission passes. Keeping the opcode mapping here prevents those two passes from
    /// silently assigning different C++ object types to the same const address.
    static bool tryGetLazyObjectKind(HGBCOpcode opcode, LazyObjectKind& outKind)
    {
        switch (opcode)
        {
        case HGBCOpcode::SlangModule:
            outKind = LazyObjectKind::SlangModule;
            return true;
        case HGBCOpcode::SlangEntryPoint:
        case HGBCOpcode::SpecializeSlangEntryPoint:
            outKind = LazyObjectKind::SlangEntryPoint;
            return true;
        case HGBCOpcode::MakeDispatcher:
            outKind = LazyObjectKind::ComputeDispatcher;
            return true;
        case HGBCOpcode::MakeGraphicsPipeline:
            outKind = LazyObjectKind::GraphicsPipeline;
            return true;
        case HGBCOpcode::TypeLayout:
            outKind = LazyObjectKind::TypeLayout;
            return true;
        case HGBCOpcode::LayoutConverter:
            outKind = LazyObjectKind::LayoutConverter;
            return true;
        default:
            return false;
        }
    }

    /// Resolves one currently defined HGBC scalar type ID during staging.
    ///
    /// For example, `OpFloat16Type` maps to `Float16` while `OpFloat64Type` maps to `Float64`.
    bool tryGetScalarType(HGBCId typeId, ScalarType& outType) const
    {
        return m_sourceProgram->tryGetScalarType(typeId, outType);
    }

    /// Resolves one currently defined HGBC scalar type ID during staging.
    ///
    /// For example, `OpFloat16Type` maps to `Float16`; a valid vector type reaches the capability
    /// error instead of being mistaken for a scalar with a default layout.
    ScalarType getScalarType(HGBCId typeId) const
    {
        ScalarType result;
        if (tryGetScalarType(typeId, result))
            return result;
        fail("HGVM does not support this scalar type yet");
    }

    /// Aligns one byte offset using a power-of-two VM alignment.
    ///
    /// For example, offset one followed by a four-byte float becomes offset four. HGBC validation
    /// and staged type construction guarantee the alignment is nonzero and a power of two.
    static uint32_t alignByteOffset(uint32_t offset, uint32_t alignment)
    {
        SLANG_ASSERT(alignment && !(alignment & (alignment - 1)));
        uint64_t result = (uint64_t(offset) + alignment - 1) & ~uint64_t(alignment - 1);
        if (result > std::numeric_limits<uint32_t>::max())
            fail("HGVM type layout exceeds its size limit");
        return uint32_t(result);
    }

    const HGBC::Record& getTypeRecord(HGBCId typeId) const
    {
        auto found = m_sourceProgram->getIndex().resultRecords.find(typeId);
        SLANG_RELEASE_ASSERT(found != m_sourceProgram->getIndex().resultRecords.end());
        return m_sourceProgram->getDecodedProgram().records[found->second];
    }

    /// Encodes HGBC operands by their schema roles, never by program-local ID numbers.
    /// Nominal children contribute their compiler identity; structural children contribute their
    /// constructor and arguments. Length prefixes make names containing punctuation unambiguous.
    std::string getTypeDefinition(const HGBC::Record& record)
    {
        std::string result = std::to_string(uint32_t(record.opcode)) + ":";
        for (size_t i = 1; i < record.operands.size(); ++i)
        {
            const auto* operand = getHGBCOperandInfo(record.opcode, i);
            SLANG_RELEASE_ASSERT(operand);
            std::string value;
            switch (operand->kind)
            {
            case HGBCOperandKind::TypeId:
                value = getTypeKey(record.operands[i]);
                break;
            case HGBCOperandKind::Id:
                {
                    const auto* name = m_sourceProgram->findString(record.operands[i]);
                    SLANG_RELEASE_ASSERT(name);
                    value = *name;
                }
                break;
            default:
                value = std::to_string(record.operands[i]);
                break;
            }
            result += std::to_string(uint32_t(operand->kind)) + ":" + std::to_string(value.size()) +
                      ":" + value;
        }
        return result;
    }

    /// Uses lowered nominal identity for declared types and constructor identity for containers.
    /// Missing nominal metadata makes the type local to its retained Program, not structural.
    std::string getTypeKey(HGBCId typeId)
    {
        auto found = m_typeKeys.find(typeId);
        if (found != m_typeKeys.end())
        {
            if (found->second.empty())
                fail("HGVM structural type identity contains an unsupported cycle");
            return found->second;
        }
        const auto& record = getTypeRecord(typeId);
        std::string key;
        if (record.opcode == HGBCOpcode::StructType || record.opcode == HGBCOpcode::ClassType ||
            record.opcode == HGBCOpcode::TaggedUnionType)
        {
            if (auto name = m_sourceProgram->findStringDecoration(
                    typeId,
                    HGBCDecorationKind::TypeMangledName))
                key = "N" + std::to_string(name->size()) + ":" + *name;
            else
                key = "L" + std::to_string(reinterpret_cast<uintptr_t>(m_sourceProgram)) + ":" +
                      std::to_string(typeId);
        }
        else
        {
            m_typeKeys.emplace(typeId, std::string());
            key = "T" + getTypeDefinition(record);
        }
        m_typeKeys[typeId] = key;
        return key;
    }

    /// Records immutable layout facts for reuse after this load commits. Provisional reference
    /// layouts are published within the transaction before their recursive payload is visited.
    void publishTypeLayout(HGBCId typeId, TypeLayout& layout, bool complete = true)
    {
        const auto& record = getTypeRecord(typeId);
        layout.complete = complete;
        layout.definition = getTypeDefinition(record);
        layout.fieldLayouts.clear();
        for (const auto& field : layout.fields)
            layout.fieldLayouts.push_back({field.byteOffset, field.byteSize});
        auto name = m_sourceProgram->getSlangTypeName(typeId);
        layout.hostType.name = name ? String(name->c_str()) : String("anonymous");
        layout.hostType.byteSize = layout.byteSize;
        layout.hostType.kind = layout.kind == TypeKind::Scalar ? HostValueType::Kind::Scalar
                                                               : HostValueType::Kind::Aggregate;
        layout.hostType.scalarType = layout.numericLayout.scalarType;
        if (layout.kind == TypeKind::HeapObject && layout.heapObjectKind == HeapObjectKind::Window)
        {
            layout.hostType.kind = HostValueType::Kind::HostObject;
            layout.hostType.hostObjectKind = HGVMHostObjectKind::Window;
        }
        // Escape support is finalized after all recursive payload layouts are complete.
        layout.hostValueSupported = false;
        auto& entry = m_pendingTypes[getTypeKey(typeId)];
        if (!entry)
            entry = std::make_shared<CanonicalLayout>();
        *entry = static_cast<const CanonicalLayout&>(layout);
        m_pendingTypesByAddress[layout.typeInfoAddress.value] = entry;
        if (complete)
        {
            // Recursive HGBC aliases can observe the fixed handle before its payload has been
            // laid out. Complete those local bindings together, before staging instructions.
            auto aliases = m_pendingTypeAliases.find(layout.typeInfoAddress.value);
            if (aliases != m_pendingTypeAliases.end())
            {
                for (HGBCId aliasId : aliases->second)
                {
                    auto& alias = m_typeLayouts.at(aliasId);
                    static_cast<CanonicalLayout&>(alias) = *entry;
                    bindFieldLayouts(aliasId, alias, *entry);
                }
                m_pendingTypeAliases.erase(aliases);
            }
        }
    }

    void bindFieldLayouts(HGBCId typeId, TypeLayout& layout, const CanonicalLayout& canonical)
    {
        const auto& record = getTypeRecord(typeId);
        if (record.opcode != HGBCOpcode::StructType && record.opcode != HGBCOpcode::ClassType)
            return;
        size_t fieldCount = (record.operands.size() - 1) / 2;
        SLANG_RELEASE_ASSERT(!canonical.complete || fieldCount == canonical.fieldLayouts.size());
        layout.fields.clear();
        for (size_t i = 0; i < fieldCount; ++i)
            layout.fields.push_back(
                {record.operands[1 + i * 2],
                 record.operands[2 + i * 2],
                 canonical.complete ? canonical.fieldLayouts[i].byteOffset : 0,
                 canonical.complete ? canonical.fieldLayouts[i].byteSize : 0});
    }

    /// Rebinds source selectors to cached offsets without calculating the layout again.
    /// The definition check rejects a second module declaring the same identity differently.
    const TypeLayout& bindTypeLayout(HGBCId typeId, const CanonicalLayout& canonical)
    {
        const auto& record = getTypeRecord(typeId);
        if (getTypeDefinition(record) != canonical.definition)
            fail(
                "HGVM loaded programs disagree on the definition of type " +
                std::string(canonical.hostType.name.getBuffer()));
        TypeLayout layout;
        static_cast<CanonicalLayout&>(layout) = canonical;
        bindFieldLayouts(typeId, layout, canonical);
        if (!canonical.complete)
            m_pendingTypeAliases[canonical.typeInfoAddress.value].push_back(typeId);
        if (hasHGBCOpcodeFlag(record.opcode, HGBCOpcodeFlags::AddressType) ||
            record.opcode == HGBCOpcode::DeferType ||
            record.opcode == HGBCOpcode::DescriptorHandleType)
            layout.valueTypeId = record.operands[1];
        switch (record.opcode)
        {
        case HGBCOpcode::ArrayType:
        case HGBCOpcode::VectorType:
            layout.elementTypeId = record.operands[2];
            break;
        case HGBCOpcode::MatrixType:
            layout.elementTypeId = record.operands[4];
            break;
        case HGBCOpcode::RuntimeArrayType:
        case HGBCOpcode::StructuredBufferType:
        case HGBCOpcode::RWStructuredBufferType:
        case HGBCOpcode::Texture2DType:
        case HGBCOpcode::RenderTarget2DType:
            layout.elementTypeId = record.operands[1];
            break;
        default:
            break;
        }
        auto inserted = m_typeLayouts.emplace(typeId, std::move(layout));
        SLANG_RELEASE_ASSERT(inserted.second);
        TypeLayout& bound = inserted.first->second;
        // Bind children even on a cache hit: later instructions still select fields using IDs
        // scoped to this program. Publishing the parent first permits recursive class handles.
        for (size_t i = 1; i < record.operands.size(); ++i)
            if (getHGBCOperandInfo(record.opcode, i)->kind == HGBCOperandKind::TypeId &&
                getTypeRecord(record.operands[i]).opcode != HGBCOpcode::VoidType)
                getTypeLayout(record.operands[i]);
        return bound;
    }

    /// Calculates and stores one scalar, address, aggregate, or managed VM layout exactly once.
    ///
    /// Consider `struct S { bool b; float f; }`: staging lays out `b` at zero, `f` at four, rounds
    /// the final size to eight, and gives `Array<S, 3>` size 24, stride eight, and alignment four.
    /// Nested aggregates recursively reuse the same rule. Managed types publish a fixed-width
    /// handle or String layout and retain their payload metadata separately. Address wrappers have
    /// the fixed layout of `Address` and retain their value type structurally, regardless of which
    /// operation returns them.
    const TypeLayout& getTypeLayout(HGBCId typeId)
    {
        auto found = m_typeLayouts.find(typeId);
        if (found != m_typeLayouts.end())
            return found->second;

        const auto key = getTypeKey(typeId);
        auto pending = m_pendingTypes.find(key);
        if (pending != m_pendingTypes.end())
            return bindTypeLayout(typeId, *pending->second);
        auto committed = m_program->staging->types.find(key);
        if (committed != m_program->staging->types.end())
            return bindTypeLayout(typeId, *committed->second);

        auto visiting = m_typesBeingStaged.emplace(typeId, true);
        if (!visiting.second)
            fail("HGVM value type contains an unsupported inline cycle");

        TypeLayout layout;
        ScalarType scalarType;
        if (tryGetScalarType(typeId, scalarType))
        {
            ScalarLayout scalarLayout = getScalarLayout(scalarType);
            layout.kind = TypeKind::Scalar;
            layout.numericLayout.scalarType = scalarType;
            layout.numericLayout.laneByteStride = scalarLayout.byteSize;
            layout.byteSize = scalarLayout.byteSize;
            layout.byteAlignment = scalarLayout.byteAlignment;
        }
        else
        {
            auto typeRecordIt = m_sourceProgram->getIndex().resultRecords.find(typeId);
            SLANG_ASSERT(typeRecordIt != m_sourceProgram->getIndex().resultRecords.end());
            const HGBC::Record& typeRecord =
                m_sourceProgram->getDecodedProgram().records[typeRecordIt->second];
            if (hasHGBCOpcodeFlag(typeRecord.opcode, HGBCOpcodeFlags::AddressType))
            {
                SLANG_ASSERT(typeRecord.operands.size() == 2);
                layout.kind = TypeKind::Address;
                layout.valueTypeId = typeRecord.operands[1];
                getTypeLayout(layout.valueTypeId);
                layout.byteSize = uint32_t(sizeof(Address));
                layout.byteAlignment = uint32_t(alignof(Address));
            }
            else
            {
                switch (typeRecord.opcode)
                {
                case HGBCOpcode::BoxType:
                    layout.kind = TypeKind::HeapObject;
                    layout.heapObjectKind = HeapObjectKind::Box;
                    layout.byteSize = sizeof(Address);
                    layout.byteAlignment = alignof(Address);
                    layout.managedLeafOffsets.push_back(0);
                    break;
                case HGBCOpcode::TaggedUnionType:
                    // The payload layout is held by its box, so recursive alternatives do not
                    // require recursively laying out every possible value here.
                    layout.kind = TypeKind::TaggedUnion;
                    layout.byteSize = sizeof(TaggedUnionValue);
                    layout.byteAlignment = alignof(TaggedUnionValue);
                    layout.managedLeafOffsets.push_back(offsetof(TaggedUnionValue, payload));
                    break;
                case HGBCOpcode::GraphicsDrawType:
                    layout.kind = TypeKind::Struct;
                    layout.byteSize = sizeof(GraphicsDrawValue);
                    layout.byteAlignment = alignof(GraphicsDrawValue);
                    layout.managedLeafOffsets.push_back(offsetof(GraphicsDrawValue, indices));
                    break;
                case HGBCOpcode::GraphicsDrawParametersType:
                    layout.kind = TypeKind::Struct;
                    layout.byteSize = sizeof(GraphicsDrawParametersValue);
                    layout.byteAlignment = alignof(GraphicsDrawParametersValue);
                    break;
                case HGBCOpcode::StringType:
                    {
                        layout.kind = TypeKind::String;
                        layout.byteSize = uint32_t(sizeof(StringValue));
                        layout.byteAlignment = uint32_t(alignof(StringValue));
                        layout.managedLeafOffsets.push_back(uint32_t(offsetof(StringValue, owner)));
                    }
                    break;
                case HGBCOpcode::ByteStreamType:
                    {
                        layout.kind = TypeKind::HeapObject;
                        layout.heapObjectKind = HeapObjectKind::ByteStream;
                        layout.byteSize = uint32_t(sizeof(Address));
                        layout.byteAlignment = uint32_t(alignof(Address));
                        layout.managedLeafOffsets.push_back(0);
                    }
                    break;
                case HGBCOpcode::TimerType:
                    {
                        layout.kind = TypeKind::HeapObject;
                        layout.heapObjectKind = HeapObjectKind::Timer;
                        layout.byteSize = uint32_t(sizeof(Address));
                        layout.byteAlignment = uint32_t(alignof(Address));
                        layout.managedLeafOffsets.push_back(0);
                    }
                    break;
                case HGBCOpcode::RaytracingPipelineType:
                    layout.kind = TypeKind::HeapObject;
                    layout.heapObjectKind = HeapObjectKind::RaytracingPipeline;
                    layout.byteSize = uint32_t(sizeof(Address));
                    layout.byteAlignment = uint32_t(alignof(Address));
                    layout.managedLeafOffsets.push_back(0);
                    break;
                case HGBCOpcode::RaytracingAccelerationStructureType:
                case HGBCOpcode::RaytracingBottomLevelAccelerationStructureType:
                    layout.kind = TypeKind::HeapObject;
                    layout.heapObjectKind = HeapObjectKind::RayAccelerationStructure;
                    layout.byteSize = uint32_t(sizeof(Address));
                    layout.byteAlignment = uint32_t(alignof(Address));
                    layout.managedLeafOffsets.push_back(0);
                    break;
                case HGBCOpcode::DeferType:
                    {
                        // A Defer<T> is one managed handle regardless of T's inline extent. The
                        // wrapped layout is staged now so async readback and wait records can embed
                        // exact transfer sizes without inspecting the type during execution.
                        SLANG_ASSERT(typeRecord.operands.size() == 2);
                        layout.kind = TypeKind::HeapObject;
                        layout.heapObjectKind = HeapObjectKind::DeferredReadback;
                        layout.valueTypeId = typeRecord.operands[1];
                        getTypeLayout(layout.valueTypeId);
                        layout.byteSize = uint32_t(sizeof(Address));
                        layout.byteAlignment = uint32_t(alignof(Address));
                        layout.managedLeafOffsets.push_back(0);
                    }
                    break;
                case HGBCOpcode::StructuredBufferType:
                case HGBCOpcode::RWStructuredBufferType:
                    {
                        SLANG_ASSERT(typeRecord.operands.size() == 2);
                        layout.kind = TypeKind::HeapObject;
                        layout.heapObjectKind = HeapObjectKind::StructuredBuffer;
                        layout.elementTypeId = typeRecord.operands[1];
                        getTypeLayout(layout.elementTypeId);
                        layout.byteSize = uint32_t(sizeof(Address));
                        layout.byteAlignment = uint32_t(alignof(Address));
                        layout.managedLeafOffsets.push_back(0);
                    }
                    break;
                case HGBCOpcode::DescriptorHandleType:
                    {
                        SLANG_ASSERT(typeRecord.operands.size() == 2);
                        auto resourceTypeRecordIt =
                            m_sourceProgram->getIndex().resultRecords.find(typeRecord.operands[1]);
                        SLANG_ASSERT(
                            resourceTypeRecordIt !=
                            m_sourceProgram->getIndex().resultRecords.end());
                        const auto& records = m_sourceProgram->getDecodedProgram().records;
                        const HGBC::Record& resourceTypeRecord =
                            records[resourceTypeRecordIt->second];
                        SLANG_ASSERT(
                            resourceTypeRecord.opcode == HGBCOpcode::StructuredBufferType ||
                            resourceTypeRecord.opcode == HGBCOpcode::RWStructuredBufferType ||
                            resourceTypeRecord.opcode == HGBCOpcode::Texture2DType ||
                            resourceTypeRecord.opcode ==
                                HGBCOpcode::RaytracingAccelerationStructureType);

                        // The host value owns the same graph resource as its source resource. The
                        // physical RHI descriptor ID is device-specific and is resolved only when
                        // a dispatch binds this value to a reflected descriptor-handle parameter.
                        layout.kind = TypeKind::HeapObject;
                        layout.valueTypeId = typeRecord.operands[1];
                        layout.heapObjectKind = getTypeLayout(layout.valueTypeId).heapObjectKind;
                        layout.descriptorAccess =
                            (resourceTypeRecord.opcode == HGBCOpcode::RWStructuredBufferType ||
                             (resourceTypeRecord.opcode == HGBCOpcode::Texture2DType &&
                              HGBCTextureAccess(resourceTypeRecord.operands[2]) !=
                                  HGBCTextureAccess::ReadOnly))
                                ? DescriptorAccess::ReadWrite
                                : DescriptorAccess::Read;
                        layout.byteSize = uint32_t(sizeof(Address));
                        layout.byteAlignment = uint32_t(alignof(Address));
                        layout.managedLeafOffsets.push_back(0);
                    }
                    break;
                case HGBCOpcode::Texture2DType:
                case HGBCOpcode::RenderTarget2DType:
                    {
                        SLANG_ASSERT(
                            typeRecord.operands.size() ==
                            (typeRecord.opcode == HGBCOpcode::Texture2DType ? 4 : 3));
                        layout.kind = TypeKind::HeapObject;
                        layout.heapObjectKind = HeapObjectKind::Texture2D;
                        layout.elementTypeId = typeRecord.operands[1];
                        getTypeLayout(layout.elementTypeId);
                        layout.byteSize = uint32_t(sizeof(Address));
                        layout.byteAlignment = uint32_t(alignof(Address));
                        layout.managedLeafOffsets.push_back(0);
                    }
                    break;
                case HGBCOpcode::WindowType:
                    {
                        layout.kind = TypeKind::HeapObject;
                        layout.heapObjectKind = HeapObjectKind::Window;
                        layout.byteSize = uint32_t(sizeof(Address));
                        layout.byteAlignment = uint32_t(alignof(Address));
                        layout.managedLeafOffsets.push_back(0);
                    }
                    break;
                case HGBCOpcode::SurfaceType:
                    {
                        SLANG_ASSERT(typeRecord.operands.size() == 2);
                        layout.kind = TypeKind::HeapObject;
                        layout.heapObjectKind = HeapObjectKind::Surface;
                        layout.byteSize = uint32_t(sizeof(Address));
                        layout.byteAlignment = uint32_t(alignof(Address));
                        layout.managedLeafOffsets.push_back(0);
                    }
                    break;
                case HGBCOpcode::ClassType:
                    {
                        const HGBC::ClassTypeDescriptor* descriptor =
                            m_sourceProgram->findClassType(typeId);
                        SLANG_ASSERT(descriptor);

                        // Publish the fixed-width handle layout before visiting the payload. A
                        // recursive field such as `Node next` can then reuse the completed value
                        // representation without recursing into `Node`'s payload again.
                        layout.kind = TypeKind::HeapObject;
                        layout.heapObjectKind = HeapObjectKind::Class;
                        layout.byteSize = uint32_t(sizeof(Address));
                        layout.byteAlignment = uint32_t(alignof(Address));
                        layout.managedLeafOffsets.push_back(0);

                        StagedRecord::TypeInfo typeInfo;
                        List<StagedRecord::StructFieldInfo> typeFields;
                        typeInfo.kind = layout.kind;
                        typeInfo.heapObjectKind = layout.heapObjectKind;
                        typeInfo.managedLeafCount = 1;
                        typeInfo.byteSize = layout.byteSize;
                        typeInfo.byteAlignment = layout.byteAlignment;
                        layout.typeInfoAddress = m_builder->emitTypeInfo(
                            typeInfo,
                            typeFields.getArrayView(),
                            makeConstArrayView(
                                layout.managedLeafOffsets.data(),
                                Index(layout.managedLeafOffsets.size())));

                        auto inserted = m_typeLayouts.emplace(typeId, std::move(layout));
                        SLANG_ASSERT(inserted.second);
                        TypeLayout& classLayout = inserted.first->second;
                        publishTypeLayout(typeId, classLayout, false);

                        auto classTypeRecordIt =
                            m_sourceProgram->getIndex().classTypeRecordIndices.find(typeId);
                        SLANG_ASSERT(
                            classTypeRecordIt !=
                            m_sourceProgram->getIndex().classTypeRecordIndices.end());
                        const HGBC::Record& classTypeRecord =
                            m_sourceProgram->getDecodedProgram().records[classTypeRecordIt->second];
                        SLANG_ASSERT(
                            classTypeRecord.operands.size() == descriptor->fields.size() * 2 + 1);

                        classLayout.allocationByteSize = uint32_t(sizeof(ClassObject));
                        classLayout.allocationByteAlignment = uint32_t(alignof(ClassObject));
                        classLayout.fields.reserve(descriptor->fields.size());
                        for (size_t fieldIndex = 0; fieldIndex < descriptor->fields.size();
                             ++fieldIndex)
                        {
                            HGBCId fieldTypeId = descriptor->fields[fieldIndex].typeId;
                            const TypeLayout& fieldType = getTypeLayout(fieldTypeId);
                            uint32_t fieldOffset = alignByteOffset(
                                classLayout.allocationByteSize,
                                fieldType.byteAlignment);
                            uint64_t fieldEnd = uint64_t(fieldOffset) + fieldType.byteSize;
                            if (fieldEnd > std::numeric_limits<uint32_t>::max())
                                fail("HGVM class allocation layout exceeds its size limit");
                            classLayout.fields.push_back(
                                {classTypeRecord.operands[fieldIndex * 2 + 1],
                                 fieldTypeId,
                                 fieldOffset,
                                 fieldType.byteSize});
                            for (uint32_t leafOffset : fieldType.managedLeafOffsets)
                            {
                                classLayout.objectManagedLeafOffsets.push_back(
                                    fieldOffset + leafOffset);
                            }
                            classLayout.allocationByteSize = uint32_t(fieldEnd);
                            if (fieldType.byteAlignment > classLayout.allocationByteAlignment)
                                classLayout.allocationByteAlignment = fieldType.byteAlignment;
                        }

                        // Both field access and destruction use offsets from the object base.
                        // Copy the destruction table after the fields so instances can outlive
                        // their staging metadata without owning another allocation.
                        classLayout.managedLeafOffsetsByteOffset = alignByteOffset(
                            classLayout.allocationByteSize,
                            uint32_t(alignof(uint32_t)));
                        uint64_t allocationEnd =
                            uint64_t(classLayout.managedLeafOffsetsByteOffset) +
                            uint64_t(classLayout.objectManagedLeafOffsets.size()) *
                                sizeof(uint32_t);
                        if (allocationEnd > std::numeric_limits<uint32_t>::max())
                            fail("HGVM class allocation layout exceeds its size limit");
                        classLayout.allocationByteSize = alignByteOffset(
                            uint32_t(allocationEnd),
                            classLayout.allocationByteAlignment);
                        publishTypeLayout(typeId, classLayout);
                        m_typesBeingStaged.erase(typeId);
                        return classLayout;
                    }
                case HGBCOpcode::RuntimeArrayType:
                    {
                        SLANG_ASSERT(typeRecord.operands.size() == 2);

                        // Publish the fixed-width handle before visiting the element layout. This
                        // permits `class Node { RuntimeArray<Node> children; }` without recursively
                        // expanding either value representation.
                        layout.kind = TypeKind::HeapObject;
                        layout.heapObjectKind = HeapObjectKind::RuntimeArray;
                        layout.elementTypeId = typeRecord.operands[1];
                        layout.byteSize = uint32_t(sizeof(Address));
                        layout.byteAlignment = uint32_t(alignof(Address));
                        layout.managedLeafOffsets.push_back(0);

                        StagedRecord::TypeInfo typeInfo;
                        List<StagedRecord::StructFieldInfo> typeFields;
                        typeInfo.kind = layout.kind;
                        typeInfo.heapObjectKind = layout.heapObjectKind;
                        typeInfo.managedLeafCount = 1;
                        typeInfo.byteSize = layout.byteSize;
                        typeInfo.byteAlignment = layout.byteAlignment;
                        layout.typeInfoAddress = m_builder->emitTypeInfo(
                            typeInfo,
                            typeFields.getArrayView(),
                            makeConstArrayView(
                                layout.managedLeafOffsets.data(),
                                Index(layout.managedLeafOffsets.size())));

                        auto inserted = m_typeLayouts.emplace(typeId, std::move(layout));
                        SLANG_ASSERT(inserted.second);
                        TypeLayout& arrayLayout = inserted.first->second;
                        publishTypeLayout(typeId, arrayLayout, false);

                        const TypeLayout& elementLayout = getTypeLayout(arrayLayout.elementTypeId);
                        arrayLayout.elementByteStride =
                            alignByteOffset(elementLayout.byteSize, elementLayout.byteAlignment);
                        arrayLayout.elementByteAlignment = elementLayout.byteAlignment;
                        arrayLayout.elementManagedLeafOffsets = elementLayout.managedLeafOffsets;
                        publishTypeLayout(typeId, arrayLayout);
                        m_typesBeingStaged.erase(typeId);
                        return arrayLayout;
                    }
                case HGBCOpcode::ArrayType:
                    {
                        SLANG_ASSERT(typeRecord.operands.size() == 3);
                        layout.kind = TypeKind::Array;
                        layout.elementCount = typeRecord.operands[1];
                        layout.elementTypeId = typeRecord.operands[2];

                        const TypeLayout& elementLayout = getTypeLayout(layout.elementTypeId);
                        layout.byteAlignment = elementLayout.byteAlignment;
                        layout.elementByteStride =
                            alignByteOffset(elementLayout.byteSize, elementLayout.byteAlignment);
                        uint64_t byteSize =
                            uint64_t(layout.elementCount) * layout.elementByteStride;
                        if (byteSize > std::numeric_limits<uint32_t>::max())
                            fail("HGVM fixed-size array layout exceeds its size limit");

                        // Even an empty array needs an addressable inline result slot. Its chosen
                        // storage size remains aligned so an outer array can use it as an ordinary
                        // element.
                        layout.byteSize = byteSize ? uint32_t(byteSize) : layout.byteAlignment;
                        for (uint32_t elementIndex = 0; elementIndex < layout.elementCount;
                             ++elementIndex)
                        {
                            uint32_t elementOffset = elementIndex * layout.elementByteStride;
                            for (uint32_t leafOffset : elementLayout.managedLeafOffsets)
                                layout.managedLeafOffsets.push_back(elementOffset + leafOffset);
                        }
                    }
                    break;
                case HGBCOpcode::VectorType:
                    {
                        SLANG_ASSERT(typeRecord.operands.size() == 3);
                        layout.kind = TypeKind::Vector;
                        layout.elementCount = typeRecord.operands[1];
                        layout.elementTypeId = typeRecord.operands[2];

                        const TypeLayout& elementLayout = getTypeLayout(layout.elementTypeId);
                        SLANG_ASSERT(elementLayout.kind == TypeKind::Scalar);
                        layout.numericLayout.scalarType = elementLayout.numericLayout.scalarType;
                        layout.numericLayout.rowCount = 1;
                        layout.numericLayout.columnCount = layout.elementCount;
                        layout.numericLayout.laneCount = layout.elementCount;
                        layout.numericLayout.laneByteStride = elementLayout.byteSize;
                        layout.byteAlignment = elementLayout.byteAlignment;
                        layout.elementByteStride = elementLayout.byteSize;
                        layout.byteSize = layout.elementCount * elementLayout.byteSize;
                    }
                    break;
                case HGBCOpcode::MatrixType:
                    {
                        SLANG_ASSERT(typeRecord.operands.size() == 5);
                        layout.kind = TypeKind::Matrix;
                        layout.numericLayout.rowCount = typeRecord.operands[1];
                        layout.numericLayout.columnCount = typeRecord.operands[2];
                        layout.matrixLayout = typeRecord.operands[3];
                        layout.elementTypeId = typeRecord.operands[4];

                        const TypeLayout& elementLayout = getTypeLayout(layout.elementTypeId);
                        SLANG_ASSERT(elementLayout.kind == TypeKind::Scalar);
                        layout.numericLayout.scalarType = elementLayout.numericLayout.scalarType;
                        layout.numericLayout.laneCount =
                            layout.numericLayout.rowCount * layout.numericLayout.columnCount;
                        layout.numericLayout.laneByteStride = elementLayout.byteSize;
                        layout.byteAlignment = elementLayout.byteAlignment;
                        layout.elementCount = layout.numericLayout.rowCount;
                        layout.elementByteStride =
                            layout.numericLayout.columnCount * elementLayout.byteSize;
                        layout.byteSize = layout.numericLayout.laneCount * elementLayout.byteSize;
                    }
                    break;
                case HGBCOpcode::StructType:
                    {
                        const HGBC::StructTypeDescriptor* descriptor =
                            m_sourceProgram->findStructType(typeId);
                        SLANG_ASSERT(descriptor);
                        layout.kind = TypeKind::Struct;
                        layout.byteSize = 0;
                        layout.byteAlignment = 1;

                        auto structTypeRecordIt =
                            m_sourceProgram->getIndex().structTypeRecordIndices.find(typeId);
                        SLANG_ASSERT(
                            structTypeRecordIt !=
                            m_sourceProgram->getIndex().structTypeRecordIndices.end());
                        const HGBC::Record& structTypeRecord =
                            m_sourceProgram->getDecodedProgram()
                                .records[structTypeRecordIt->second];
                        SLANG_ASSERT(
                            structTypeRecord.operands.size() == descriptor->fields.size() * 2 + 1);

                        layout.fields.reserve(descriptor->fields.size());
                        for (size_t fieldIndex = 0; fieldIndex < descriptor->fields.size();
                             ++fieldIndex)
                        {
                            HGBCId fieldTypeId = descriptor->fields[fieldIndex].typeId;
                            const TypeLayout& fieldType = getTypeLayout(fieldTypeId);
                            uint32_t fieldOffset =
                                alignByteOffset(layout.byteSize, fieldType.byteAlignment);
                            uint64_t fieldEnd = uint64_t(fieldOffset) + fieldType.byteSize;
                            if (fieldEnd > std::numeric_limits<uint32_t>::max())
                                fail("HGVM struct layout exceeds its size limit");
                            layout.fields.push_back(
                                {structTypeRecord.operands[fieldIndex * 2 + 1],
                                 fieldTypeId,
                                 fieldOffset,
                                 fieldType.byteSize});
                            for (uint32_t leafOffset : fieldType.managedLeafOffsets)
                                layout.managedLeafOffsets.push_back(fieldOffset + leafOffset);
                            layout.byteSize = uint32_t(fieldEnd);
                            if (fieldType.byteAlignment > layout.byteAlignment)
                                layout.byteAlignment = fieldType.byteAlignment;
                        }

                        // Empty structs need an addressable inline representation even though they
                        // have no fields. One byte matches the usual value-type convention and
                        // keeps frame allocation independent of special zero-sized slots.
                        if (layout.fields.empty())
                            layout.byteSize = 1;
                        layout.byteSize = alignByteOffset(layout.byteSize, layout.byteAlignment);
                    }
                    break;
                default:
                    fail("HGVM does not support this value type yet");
                }
            }
        }

        StagedRecord::TypeInfo typeInfo;
        List<StagedRecord::StructFieldInfo> typeFields;
        typeInfo.kind = layout.kind;
        typeInfo.heapObjectKind = layout.heapObjectKind;
        typeInfo.descriptorAccess = layout.descriptorAccess;
        typeInfo.numericLayout = layout.numericLayout;
        switch (layout.kind)
        {
        case TypeKind::Address:
            typeInfo.valueTypeInfoAddress = getTypeLayout(layout.valueTypeId).typeInfoAddress;
            break;
        case TypeKind::Array:
        case TypeKind::Vector:
        case TypeKind::Matrix:
            typeInfo.elementTypeInfoAddress = getTypeLayout(layout.elementTypeId).typeInfoAddress;
            typeInfo.elementByteStride = layout.elementByteStride;
            typeInfo.elementCount = layout.elementCount;
            break;
        case TypeKind::HeapObject:
            if (layout.valueTypeId)
            {
                typeInfo.valueTypeInfoAddress = getTypeLayout(layout.valueTypeId).typeInfoAddress;
            }
            else if (layout.elementTypeId)
            {
                typeInfo.elementTypeInfoAddress =
                    getTypeLayout(layout.elementTypeId).typeInfoAddress;
                typeInfo.elementByteStride = layout.elementByteStride;
                typeInfo.elementCount = layout.elementCount;
            }
            break;
        case TypeKind::Struct:
            {
                typeInfo.fieldCount = uint32_t(layout.fields.size());
                if (!layout.fields.empty())
                {

                    for (const FieldLayout& field : layout.fields)
                    {
                        StagedRecord::StructFieldInfo fieldInfo;
                        fieldInfo.typeInfoAddress = getTypeLayout(field.typeId).typeInfoAddress;
                        const auto* name = m_sourceProgram->findString(field.nameId);
                        SLANG_RELEASE_ASSERT(name);
                        fieldInfo.fieldNameByteCount = uint32_t(name->size());
                        fieldInfo.fieldNameAddress = Address::makeAbsoluteInstruction(
                            m_builder->appendAlignedBytes(name->data(), uint32_t(name->size())));
                        fieldInfo.sourceByteOffset = field.byteOffset;
                        typeFields.add(fieldInfo);
                    }
                }
            }
            break;
        case TypeKind::Scalar:
        case TypeKind::String:
        case TypeKind::TaggedUnion:
            break;
        }
        typeInfo.matrixLayout = layout.matrixLayout;
        typeInfo.managedLeafCount = uint32_t(layout.managedLeafOffsets.size());
        typeInfo.byteSize = layout.byteSize;
        typeInfo.byteAlignment = layout.byteAlignment;
        layout.typeInfoAddress = m_builder->emitTypeInfo(
            typeInfo,
            typeFields.getArrayView(),
            makeConstArrayView(
                layout.managedLeafOffsets.data(),
                Index(layout.managedLeafOffsets.size())));

        m_typesBeingStaged.erase(typeId);
        auto inserted = m_typeLayouts.emplace(typeId, std::move(layout));
        SLANG_ASSERT(inserted.second);
        publishTypeLayout(typeId, inserted.first->second);
        return inserted.first->second;
    }

    /// Classifies values that may outlive an invocation using their complete payload types.
    ///
    /// Consider `class Node { Node next; int value; }`: its recursive owning field is safe, while
    /// adding a borrowed pointer anywhere in that graph makes every containing value unsafe.
    /// All staged value representations own their managed leaves; only addresses borrow storage.
    /// Propagate borrowed dependencies through fields and typed payloads until stable, so recursive
    /// owning types remain valid without maintaining a separate list of interop resource kinds.
    void finalizeHostValueSupport()
    {
        for (auto& pair : m_typeLayouts)
        {
            TypeLayout& layout = pair.second;
            SLANG_RELEASE_ASSERT(layout.complete);
            layout.hostValueSupported = layout.kind != TypeKind::Address;
        }

        bool changed;
        do
        {
            changed = false;
            for (auto& pair : m_typeLayouts)
            {
                TypeLayout& layout = pair.second;
                if (!layout.hostValueSupported)
                    continue;
                for (const auto& field : layout.fields)
                    layout.hostValueSupported &= m_typeLayouts.at(field.typeId).hostValueSupported;
                if (layout.elementTypeId)
                    layout.hostValueSupported &=
                        m_typeLayouts.at(layout.elementTypeId).hostValueSupported;
                if (layout.valueTypeId)
                    layout.hostValueSupported &=
                        m_typeLayouts.at(layout.valueTypeId).hostValueSupported;
                changed |= !layout.hostValueSupported;
            }
        } while (changed);

        for (const auto& pair : m_typeLayouts)
        {
            const TypeLayout& layout = pair.second;
            auto pending = m_pendingTypesByAddress.find(layout.typeInfoAddress.value);
            if (pending != m_pendingTypesByAddress.end())
                pending->second->hostValueSupported = layout.hostValueSupported;
            else
                SLANG_RELEASE_ASSERT(
                    m_program->staging->findType(layout.typeInfoAddress).hostValueSupported ==
                    layout.hostValueSupported);
        }
    }

    /// Stages descriptors for every supported value and address type in the decoded program.
    ///
    /// For example, a nested struct causes its field type descriptors to be appended before its
    /// own descriptor. Executable records later refer only to layouts that are already immutable.
    void stageTypes()
    {
        for (const HGBC::Record& record : m_sourceProgram->getDecodedProgram().records)
        {
            if (hasHGBCOpcodeFlag(record.opcode, HGBCOpcodeFlags::AddressType))
            {
                getTypeLayout(record.operands[0]);
                continue;
            }
            switch (record.opcode)
            {
            case HGBCOpcode::UIntType:
            case HGBCOpcode::IntType:
            case HGBCOpcode::Float16Type:
            case HGBCOpcode::Float32Type:
            case HGBCOpcode::Float64Type:
            case HGBCOpcode::BoolType:
            case HGBCOpcode::StructType:
            case HGBCOpcode::BoxType:
            case HGBCOpcode::TaggedUnionType:
            case HGBCOpcode::ClassType:
            case HGBCOpcode::RuntimeArrayType:
            case HGBCOpcode::ArrayType:
            case HGBCOpcode::VectorType:
            case HGBCOpcode::MatrixType:
            case HGBCOpcode::StringType:
            case HGBCOpcode::ByteStreamType:
            case HGBCOpcode::StructuredBufferType:
            case HGBCOpcode::RWStructuredBufferType:
            case HGBCOpcode::DescriptorHandleType:
            case HGBCOpcode::Texture2DType:
            case HGBCOpcode::RenderTarget2DType:
            case HGBCOpcode::WindowType:
            case HGBCOpcode::GraphicsDrawType:
            case HGBCOpcode::GraphicsDrawParametersType:
            case HGBCOpcode::RaytracingPipelineType:
            case HGBCOpcode::RaytracingAccelerationStructureType:
            case HGBCOpcode::RaytracingBottomLevelAccelerationStructureType:
            case HGBCOpcode::SurfaceType:
                getTypeLayout(record.operands[0]);
                break;
            default:
                break;
            }
        }
    }

    /// Returns a previously assigned direct address for a validated HGBC value ID.
    ///
    /// For example, an `OpAdd` operand may resolve to immutable literal storage or a frame-relative
    /// result slot. Missing IDs are validator or stager bugs and are not revalidated here.
    const StagedValueInfo& getValue(HGBCId valueId) const
    {
        auto found = m_values.find(valueId);
        SLANG_RELEASE_ASSERT(found != m_values.end());
        return found->second;
    }

    /// Returns the immutable byte payload retained for one validated literal operand.
    ///
    /// Consider an embedded module name: its ordinary String address names a 24-byte value so it
    /// can participate in calls, but module realization needs the original UTF-8 bytes directly.
    /// The validator has already established that embedded descriptors use literal operands.
    const StagedValueInfo& getLiteralPayload(HGBCId valueId) const
    {
        const StagedValueInfo& value = getValue(valueId);
        SLANG_ASSERT(value.literalPayloadAddress.getKind() == AddressKind::InstructionAbsolute);
        return value;
    }

    /// Appends one literal descriptor and its separately aligned immutable data.
    ///
    /// For example, a `Float64` literal will eventually request eight-byte alignment, while a large
    /// binary payload remains outside the fixed descriptor's traversable word count.
    void appendLiteral(
        const HGBC::Record& sourceRecord,
        ValueKind kind,
        ScalarType scalarType,
        const void* data,
        uint32_t byteSize,
        uint32_t byteAlignment)
    {
        uint32_t dataProgramCounter = m_builder->appendAlignedBytes(data, byteSize, byteAlignment);
        Address dataAddress = Address::makeAbsoluteInstruction(dataProgramCounter);
        m_builder->emitLiteral(
            sourceRecord.opcode,
            dataAddress,
            kind,
            scalarType,
            dataAddress,
            byteSize);

        HGBCId resultId = HGBC::getResultId(sourceRecord);
        int typeOperandIndex = HGBC::getTypeOperandIndex(sourceRecord.opcode);
        HGBCId typeId = typeOperandIndex >= 0 ? sourceRecord.operands[size_t(typeOperandIndex)] : 0;
        m_values.emplace(
            resultId,
            StagedValueInfo{
                kind,
                scalarType,
                typeId,
                dataAddress,
                byteSize,
                dataAddress,
                byteSize});
    }

    /// Appends allocation-free literal bytes plus their ordinary fixed-width String value.
    ///
    /// Consider passing `"hello"` to a helper: the argument copy reads the 24-byte value appended
    /// here, not the five raw characters. The value's null `owner` makes retain and release no-ops,
    /// while embedded-module staging can still use the separately recorded literal payload.
    void appendStringLiteral(const HGBC::Record& sourceRecord)
    {
        SLANG_RELEASE_ASSERT(sourceRecord.payload.size() <= std::numeric_limits<uint32_t>::max());
        uint32_t payloadByteSize = uint32_t(sourceRecord.payload.size());

        uint32_t payloadProgramCounter =
            m_builder->appendAlignedBytes(sourceRecord.payload.data(), payloadByteSize, 1);
        Address payloadAddress = Address::makeAbsoluteInstruction(payloadProgramCounter);
        StringValue value;
        value.literalBytes = payloadAddress;
        value.byteCount = payloadByteSize;
        uint32_t valueProgramCounter = m_builder->appendAlignedBytes(
            &value,
            uint32_t(sizeof(value)),
            uint32_t(alignof(StringValue)));
        Address valueAddress = Address::makeAbsoluteInstruction(valueProgramCounter);

        m_builder->emitLiteral(
            sourceRecord.opcode,
            valueAddress,
            ValueKind::String,
            ScalarType::UInt8,
            payloadAddress,
            payloadByteSize);

        HGBCId resultId = HGBC::getResultId(sourceRecord);
        HGBCId typeId = sourceRecord.operands[0];
        auto inserted = m_values.emplace(
            resultId,
            StagedValueInfo{
                ValueKind::String,
                ScalarType::UInt8,
                typeId,
                valueAddress,
                uint32_t(sizeof(StringValue)),
                payloadAddress,
                payloadByteSize});
        SLANG_ASSERT(inserted.second);
    }

    /// Converts every current LITR opcode into immutable width-aware staged storage.
    ///
    /// Consider `%minusOne : Int32 = OpIntLit 0xffffffff`: the bytes remain `0xffffffff`, while
    /// its staged scalar code determines whether later operations interpret them as signed bits.
    void stageLiterals()
    {
        for (const HGBC::Record& record : m_sourceProgram->getDecodedProgram().records)
        {
            switch (record.opcode)
            {
            case HGBCOpcode::IntLit:
            case HGBCOpcode::FloatLit:
                {
                    ScalarType scalarType = getScalarType(record.operands[0]);
                    ScalarLayout layout = getScalarLayout(scalarType);
                    // Literal words are ordered from low to high significance. Validation ensures
                    // that binary64 has both words; narrower scalars use the low bytes only.
                    uint64_t bits = record.operands[2];
                    if (layout.logicalBitWidth == 64)
                        bits |= uint64_t(record.operands[3]) << 32;
                    appendLiteral(
                        record,
                        ValueKind::Scalar,
                        scalarType,
                        &bits,
                        layout.byteSize,
                        layout.byteAlignment);
                }
                break;
            case HGBCOpcode::BoolLit:
                {
                    uint8_t value = record.operands[2] ? 1 : 0;
                    appendLiteral(
                        record,
                        ValueKind::Scalar,
                        ScalarType::Bool,
                        &value,
                        uint32_t(sizeof(value)),
                        uint32_t(alignof(uint8_t)));
                }
                break;
            case HGBCOpcode::NullLit:
                {
                    const TypeLayout& layout = getTypeLayout(record.operands[0]);
                    if (layout.kind == TypeKind::TaggedUnion)
                    {
                        TaggedUnionValue empty;
                        appendLiteral(
                            record,
                            ValueKind::Struct,
                            ScalarType::UInt8,
                            &empty,
                            uint32_t(sizeof(empty)),
                            uint32_t(alignof(TaggedUnionValue)));
                        break;
                    }
                    SLANG_ASSERT(layout.kind == TypeKind::HeapObject);
                    Address nullAddress;
                    appendLiteral(
                        record,
                        ValueKind::HeapObject,
                        ScalarType::UInt8,
                        &nullAddress,
                        uint32_t(sizeof(nullAddress)),
                        uint32_t(alignof(Address)));
                }
                break;
            case HGBCOpcode::StringLit:
                appendStringLiteral(record);
                break;
            case HGBCOpcode::BinaryLit:
                {
                    SLANG_RELEASE_ASSERT(
                        record.payload.size() <= std::numeric_limits<uint32_t>::max());
                    appendLiteral(
                        record,
                        ValueKind::Binary,
                        ScalarType::UInt8,
                        record.payload.data(),
                        uint32_t(record.payload.size()),
                        1);
                }
                break;
            default:
                break;
            }
        }
    }

    /// Translates validated, immutable wire state to RHI descriptors once during staging.
    /// Only the draw records contain resource values; these descriptors need no runtime handles.
    uint32_t stageGraphicsPipeline(const HGBC::Record& record)
    {
        const auto& records = m_sourceProgram->getDecodedProgram().records;
        const auto& index = m_sourceProgram->getIndex().resultRecords;
        const auto& state = records[index.at(record.operands[4])].operands;
        const auto& attachments = records[index.at(record.operands[5])].operands;
        if (attachments.size() != 3)
            fail("HGVM currently supports one color attachment per graphics pipeline");
        const auto& color = records[index.at(attachments[2])].operands;
        const auto& texture = records[index.at(color[1])].operands;
        Address resultVertexEntryPoint = {};
        Address resultFragmentEntryPoint = {};
        rhi::PrimitiveTopology resultTopology = rhi::PrimitiveTopology::TriangleList;
        rhi::RasterizerDesc resultRasterizer = {};
        rhi::DepthStencilDesc resultDepthStencil = {};
        rhi::MultisampleDesc resultMultisample = {};
        rhi::ColorTargetDesc resultColorTarget = {};

        resultVertexEntryPoint = getValue(record.operands[2]).address;
        resultFragmentEntryPoint = getValue(record.operands[3]).address;
        using S = HGBCGraphicsPipelineStateOperands;
        switch (HGBCPrimitiveTopology(state[S::topology]))
        {
        case HGBCPrimitiveTopology::TriangleList:
            resultTopology = rhi::PrimitiveTopology::TriangleList;
            break;
        case HGBCPrimitiveTopology::PointList:
            resultTopology = rhi::PrimitiveTopology::PointList;
            break;
        case HGBCPrimitiveTopology::LineList:
            resultTopology = rhi::PrimitiveTopology::LineList;
            break;
        case HGBCPrimitiveTopology::LineStrip:
            resultTopology = rhi::PrimitiveTopology::LineStrip;
            break;
        case HGBCPrimitiveTopology::TriangleStrip:
            resultTopology = rhi::PrimitiveTopology::TriangleStrip;
            break;
        case HGBCPrimitiveTopology::PatchList:
            fail("HGVM patch draws require tessellation-stage support");
        }
        resultRasterizer.fillMode = rhi::FillMode(state[S::fillMode]);
        resultRasterizer.cullMode = rhi::CullMode(state[S::cullMode]);
        resultRasterizer.frontFace = rhi::FrontFaceMode(state[S::frontFace]);
        std::memcpy(&resultRasterizer.depthBias, &state[S::depthBias], sizeof(int32_t));
        std::memcpy(&resultRasterizer.depthBiasClamp, &state[S::depthBiasClamp], sizeof(float));
        std::memcpy(
            &resultRasterizer.slopeScaledDepthBias,
            &state[S::slopeScaledDepthBias],
            sizeof(float));
        resultRasterizer.depthClipEnable = state[S::depthClipEnable] != 0;
        resultRasterizer.scissorEnable = state[S::scissorEnable] != 0;
        resultRasterizer.multisampleEnable = state[S::multisampleEnable] != 0;
        resultRasterizer.antialiasedLineEnable = state[S::antialiasedLineEnable] != 0;
        resultRasterizer.enableConservativeRasterization =
            state[S::enableConservativeRasterization] != 0;
        resultRasterizer.forcedSampleCount = state[S::forcedSampleCount];
        resultDepthStencil.format = getRHIResourceFormat(HGBCResourceFormat(attachments[1]));
        resultDepthStencil.depthTestEnable = state[S::depthTestEnable] != 0;
        resultDepthStencil.depthWriteEnable = state[S::depthWriteEnable] != 0;
        resultDepthStencil.depthFunc = rhi::ComparisonFunc(state[S::depthFunc]);
        resultDepthStencil.stencilEnable = state[S::stencilEnable] != 0;
        resultDepthStencil.stencilReadMask = state[S::stencilReadMask];
        resultDepthStencil.stencilWriteMask = state[S::stencilWriteMask];
        resultDepthStencil.frontFace.stencilFailOp = rhi::StencilOp(state[S::frontStencilFailOp]);
        resultDepthStencil.frontFace.stencilDepthFailOp =
            rhi::StencilOp(state[S::frontStencilDepthFailOp]);
        resultDepthStencil.frontFace.stencilPassOp = rhi::StencilOp(state[S::frontStencilPassOp]);
        resultDepthStencil.frontFace.stencilFunc = rhi::ComparisonFunc(state[S::frontStencilFunc]);
        resultDepthStencil.backFace.stencilFailOp = rhi::StencilOp(state[S::backStencilFailOp]);
        resultDepthStencil.backFace.stencilDepthFailOp =
            rhi::StencilOp(state[S::backStencilDepthFailOp]);
        resultDepthStencil.backFace.stencilPassOp = rhi::StencilOp(state[S::backStencilPassOp]);
        resultDepthStencil.backFace.stencilFunc = rhi::ComparisonFunc(state[S::backStencilFunc]);
        resultMultisample.sampleMask = state[S::sampleMask];
        resultMultisample.alphaToCoverageEnable = state[S::alphaToCoverageEnable] != 0;
        resultMultisample.alphaToOneEnable = state[S::alphaToOneEnable] != 0;
        using C = HGBCGraphicsColorTargetStateOperands;
        resultColorTarget.format = getRHIResourceFormat(HGBCResourceFormat(texture[3]));
        resultColorTarget.enableBlend = color[C::enableBlend] != 0;
        resultColorTarget.logicOp = rhi::LogicOp(color[C::logicOp]);
        resultColorTarget.writeMask = rhi::RenderTargetWriteMask(color[C::writeMask]);
        resultColorTarget.color.srcFactor = rhi::BlendFactor(color[C::colorSrcFactor]);
        resultColorTarget.color.dstFactor = rhi::BlendFactor(color[C::colorDstFactor]);
        resultColorTarget.color.op = rhi::BlendOp(color[C::colorOp]);
        resultColorTarget.alpha.srcFactor = rhi::BlendFactor(color[C::alphaSrcFactor]);
        resultColorTarget.alpha.dstFactor = rhi::BlendFactor(color[C::alphaDstFactor]);
        resultColorTarget.alpha.op = rhi::BlendOp(color[C::alphaOp]);
        return uint32_t(m_builder
                            ->emitGraphicsPipeline(
                                resultVertexEntryPoint,
                                resultFragmentEntryPoint,
                                resultTopology,
                                resultRasterizer,
                                resultDepthStencil,
                                resultMultisample,
                                resultColorTarget)
                            .getUnsignedPayload());
    }

    /// Lays out const-heap objects and emits their immutable lazy descriptors.
    ///
    /// Consider module -> entry point -> dispatcher: the first pass gives every result a stable
    /// const address, and the second pass can embed dependencies regardless of source-record order.
    /// Expensive Slang and RHI work remains deferred until execution.
    void stageEmbeddedObjects()
    {
        const auto& records = m_sourceProgram->getDecodedProgram().records;
        for (const HGBC::Record& record : records)
        {
            LazyObjectKind lazyKind;
            if (!tryGetLazyObjectKind(record.opcode, lazyKind))
                continue;

            HGBCId typeId = 0;
            if (record.opcode == HGBCOpcode::MakeDispatcher ||
                record.opcode == HGBCOpcode::MakeGraphicsPipeline)
                typeId = record.operands[0];

            ConstObjectStorageLayout storageLayout = getConstObjectStorageLayout(lazyKind);
            if (storageLayout.byteAlignment > kStorageBaseAlignment)
                fail("HGVM const object exceeds the arena's base alignment");
            uint32_t byteOffset =
                alignByteOffset(m_program->constHeapByteSize, storageLayout.byteAlignment);
            uint64_t endOffset = uint64_t(byteOffset) + storageLayout.byteSize;
            if (byteOffset >= kFramePayloadLimit || endOffset > kFramePayloadLimit)
                fail("HGVM const heap exceeds its address limit");
            m_program->constHeapByteSize = uint32_t(endOffset);

            HGBCId resultId = HGBC::getResultId(record);
            uint32_t layoutIndex = uint32_t(m_program->constObjectLayouts.getCount());
            m_program->constObjectLayouts.add(
                {lazyKind, byteOffset, kInvalidInstructionWordOffset});
            auto layoutInserted = m_constObjectLayoutIndices.emplace(resultId, layoutIndex);
            SLANG_ASSERT(layoutInserted.second);

            Address address = Address::makeConstHeap(byteOffset);
            auto inserted = m_values.emplace(
                resultId,
                StagedValueInfo{
                    ValueKind::ConstObject,
                    ScalarType::UInt32,
                    typeId,
                    address,
                    storageLayout.byteSize});
            SLANG_ASSERT(inserted.second);
        }

        for (const HGBC::Record& record : records)
        {
            LazyObjectKind lazyKind;
            if (!tryGetLazyObjectKind(record.opcode, lazyKind))
                continue;
            uint32_t descriptorWordOffset = kInvalidInstructionWordOffset;

            switch (record.opcode)
            {
            case HGBCOpcode::MakeGraphicsPipeline:
                descriptorWordOffset = stageGraphicsPipeline(record);
                break;
            case HGBCOpcode::TypeLayout:
                {
                    // A layout descriptor keeps both sides of lazy resolution explicit. For a host
                    // layout the staged type-info address is immediately usable; for a shader
                    // layout the context points at the dispatcher whose reflected parameter
                    // supplies offsets.
                    Address contextAddress;
                    bool graphicsContext = false;
                    if (record.operands[4])
                    {
                        contextAddress = getValue(record.operands[4]).address;
                        size_t contextIndex =
                            m_sourceProgram->getIndex().resultRecords.at(record.operands[4]);
                        graphicsContext =
                            records[contextIndex].opcode == HGBCOpcode::MakeGraphicsPipeline;
                    }
                    descriptorWordOffset =
                        uint32_t(m_builder
                                     ->emitTypeLayoutObject(
                                         record.operands[1],
                                         HGBCLayoutDomain(record.operands[2]),
                                         HGBCLayoutUsage(record.operands[3]),
                                         getTypeLayout(record.operands[1]).typeInfoAddress,
                                         contextAddress,
                                         record.operands[5],
                                         record.operands.size() == 7 ? record.operands[6] : 0,
                                         graphicsContext)
                                     .getUnsignedPayload());
                }
                break;
            case HGBCOpcode::LayoutConverter:
                {
                    descriptorWordOffset = uint32_t(m_builder
                                                        ->emitLayoutConverterObject(
                                                            getValue(record.operands[1]).address,
                                                            getValue(record.operands[2]).address)
                                                        .getUnsignedPayload());
                }
                break;
            default:
                {
                    switch (record.opcode)
                    {
                    case HGBCOpcode::SlangModule:
                        {
                            const auto& name = getLiteralPayload(record.operands[1]);
                            const auto& binary = getLiteralPayload(record.operands[2]);
                            List<Address> dependencies;
                            for (size_t i = 3; i < record.operands.size(); ++i)
                                dependencies.add(getValue(record.operands[i]).address);
                            descriptorWordOffset = uint32_t(m_builder
                                                                ->emitSlangModule(
                                                                    name.literalPayloadAddress,
                                                                    name.literalPayloadByteSize,
                                                                    binary.literalPayloadAddress,
                                                                    binary.literalPayloadByteSize,
                                                                    dependencies.getArrayView())
                                                                .getUnsignedPayload());
                        }
                        break;
                    case HGBCOpcode::SlangEntryPoint:
                        {
                            const auto& name = getLiteralPayload(record.operands[2]);
                            descriptorWordOffset =
                                uint32_t(m_builder
                                             ->emitSlangEntryPoint(
                                                 getValue(record.operands[1]).address,
                                                 name.literalPayloadAddress,
                                                 name.literalPayloadByteSize)
                                             .getUnsignedPayload());
                        }
                        break;
                    case HGBCOpcode::SpecializeSlangEntryPoint:
                        {
                            List<StagedRecord::SpecializationArgument> arguments;
                            for (size_t i = 2; i < record.operands.size(); i += 2)
                            {
                                const auto& value = getLiteralPayload(record.operands[i + 1]);
                                arguments.add(
                                    {HGBCSpecializationArgKind(record.operands[i]),
                                     value.literalPayloadAddress,
                                     value.literalPayloadByteSize});
                            }
                            descriptorWordOffset =
                                uint32_t(m_builder
                                             ->emitSpecializeSlangEntryPoint(
                                                 getValue(record.operands[1]).address,
                                                 arguments.getArrayView())
                                             .getUnsignedPayload());
                        }
                        break;
                    case HGBCOpcode::MakeDispatcher:
                        if (HGBCDispatchStage(record.operands[2]) != HGBCDispatchStage::Compute)
                            fail("HGVM only supports compute dispatchers in this milestone");
                        descriptorWordOffset = uint32_t(
                            m_builder->emitMakeDispatcher(getValue(record.operands[3]).address)
                                .getUnsignedPayload());
                        break;
                    default:
                        SLANG_UNREACHABLE("lazy-object opcode classification mismatch");
                    }
                }
                break;
            }

            auto layoutIndex = m_constObjectLayoutIndices.find(HGBC::getResultId(record));
            SLANG_ASSERT(layoutIndex != m_constObjectLayoutIndices.end());
            ConstObjectLayout& objectLayout = m_program->constObjectLayouts[layoutIndex->second];
            SLANG_ASSERT(
                objectLayout.kind == lazyKind &&
                objectLayout.descriptorWordOffset == kInvalidInstructionWordOffset);
            objectLayout.descriptorWordOffset = descriptorWordOffset;
            if (lazyKind == LazyObjectKind::SlangEntryPoint)
                m_debugMapBuilder->setEntryPointName(
                    descriptorWordOffset,
                    HGBC::getResultId(record));
            if (lazyKind == LazyObjectKind::ComputeDispatcher ||
                lazyKind == LazyObjectKind::GraphicsPipeline)
                m_debugMapBuilder->setPipelineName(descriptorWordOffset, record);
        }
    }

    /// Allocates storage for a record solely from its declared result type.
    ///
    /// Consider both `OpAdd Float32 ...` and `OpParam Float32`: their opcode has no bearing on
    /// storage planning. Each result receives the four-byte alignment and extent selected by the
    /// shared Float32 layout. A result whose type is `Ptr<Float32>` instead receives an address
    /// slot because the staged type descriptor says it is an address; no producer-opcode list is
    /// involved. `OpVar` additionally allocates the backing object required by its own semantics.
    void allocateRecordResult(const HGBC::Record& record)
    {
        int resultOperandIndex = HGBC::getResultOperandIndex(record.opcode);
        int typeOperandIndex = HGBC::getTypeOperandIndex(record.opcode);
        if (resultOperandIndex < 0 || typeOperandIndex < 0)
            return;

        HGBCId resultId = record.operands[size_t(resultOperandIndex)];
        HGBCId typeId = record.operands[size_t(typeOperandIndex)];
        auto typeRecordIt = m_sourceProgram->getIndex().resultRecords.find(typeId);
        SLANG_RELEASE_ASSERT(typeRecordIt != m_sourceProgram->getIndex().resultRecords.end());
        const HGBC::Record& typeRecord =
            m_sourceProgram->getDecodedProgram().records[typeRecordIt->second];
        if (typeRecord.opcode == HGBCOpcode::VoidType)
            return;

        const TypeLayout& resultLayout = getTypeLayout(typeId);
        bool isAddressResult = resultLayout.kind == TypeKind::Address;
        HGBCId valueTypeId = isAddressResult ? resultLayout.valueTypeId : typeId;
        const TypeLayout& valueLayout = getTypeLayout(valueTypeId);
        Address address = m_frameLayout.allocate(resultLayout.byteSize, resultLayout.byteAlignment);
        ValueKind kind = ValueKind::Scalar;
        switch (resultLayout.kind)
        {
        case TypeKind::Scalar:
            kind = ValueKind::Scalar;
            break;
        case TypeKind::Vector:
            kind = ValueKind::Vector;
            break;
        case TypeKind::Matrix:
            kind = ValueKind::Matrix;
            break;
        case TypeKind::Struct:
        case TypeKind::TaggedUnion:
            kind = ValueKind::Struct;
            break;
        case TypeKind::Array:
            kind = ValueKind::Array;
            break;
        case TypeKind::Address:
            kind = ValueKind::Address;
            break;
        case TypeKind::HeapObject:
            kind = ValueKind::HeapObject;
            break;
        case TypeKind::String:
            kind = ValueKind::String;
            break;
        }
        auto inserted = m_values.emplace(
            resultId,
            StagedValueInfo{
                kind,
                valueLayout.numericLayout.scalarType,
                valueTypeId,
                address,
                resultLayout.byteSize});
        SLANG_RELEASE_ASSERT(inserted.second);
        if (!isAddressResult && !resultLayout.managedLeafOffsets.empty())
            m_owningSlots.push_back({address, typeId});

        if (record.opcode == HGBCOpcode::Var)
        {
            SLANG_ASSERT(isAddressResult);
            Address storageAddress =
                m_frameLayout.allocate(valueLayout.byteSize, valueLayout.byteAlignment);
            auto storageInserted = m_variableStorageAddresses.emplace(resultId, storageAddress);
            SLANG_ASSERT(storageInserted.second);
            if (!valueLayout.managedLeafOffsets.empty())
                m_owningSlots.push_back({storageAddress, valueTypeId});
        }
    }

    /// Reports whether HGVM has an evaluator for an otherwise valid HGBC instruction.
    ///
    /// For example, `OpAdd`, direct `OpCall`, and `OpAllocateResource` return true because their
    /// staged evaluators exist. An opcode without an execution path returns false; descriptor-level
    /// capability gaps are reported by its staging routine. This choice is independent of whether
    /// an instruction has a stored result.
    bool supportsInstruction(HGBCOpcode opcode) const
    {
        switch (opcode)
        {
        case HGBCOpcode::Add:
        case HGBCOpcode::Sub:
        case HGBCOpcode::Mul:
        case HGBCOpcode::IDiv:
        case HGBCOpcode::IRem:
        case HGBCOpcode::Lsh:
        case HGBCOpcode::FDiv:
        case HGBCOpcode::Pow:
        case HGBCOpcode::Min:
        case HGBCOpcode::Max:
        case HGBCOpcode::Atan2:
        case HGBCOpcode::Eql:
        case HGBCOpcode::Neq:
        case HGBCOpcode::Less:
        case HGBCOpcode::Greater:
        case HGBCOpcode::Leq:
        case HGBCOpcode::Geq:
        case HGBCOpcode::Neg:
        case HGBCOpcode::Not:
        case HGBCOpcode::Sin:
        case HGBCOpcode::Cos:
        case HGBCOpcode::Tan:
        case HGBCOpcode::Asin:
        case HGBCOpcode::Sqrt:
        case HGBCOpcode::Ceil:
        case HGBCOpcode::Floor:
        case HGBCOpcode::Exp:
        case HGBCOpcode::Exp2:
        case HGBCOpcode::Log:
        case HGBCOpcode::Log2:
        case HGBCOpcode::Any:
        case HGBCOpcode::ScalarTypeCast:
        case HGBCOpcode::Param:
        case HGBCOpcode::Var:
        case HGBCOpcode::Load:
        case HGBCOpcode::ResourceToDescriptorHandle:
        case HGBCOpcode::DescriptorHandleToResource:
        case HGBCOpcode::Store:
        case HGBCOpcode::FieldAddress:
        case HGBCOpcode::ElementAddress:
        case HGBCOpcode::FieldExtract:
        case HGBCOpcode::ExtractElement:
        case HGBCOpcode::MakeStruct:
        case HGBCOpcode::MakeArray:
        case HGBCOpcode::MakeVector:
        case HGBCOpcode::MakeMatrix:
        case HGBCOpcode::Call:
        case HGBCOpcode::FormatString:
        case HGBCOpcode::Print:
        case HGBCOpcode::Fail:
        case HGBCOpcode::StringGetByteCount:
        case HGBCOpcode::StringReadByte:
        case HGBCOpcode::CreateTimer:
        case HGBCOpcode::TimerElapsed:
        case HGBCOpcode::ResetTimer:
        case HGBCOpcode::MakeRaytracingPipeline:
        case HGBCOpcode::AllocAccelerationStructure:
        case HGBCOpcode::BuildAccelerationStructure:
        case HGBCOpcode::DispatchRays:
        case HGBCOpcode::SetRayMiss:
        case HGBCOpcode::SetRayHitGroup:
        case HGBCOpcode::Return:
        case HGBCOpcode::Branch:
        case HGBCOpcode::BranchConditional:
        case HGBCOpcode::AllocateObject:
        case HGBCOpcode::Box:
        case HGBCOpcode::Unbox:
        case HGBCOpcode::MakeTaggedUnion:
        case HGBCOpcode::GetTaggedUnionCase:
        case HGBCOpcode::GetTaggedUnionPayload:
        case HGBCOpcode::ExtractTaggedUnion:
        case HGBCOpcode::ConvertTaggedUnion:
        case HGBCOpcode::AllocateRuntimeArray:
        case HGBCOpcode::RuntimeArrayGetCount:
        case HGBCOpcode::CopyArrayRange:
        case HGBCOpcode::AllocateResource:
        case HGBCOpcode::ReadBackBufferElement:
        case HGBCOpcode::AsyncReadBackBufferElement:
        case HGBCOpcode::WaitDefer:
        case HGBCOpcode::StructuredBufferGetDimensions:
        case HGBCOpcode::TextureGetDimensions:
        case HGBCOpcode::BlitTexture2D:
        case HGBCOpcode::ClearTexture:
        case HGBCOpcode::MakeGraphicsDraw:
        case HGBCOpcode::MakeGraphicsDrawParameters:
        case HGBCOpcode::CreateWindow:
        case HGBCOpcode::ProcessWindowEvents:
        case HGBCOpcode::PollInputEvent:
        case HGBCOpcode::CloseWindow:
        case HGBCOpcode::SetWindowTitle:
        case HGBCOpcode::CreateSurface:
        case HGBCOpcode::AcquireSurfaceTexture:
        case HGBCOpcode::PresentSurface:
        case HGBCOpcode::TryAcquireSurfaceTexture:
        case HGBCOpcode::TryPresentSurface:
        case HGBCOpcode::ResizeSurface:
        case HGBCOpcode::ByteStreamFromFile:
        case HGBCOpcode::CloseByteStream:
        case HGBCOpcode::ByteStreamGetSize:
        case HGBCOpcode::ByteStreamRead:
        case HGBCOpcode::ByteStreamWrite:
        case HGBCOpcode::ImageDecode:
        case HGBCOpcode::ImageEncode:
            return true;
        default:
            return false;
        }
    }

    /// Assigns frame storage and rejects only operations outside the current HGVM capability set.
    ///
    /// For example, all arithmetic results are assigned before emission, so a use in a backward
    /// loop can embed its source address without depending on source record order.
    void analyzeFunction(const HGBC::Function& function)
    {
        m_frameLayout = FrameLayoutBuilder();
        m_blockProgramCounters.clear();
        m_branchFixups.clear();
        m_conditionalFixups.clear();
        m_owningSlots.clear();
        m_debugVariables.clear();

        for (const HGBC::Block& block : function.blocks)
        {
            for (size_t recordIndex : block.recordIndices)
            {
                const HGBC::Record& record =
                    m_sourceProgram->getDecodedProgram().records[recordIndex];
                if (hasHGBCOpcodeFlag(record.opcode, HGBCOpcodeFlags::Debug))
                {
                    if (m_preserveDebugScopes && record.opcode == HGBCOpcode::DebugVariable)
                    {
                        const auto& layout = getTypeLayout(record.operands[2]);
                        StagedValueInfo value;
                        value.typeId = record.operands[2];
                        value.byteSize = layout.byteSize;
                        value.address =
                            m_frameLayout.allocate(layout.byteSize, layout.byteAlignment);
                        m_debugVariables.emplace(record.operands[0], value);
                    }
                    continue;
                }

                if (!supportsInstruction(record.opcode))
                {
                    const HGBCOpcodeInfo* opcodeInfo = getHGBCOpcodeInfo(record.opcode);
                    std::string opcodeName = opcodeInfo ? opcodeInfo->name : "unknown opcode";
                    fail("HGVM staging does not support " + opcodeName + " yet");
                }
                allocateRecordResult(record);
            }
        }
    }

    /// Resolves an inline aggregate update without reading its untouched siblings.
    void stageDebugValue(const HGBC::Record& record)
    {
        if (record.operands.size() < 2)
            return;
        auto variable = m_debugVariables.find(record.operands[0]);
        if (variable == m_debugVariables.end())
            fail("HGVM debug update has no variable declaration in this function");
        const auto& source = getValue(record.operands[1]);
        HGBCId type = variable->second.typeId;
        uint64_t offset = 0;
        List<StagedRecord::DebugElementOffset> elements;
        for (size_t i = 2; i + 1 < record.operands.size(); i += 2)
        {
            const auto& layout = getTypeLayout(type);
            HGBCId selectedType = record.operands[i];
            HGBCId selector = record.operands[i + 1];
            // A class/array handle is a live binding: its ordinary field/element store has
            // already updated the object. Only inline subobjects need copies into debug slots.
            if (layout.kind == TypeKind::HeapObject)
                return;
            if (layout.kind == TypeKind::Struct)
            {
                bool found = false;
                for (const auto& field : layout.fields)
                    if (field.nameId == selector && field.typeId == selectedType)
                    {
                        offset += field.byteOffset;
                        found = true;
                        break;
                    }
                if (!found)
                    fail("HGVM debug update selects an invalid struct field");
            }
            else if (
                layout.kind == TypeKind::Array || layout.kind == TypeKind::Vector ||
                layout.kind == TypeKind::Matrix)
            {
                const auto& index = getValue(selector);
                if (index.kind != ValueKind::Scalar || (index.scalarType != ScalarType::UInt32 &&
                                                        index.scalarType != ScalarType::Int32))
                    fail("HGVM debug update requires a 32-bit element index");
                if (getTypeLayout(selectedType).byteSize > layout.elementByteStride)
                    fail("HGVM debug update element exceeds its stride");
                elements.add({index.address, layout.elementByteStride, layout.elementCount});
            }
            else
                fail("HGVM debug update requires an inline aggregate");
            type = selectedType;
        }
        if (getTypeLayout(type).byteSize != source.byteSize ||
            offset + source.byteSize > variable->second.byteSize)
            fail("HGVM debug update has an inconsistent value layout");
        m_builder->emitDebugValue(
            variable->second.address,
            source.address,
            source.byteSize,
            variable->second.byteSize,
            uint32_t(offset),
            elements.getArrayView());
    }

    /// Emits one generic print whose type and storage are already resolved.
    ///
    /// For example, a computed `%sum` embeds its frame address and `UInt32`, while a string literal
    /// embeds its immutable payload address and byte count.
    void stagePrint(const HGBC::Record& record)
    {
        const StagedValueInfo& value = getValue(record.operands[1]);
        ScalarType scalarType = value.kind == ValueKind::Scalar ? getScalarType(record.operands[0])
                                                                : ScalarType::UInt32;
        m_builder->emitPrint(value.kind, scalarType, value.address);
    }

    /// Emits one format operation with every repeated type/value pair classified once.
    ///
    /// Consider `format("{} {:.1f}", name, rate)`: staging records a String argument followed by a
    /// Float32 argument. Execution can therefore parse the runtime pattern without consulting an
    /// HGBC type table for either replacement.
    void stageFormatString(const HGBC::Record& record)
    {
        size_t argumentCount = (record.operands.size() - 3) / 2;

        const TypeLayout& resultLayout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(resultLayout.kind == TypeKind::String);
        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        // A loop can execute the same SSA record more than once because frame slots are static.
        // Release the preceding dynamic string before transferring a new initial reference here.
        stageReleaseValue(resultAddress, resultLayout);

        const StagedValueInfo& pattern = getValue(record.operands[2]);
        SLANG_ASSERT(pattern.kind == ValueKind::String);
        List<StagedRecord::FormatArgument> arguments;

        for (size_t argumentIndex = 0; argumentIndex < argumentCount; ++argumentIndex)
        {
            size_t typeOperandIndex = 3 + argumentIndex * 2;
            const TypeLayout& argumentLayout = getTypeLayout(record.operands[typeOperandIndex]);
            const StagedValueInfo& value = getValue(record.operands[typeOperandIndex + 1]);
            StagedRecord::FormatArgument argument;
            argument.valueAddress = value.address;
            if (argumentLayout.kind == TypeKind::String)
            {
                argument.kind = ValueKind::String;
            }
            else
            {
                SLANG_ASSERT(argumentLayout.kind == TypeKind::Scalar);
                argument.kind = ValueKind::Scalar;
                argument.scalarType = argumentLayout.numericLayout.scalarType;
            }
            arguments.add(argument);
        }
        Address stagedAddress =
            m_builder->emitFormatString(resultAddress, pattern.address, arguments.getArrayView());
        m_debugMapBuilder->setRecordType(
            uint32_t(stagedAddress.getUnsignedPayload()),
            record.operands[0]);
    }

    /// Emits `OpStringGetByteCount` or `OpFail` with one direct String value address.
    ///
    /// For example, both operations consume the same literal-or-owned representation; only their
    /// HGBC opcode determines whether execution writes UInt32 or begins exceptional unwinding.
    void stageStringOperation(const HGBC::Record& record, size_t stringOperandIndex)
    {
        const StagedValueInfo& value = getValue(record.operands[stringOperandIndex]);
        SLANG_ASSERT(value.kind == ValueKind::String);
        Address resultAddress;
        int resultOperandIndex = HGBC::getResultOperandIndex(record.opcode);
        if (resultOperandIndex >= 0)
        {
            const StagedValueInfo& result = getValue(record.operands[size_t(resultOperandIndex)]);
            resultAddress = result.address;
        }
        m_builder->emitStringOperation(record.opcode, resultAddress, value.address);
    }

    /// Emits one byte-indexing operation using direct String and UInt32 addresses.
    ///
    /// Consider `readStringByte(text, i)`: validation proves the types, and staging discards those
    /// IDs so execution only reads the byte count, checks `i`, and loads the selected byte.
    void stageStringReadByte(const HGBC::Record& record)
    {
        const StagedValueInfo& value = getValue(record.operands[2]);
        const StagedValueInfo& index = getValue(record.operands[3]);
        SLANG_ASSERT(value.kind == ValueKind::String);
        SLANG_ASSERT(index.kind == ValueKind::Scalar && index.scalarType == ScalarType::UInt32);
        m_builder->emitStringReadByte(
            getValue(HGBC::getResultId(record)).address,
            value.address,
            index.address);
    }

    /// Emits timer operations after converting their optional handle into a direct address.
    ///
    /// Consider `elapsedSeconds(timer)`: staging embeds the timer handle location and the already
    /// allocated Float32 result location, leaving execution with no HGBC ID or type lookup.
    void stageTimerOperation(const HGBC::Record& record)
    {
        Address resultAddress;
        Address timerAddress;
        if (record.opcode == HGBCOpcode::CreateTimer)
        {
            const TypeLayout& resultLayout = getTypeLayout(record.operands[0]);
            resultAddress = getValue(HGBC::getResultId(record)).address;
            stageReleaseValue(resultAddress, resultLayout);
        }
        else
        {
            timerAddress = getValue(record.operands.back()).address;
            if (record.opcode == HGBCOpcode::TimerElapsed)
                resultAddress = getValue(HGBC::getResultId(record)).address;
        }
        Address stagedAddress =
            m_builder->emitTimerOperation(record.opcode, resultAddress, timerAddress);
        if (record.opcode == HGBCOpcode::CreateTimer)
            m_debugMapBuilder->setRecordType(
                uint32_t(stagedAddress.getUnsignedPayload()),
                record.operands[0]);
    }

    /// Retains source argument types for deferred registration instead of inventing a static
    /// destination pipeline. Target layouts become known only after host registration executes.
    void stageRayShaderOperation(const HGBC::Record& record)
    {
        if (record.opcode == HGBCOpcode::DispatchRays)
        {
            StagedRecord::DispatchRays dispatch;
            dispatch.entryPointAddress = getValue(record.operands[0]).address;
            dispatch.extentAddress = getValue(record.operands[1]).address;
            dispatch.maxRecursion = record.operands[2];
            List<StagedRecord::RayShaderArgument> arguments;
            for (size_t i = 3; i < record.operands.size(); i += 3)
            {
                StagedRecord::RayShaderArgument argument;
                argument.parameterIndex = record.operands[i];
                argument.typeId = record.operands[i + 1];
                argument.typeInfoAddress = getTypeLayout(record.operands[i + 1]).typeInfoAddress;
                argument.valueAddress = getValue(record.operands[i + 2]).address;
                arguments.add(argument);
            }
            m_builder->emitDispatchRays(dispatch, arguments.getArrayView());
            return;
        }
        if (record.opcode == HGBCOpcode::MakeRaytracingPipeline)
        {
            Address result = getValue(record.operands[1]).address;
            stageReleaseValue(result, getTypeLayout(record.operands[0]));
            Address staged =
                m_builder->emitMakeRaytracingPipeline(result, getValue(record.operands[2]).address);
            m_debugMapBuilder->setRecordType(
                uint32_t(staged.getUnsignedPayload()),
                record.operands[0]);
            return;
        }

        StagedRecord::SetRayShader registration;
        registration.pipelineAddress = getValue(record.operands[0]).address;
        const bool isHit = record.opcode == HGBCOpcode::SetRayHitGroup;
        if (isHit)
        {
            registration.geometrySlotAddress = getValue(record.operands[1]).address;
            registration.raySlotAddress = getValue(record.operands[2]).address;
            const auto& group =
                m_sourceProgram->getDecodedProgram()
                    .records[m_sourceProgram->getIndex().resultRecords.at(record.operands[3])];
            SLANG_ASSERT(group.opcode == HGBCOpcode::RayHitGroup);
            registration.geometryKind = HGBCRayGeometryKind(group.operands[1]);
            registration.payloadType = group.operands[2];
            registration.attributeType = group.operands[3];
            for (size_t i = 4; i < group.operands.size(); i += 2)
                registration.entryPoints[group.operands[i]] =
                    getValue(group.operands[i + 1]).address;
        }
        else
        {
            registration.raySlotAddress = getValue(record.operands[1]).address;
            registration.entryPoints[0] = getValue(record.operands[2]).address;
            registration.payloadType = record.operands[3];
        }

        List<StagedRecord::RayShaderArgument> arguments;
        for (size_t i = 4; i < record.operands.size();)
        {
            StagedRecord::RayShaderArgument argument;
            if (isHit)
                argument.stage = HGBCRayStageRole(record.operands[i++]);
            argument.parameterIndex = record.operands[i++];
            argument.typeId = record.operands[i++];
            argument.typeInfoAddress = getTypeLayout(argument.typeId).typeInfoAddress;
            argument.valueAddress = getValue(record.operands[i++]).address;
            arguments.add(argument);
        }
        m_builder->emitSetRayShader(record.opcode, registration, arguments.getArrayView());
    }

    /// Resolves the AS descriptor's specified field roles to canonical host byte offsets.
    /// Field declaration order is immaterial; the validated schema names every role explicitly.
    void stageRayAccelerationStructure(const HGBC::Record& record)
    {
        if (record.opcode == HGBCOpcode::AllocAccelerationStructure)
        {
            Address result = getValue(record.operands[1]).address;
            stageReleaseValue(result, getTypeLayout(record.operands[0]));
            m_builder->emitAllocRayAccelerationStructure(result, record.operands[2] == 1);
            return;
        }
        StagedRecord::BuildRayAccelerationStructure build;
        build.topLevel = record.operands[0] == 1;
        build.inputKind = HGBCAccelerationStructureInputKind(record.operands[1]);
        build.structureAddress = getValue(record.operands[3]).address;
        build.inputsAddress = getValue(record.operands[4]).address;
        build.inputsTypeAddress =
            getTypeLayout(getValue(record.operands[4]).typeId).typeInfoAddress;
        build.buildFlagsAddress = getValue(record.operands[5]).address;
        build.buildModeAddress = getValue(record.operands[6]).address;
        const char* triangleFields[] = {
            "vertices",
            "indices",
            "vertexOffset",
            "vertexCount",
            "indexOffset",
            "indexCount",
            "flags"};
        const char* instanceFields[] = {"blas", "transform", "instanceID", "mask", "flags"};
        const auto& layout = getTypeLayout(record.operands[2]);
        const char* aabbFields[] = {"bounds", "primitiveOffset", "primitiveCount", "flags"};
        const bool aabbs = build.inputKind == HGBCAccelerationStructureInputKind::AABBs;
        uint32_t fieldCount = aabbs ? 4 : build.topLevel ? 5 : 7;
        for (uint32_t i = 0; i < fieldCount; ++i)
        {
            const char* name = aabbs            ? aabbFields[i]
                               : build.topLevel ? instanceFields[i]
                                                : triangleFields[i];
            bool found = false;
            for (const auto& field : layout.fields)
            {
                const std::string* fieldName = m_sourceProgram->findString(field.nameId);
                if (fieldName && *fieldName == name)
                {
                    build.fieldOffsets[i] = field.byteOffset;
                    found = true;
                    break;
                }
            }
            SLANG_RELEASE_ASSERT(found);
        }
        m_builder->emitBuildRayAccelerationStructure(build);
    }

    /// Returns the packed numeric layout for a scalar, vector, or matrix type.
    ///
    /// Consider `float3`: staging receives its `Float32` leaf and lane count three here, while a
    /// struct reaches the assertion because aggregate arithmetic is not a numeric operation.
    const NumericLayout& getNumericLayout(HGBCId typeId)
    {
        const TypeLayout& layout = getTypeLayout(typeId);
        SLANG_ASSERT(
            layout.kind == TypeKind::Scalar || layout.kind == TypeKind::Vector ||
            layout.kind == TypeKind::Matrix);
        return layout.numericLayout;
    }

    /// Emits one unary numeric operation with a direct result and operand address.
    ///
    /// For example, `OpNot` no longer resolves either its bool type ID or input value ID during
    /// execution; both were converted here after shared HGBC validation.
    void stageUnary(const HGBC::Record& record)
    {
        const NumericLayout& numericLayout = getNumericLayout(record.operands[0]);
        m_builder->emitUnary(
            record.opcode,
            getValue(HGBC::getResultId(record)).address,
            numericLayout.scalarType,
            numericLayout.laneCount,
            getValue(record.operands[2]).address);
    }

    /// Emits one scalar Boolean reduction with the operand's packed numeric layout.
    void stageAny(const HGBC::Record& record)
    {
        const NumericLayout& numericLayout = getNumericLayout(record.operands[2]);
        m_builder->emitUnary(
            record.opcode,
            getValue(HGBC::getResultId(record)).address,
            numericLayout.scalarType,
            numericLayout.laneCount,
            getValue(record.operands[3]).address);
    }

    /// Replaces one scalar cast's type and value IDs with compact codes and direct addresses.
    ///
    /// Consider `%f = OpScalarTypeCast Int32 Float32 %i`: the staged instruction stores `Int32`,
    /// `Float32`, and `%i`'s resolved address. The destination address was allocated from `Float32`
    /// through the ordinary result-type path, so this operation needs no size fields of its own.
    void stageScalarTypeCast(const HGBC::Record& record)
    {
        const TypeLayout& sourceLayout = getTypeLayout(record.operands[1]);
        const TypeLayout& destinationLayout = getTypeLayout(record.operands[2]);
        SLANG_ASSERT(
            sourceLayout.kind == TypeKind::Scalar && destinationLayout.kind == TypeKind::Scalar);

        m_builder->emitScalarTypeCast(
            getValue(HGBC::getResultId(record)).address,
            sourceLayout.numericLayout.scalarType,
            destinationLayout.numericLayout.scalarType,
            getValue(record.operands[3]).address);
    }

    /// Emits one typed binary operation using the common HGBC operand layout.
    ///
    /// Consider `%sum : float = OpAdd float %left %right` and
    /// `%test : bool = OpLess float %left %right`: both put the operation type at operand two and
    /// the values at operands three and four. Staging therefore needs no opcode-family branch.
    void stageTypedBinaryOp(const HGBC::Record& record)
    {
        const NumericLayout& numericLayout = getNumericLayout(record.operands[2]);
        m_builder->emitBinary(
            record.opcode,
            getValue(HGBC::getResultId(record)).address,
            numericLayout.scalarType,
            numericLayout.laneCount,
            getValue(record.operands[3]).address,
            getValue(record.operands[4]).address);
    }

    /// Finds one precomputed field layout while translating a field-specific HGBC operation.
    ///
    /// For example, looking up `weight` in `Sample` returns offset four. HGBC validation owns the
    /// field-existence rule; reaching the assertion means the validator and stager disagree.
    const FieldLayout& getFieldLayout(HGBCId aggregateTypeId, HGBCId fieldNameId)
    {
        const TypeLayout& layout = getTypeLayout(aggregateTypeId);
        SLANG_ASSERT(
            layout.kind == TypeKind::Struct || (layout.kind == TypeKind::HeapObject &&
                                                layout.heapObjectKind == HeapObjectKind::Class));
        for (const FieldLayout& field : layout.fields)
        {
            if (field.nameId == fieldNameId)
                return field;
        }
        SLANG_UNREACHABLE("validated HGBC field is missing from its staged aggregate layout");
    }

    /// Emits an `OpVar` that materializes a canonical absolute address at runtime.
    ///
    /// For example, the backing bytes may be at relative frame offset 32, but the result slot gets
    /// an absolute address formed from the invocation's frame base plus 32.
    void stageVar(const HGBC::Record& record)
    {
        HGBCId resultId = HGBC::getResultId(record);
        auto storage = m_variableStorageAddresses.find(resultId);
        SLANG_ASSERT(storage != m_variableStorageAddresses.end());
        SLANG_ASSERT(storage->second.getKind() == AddressKind::FrameRelative);

        m_builder->emitVar(
            getValue(resultId).address,
            uint32_t(storage->second.getUnsignedPayload()));
    }

    /// Emits explicit retain or release records for every managed leaf in one staged value.
    ///
    /// Consider storing `struct S { RWStructuredBuffer<uint> a; uint x; }` through a pointer: the
    /// single leaf record reads the destination address at runtime, adds `a`'s staged byte offset,
    /// and updates that handle. Pure POD layouts emit no records.
    void stageReferenceOperations(
        StagedRecord::InternalOpcode opcode,
        Address targetAddress,
        const TypeLayout& layout,
        bool isIndirect)
    {
        for (uint32_t leafByteOffset : layout.managedLeafOffsets)
        {
            m_builder->emitReference(opcode, targetAddress, leafByteOffset, isIndirect ? 1u : 0u);
        }
    }

    /// Emits the release half of constructing over an existing owning value.
    void stageReleaseValue(Address targetAddress, const TypeLayout& layout, bool isIndirect = false)
    {
        stageReferenceOperations(
            StagedRecord::InternalOpcode::MaybeDecRefAndRelease,
            targetAddress,
            layout,
            isIndirect);
    }

    /// Emits the retain half that establishes ownership after a fixed-size bit copy.
    void stageRetainValue(Address targetAddress, const TypeLayout& layout, bool isIndirect = false)
    {
        stageReferenceOperations(
            StagedRecord::InternalOpcode::IncRef,
            targetAddress,
            layout,
            isIndirect);
    }

    /// Emits a raw load with the pointee type's byte extent embedded in the record.
    ///
    /// For example, loading `Sample` records eight bytes; execution reads the address token and
    /// performs one copy regardless of how many fields `Sample` contains.
    void stageLoad(const HGBC::Record& record)
    {
        const StagedValueInfo& pointer = getValue(record.operands[2]);
        SLANG_ASSERT(pointer.kind == ValueKind::Address);
        const TypeLayout& typeLayout = getTypeLayout(record.operands[0]);
        Address resultAddress = getValue(HGBC::getResultId(record)).address;

        // TODO(HGVM workaround): These reference records inline the synthesized copy constructor.
        // Replace this expansion with a normal staged call once generated support functions can be
        // emitted independently of source HGBC functions.
        stageReleaseValue(resultAddress, typeLayout);

        m_builder->emitLoad(resultAddress, pointer.address, typeLayout.byteSize);
        stageRetainValue(resultAddress, typeLayout);
    }

    /// Emits a managed resource-to-handle conversion without allocating a wrapper object.
    ///
    /// The source resource and result handle intentionally store the same graph-object address. The
    /// retain after the copy gives the handle independent ownership if the source local dies.
    void stageResourceToDescriptorHandle(const HGBC::Record& record)
    {
        const StagedValueInfo& resource = getValue(record.operands[2]);
        SLANG_ASSERT(resource.kind == ValueKind::HeapObject);
        const TypeLayout& handleLayout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(
            handleLayout.kind == TypeKind::HeapObject &&
            handleLayout.descriptorAccess != DescriptorAccess::None);
        Address resultAddress = getValue(HGBC::getResultId(record)).address;

        stageReleaseValue(resultAddress, handleLayout);

        m_builder->emitResourceToDescriptorHandle(resultAddress, resource.address);
        stageRetainValue(resultAddress, handleLayout);
    }

    /// Emits a handle-to-resource conversion as an owning graph-object address copy.
    void stageDescriptorHandleToResource(const HGBC::Record& record)
    {
        const StagedValueInfo& handle = getValue(record.operands[2]);
        SLANG_ASSERT(handle.kind == ValueKind::HeapObject);
        const TypeLayout& resourceLayout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(
            resourceLayout.kind == TypeKind::HeapObject &&
            resourceLayout.descriptorAccess == DescriptorAccess::None);
        Address resultAddress = getValue(HGBC::getResultId(record)).address;

        stageReleaseValue(resultAddress, resourceLayout);

        m_builder->emitDescriptorHandleToResource(resultAddress, handle.address);
        stageRetainValue(resultAddress, resourceLayout);
    }

    /// Emits a raw store whose byte extent comes from the addressed type.
    ///
    /// For example, assigning one nested struct local to another embeds the nested struct's padded
    /// size and copies those bytes directly; no runtime type or field lookup is required.
    void stageStore(const HGBC::Record& record)
    {
        const StagedValueInfo& pointer = getValue(record.operands[0]);
        const StagedValueInfo& value = getValue(record.operands[1]);
        SLANG_ASSERT(pointer.kind == ValueKind::Address);
        const TypeLayout& typeLayout = getTypeLayout(pointer.typeId);

        stageReleaseValue(pointer.address, typeLayout, true);

        m_builder->emitStore(pointer.address, value.address, typeLayout.byteSize);
        stageRetainValue(pointer.address, typeLayout, true);
    }

    /// Emits a field-address operation with the source field name replaced by its byte offset.
    ///
    /// Consider `sample.inner.value`: each source `OpFieldAddress` becomes one address read and one
    /// precomputed offset addition, so nested access composes without inspecting either struct.
    void stageFieldAddress(const HGBC::Record& record)
    {
        const StagedValueInfo& base = getValue(record.operands[2]);
        SLANG_ASSERT(base.kind == ValueKind::Address);
        const TypeLayout& baseLayout = getTypeLayout(base.typeId);
        const FieldLayout& field = getFieldLayout(base.typeId, record.operands[3]);

        Address classTypeAddress;
        if (baseLayout.kind == TypeKind::HeapObject &&
            baseLayout.heapObjectKind == HeapObjectKind::Class)
        {
            classTypeAddress = baseLayout.typeInfoAddress;
        }
        m_builder->emitFieldAddress(
            getValue(HGBC::getResultId(record)).address,
            base.address,
            field.byteOffset,
            classTypeAddress);
    }

    /// Emits a field extraction with a direct base address, offset, and copy extent.
    ///
    /// For example, extracting a four-byte scalar from offset 12 resolves exactly the first 16
    /// source bytes and copies bytes 12 through 15 into the inline result slot.
    void stageFieldExtract(const HGBC::Record& record)
    {
        const StagedValueInfo& base = getValue(record.operands[2]);
        const TypeLayout& baseLayout = getTypeLayout(base.typeId);
        const FieldLayout& field = getFieldLayout(base.typeId, record.operands[3]);
        const TypeLayout& resultLayout = getTypeLayout(record.operands[0]);
        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        stageReleaseValue(resultAddress, resultLayout);

        Address classTypeAddress;
        if (baseLayout.kind == TypeKind::HeapObject &&
            baseLayout.heapObjectKind == HeapObjectKind::Class)
        {
            classTypeAddress = baseLayout.typeInfoAddress;
        }
        m_builder->emitFieldExtract(
            resultAddress,
            base.address,
            field.byteOffset,
            field.byteSize,
            classTypeAddress);
        stageRetainValue(resultAddress, resultLayout);
    }

    /// Emits an indexable-element address with the base layout reduced to stride and count.
    ///
    /// Consider `&values[index]` for `Array<Pair, 3>`: staging embeds `sizeof(Pair)` rounded to its
    /// alignment as the stride. Execution reads only the base address and index before adding the
    /// resulting byte offset.
    void stageElementAddress(const HGBC::Record& record)
    {
        const StagedValueInfo& base = getValue(record.operands[2]);
        const StagedValueInfo& index = getValue(record.operands[3]);
        SLANG_ASSERT(index.kind == ValueKind::Scalar);
        const TypeLayout& layout = getTypeLayout(base.typeId);
        bool isRuntimeArray = layout.kind == TypeKind::HeapObject &&
                              layout.heapObjectKind == HeapObjectKind::RuntimeArray;
        if (isRuntimeArray)
        {
            SLANG_ASSERT(base.kind == ValueKind::HeapObject);
            SLANG_ASSERT(
                index.scalarType == ScalarType::Int32 || index.scalarType == ScalarType::UInt32);
        }
        else
        {
            SLANG_ASSERT(base.kind == ValueKind::Address);
            SLANG_ASSERT(index.scalarType == ScalarType::UInt32);
            SLANG_ASSERT(
                layout.kind == TypeKind::Array || layout.kind == TypeKind::Vector ||
                layout.kind == TypeKind::Matrix);
        }

        m_builder->emitElementAddress(
            getValue(HGBC::getResultId(record)).address,
            base.address,
            index.address,
            layout.elementByteStride,
            layout.elementCount,
            isRuntimeArray ? layout.typeInfoAddress : Address{},
            index.scalarType);
    }

    /// Emits an indexable-element extraction with a direct base, index, stride, and copy extent.
    ///
    /// Consider `values[index]` where each value is an eight-byte struct: execution computes
    /// `index * elementByteStride` and performs one eight-byte copy. It never inspects the struct
    /// or array type descriptor.
    void stageElementExtract(const HGBC::Record& record)
    {
        const StagedValueInfo& base = getValue(record.operands[2]);
        const StagedValueInfo& index = getValue(record.operands[3]);
        SLANG_ASSERT(
            base.kind == ValueKind::Array || base.kind == ValueKind::Vector ||
            base.kind == ValueKind::Matrix);
        SLANG_ASSERT(index.kind == ValueKind::Scalar && index.scalarType == ScalarType::UInt32);
        const TypeLayout& layout = getTypeLayout(base.typeId);
        SLANG_ASSERT(
            layout.kind == TypeKind::Array || layout.kind == TypeKind::Vector ||
            layout.kind == TypeKind::Matrix);
        const TypeLayout& resultLayout = getTypeLayout(record.operands[0]);
        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        stageReleaseValue(resultAddress, resultLayout);

        m_builder->emitElementExtract(
            resultAddress,
            base.address,
            index.address,
            layout.elementByteStride,
            resultLayout.byteSize,
            layout.elementCount);
        stageRetainValue(resultAddress, resultLayout);
    }

    /// Emits one struct construction using its precomputed field offsets and sizes.
    ///
    /// Consider `Outer { enabled, inner }`: staging emits one copy for each declared field using
    /// the field's independently calculated offset and size. The runtime can still consume the
    /// generic aggregate-copy record without knowing that these members came from a struct.
    void stageMakeStruct(const HGBC::Record& record)
    {
        const TypeLayout& layout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(layout.kind == TypeKind::Struct);
        SLANG_ASSERT(record.operands.size() == layout.fields.size() + 2);

        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        stageReleaseValue(resultAddress, layout);

        List<StagedRecord::AggregateElementCopy> elements;

        for (size_t fieldIndex = 0; fieldIndex < layout.fields.size(); ++fieldIndex)
        {
            const FieldLayout& field = layout.fields[fieldIndex];
            StagedRecord::AggregateElementCopy copy;
            copy.sourceAddress = getValue(record.operands[fieldIndex + 2]).address;
            copy.destinationByteOffset = field.byteOffset;
            copy.byteSize = field.byteSize;
            elements.add(copy);
        }
        m_builder->emitMakeAggregate(
            record.opcode,
            resultAddress,
            layout.byteSize,
            elements.getArrayView());
        stageRetainValue(resultAddress, layout);
    }

    /// Emits one fixed-array construction using its uniform precomputed element layout.
    ///
    /// Consider `Array<Inner, 3>(a, b, c)`: every source has the same byte size, while destination
    /// offsets are zero, one stride, and two strides. Keeping this separate from struct staging
    /// avoids conditional field-versus-element interpretation during staging.
    void stageMakeArray(const HGBC::Record& record)
    {
        const TypeLayout& layout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(layout.kind == TypeKind::Array);
        SLANG_ASSERT(record.operands.size() == size_t(layout.elementCount) + 2);

        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        stageReleaseValue(resultAddress, layout);

        List<StagedRecord::AggregateElementCopy> elements;

        const TypeLayout& elementLayout = getTypeLayout(layout.elementTypeId);
        for (uint32_t elementIndex = 0; elementIndex < layout.elementCount; ++elementIndex)
        {
            uint64_t byteOffset = uint64_t(elementIndex) * layout.elementByteStride;
            SLANG_ASSERT(byteOffset <= std::numeric_limits<uint32_t>::max());
            StagedRecord::AggregateElementCopy copy;
            copy.sourceAddress = getValue(record.operands[size_t(elementIndex) + 2]).address;
            copy.destinationByteOffset = uint32_t(byteOffset);
            copy.byteSize = elementLayout.byteSize;
            elements.add(copy);
        }
        m_builder->emitMakeAggregate(
            record.opcode,
            resultAddress,
            layout.byteSize,
            elements.getArrayView());
        stageRetainValue(resultAddress, layout);
    }

    /// Emits one vector construction as a packed sequence of scalar source addresses.
    ///
    /// Consider `float3(x, y, z)`: the fixed prefix records `Float32` and three lanes, followed by
    /// the direct addresses of `x`, `y`, and `z`. Destination offsets are implicit scalar strides.
    void stageMakeVector(const HGBC::Record& record) { stageMakeNumeric(record, TypeKind::Vector); }

    /// Emits one matrix construction as a packed row-major sequence of scalar source addresses.
    ///
    /// Consider `float2x2(a, b, c, d)`: HGBC has already flattened its rows, so staging only
    /// preserves the four source addresses and does not interpret the source matrix-layout flag.
    void stageMakeMatrix(const HGBC::Record& record) { stageMakeNumeric(record, TypeKind::Matrix); }

    /// Emits the common packed-lane representation used by vector and matrix construction.
    ///
    /// The public staging functions keep the two source operations distinct, while this helper
    /// centralizes their identical execution record and record-size calculation.
    void stageMakeNumeric(const HGBC::Record& record, TypeKind expectedKind)
    {
        const TypeLayout& layout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(layout.kind == expectedKind);
        SLANG_ASSERT(record.operands.size() == size_t(layout.numericLayout.laneCount) + 2);
        List<Address> lanes;
        for (uint32_t i = 0; i < layout.numericLayout.laneCount; ++i)
            lanes.add(getValue(record.operands[size_t(i) + 2]).address);
        m_builder->emitMakeNumeric(
            record.opcode,
            getValue(HGBC::getResultId(record)).address,
            layout.numericLayout.scalarType,
            lanes.getArrayView());
    }

    /// Emits one positional parameter-transfer descriptor before the function body.
    ///
    /// For example, a value struct records its padded byte size, while `out Struct` records eight
    /// bytes because its allocated parameter slot contains only an absolute address token.
    void stageParameter(const HGBC::Record& record)
    {
        const StagedValueInfo& value = getValue(HGBC::getResultId(record));
        const TypeLayout& layout = getTypeLayout(record.operands[0]);
        m_builder->emitParameter(
            value.address,
            value.byteSize,
            {layout.managedLeafOffsets.data(), Index(layout.managedLeafOffsets.size())});
    }

    /// Stages one complete class allocation with the ownership map needed by its destructor.
    ///
    /// Consider `new Holder()` where two fields are managed handles: the staged record owns the
    /// allocation size/alignment and is followed by the two object-relative offsets. Execution can
    /// construct a self-contained `ClassObject` without looking up HGBC or staging-time layouts.
    void stageAllocateObject(const HGBC::Record& record)
    {
        const TypeLayout& layout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(
            layout.kind == TypeKind::HeapObject && layout.heapObjectKind == HeapObjectKind::Class);
        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        stageReleaseValue(resultAddress, layout);

        Address stagedAddress = m_builder->emitAllocateObject(
            resultAddress,
            layout.typeInfoAddress,
            layout.allocationByteSize,
            layout.allocationByteAlignment,
            layout.managedLeafOffsetsByteOffset,
            {layout.objectManagedLeafOffsets.data(),
             Index(layout.objectManagedLeafOffsets.size())});
        m_debugMapBuilder->setRecordType(
            uint32_t(stagedAddress.getUnsignedPayload()),
            record.operands[0]);
    }

    /// Emits owned-erasure operations while their concrete types and legal cases are available.
    /// The evaluator establishes replacement ownership before releasing any previous result.
    void stageOwnedErasure(const HGBC::Record& record)
    {
        HGBCId resultTypeId = record.operands[0];
        const auto& records = m_sourceProgram->getDecodedProgram().records;
        const auto& resultType =
            records[m_sourceProgram->getIndex().resultRecords.at(resultTypeId)];
        Address resultAddress = resultType.opcode == HGBCOpcode::VoidType
                                    ? Address{}
                                    : getValue(HGBC::getResultId(record)).address;
        Address stagedAddress;
        switch (record.opcode)
        {
        case HGBCOpcode::Box:
            {
                const TypeLayout& payloadLayout = getTypeLayout(record.operands[2]);
                stagedAddress = m_builder->emitBox(
                    resultAddress,
                    getValue(record.operands[3]).address,
                    payloadLayout.typeInfoAddress,
                    payloadLayout.byteSize,
                    payloadLayout.byteAlignment,
                    {payloadLayout.managedLeafOffsets.data(),
                     Index(payloadLayout.managedLeafOffsets.size())});
            }
            break;
        case HGBCOpcode::Unbox:
        case HGBCOpcode::ExtractTaggedUnion:
            {
                const TypeLayout* layout = resultType.opcode == HGBCOpcode::VoidType
                                               ? nullptr
                                               : &getTypeLayout(resultTypeId);
                stagedAddress = m_builder->emitUnbox(
                    record.opcode,
                    resultAddress,
                    getValue(record.operands[2]).address,
                    layout ? layout->typeInfoAddress : Address{},
                    layout ? layout->byteSize : 0,
                    record.opcode == HGBCOpcode::ExtractTaggedUnion ? record.operands[3] : 0,
                    layout ? makeConstArrayView(
                                 layout->managedLeafOffsets.data(),
                                 Index(layout->managedLeafOffsets.size()))
                           : ConstArrayView<uint32_t>{});
            }
            break;
        case HGBCOpcode::MakeTaggedUnion:
            {
                const auto* descriptor = m_sourceProgram->findTaggedUnionType(resultTypeId);
                SLANG_ASSERT(descriptor);
                List<StagedRecord::TaggedUnionCase> cases;
                for (const auto& item : descriptor->cases)
                {
                    const auto& payloadType =
                        records[m_sourceProgram->getIndex().resultRecords.at(item.payloadTypeId)];
                    cases.add(
                        {item.caseTag,
                         payloadType.opcode == HGBCOpcode::VoidType
                             ? Address{}
                             : getTypeLayout(item.payloadTypeId).typeInfoAddress});
                }
                stagedAddress = m_builder->emitMakeTaggedUnion(
                    resultAddress,
                    getValue(record.operands[2]).address,
                    getValue(record.operands[3]).address,
                    cases.getArrayView());
            }
            break;
        case HGBCOpcode::GetTaggedUnionCase:
        case HGBCOpcode::GetTaggedUnionPayload:
            stagedAddress = m_builder->emitTaggedUnionAccess(
                record.opcode,
                resultAddress,
                getValue(record.operands[2]).address);
            break;
        case HGBCOpcode::ConvertTaggedUnion:
            {
                const auto* descriptor = m_sourceProgram->findTaggedUnionType(resultTypeId);
                SLANG_ASSERT(descriptor);
                List<StagedRecord::TaggedUnionCaseMapping> mappings;
                for (size_t i = 3; i < record.operands.size(); i += 2)
                {
                    mappings.add(
                        {record.operands[i],
                         record.operands[i + 1],
                         uint32_t(descriptor->caseIndices.at(record.operands[i + 1]) + 1)});
                }
                stagedAddress = m_builder->emitConvertTaggedUnion(
                    resultAddress,
                    getValue(record.operands[2]).address,
                    mappings.getArrayView());
            }
            break;
        default:
            SLANG_UNREACHABLE("unexpected owned-erasure operation");
        }
        m_debugMapBuilder->setRecordType(
            uint32_t(stagedAddress.getUnsignedPayload()),
            record.opcode == HGBCOpcode::Box ? record.operands[2] : resultTypeId);
    }

    /// Stages one runtime-array allocation with the fixed layout of a single element.
    void stageAllocateRuntimeArray(const HGBC::Record& record)
    {
        const TypeLayout& layout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(
            layout.kind == TypeKind::HeapObject &&
            layout.heapObjectKind == HeapObjectKind::RuntimeArray);
        const StagedValueInfo& count = getValue(record.operands[2]);
        SLANG_ASSERT(
            count.kind == ValueKind::Scalar &&
            (count.scalarType == ScalarType::Int32 || count.scalarType == ScalarType::UInt32));
        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        stageReleaseValue(resultAddress, layout);

        Address stagedAddress = m_builder->emitAllocateRuntimeArray(
            resultAddress,
            count.address,
            layout.typeInfoAddress,
            layout.elementByteStride,
            layout.elementByteAlignment,
            count.scalarType,
            {layout.elementManagedLeafOffsets.data(),
             Index(layout.elementManagedLeafOffsets.size())});
        m_debugMapBuilder->setRecordType(
            uint32_t(stagedAddress.getUnsignedPayload()),
            record.operands[0]);
    }

    /// Stages an immutable count query for one runtime-array handle.
    void stageRuntimeArrayGetCount(const HGBC::Record& record)
    {
        const StagedValueInfo& array = getValue(record.operands[3]);
        const TypeLayout& layout = getTypeLayout(record.operands[2]);
        SLANG_ASSERT(
            array.kind == ValueKind::HeapObject && array.typeId == record.operands[2] &&
            layout.kind == TypeKind::HeapObject &&
            layout.heapObjectKind == HeapObjectKind::RuntimeArray);

        m_builder->emitRuntimeArrayGetCount(
            getValue(HGBC::getResultId(record)).address,
            array.address,
            layout.typeInfoAddress);
    }

    void stageCopyArrayRange(const HGBC::Record& record)
    {
        const TypeLayout& destinationLayout = getTypeLayout(record.operands[0]);
        const TypeLayout& sourceLayout = getTypeLayout(record.operands[1]);
        SLANG_ASSERT(
            destinationLayout.kind == TypeKind::HeapObject &&
            sourceLayout.kind == TypeKind::HeapObject);
        SLANG_ASSERT(
            (destinationLayout.heapObjectKind == HeapObjectKind::RuntimeArray ||
             destinationLayout.heapObjectKind == HeapObjectKind::StructuredBuffer) &&
            (sourceLayout.heapObjectKind == HeapObjectKind::RuntimeArray ||
             sourceLayout.heapObjectKind == HeapObjectKind::StructuredBuffer));

        const StagedValueInfo& destination = getValue(record.operands[3]);
        const StagedValueInfo& source = getValue(record.operands[5]);
        const StagedValueInfo& destinationOffset = getValue(record.operands[4]);
        const StagedValueInfo& sourceOffset = getValue(record.operands[6]);
        const StagedValueInfo& elementCount = getValue(record.operands[7]);
        SLANG_ASSERT(
            destination.typeId == record.operands[0] && source.typeId == record.operands[1] &&
            destinationOffset.scalarType == ScalarType::UInt32 &&
            sourceOffset.scalarType == ScalarType::UInt32 &&
            elementCount.scalarType == ScalarType::UInt32);

        Address destinationTypeAddress =
            destinationLayout.heapObjectKind == HeapObjectKind::RuntimeArray
                ? destinationLayout.typeInfoAddress
                : Address{};
        Address sourceTypeAddress = sourceLayout.heapObjectKind == HeapObjectKind::RuntimeArray
                                        ? sourceLayout.typeInfoAddress
                                        : Address{};
        m_builder->emitCopyArrayRange(
            getValue(record.operands[2]).address,
            destination.address,
            destinationOffset.address,
            source.address,
            sourceOffset.address,
            elementCount.address,
            destinationTypeAddress,
            sourceTypeAddress,
            destinationLayout.heapObjectKind,
            sourceLayout.heapObjectKind);
    }

    /// Emits one allocation record whose resource semantics come entirely from explicit HGBC
    /// operands.
    ///
    /// Consider `OpAllocateResource Texture Texture2D RGBA32Float ...`: staging translates the
    /// enums to RHI values and resolves the three extent slots. The result type supplies only the
    /// shader access and element width used by backend-specific safety checks; it does not select
    /// the resource kind, format, state, or usage. Upload buffers remain an HGVM capability gap;
    /// depth resources use the ordinary texture wrapper.
    void stageAllocateResource(const HGBC::Record& record)
    {
        HGBCResourceKind kind = HGBCResourceKind(record.operands[2]);
        HGBCResourceMemoryType memoryType = HGBCResourceMemoryType(record.operands[5]);
        if (memoryType != HGBCResourceMemoryType::DeviceLocal)
            fail("HGVM resource allocation currently supports only device-local resources");

        const TypeLayout& resultLayout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(resultLayout.kind == TypeKind::HeapObject);
        const TypeLayout& elementLayout = getTypeLayout(record.operands[7]);
        if (kind == HGBCResourceKind::Buffer && elementLayout.kind != TypeKind::Scalar &&
            elementLayout.kind != TypeKind::Vector && elementLayout.kind != TypeKind::Struct &&
            elementLayout.kind != TypeKind::Array)
        {
            fail("HGVM structured-buffer element type is not supported yet");
        }

        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        stageReleaseValue(resultAddress, resultLayout);

        HGBCTextureAccess textureAccess = HGBCTextureAccess::ReadWrite;
        rhi::BufferUsage bufferUsage = rhi::BufferUsage::None;
        rhi::TextureUsage textureUsage = rhi::TextureUsage::None;
        uint32_t textureElementByteSize = 0;
        if (kind == HGBCResourceKind::Texture)
        {
            auto typeIt = m_sourceProgram->getIndex().resultRecords.find(record.operands[0]);
            SLANG_ASSERT(typeIt != m_sourceProgram->getIndex().resultRecords.end());
            const HGBC::Record& textureType =
                m_sourceProgram->getDecodedProgram().records[typeIt->second];
            if (textureType.opcode == HGBCOpcode::Texture2DType)
            {
                SLANG_ASSERT(textureType.operands.size() == 4);
                textureAccess = HGBCTextureAccess(textureType.operands[2]);
                textureElementByteSize = elementLayout.byteSize;
            }
        }
        for (size_t operandIndex = 15; operandIndex < record.operands.size(); ++operandIndex)
        {
            HGBCResourceUsage usage = HGBCResourceUsage(record.operands[operandIndex]);
            if (kind == HGBCResourceKind::Buffer)
                bufferUsage |= getRHIBufferUsage(usage);
            else
                textureUsage |= getRHITextureUsage(usage);
        }
        Address stagedAddress = m_builder->emitAllocateResource(
            resultAddress,
            getValue(record.operands[8]).address,
            getValue(record.operands[9]).address,
            getValue(record.operands[10]).address,
            getValue(record.operands[14]).address,
            kind,
            HGBCResourceDimension(record.operands[3]),
            textureAccess,
            getRHIResourceFormat(HGBCResourceFormat(record.operands[4])),
            getRHIResourceMemoryType(memoryType),
            getRHIResourceState(HGBCResourceState(record.operands[6])),
            bufferUsage,
            textureUsage,
            record.operands[11],
            record.operands[12],
            record.operands[13],
            textureElementByteSize);
        m_debugMapBuilder->setRecordType(
            uint32_t(stagedAddress.getUnsignedPayload()),
            record.operands[0]);
    }

    /// Emits one buffer-element readback with an explicit eventual POD result extent.
    ///
    /// For example, both `readBack(buffer, 0)` returning `uint` and its `Defer<uint>` async form
    /// record four bytes. The async result slot is released before receiving the new deferred
    /// object's initial program reference. Resource-containing elements remain a capability error
    /// until GPU/interpreter aggregate conversion can construct managed host values.
    void stageReadBackBufferElement(const HGBC::Record& record)
    {
        const TypeLayout& operationResultLayout = getTypeLayout(record.operands[0]);
        const TypeLayout* valueLayout = nullptr;
        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        switch (record.opcode)
        {
        case HGBCOpcode::ReadBackBufferElement:
            valueLayout = &operationResultLayout;
            break;
        case HGBCOpcode::AsyncReadBackBufferElement:
            SLANG_ASSERT(
                operationResultLayout.kind == TypeKind::HeapObject &&
                operationResultLayout.heapObjectKind == HeapObjectKind::DeferredReadback);
            valueLayout = &getTypeLayout(operationResultLayout.valueTypeId);
            stageReleaseValue(resultAddress, operationResultLayout);
            break;
        default:
            SLANG_UNREACHABLE("non-readback operation reached readback staging");
        }
        if (!valueLayout->managedLeafOffsets.empty())
            fail("HGVM buffer readback does not support resource-containing elements yet");

        Address stagedAddress = m_builder->emitReadBackBufferElement(
            record.opcode,
            resultAddress,
            getValue(record.operands[3]).address,
            getValue(record.operands[4]).address,
            getValue(record.operands[5]).address,
            valueLayout->byteSize);
        if (record.opcode == HGBCOpcode::AsyncReadBackBufferElement)
            m_debugMapBuilder->setRecordType(
                uint32_t(stagedAddress.getUnsignedPayload()),
                record.operands[0]);
    }

    /// Emits a deferred wait whose result extent comes directly from its declared result type.
    ///
    /// Consider `uint value = pending.wait()`: the staged record names the managed `pending` slot
    /// and embeds four bytes. Runtime resolution consequently never visits `Defer<uint>` or
    /// `uint` type records.
    void stageWaitDefer(const HGBC::Record& record)
    {
        const TypeLayout& resultLayout = getTypeLayout(record.operands[0]);
        if (!resultLayout.managedLeafOffsets.empty())
            fail("HGVM deferred readback does not support resource-containing results yet");

        m_builder->emitWaitDefer(
            getValue(HGBC::getResultId(record)).address,
            getValue(record.operands[2]).address,
            resultLayout.byteSize);
    }

    /// Emits one buffer-dimensions query whose result is the validated `uint2` layout.
    void stageStructuredBufferDimensions(const HGBC::Record& record)
    {
        m_builder->emitStructuredBufferDimensions(
            getValue(HGBC::getResultId(record)).address,
            getValue(record.operands[3]).address);
    }

    /// Emits a texture-dimensions query using one already resolved texture slot.
    void stageTextureDimensions(const HGBC::Record& record)
    {
        m_builder->emitTextureDimensions(
            getValue(HGBC::getResultId(record)).address,
            getValue(record.operands[2]).address);
    }

    /// Emits one window construction after releasing a prior result-slot owner, if any.
    void stageCreateWindow(const HGBC::Record& record)
    {
        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        const TypeLayout& resultLayout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(
            resultLayout.kind == TypeKind::HeapObject &&
            resultLayout.heapObjectKind == HeapObjectKind::Window);
        stageReleaseValue(resultAddress, resultLayout);

        Address stagedAddress = m_builder->emitCreateWindow(
            resultAddress,
            getValue(record.operands[2]).address,
            getValue(record.operands[3]).address,
            getValue(record.operands[4]).address);
        m_debugMapBuilder->setRecordType(
            uint32_t(stagedAddress.getUnsignedPayload()),
            record.operands[0]);
    }

    /// Emits a process-events or close operation with one direct window location.
    void stageWindowOperation(const HGBC::Record& record)
    {
        Address resultAddress;
        if (record.opcode == HGBCOpcode::ProcessWindowEvents)
            resultAddress = getValue(HGBC::getResultId(record)).address;
        m_builder->emitWindowOperation(
            record.opcode,
            resultAddress,
            getValue(record.operands.back()).address);
    }

    /// Emits a window-title change with direct host-object and String locations.
    void stageSetWindowTitle(const HGBC::Record& record)
    {
        m_builder->emitSetWindowTitle(
            getValue(record.operands[0]).address,
            getValue(record.operands[1]).address);
    }

    /// Emits an input poll with the destination struct's six field offsets already flattened.
    void stagePollInputEvent(const HGBC::Record& record)
    {
        const TypeLayout& resultLayout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(resultLayout.kind == TypeKind::Struct && resultLayout.fields.size() == 6);
        uint32_t fieldByteOffsets[6];
        for (uint32_t fieldIndex = 0; fieldIndex < 6; ++fieldIndex)
            fieldByteOffsets[fieldIndex] = resultLayout.fields[fieldIndex].byteOffset;
        m_builder->emitPollInputEvent(
            getValue(HGBC::getResultId(record)).address,
            getValue(record.operands[2]).address,
            makeConstArrayView(fieldByteOffsets));
    }

    /// Emits surface creation with its logical format converted once to the RHI enum.
    void stageCreateSurface(const HGBC::Record& record)
    {
        const TypeLayout& resultLayout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(
            resultLayout.kind == TypeKind::HeapObject &&
            resultLayout.heapObjectKind == HeapObjectKind::Surface);
        auto typeIt = m_sourceProgram->getIndex().resultRecords.find(record.operands[0]);
        SLANG_ASSERT(typeIt != m_sourceProgram->getIndex().resultRecords.end());
        const HGBC::Record& surfaceType =
            m_sourceProgram->getDecodedProgram().records[typeIt->second];
        SLANG_ASSERT(surfaceType.opcode == HGBCOpcode::SurfaceType);

        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        stageReleaseValue(resultAddress, resultLayout);
        Address stagedAddress = m_builder->emitCreateSurface(
            resultAddress,
            getValue(record.operands[2]).address,
            getValue(record.operands[3]).address,
            getRHIResourceFormat(HGBCResourceFormat(surfaceType.operands[1])));
        m_debugMapBuilder->setRecordType(
            uint32_t(stagedAddress.getUnsignedPayload()),
            record.operands[0]);
    }

    /// Emits acquisition or presentation with one direct surface location.
    void stageSurfaceOperation(const HGBC::Record& record)
    {
        Address resultAddress;
        if (record.opcode != HGBCOpcode::PresentSurface)
        {
            resultAddress = getValue(HGBC::getResultId(record)).address;
            stageReleaseValue(resultAddress, getTypeLayout(record.operands[0]));
        }
        Address stagedAddress = m_builder->emitSurfaceOperation(
            record.opcode,
            resultAddress,
            getValue(record.operands.back()).address);
        if (record.opcode == HGBCOpcode::AcquireSurfaceTexture)
            m_debugMapBuilder->setRecordType(
                uint32_t(stagedAddress.getUnsignedPayload()),
                record.operands[0]);
    }

    /// Uses the same indirect owning-slot cleanup as a store before acquiring another image.
    void stageTryAcquireSurfaceTexture(const HGBC::Record& record)
    {
        const StagedValueInfo& destination = getValue(record.operands[3]);
        SLANG_ASSERT(destination.kind == ValueKind::Address);
        stageReleaseValue(destination.address, getTypeLayout(destination.typeId), true);
        m_builder->emitTryAcquireSurfaceTexture(
            getValue(HGBC::getResultId(record)).address,
            getValue(record.operands[2]).address,
            destination.address);
    }

    /// Preserves explicit resize dimensions for the host without querying window state in HGVM.
    void stageResizeSurface(const HGBC::Record& record)
    {
        m_builder->emitResizeSurface(
            getValue(HGBC::getResultId(record)).address,
            getValue(record.operands[2]).address,
            getValue(record.operands[3]).address,
            getValue(record.operands[4]).address);
    }

    /// Returns the shader scalar family declared by one validated texture type.
    TextureScalarClass getTextureScalarClass(HGBCId textureTypeId)
    {
        auto typeIt = m_sourceProgram->getIndex().resultRecords.find(textureTypeId);
        SLANG_ASSERT(typeIt != m_sourceProgram->getIndex().resultRecords.end());
        const HGBC::Record& textureType =
            m_sourceProgram->getDecodedProgram().records[typeIt->second];
        SLANG_ASSERT(
            (textureType.opcode == HGBCOpcode::Texture2DType && textureType.operands.size() == 4) ||
            (textureType.opcode == HGBCOpcode::RenderTarget2DType &&
             textureType.operands.size() == 3));
        ScalarType scalarType = getTypeLayout(textureType.operands[1]).numericLayout.scalarType;
        switch (scalarType)
        {
        case ScalarType::Float4E2M1:
        case ScalarType::Float8E4M3:
        case ScalarType::Float8E5M2:
        case ScalarType::Float16:
        case ScalarType::BFloat16:
        case ScalarType::Float32:
        case ScalarType::Float64:
            return TextureScalarClass::Float;
        case ScalarType::Int4:
        case ScalarType::Int8:
        case ScalarType::Int16:
        case ScalarType::Int32:
        case ScalarType::Int64:
            return TextureScalarClass::Int;
        case ScalarType::UInt4:
        case ScalarType::UInt8:
        case ScalarType::UInt16:
        case ScalarType::UInt32:
        case ScalarType::UInt64:
            return TextureScalarClass::UInt;
        case ScalarType::Bool:
            break;
        }
        SLANG_UNREACHABLE("validated texture has an unsupported blit scalar type");
    }

    /// Emits a complete-texture blit after consuming its explicit endpoint types at staging time.
    void stageBlitTexture(const HGBC::Record& record)
    {
        m_builder->emitBlitTexture(
            getValue(record.operands[2]).address,
            getValue(record.operands[3]).address,
            getTextureScalarClass(record.operands[0]),
            getTextureScalarClass(record.operands[1]));
    }

    /// Resolves clear types and generic aspect selection once; mip/layer bounds stay dynamic.
    /// A combined depth/stencil clear carries separate float and uint addresses, avoiding any
    /// dependence on the host layout of the library's DepthStencilValue struct.
    void stageClearTexture(const HGBC::Record& record)
    {
        using O = HGBCClearTextureOperands;
        const auto& textureType = m_sourceProgram->getDecodedProgram()
                                      .records[m_sourceProgram->getIndex().resultRecords.at(
                                          record.operands[O::textureType])];
        HGBCResourceFormat format = HGBCResourceFormat(textureType.operands.back());
        uint32_t valueByteSize =
            isHGBCDepthFormat(format) ? 4 : getTypeLayout(textureType.operands[1]).byteSize;
        SLANG_RELEASE_ASSERT(valueByteSize <= 16);
        Address stencilAddress;
        if (record.operands.size() == 9)
            stencilAddress = getValue(record.operands[8]).address;
        m_builder->emitClearTexture(
            getValue(record.operands[O::texture]).address,
            getValue(record.operands[O::clearValue]).address,
            stencilAddress,
            getValue(record.operands[O::mip]).address,
            getValue(record.operands[O::mipCount]).address,
            getValue(record.operands[O::layer]).address,
            getValue(record.operands[O::layerCount]).address,
            HGBCTextureAspect(record.operands[O::aspect]),
            format,
            valueByteSize,
            getTextureScalarClass(record.operands[O::textureType]));
    }

    /// Emits one file-stream construction with no runtime access-mode value lookup.
    void stageByteStreamFromFile(const HGBC::Record& record)
    {
        Address resultAddress = getValue(HGBC::getResultId(record)).address;
        const TypeLayout& resultLayout = getTypeLayout(record.operands[0]);
        SLANG_ASSERT(
            resultLayout.kind == TypeKind::HeapObject &&
            resultLayout.heapObjectKind == HeapObjectKind::ByteStream);
        stageReleaseValue(resultAddress, resultLayout);

        Address stagedAddress = m_builder->emitByteStreamFromFile(
            resultAddress,
            HGBCByteStreamAccess(record.operands[2]),
            getValue(record.operands[3]).address);
        m_debugMapBuilder->setRecordType(
            uint32_t(stagedAddress.getUnsignedPayload()),
            record.operands[0]);
    }

    /// Emits close or size-query records using the same direct stream value address.
    void stageByteStreamOperation(const HGBC::Record& record)
    {
        Address resultAddress;
        auto result = m_values.find(HGBC::getResultId(record));
        if (result != m_values.end())
            resultAddress = result->second.address;
        m_builder->emitCloseByteStream(
            record.opcode,
            resultAddress,
            getValue(record.operands.back()).address);
    }

    /// Emits one raw stream transfer with the selected endpoint representation and host extent.
    void stageByteStreamTransfer(const HGBC::Record& record)
    {
        HGBCByteStreamEndpointKind endpointKind = HGBCByteStreamEndpointKind(record.operands[2]);
        uint32_t hostByteExtent = 0;
        Address runtimeArrayTypeAddress;
        const StagedValueInfo& endpoint = getValue(record.operands[4]);
        if (endpointKind == HGBCByteStreamEndpointKind::HostAddress)
        {
            SLANG_ASSERT(endpoint.kind == ValueKind::Address);
            hostByteExtent = getTypeLayout(endpoint.typeId).byteSize;
        }
        else if (endpointKind == HGBCByteStreamEndpointKind::RuntimeArray)
        {
            const TypeLayout& layout = getTypeLayout(endpoint.typeId);
            SLANG_ASSERT(
                endpoint.kind == ValueKind::HeapObject &&
                layout.heapObjectKind == HeapObjectKind::RuntimeArray);
            runtimeArrayTypeAddress = layout.typeInfoAddress;
        }
        m_builder->emitByteStreamTransfer(
            record.opcode,
            getValue(HGBC::getResultId(record)).address,
            endpointKind,
            getValue(record.operands[3]).address,
            endpoint.address,
            getValue(record.operands[5]).address,
            getValue(record.operands[6]).address,
            hostByteExtent,
            runtimeArrayTypeAddress);
    }

    /// Emits one general image transform and normalizes encode/decode operand direction.
    void stageImageOperation(const HGBC::Record& record)
    {
        bool isDecode = record.opcode == HGBCOpcode::ImageDecode;
        m_builder->emitImageOperation(
            record.opcode,
            getValue(HGBC::getResultId(record)).address,
            HGBCImageCodec(record.operands[2]),
            HGBCEncodedImageEndpointKind(record.operands[isDecode ? 3 : 4]),
            HGBCPixelImageEndpointKind(record.operands[isDecode ? 4 : 3]),
            getValue(record.operands[isDecode ? 5 : 6]).address,
            getValue(record.operands[isDecode ? 6 : 5]).address);
    }

    /// Emits a dispatcher-shaped `OpCall` with pre-resolved argument type descriptors.
    ///
    /// Consider `dispatcher(groups, buffer, value)`: the group count is stored in the fixed prefix,
    /// while trailing binding records let execution bind both arguments without HGBC ID lookups.
    void stageMakeGraphicsDraw(const HGBC::Record& record)
    {
        Address indices;
        if (record.operands.size() == 8)
            indices = getValue(record.operands[7]).address;
        m_builder->emitMakeGraphicsDraw(
            getValue(HGBC::getResultId(record)).address,
            getValue(record.operands[2]).address,
            getValue(record.operands[3]).address,
            getValue(record.operands[4]).address,
            getValue(record.operands[5]).address,
            getValue(record.operands[6]).address,
            indices);
    }

    void stageMakeGraphicsDrawParameters(const HGBC::Record& record)
    {
        Address operands[13];
        for (size_t i = 0; i < 13; ++i)
            operands[i] = getValue(record.operands[2 + i]).address;
        m_builder->emitMakeGraphicsDrawParameters(
            getValue(HGBC::getResultId(record)).address,
            makeConstArrayView(operands));
    }

    void stageGraphicsCall(const HGBC::Record& record, const HGBC::Record& pipeline)
    {
        uint32_t drawCount = pipeline.operands[6];
        uint32_t argumentCount = uint32_t(pipeline.operands.size() - 8);
        Address depthAddress;
        if (drawCount == 4)
            depthAddress = getValue(record.operands[4]).address;
        List<StagedRecord::DispatchArgument> arguments;
        for (uint32_t i = 0; i < argumentCount; ++i)
        {
            StagedRecord::DispatchArgument argument;
            argument.valueAddress = getValue(record.operands[3 + drawCount + i]).address;
            argument.converterAddress = getValue(pipeline.operands[8 + i]).address;
            arguments.add(argument);
        }
        m_builder->emitGraphicsCall(
            getValue(record.operands[2]).address,
            getValue(record.operands[3]).address,
            depthAddress,
            getValue(record.operands[3 + drawCount - 2]).address,
            getValue(record.operands[3 + drawCount - 1]).address,
            arguments.getArrayView());
    }

    void stageComputeCall(const HGBC::Record& record)
    {
        size_t argumentCount = record.operands.size() - 4;
        auto dispatcherRecordIt =
            m_sourceProgram->getIndex().resultRecords.find(record.operands[2]);
        SLANG_ASSERT(dispatcherRecordIt != m_sourceProgram->getIndex().resultRecords.end());
        const HGBC::Record& dispatcherRecord =
            m_sourceProgram->getDecodedProgram().records[dispatcherRecordIt->second];
        SLANG_ASSERT(dispatcherRecord.opcode == HGBCOpcode::MakeDispatcher);
        SLANG_ASSERT(dispatcherRecord.operands.size() == argumentCount + 4);

        List<StagedRecord::DispatchArgument> arguments;

        for (size_t argumentIndex = 0; argumentIndex < argumentCount; ++argumentIndex)
        {
            const StagedValueInfo& value = getValue(record.operands[argumentIndex + 4]);
            StagedRecord::DispatchArgument argument;
            argument.valueAddress = value.address;
            argument.converterAddress =
                getValue(dispatcherRecord.operands[argumentIndex + 4]).address;
            arguments.add(argument);
        }
        m_builder->emitComputeCall(
            getValue(record.operands[2]).address,
            getValue(record.operands[3]).address,
            arguments.getArrayView());
    }

    /// Emits one direct call prefix followed by unclassified argument source addresses.
    ///
    /// Consider `update(value, address)`: both trailing words name caller slots. The matching
    /// parameter records decide to copy the value's bytes or the address's eight bytes, so the call
    /// record needs no per-argument kind information.
    void stageCall(const HGBC::Record& record)
    {
        HGBCId targetFunctionId = record.operands[2];
        if (!m_sourceProgram->getIndex().functionIndices.contains(targetFunctionId))
        {
            const StagedValueInfo& callee = getValue(targetFunctionId);
            if (callee.kind == ValueKind::ConstObject &&
                callee.address.getKind() == AddressKind::ConstHeap)
            {
                const auto& index = m_sourceProgram->getIndex();
                const auto& producer = m_sourceProgram->getDecodedProgram()
                                           .records[index.resultRecords.at(targetFunctionId)];
                if (producer.opcode == HGBCOpcode::MakeGraphicsPipeline)
                    stageGraphicsCall(record, producer);
                else
                    stageComputeCall(record);
                return;
            }
            fail("HGVM calls support direct HGVM functions and compute dispatchers only");
        }

        List<Address> arguments;
        for (size_t i = 3; i < record.operands.size(); ++i)
            arguments.add(getValue(record.operands[i]).address);
        Address resultAddress;
        auto result = m_values.find(HGBC::getResultId(record));
        if (result != m_values.end())
            resultAddress = result->second.address;
        Address callAddress = m_builder->emitCall(resultAddress, arguments.getArrayView());
        m_callFixups.push_back({uint32_t(callAddress.getUnsignedPayload()), targetFunctionId});
    }

    /// Lowers a return value to a sized raw store followed by an ordinary void-shaped `OpReturn`.
    ///
    /// For example, returning a 16-byte struct first stores those bytes through the absolute return
    /// destination saved in the frame header. The following return then enters the shared exit
    /// block, exactly like a void return.
    void stageReturn(const HGBC::Record& record)
    {
        if (!record.operands.empty())
        {
            const StagedValueInfo& value = getValue(record.operands[0]);
            const TypeLayout& layout = getTypeLayout(value.typeId);
            Address returnDestinationAddress = Address::makeRelativeFrame(
                uint32_t(offsetof(FrameHeader, returnDestinationAddress)));
            stageReleaseValue(returnDestinationAddress, layout, true);
            Address storeRecordPointerAddress = {};
            Address storeRecordValueAddress = {};
            uint32_t storeRecordByteSize = 0;

            storeRecordPointerAddress = returnDestinationAddress;
            storeRecordValueAddress = value.address;
            storeRecordByteSize = value.byteSize;
            m_builder->emitStore(
                storeRecordPointerAddress,
                storeRecordValueAddress,
                storeRecordByteSize);
            stageRetainValue(returnDestinationAddress, layout, true);
        }

        m_builder->emitReturn();
    }

    /// Emits an unconditional branch containing only its direct target program counter.
    ///
    /// Consider `OpBranch %loop`: the block ID is retained in a staging-only fixup and replaced by
    /// `%loop`'s direct PC after every block has been emitted.
    void stageBranch(const HGBC::Record& record)
    {
        Address address = m_builder->emitBranch();
        m_branchFixups.push_back({uint32_t(address.getUnsignedPayload()), record.operands[0]});
    }

    /// Emits a conditional branch with two direct-PC fields to patch after block emission.
    ///
    /// For example, a source entry block can target forward `%then` and `%else` blocks while its
    /// condition is already a direct frame or literal address.
    void stageConditionalBranch(const HGBC::Record& record)
    {
        Address address = m_builder->emitBranchConditional(getValue(record.operands[0]).address);
        m_conditionalFixups.push_back(
            {uint32_t(address.getUnsignedPayload()), record.operands[1], record.operands[2]});
    }

    /// Replaces all validated block IDs with direct staged body program counters.
    ///
    /// For example, both a backward loop edge and a forward merge edge become one PC assignment in
    /// the hot interpreter; the temporary maps and source IDs are discarded with this stager.
    void patchBranches()
    {
        for (BranchFixup& fixup : m_branchFixups)
        {
            auto target = m_blockProgramCounters.find(fixup.targetBlockId);
            SLANG_RELEASE_ASSERT(target != m_blockProgramCounters.end());
            m_builder->patchBranchTarget(
                Address::makeAbsoluteInstruction(fixup.programCounter),
                target->second);
        }
        for (ConditionalFixup& fixup : m_conditionalFixups)
        {
            auto trueTarget = m_blockProgramCounters.find(fixup.trueBlockId);
            auto falseTarget = m_blockProgramCounters.find(fixup.falseBlockId);
            SLANG_RELEASE_ASSERT(trueTarget != m_blockProgramCounters.end());
            SLANG_RELEASE_ASSERT(falseTarget != m_blockProgramCounters.end());
            m_builder->patchConditionalTargets(
                Address::makeAbsoluteInstruction(fixup.programCounter),
                trueTarget->second,
                falseTarget->second);
        }
    }

    /// Replaces every direct HGBC callee ID with its staged function-record address.
    ///
    /// For example, a forward call from `main` to a helper emitted later becomes one immutable
    /// absolute-instruction address after all function ranges have been assigned.
    void patchCalls()
    {
        for (CallFixup& fixup : m_callFixups)
        {
            auto target = m_program->functionAddresses.find(fixup.targetFunctionId);
            SLANG_ASSERT(target != m_program->functionAddresses.end());
            m_builder->patchCallTarget(
                Address::makeAbsoluteInstruction(fixup.programCounter),
                target->second);
        }
    }

    /// Stages every validated block of one function and appends its mandatory exit block.
    ///
    /// Consider an if/else followed by a merge: each source block is emitted once, its terminator
    /// transfers control by direct PC, and every `OpReturn` jumps to the one generated cleanup
    /// exit.
    void stageFunction(const HGBC::Function& function)
    {
        analyzeFunction(function);
        m_debugMapBuilder->beginFunction(function.id, m_builder->getWordOffset());

        Address functionAddress = m_builder->emitFunction(
            m_frameLayout.getFrameSize(),
            m_frameLayout.getFrameAlignment());
        m_program->functionAddresses[function.id] = functionAddress;
        if (m_preserveDebugScopes)
        {
            List<Address> ownedSlots;
            for (const auto& slot : m_owningSlots)
                for (auto offset : getTypeLayout(slot.typeId).managedLeafOffsets)
                    ownedSlots.add(slot.address.addByteOffset(offset));
            m_program->debugOwnedSlots.add(
                uint32_t(functionAddress.getUnsignedPayload()),
                std::move(ownedSlots));
        }
        uint32_t parameterCount = 0;

        uint32_t firstParameterProgramCounter = m_builder->getWordOffset();
        for (size_t recordIndex : function.blocks.front().recordIndices)
        {
            const HGBC::Record& record = m_sourceProgram->getDecodedProgram().records[recordIndex];
            if (record.opcode != HGBCOpcode::Param)
                continue;
            stageParameter(record);
            ++parameterCount;
        }

        for (const HGBC::Block& block : function.blocks)
        {
            m_blockProgramCounters.emplace(block.id, m_builder->getWordOffset());
            m_debugMapBuilder->beginBlock(m_builder->getWordOffset());
            for (size_t recordIndex : block.recordIndices)
            {
                const HGBC::Record& record =
                    m_sourceProgram->getDecodedProgram().records[recordIndex];
                if (hasHGBCOpcodeFlag(record.opcode, HGBCOpcodeFlags::Debug))
                {
                    m_debugMapBuilder->processDebugRecord(record, m_builder->getWordOffset());
                    if (m_preserveDebugScopes && (record.opcode == HGBCOpcode::DebugScope ||
                                                  record.opcode == HGBCOpcode::DebugNoScope))
                    {
                        uint32_t scopeIndex = m_debugMapBuilder->getScopeIndex();
                        const auto& scope = m_debugMapBuilder->getDebugMap()->getScope(scopeIndex);
                        m_builder->emitDebugScope(
                            scopeIndex,
                            uint32_t(scope.inlineSites.getCount()));
                    }
                    if (m_preserveDebugScopes && record.opcode == HGBCOpcode::DebugVariable)
                        m_debugMapBuilder->addVariable(
                            record,
                            uint32_t(m_debugVariables.at(record.operands[0])
                                         .address.getUnsignedPayload()),
                            m_builder->getWordOffset());
                    if (m_preserveDebugScopes && record.opcode == HGBCOpcode::DebugValue)
                        stageDebugValue(record);
                    continue;
                }

                uint32_t instructionStart = m_builder->getWordOffset();
                switch (record.opcode)
                {
                case HGBCOpcode::Add:
                case HGBCOpcode::Sub:
                case HGBCOpcode::Mul:
                case HGBCOpcode::IDiv:
                case HGBCOpcode::IRem:
                case HGBCOpcode::Lsh:
                case HGBCOpcode::FDiv:
                case HGBCOpcode::Pow:
                case HGBCOpcode::Atan2:
                case HGBCOpcode::Min:
                case HGBCOpcode::Max:
                case HGBCOpcode::Eql:
                case HGBCOpcode::Neq:
                case HGBCOpcode::Less:
                case HGBCOpcode::Greater:
                case HGBCOpcode::Leq:
                case HGBCOpcode::Geq:
                    stageTypedBinaryOp(record);
                    break;
                case HGBCOpcode::Neg:
                case HGBCOpcode::Not:
                case HGBCOpcode::Sin:
                case HGBCOpcode::Cos:
                case HGBCOpcode::Tan:
                case HGBCOpcode::Asin:
                case HGBCOpcode::Sqrt:
                case HGBCOpcode::Ceil:
                case HGBCOpcode::Floor:
                case HGBCOpcode::Exp:
                case HGBCOpcode::Exp2:
                case HGBCOpcode::Log:
                case HGBCOpcode::Log2:
                    stageUnary(record);
                    break;
                case HGBCOpcode::Any:
                    stageAny(record);
                    break;
                case HGBCOpcode::ScalarTypeCast:
                    stageScalarTypeCast(record);
                    break;
                case HGBCOpcode::Param:
                    break;
                case HGBCOpcode::Var:
                    stageVar(record);
                    break;
                case HGBCOpcode::Load:
                    stageLoad(record);
                    break;
                case HGBCOpcode::ResourceToDescriptorHandle:
                    stageResourceToDescriptorHandle(record);
                    break;
                case HGBCOpcode::DescriptorHandleToResource:
                    stageDescriptorHandleToResource(record);
                    break;
                case HGBCOpcode::Store:
                    stageStore(record);
                    break;
                case HGBCOpcode::FieldAddress:
                    stageFieldAddress(record);
                    break;
                case HGBCOpcode::ElementAddress:
                    stageElementAddress(record);
                    break;
                case HGBCOpcode::FieldExtract:
                    stageFieldExtract(record);
                    break;
                case HGBCOpcode::ExtractElement:
                    stageElementExtract(record);
                    break;
                case HGBCOpcode::MakeStruct:
                    stageMakeStruct(record);
                    break;
                case HGBCOpcode::MakeArray:
                    stageMakeArray(record);
                    break;
                case HGBCOpcode::MakeVector:
                    stageMakeVector(record);
                    break;
                case HGBCOpcode::MakeMatrix:
                    stageMakeMatrix(record);
                    break;
                case HGBCOpcode::Call:
                    stageCall(record);
                    break;
                case HGBCOpcode::FormatString:
                    stageFormatString(record);
                    break;
                case HGBCOpcode::Print:
                    stagePrint(record);
                    break;
                case HGBCOpcode::Fail:
                    stageStringOperation(record, 0);
                    break;
                case HGBCOpcode::StringGetByteCount:
                    stageStringOperation(record, 2);
                    break;
                case HGBCOpcode::StringReadByte:
                    stageStringReadByte(record);
                    break;
                case HGBCOpcode::CreateTimer:
                case HGBCOpcode::TimerElapsed:
                case HGBCOpcode::ResetTimer:
                    stageTimerOperation(record);
                    break;
                case HGBCOpcode::MakeRaytracingPipeline:
                case HGBCOpcode::SetRayMiss:
                case HGBCOpcode::SetRayHitGroup:
                case HGBCOpcode::DispatchRays:
                    stageRayShaderOperation(record);
                    break;
                case HGBCOpcode::AllocAccelerationStructure:
                case HGBCOpcode::BuildAccelerationStructure:
                    stageRayAccelerationStructure(record);
                    break;
                case HGBCOpcode::Branch:
                    stageBranch(record);
                    break;
                case HGBCOpcode::BranchConditional:
                    stageConditionalBranch(record);
                    break;
                case HGBCOpcode::Return:
                    stageReturn(record);
                    break;
                case HGBCOpcode::AllocateObject:
                    stageAllocateObject(record);
                    break;
                case HGBCOpcode::Box:
                case HGBCOpcode::Unbox:
                case HGBCOpcode::MakeTaggedUnion:
                case HGBCOpcode::GetTaggedUnionCase:
                case HGBCOpcode::GetTaggedUnionPayload:
                case HGBCOpcode::ExtractTaggedUnion:
                case HGBCOpcode::ConvertTaggedUnion:
                    stageOwnedErasure(record);
                    break;
                case HGBCOpcode::AllocateRuntimeArray:
                    stageAllocateRuntimeArray(record);
                    break;
                case HGBCOpcode::RuntimeArrayGetCount:
                    stageRuntimeArrayGetCount(record);
                    break;
                case HGBCOpcode::CopyArrayRange:
                    stageCopyArrayRange(record);
                    break;
                case HGBCOpcode::AllocateResource:
                    stageAllocateResource(record);
                    break;
                case HGBCOpcode::MakeGraphicsDraw:
                    stageMakeGraphicsDraw(record);
                    break;
                case HGBCOpcode::MakeGraphicsDrawParameters:
                    stageMakeGraphicsDrawParameters(record);
                    break;
                case HGBCOpcode::ReadBackBufferElement:
                case HGBCOpcode::AsyncReadBackBufferElement:
                    stageReadBackBufferElement(record);
                    break;
                case HGBCOpcode::WaitDefer:
                    stageWaitDefer(record);
                    break;
                case HGBCOpcode::StructuredBufferGetDimensions:
                    stageStructuredBufferDimensions(record);
                    break;
                case HGBCOpcode::TextureGetDimensions:
                    stageTextureDimensions(record);
                    break;
                case HGBCOpcode::BlitTexture2D:
                    stageBlitTexture(record);
                    break;
                case HGBCOpcode::ClearTexture:
                    stageClearTexture(record);
                    break;
                case HGBCOpcode::CreateWindow:
                    stageCreateWindow(record);
                    break;
                case HGBCOpcode::ProcessWindowEvents:
                case HGBCOpcode::CloseWindow:
                    stageWindowOperation(record);
                    break;
                case HGBCOpcode::PollInputEvent:
                    stagePollInputEvent(record);
                    break;
                case HGBCOpcode::SetWindowTitle:
                    stageSetWindowTitle(record);
                    break;
                case HGBCOpcode::CreateSurface:
                    stageCreateSurface(record);
                    break;
                case HGBCOpcode::AcquireSurfaceTexture:
                case HGBCOpcode::PresentSurface:
                case HGBCOpcode::TryPresentSurface:
                    stageSurfaceOperation(record);
                    break;
                case HGBCOpcode::TryAcquireSurfaceTexture:
                    stageTryAcquireSurfaceTexture(record);
                    break;
                case HGBCOpcode::ResizeSurface:
                    stageResizeSurface(record);
                    break;
                case HGBCOpcode::ByteStreamFromFile:
                    stageByteStreamFromFile(record);
                    break;
                case HGBCOpcode::CloseByteStream:
                case HGBCOpcode::ByteStreamGetSize:
                    stageByteStreamOperation(record);
                    break;
                case HGBCOpcode::ByteStreamRead:
                case HGBCOpcode::ByteStreamWrite:
                    stageByteStreamTransfer(record);
                    break;
                case HGBCOpcode::ImageDecode:
                case HGBCOpcode::ImageEncode:
                    stageImageOperation(record);
                    break;
                default:
                    SLANG_UNREACHABLE("unsupported operation survived HGVM function analysis");
                }
                m_builder->markInstructionContinuations(instructionStart);
            }
        }

        patchBranches();
        uint32_t entryProgramCounter =
            m_blockProgramCounters.find(function.blocks.front().id)->second;
        uint32_t exitProgramCounter = m_builder->getWordOffset();
        m_debugMapBuilder->endFunction(entryProgramCounter, exitProgramCounter);
        m_debugMapBuilder->clearSourceLocation(exitProgramCounter);
        for (const OwningSlot& slot : m_owningSlots)
            stageReleaseValue(slot.address, getTypeLayout(slot.typeId));
        m_builder->emitFinishFrameExit();
        m_builder->patchFunction(
            functionAddress,
            entryProgramCounter,
            exitProgramCounter,
            firstParameterProgramCounter,
            parameterCount);
    }
};

struct InterpreterState;

void copyScalarFromDevice(
    ScalarType scalarType,
    const uint8_t* source,
    uint32_t sourceByteSize,
    uint8_t* destination,
    uint32_t destinationByteSize);

void copyScalarForBinding(
    ScalarType scalarType,
    const uint8_t* source,
    uint32_t sourceByteSize,
    uint8_t* destination,
    uint32_t destinationByteSize);

/// Owns the persistent RHI-facing state and operations shared by HGVM invocations.
///
/// Consider a compute call that binds a buffer and a texture: `InterpreterState` resolves their
/// logical addresses into the wrapper pointers below, then this object reflects parameters, records
/// commands, and retains RHI resources until submission completion. Keeping that work here leaves
/// the interpreter core responsible only for staged traversal and address access.
struct DeviceState
{
    /// Owns one RHI structured buffer and the metadata required by device operations.
    ///
    /// Consider `RWStructuredBuffer<uint>::alloc(4)`: the program's heap `Address` points to this
    /// wrapper, while `resource` owns the backend allocation and dimensions/readback use the cached
    /// element count and stride without consulting HGBC.
    struct StructuredBufferObject : HeapObject
    {
        static constexpr HeapObjectKind kKind = HeapObjectKind::StructuredBuffer;

        StructuredBufferObject()
            : HeapObject(kKind)
        {
        }

        ComPtr<rhi::IBuffer> resource;
        RefPtr<BufferPool> pool;
        rhi::BufferDesc allocationDesc = {};

        ~StructuredBufferObject()
        {
            if (pool)
                pool->recycle(allocationDesc, std::move(resource));
        }
        uint64_t elementCount = 0;
        uint32_t elementStride = 0;
        // Borrows the allocating program instance's resolved layout. The VM retains programs
        // until close and releases external values and submitted work before those programs.
        TypeLayoutObject* elementLayout = nullptr;
    };

    /// Owns one writable 2D texture used by host operations and shader dispatches.
    ///
    /// Consider `allocateTexture2D<float4>(64, 32)`: the program's heap `Address` points to this
    /// wrapper and copies retain it, while `resource` owns the single RHI texture allocation.
    struct Texture2DObject : HeapObject
    {
        static constexpr HeapObjectKind kKind = HeapObjectKind::Texture2D;

        Texture2DObject()
            : HeapObject(kKind)
        {
        }

        ComPtr<rhi::ITexture> resource;
        // Writable views name mip zero; sampled views retain the complete mip chain.
        ComPtr<rhi::ITextureView> readDescriptorView;
        ComPtr<rhi::ITextureView> writeDescriptorView;
    };

    /// Keeps logical scene topology independent of the tracker used to trace it.
    /// TLAS builds snapshot native BLAS versions; rebuilding a BLAS does not mutate old scenes.
    struct RayAccelerationStructureObject : HeapObject
    {
        static constexpr HeapObjectKind kKind = HeapObjectKind::RayAccelerationStructure;
        explicit RayAccelerationStructureObject(bool isTopLevel)
            : HeapObject(kKind), topLevel(isTopLevel)
        {
        }
        bool topLevel;
        uint64_t version = 0;
        uint32_t geometryCount = 0;
        List<HGBCRayGeometryKind> geometryKinds;
        uint32_t buildFlags = 0;
        ComPtr<rhi::IAccelerationStructure> resource;
        List<ComPtr<rhi::IAccelerationStructure>> bottomLevels;
        List<rhi::AccelerationStructureInstanceDescGeneric> instances;
        Dictionary<uint32_t, ComPtr<rhi::IAccelerationStructure>> realizations;
    };

    /// Describes whether a fixed-size readback-pool slot can accept another device copy.
    ///
    /// Consider a `Defer<uint>` that is dropped without being waited: its slot changes from
    /// `Leased` to `AwaitingCompletion`. The pool can reuse it after the copy's fence completes,
    /// but never while the device may still be writing into it.
    enum class ReadbackBufferState : uint8_t
    {
        Available,
        Leased,
        AwaitingCompletion,
    };

    /// Owns one reusable 64-byte device-to-host transfer allocation.
    ///
    /// For example, two simultaneously pending `Defer<uint>` values lease two slots. After both
    /// are waited, a third readback reuses either allocation instead of calling `createBuffer`.
    struct ReadbackBufferSlot
    {
        ComPtr<rhi::IBuffer> resource;
        ReadbackBufferState state = ReadbackBufferState::Available;
        uint64_t completionFenceValue = 0;
    };

    /// Retains the RHI objects referenced by one encoded or submitted command batch.
    ///
    /// Consider two dispatches that use the same buffer after its last program reference is
    /// overwritten: each binding retains its wrapper here, so dropping the program-side reference
    /// cannot recycle a buffer that the device has not finished using. Duplicate holds
    /// are intentional; appending a reference is cheaper than a hash lookup on every binding.
    struct SubmissionResourceHolds
    {
        // Retain the lease, not just IBuffer: recycling must wait for GPU completion as well as
        // the last program alias. Raw holds below are reserved for internal non-pooled buffers.
        List<RefPtr<StructuredBufferObject>> bufferLeases;
        List<ComPtr<rhi::IBuffer>> buffers;
        List<ComPtr<rhi::ITexture>> textures;
        List<ComPtr<rhi::ITextureView>> textureViews;
        List<ComPtr<rhi::IComputePipeline>> computePipelines;
        List<ComPtr<rhi::IRenderPipeline>> renderPipelines;
        List<ComPtr<rhi::IRayTracingPipeline>> rayPipelines;
        List<ComPtr<rhi::IShaderTable>> shaderTables;
        List<ComPtr<rhi::IAccelerationStructure>> accelerationStructures;

        /// Exchanges all retained objects without incrementing their COM reference counts.
        void swapWith(SubmissionResourceHolds& other)
        {
            buffers.swapWith(other.buffers);
            bufferLeases.swapWith(other.bufferLeases);
            textures.swapWith(other.textures);
            textureViews.swapWith(other.textureViews);
            computePipelines.swapWith(other.computePipelines);
            renderPipelines.swapWith(other.renderPipelines);
            rayPipelines.swapWith(other.rayPipelines);
            shaderTables.swapWith(other.shaderTables);
            accelerationStructures.swapWith(other.accelerationStructures);
        }

        /// Releases every independent RHI hold rather than only changing each list's logical size.
        void clear()
        {
            buffers.clearAndDeallocate();
            bufferLeases.clearAndDeallocate();
            textures.clearAndDeallocate();
            textureViews.clearAndDeallocate();
            computePipelines.clearAndDeallocate();
            renderPipelines.clearAndDeallocate();
            rayPipelines.clearAndDeallocate();
            shaderTables.clearAndDeallocate();
            accelerationStructures.clearAndDeallocate();
        }
    };

    /// Owns commands that have been encoded but not submitted to the graphics queue.
    ///
    /// Consider `dispatch(write)` followed by `dispatch(read)`: both passes share this encoder,
    /// `operationCount` causes a global barrier before the second pass, and their resource holds
    /// move together into one `InFlightSubmission` when a readback requires execution.
    struct PendingCommandBatch
    {
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
        uint64_t trackingId = 0;
        DeviceExecutionBatch executions;
#endif
        ComPtr<rhi::ICommandEncoder> encoder;
        SubmissionResourceHolds holds;
        uint32_t operationCount = 0;

        /// Drops an unsubmitted command stream after an interpreted operation fails.
        void abandon()
        {
            encoder = nullptr;
            holds.clear();
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
            executions.clear();
#endif
            operationCount = 0;
        }
    };

    /// Retains one queue submission until its signaled timeline-fence value has completed.
    ///
    /// For example, submission seven owns its command buffer, pipeline, and bound resources until
    /// `completionFence` reaches seven. Program references may disappear immediately after submit
    /// because this interpreter-side object independently preserves the RHI lifetimes.
    struct InFlightSubmission
    {
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
        DeviceExecutionBatch executions;
#endif
        ComPtr<rhi::ICommandBuffer> commandBuffer;
        SubmissionResourceHolds holds;
        uint64_t completionFenceValue = 0;

        /// Releases the completed batch before its list slot is removed or reused.
        void release()
        {
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
            executions.clear();
#endif
            commandBuffer = nullptr;
            holds.clear();
            completionFenceValue = 0;
        }
    };

    ComPtr<rhi::IDevice> device;
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    RefPtr<EventTracker> eventTracker;
#endif
    bool synchronousGPUOperations = false;
    RefPtr<BufferPool> bufferPool;
    ComPtr<rhi::ICommandQueue> commandQueue;
    ComPtr<rhi::IFence> completionFence;
    bool attemptedCompletionFenceCreation = false;
    uint64_t nextCompletionFenceValue = 1;
    uint64_t lastCompletedFenceValue = 0;
    PendingCommandBatch pendingBatch;
    uint64_t pendingRawTextureUploadBytes = 0;
    List<InFlightSubmission> inFlightSubmissions;
    List<ReadbackBufferSlot> readbackBufferPool;
    List<RefPtr<RayPipelineObject>> rayPipelines;
    // Multiple trackers will share a dispatch-wide stride and receive distinct column offsets.
    // This field is set only while preparing one ray dispatch; other dispatches use stride one.
    uint32_t rayBindingStride = 1;
    RaytracingPipelineObject* rayBindingTracker = nullptr;
    ComPtr<slang::IComponentType> blitComponent;
    ComPtr<rhi::IShaderProgram> blitProgram;
    ComPtr<rhi::IInputLayout> blitInputLayout;
    ComPtr<rhi::IRenderPipeline> blitPipeline;
    rhi::Format blitDestinationFormat = rhi::Format::Undefined;
    TextureScalarClass blitDestinationScalarClass = TextureScalarClass::Float;
    TextureScalarClass blitSourceScalarClass = TextureScalarClass::Float;

    /// Loads one embedded module after the interpreter has resolved its name, bytes, and imports.
    ///
    /// Consider the first use of module `kernels`: this creates the program instance's Slang
    /// session if necessary and loads the supplied IR blob. Later reads observe the cached module
    /// in `object` and never re-enter this method.
    void loadSlangModule(
        SlangModuleObject* object,
        ComPtr<slang::ISession>& slangSession,
        const String& name,
        const void* binary,
        uint32_t binaryByteSize)
    {
        if (!slangSession)
        {
            if (!device)
                fail("HGVM embedded module reflection requires an RHI device");
            if (SLANG_FAILED(device->createSlangSession(slangSession.writeRef())))
                fail("HGVM failed to create a Slang session for an embedded module");
        }

        object->name = name;
        ComPtr<ISlangBlob> blob;
        blob.attach(slang_createBlob(binary, binaryByteSize));
        if (!blob)
            fail("HGVM failed to create an embedded module blob");

        ComPtr<ISlangBlob> diagnostics;
        // TODO(HGVM workaround): Preserve OriginalPath decorations while staging embedded modules
        // so source-relative diagnostics do not have to use the module name as their path.
        SLANG_HGVM_BEGIN_TIMING(eventTracker.get(), loadStart);
        object->module = slangSession->loadModuleFromIRBlob(
            object->name.getBuffer(),
            object->name.getBuffer(),
            blob,
            diagnostics.writeRef());
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            trackModuleLoad(
                object->module.get(),
                object->name.getBuffer(),
                binaryByteSize,
                TrackedModuleFormat::IR,
                object->module ? SLANG_OK : SLANG_FAIL,
                loadStart));
        if (!object->module)
        {
            std::string message = "HGVM failed to load embedded Slang module '";
            message += object->name.getBuffer();
            message += "'";
            if (diagnostics && diagnostics->getBufferSize())
            {
                message += ":\n";
                message.append(
                    static_cast<const char*>(diagnostics->getBufferPointer()),
                    diagnostics->getBufferSize());
            }
            fail(message);
        }
    }

    /// Checks and specializes one embedded stage using its stored Slang specialization arguments.
    /// Both compute and graphics use this path so closure types retain the same lookup context.
    ComPtr<slang::IComponentType> specializeEntryPoint(
        SlangEntryPointObject* entryPoint,
        SlangStage stage)
    {
        ComPtr<ISlangBlob> diagnostics;
        ComPtr<slang::IEntryPoint> checkedEntryPoint;
        SlangResult result = entryPoint->module->module->findAndCheckEntryPoint(
            entryPoint->name.getBuffer(),
            stage,
            checkedEntryPoint.writeRef(),
            diagnostics.writeRef());
        if (SLANG_FAILED(result) || !checkedEntryPoint)
            fail("HGVM failed to check an embedded shader entry point");

        ComPtr<slang::IComponentType> entryPointComponent;
        entryPointComponent = static_cast<slang::IComponentType*>(checkedEntryPoint.get());
        if (entryPoint->specializationArguments.getCount())
        {
            SLANG_ASSERT(entryPoint->typeLookupComponent);
            slang::ProgramLayout* lookupLayout = entryPoint->typeLookupComponent->getLayout();
            SLANG_ASSERT(lookupLayout);

            std::vector<slang::SpecializationArg> specializationArguments;
            specializationArguments.reserve(size_t(entryPoint->specializationArguments.getCount()));
            for (const SlangEntryPointObject::SpecializationArgument& argument :
                 entryPoint->specializationArguments)
            {
                switch (argument.kind)
                {
                case HGBCSpecializationArgKind::Type:
                    {
                        slang::TypeReflection* type =
                            lookupLayout->findTypeByName(argument.value.getBuffer());
                        if (!type)
                        {
                            fail(
                                std::string("HGVM could not resolve specialization type '") +
                                argument.value.getBuffer() + "' for entry point '" +
                                entryPoint->module->name.getBuffer() + "." +
                                entryPoint->name.getBuffer() + "' at argument " +
                                std::to_string(specializationArguments.size()));
                        }
                        specializationArguments.push_back(slang::SpecializationArg::fromType(type));
                    }
                    break;
                case HGBCSpecializationArgKind::Expr:
                    specializationArguments.push_back(
                        slang::SpecializationArg::fromExpr(argument.value.getBuffer()));
                    break;
                default:
                    SLANG_UNREACHABLE("unknown staged specialization argument kind");
                }
            }

            ComPtr<slang::IComponentType> specializedEntryPoint;
            diagnostics.setNull();
            result = checkedEntryPoint->specialize(
                specializationArguments.data(),
                SlangInt(specializationArguments.size()),
                specializedEntryPoint.writeRef(),
                diagnostics.writeRef());
            if (SLANG_FAILED(result) || !specializedEntryPoint)
                fail("HGVM failed to specialize an embedded shader entry point");
            entryPointComponent = specializedEntryPoint;
        }

        return entryPointComponent;
    }

    /// Creates and caches the RHI compute pipeline for one resolved embedded entry point.
    ///
    /// Consider the first call to `dispatch(writeValue, groups)`: the interpreter supplies the
    /// already resolved entry-point object, and this method checks, composes, links, and creates
    /// all device objects subsequently reused by the dispatcher.
    void createComputeDispatcher(
        ComputeDispatcherObject* object,
        SlangEntryPointObject* entryPoint,
        slang::ISession* slangSession,
        DebugMap* debugMap)
    {
        if (!device)
            fail("HGVM compute dispatch requires an RHI device");
        SLANG_ASSERT(entryPoint && entryPoint->module && slangSession);
        object->entryPoint = entryPoint;

        ComPtr<ISlangBlob> diagnostics;
        ComPtr<slang::IComponentType> entryPointComponent =
            specializeEntryPoint(entryPoint, SLANG_STAGE_COMPUTE);
        SlangResult result;

        slang::IComponentType* components[] = {
            entryPoint->module->module,
            entryPointComponent,
        };
        ComPtr<slang::IComponentType> composite;
        diagnostics.setNull();
        result = slangSession->createCompositeComponentType(
            components,
            2,
            composite.writeRef(),
            diagnostics.writeRef());
        if (SLANG_FAILED(result) || !composite)
            fail("HGVM failed to compose an embedded compute program");

        diagnostics.setNull();
        result = composite->link(object->linkedComponent.writeRef(), diagnostics.writeRef());
        if (SLANG_FAILED(result) || !object->linkedComponent)
            fail("HGVM failed to link an embedded compute program");

        diagnostics.setNull();
        object->program =
            device->createShaderProgram(object->linkedComponent, diagnostics.writeRef());
        if (!object->program)
            fail("HGVM failed to create an embedded compute shader program");

        rhi::ComputePipelineDesc pipelineDesc = {};
        pipelineDesc.program = object->program;
        auto name = debugMap->findPipelineName(object->descriptorWordOffset);
        SLANG_RELEASE_ASSERT(name);
        pipelineDesc.label = name->getBuffer();
        SLANG_HGVM_BEGIN_TIMING(eventTracker.get(), creationStart);
        object->pipeline = device->createComputePipeline(pipelineDesc);
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            trackPipelineCreation(
                object->pipeline.get(),
                TrackedPipelineKind::Compute,
                object->pipeline ? SLANG_OK : SLANG_FAIL,
                creationStart,
                {debugMap->getProgramId(), object->descriptorWordOffset},
                {{SLANG_STAGE_COMPUTE,
                  {debugMap->getProgramId(), entryPoint->descriptorWordOffset}}}));
        if (!object->pipeline)
            fail("HGVM failed to create an embedded compute pipeline");
        object->label = *name;
    }

    void createGraphicsPipeline(
        GraphicsPipelineObject* object,
        const StagedRecord::GraphicsPipeline& state,
        SlangEntryPointObject* vertex,
        SlangEntryPointObject* fragment,
        slang::ISession* session,
        DebugMap* debugMap)
    {
        if (!device)
            fail("HGVM graphics pipelines require an RHI device");
        if (state.depthStencil.stencilEnable &&
            !rhi::getRHI()->getFormatInfo(state.depthStencil.format).hasStencil)
            fail("HGVM stencil testing requires a stencil-capable attachment format");
        auto vertexComponent = specializeEntryPoint(vertex, SLANG_STAGE_VERTEX);
        auto fragmentComponent = specializeEntryPoint(fragment, SLANG_STAGE_FRAGMENT);
        List<slang::IComponentType*> components;
        components.add(vertex->module->module);
        if (fragment->module != vertex->module)
            components.add(fragment->module->module);
        components.add(vertexComponent);
        components.add(fragmentComponent);
        ComPtr<slang::IComponentType> composite;
        ComPtr<ISlangBlob> diagnostics;
        if (SLANG_FAILED(session->createCompositeComponentType(
                components.getBuffer(),
                components.getCount(),
                composite.writeRef(),
                diagnostics.writeRef())) ||
            !composite)
            fail("HGVM failed to compose the graphics program");
        diagnostics.setNull();
        // Both specialized stages execute in one graphics pipeline. Declare that
        // coexistence explicitly so Vulkan shader offsets and host uploads agree.
        ComPtr<slang::IEntryPointUniformLayoutService_Experimental> layoutService;
        if (SLANG_FAILED(composite->queryInterface(SLANG_IID_PPV_ARGS(layoutService.writeRef()))))
            fail("HGVM could not query entry-point layout constraints");
        SlangInt stageIndices[] = {0, 1};
        slang::EntryPointUniformLayoutGroup group = {stageIndices, 2};
        ComPtr<slang::IComponentType> constraints;
        if (SLANG_FAILED(layoutService->createEntryPointUniformLayoutConstraints(
                &group,
                1,
                constraints.writeRef(),
                diagnostics.writeRef())))
            fail("HGVM failed to declare graphics entry-point coexistence");
        slang::IComponentType* constrainedParts[] = {composite, constraints};
        ComPtr<slang::IComponentType> constrainedProgram;
        if (SLANG_FAILED(session->createCompositeComponentType(
                constrainedParts,
                2,
                constrainedProgram.writeRef(),
                diagnostics.writeRef())))
            fail("HGVM failed to compose graphics layout constraints");
        if (SLANG_FAILED(constrainedProgram->link(
                object->linkedComponent.writeRef(),
                diagnostics.writeRef())))
            fail("HGVM failed to link the graphics program");
        rhi::ShaderProgramDesc programDesc;
        programDesc.slangGlobalScope = object->linkedComponent;
        if (device->getInfo().deviceType == rhi::DeviceType::Vulkan)
            programDesc.entryPointUniformLayoutGroupIndex = 0;
        object->program = device->createShaderProgram(programDesc, diagnostics.writeRef());
        if (!object->program)
            fail("HGVM failed to create the graphics shader program");
        rhi::RenderPipelineDesc description;
        auto name = debugMap->findPipelineName(object->descriptorWordOffset);
        SLANG_RELEASE_ASSERT(name);
        description.label = name->getBuffer();
        description.program = object->program;
        description.primitiveTopology = state.topology;
        description.targets = &state.colorTarget;
        description.targetCount = 1;
        description.depthStencil = state.depthStencil;
        description.rasterizer = state.rasterizer;
        description.multisample = state.multisample;
        description.compilationPolicy = rhi::PipelineCompilationPolicy::Immediate;
        SLANG_HGVM_BEGIN_TIMING(eventTracker.get(), creationStart);
        object->pipeline = device->createRenderPipeline(description);
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            trackPipelineCreation(
                object->pipeline.get(),
                TrackedPipelineKind::Graphics,
                object->pipeline ? SLANG_OK : SLANG_FAIL,
                creationStart,
                {debugMap->getProgramId(), object->descriptorWordOffset},
                {TrackedPipelineShader{
                     SLANG_STAGE_VERTEX,
                     {debugMap->getProgramId(), vertex->descriptorWordOffset}},
                 TrackedPipelineShader{
                     SLANG_STAGE_FRAGMENT,
                     {debugMap->getProgramId(), fragment->descriptorWordOffset}}}));
        if (!object->pipeline)
            fail("HGVM failed to prepare the graphics pipeline");
        object->colorFormat = state.colorTarget.format;
        object->depthFormat = state.depthStencil.format;
        object->scissorEnable = state.rasterizer.scissorEnable;
    }

    /// Allocates one explicitly described resource and returns its initial program-owned wrapper.
    ///
    /// Consider a four-element buffer with stride 16: the staged record supplies device-local
    /// memory, unordered-access state, and every usage bit. A 2D texture follows the other switch
    /// arm with its explicit format. This method never derives resource semantics from an HGBC
    /// result type.
    HeapObject* allocateResource(
        const StagedRecord::AllocateResource& record,
        TypeLayoutObject* layout,
        uint32_t widthOrElementCount,
        uint32_t height,
        uint32_t depth)
    {
        if (!device)
            fail("HGVM resource allocation requires an RHI device");

        if (record.kind == HGBCResourceKind::Buffer)
        {
            SLANG_ASSERT(layout && layout->domain == HGBCLayoutDomain::DeviceMemory);
            const uint32_t elementStride = layout->byteStride;
            if (!elementStride ||
                widthOrElementCount > std::numeric_limits<uint64_t>::max() / elementStride)
            {
                fail("HGVM structured-buffer allocation size is invalid");
            }
            uint64_t byteSize = uint64_t(widthOrElementCount) * elementStride;
            uint64_t maxBufferSize = device->getInfo().limits.maxBufferSize;
            if (maxBufferSize && byteSize > maxBufferSize)
                fail("HGVM structured-buffer allocation exceeds the RHI device limit");

            rhi::BufferDesc desc = {};
            desc.size = byteSize;
            desc.elementSize = elementStride;
            desc.format = record.format;
            desc.memoryType = record.memoryType;
            desc.usage = record.bufferUsage;
            desc.defaultState = record.initialState;
            desc.label = "HGVM buffer";

            ComPtr<rhi::IBuffer> resource = bufferPool->acquire(desc);
            if (!resource)
            {
                // Poll only on a miss. Finished batches may return the exact allocation we need.
                retireCompletedSubmissions();
                resource = bufferPool->acquire(desc);
            }
            if (!resource)
            {
                bufferPool->recordMiss();
                SLANG_HGVM_BEGIN_TIMING(eventTracker.get(), creationStart);
                SlangResult result = device->createBuffer(desc, nullptr, resource.writeRef());
                SLANG_HGVM_TRACK(
                    eventTracker.get(),
                    trackBufferCreation(
                        resource.get(),
                        desc,
                        TrackedBufferPurpose::Program,
                        result,
                        creationStart));
                if (result == SLANG_E_OUT_OF_MEMORY)
                {
                    // An allocation failure is the only implicit blocking path. Draining first
                    // also makes scratch from the current unsubmitted batch eligible for reuse.
                    trimBufferPool();
                    SLANG_HGVM_BEGIN_TIMING(eventTracker.get(), retryStart);
                    result = device->createBuffer(desc, nullptr, resource.writeRef());
                    SLANG_HGVM_TRACK(
                        eventTracker.get(),
                        trackBufferCreation(
                            resource.get(),
                            desc,
                            TrackedBufferPurpose::Program,
                            result,
                            retryStart));
                }
                if (SLANG_FAILED(result))
                    fail("HGVM RHI failed to create a buffer");
            }
            if (!resource)
                fail("HGVM RHI failed to create a buffer");
            StructuredBufferObject* object =
                allocateHeapObject<StructuredBufferObject>(SLANG_HGVM_TRACKER(eventTracker));
            object->resource = std::move(resource);
            object->pool = bufferPool;
            object->allocationDesc = desc;
            bufferPool->beginLease(desc.size);
            object->elementCount = widthOrElementCount;
            object->elementStride = elementStride;
            object->elementLayout = layout;
            return object;
        }

        SLANG_ASSERT(record.dimension == HGBCResourceDimension::Texture2D);
        if (!widthOrElementCount || !height || depth != 1)
            fail("HGVM 2D texture allocation requires nonzero width/height and depth one");
        if (record.mipCount > Math::Log2Floor(std::max(widthOrElementCount, height)) + 1)
            fail("HGVM texture mip count exceeds its dimensions");
        uint32_t textureLimit = device->getInfo().limits.maxTextureDimension2D;
        if (textureLimit && (widthOrElementCount > textureLimit || height > textureLimit))
            fail("HGVM texture allocation exceeds the RHI 2D dimension limit");
        if (record.format == rhi::Format::Undefined)
            fail("HGVM texture allocation has no RHI format");

        // CUDA storage textures are surfaces, whose typed loads/stores copy raw bytes instead of
        // applying the format conversion performed by texture objects. Reject a logical element
        // wider than its physical texel so an RGBA8 surface cannot masquerade as float4 and return
        // garbage. Read-only textures intentionally take the hardware texture-object path.
        const rhi::FormatInfo& formatInfo = rhi::getRHI()->getFormatInfo(record.format);
        if (device->getDeviceType() == rhi::DeviceType::CUDA &&
            record.textureAccess != HGBCTextureAccess::ReadOnly &&
            record.textureElementByteSize != 0 &&
            record.textureElementByteSize != formatInfo.blockSizeInBytes)
        {
            fail("HGVM CUDA writable textures require a physical texel that matches the shader "
                 "element width; use a read-only Texture2D for hardware format conversion");
        }

        rhi::TextureDesc desc = {};
        desc.type = rhi::TextureType::Texture2D;
        desc.size = {widthOrElementCount, height, depth};
        desc.mipCount = record.mipCount;
        desc.sampleCount = record.sampleCount;
        desc.sampleQuality = record.sampleQuality;
        desc.format = record.format;
        desc.memoryType = record.memoryType;
        desc.usage = record.textureUsage;
        desc.defaultState = record.initialState;
        desc.label = "HGVM texture";

        ComPtr<rhi::ITexture> resource = device->createTexture(desc);
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            trackTextureCreation(resource.get(), desc, resource ? SLANG_OK : SLANG_FAIL));
        if (!resource)
            fail("HGVM RHI failed to create a texture");
        Texture2DObject* object =
            allocateHeapObject<Texture2DObject>(SLANG_HGVM_TRACKER(eventTracker));
        object->resource = std::move(resource);
        return object;
    }

    /// Writes a texture wrapper's cached RHI width and height into resolved interpreter storage.
    ///
    /// For example, an 8-by-4 texture writes `{8, 4}` without receiving an interpreter `Address`.
    void getTextureDimensions(Texture2DObject* texture, uint32_t* outDimensions) const
    {
        const rhi::TextureDesc& desc = texture->resource->getDesc();
        outDimensions[0] = desc.size.width;
        outDimensions[1] = desc.size.height;
    }

    /// Wraps one host-acquired texture with an initial program reference.
    ///
    /// Consider a swap-chain image returned by `acquireSurfaceTexture`: the host keeps its own
    /// surface state, while this wrapper gives ordinary HGVM copies and cleanup the same ownership
    /// behavior as an allocated texture.
    Texture2DObject* wrapTexture(ComPtr<rhi::ITexture> resource)
    {
        if (!resource)
            fail("HGVM cannot wrap a null texture");
        Texture2DObject* object =
            allocateHeapObject<Texture2DObject>(SLANG_HGVM_TRACKER(eventTracker));
        object->resource = std::move(resource);
        return object;
    }

    /// Creates the fullscreen blit shader and the pipeline matching one concrete target format.
    ///
    /// The shader-facing type remains `float4` for normalized and floating storage. For example,
    /// RGBA32Float to BGRA8 sRGB uses the same shader and only recreates the render pipeline for
    /// the destination attachment format; the backend performs channel packing and sRGB encoding.
    void ensureBlitPipeline(
        rhi::Format destinationFormat,
        TextureScalarClass destinationScalarClass,
        TextureScalarClass sourceScalarClass)
    {
        if (blitPipeline && blitDestinationFormat == destinationFormat &&
            blitDestinationScalarClass == destinationScalarClass &&
            blitSourceScalarClass == sourceScalarClass)
            return;
        if (!device)
            fail("HGVM texture blitting requires an RHI device");

        if (!blitComponent || blitDestinationScalarClass != destinationScalarClass ||
            blitSourceScalarClass != sourceScalarClass)
        {
            auto getShaderType = [](TextureScalarClass scalarClass) -> const char*
            {
                switch (scalarClass)
                {
                case TextureScalarClass::Float:
                    return "float4";
                case TextureScalarClass::Int:
                    return "int4";
                case TextureScalarClass::UInt:
                    return "uint4";
                }
                SLANG_UNREACHABLE("unknown staged blit scalar class");
            };
            std::string blitSource = "#define SOURCE_TYPE ";
            blitSource += getShaderType(sourceScalarClass);
            blitSource += "\n#define DESTINATION_TYPE ";
            blitSource += getShaderType(destinationScalarClass);
            blitSource += R"(
struct BlitVarying
{
    float4 position : SV_Position;
};

[shader("vertex")]
BlitVarying vertexMain(uint vertexId : SV_VertexID)
{
    float2 position = vertexId == 0 ? float2(-1.0, -1.0)
        : vertexId == 1 ? float2(-1.0, 3.0)
                        : float2(3.0, -1.0);
    BlitVarying result;
    result.position = float4(position, 0.0, 1.0);
    return result;
}

[shader("fragment")]
DESTINATION_TYPE fragmentMain(
    BlitVarying input,
    Texture2D<SOURCE_TYPE> source,
    uniform uint2 outputSize) : SV_Target
{
    uint sourceWidth;
    uint sourceHeight;
    source.GetDimensions(sourceWidth, sourceHeight);
    uint2 pixel = min(
        uint2(input.position.xy) * uint2(sourceWidth, sourceHeight) / outputSize,
        uint2(sourceWidth - 1, sourceHeight - 1));
    return DESTINATION_TYPE(source.Load(int3(pixel, 0)));
}
)";
            ComPtr<slang::ISession> session = device->getSlangSession();
            if (!session)
                fail("HGVM RHI device has no Slang session for texture blitting");
            ComPtr<ISlangBlob> diagnostics;
            std::string moduleName = "__slang_hgvm_blit_";
            moduleName += char('0' + uint32_t(sourceScalarClass));
            moduleName += '_';
            moduleName += char('0' + uint32_t(destinationScalarClass));
            std::string sourcePath = moduleName + ".slang";
            SLANG_HGVM_BEGIN_TIMING(eventTracker.get(), loadStart);
            slang::IModule* module = session->loadModuleFromSourceString(
                moduleName.c_str(),
                sourcePath.c_str(),
                blitSource.c_str(),
                diagnostics.writeRef());
            SLANG_HGVM_TRACK(
                eventTracker.get(),
                trackModuleLoad(
                    module,
                    moduleName.c_str(),
                    blitSource.size(),
                    TrackedModuleFormat::Source,
                    module ? SLANG_OK : SLANG_FAIL,
                    loadStart));
            if (!module)
            {
                std::string message = "HGVM failed to compile its texture blitter";
                if (diagnostics && diagnostics->getBufferSize())
                {
                    message += ": ";
                    message.append(
                        static_cast<const char*>(diagnostics->getBufferPointer()),
                        diagnostics->getBufferSize());
                }
                fail(message);
            }

            ComPtr<slang::IEntryPoint> vertex;
            SlangResult result = module->findAndCheckEntryPoint(
                "vertexMain",
                SLANG_STAGE_VERTEX,
                vertex.writeRef(),
                diagnostics.writeRef());
            if (SLANG_FAILED(result) || !vertex)
                fail("HGVM failed to check its blit vertex entry point");
            ComPtr<slang::IEntryPoint> fragment;
            result = module->findAndCheckEntryPoint(
                "fragmentMain",
                SLANG_STAGE_FRAGMENT,
                fragment.writeRef(),
                diagnostics.writeRef());
            if (SLANG_FAILED(result) || !fragment)
                fail("HGVM failed to check its blit fragment entry point");

            slang::IComponentType* components[] = {module, vertex, fragment};
            ComPtr<slang::IComponentType> composite;
            result = session->createCompositeComponentType(
                components,
                SLANG_COUNT_OF(components),
                composite.writeRef(),
                diagnostics.writeRef());
            if (SLANG_FAILED(result) || !composite)
                fail("HGVM failed to compose its texture blitter");
            result = composite->link(blitComponent.writeRef(), diagnostics.writeRef());
            if (SLANG_FAILED(result) || !blitComponent)
                fail("HGVM failed to link its texture blitter");
            blitProgram = device->createShaderProgram(blitComponent, diagnostics.writeRef());
            if (!blitProgram)
                fail("HGVM failed to create its texture blit program");
            blitDestinationScalarClass = destinationScalarClass;
            blitSourceScalarClass = sourceScalarClass;
        }

        if (!blitInputLayout)
        {
            // The full-screen triangle uses SV_VertexID and has no vertex-buffer inputs.
            rhi::InputLayoutDesc inputLayoutDesc = {};
            blitInputLayout = device->createInputLayout(inputLayoutDesc);
            if (!blitInputLayout)
                fail("HGVM RHI failed to create its texture blit input layout");
        }

        rhi::ColorTargetDesc target = {};
        target.format = destinationFormat;
        rhi::RenderPipelineDesc desc = {};
        desc.program = blitProgram;
        desc.inputLayout = blitInputLayout;
        desc.primitiveTopology = rhi::PrimitiveTopology::TriangleList;
        desc.targets = &target;
        desc.targetCount = 1;
        desc.depthStencil.depthTestEnable = false;
        desc.depthStencil.depthWriteEnable = false;
        desc.label = "HGVM texture blitter";
        SLANG_HGVM_BEGIN_TIMING(eventTracker.get(), creationStart);
        blitPipeline = device->createRenderPipeline(desc);
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            trackPipelineCreation(
                blitPipeline.get(),
                TrackedPipelineKind::Blit,
                blitPipeline ? SLANG_OK : SLANG_FAIL,
                creationStart));
        if (!blitPipeline)
            fail("HGVM RHI failed to create its texture blit pipeline");
        blitDestinationFormat = destinationFormat;
    }

    /// Records one complete texture conversion into the current command batch.
    void blitTexture(
        Texture2DObject* destination,
        Texture2DObject* source,
        TextureScalarClass destinationScalarClass,
        TextureScalarClass sourceScalarClass)
    {
        SLANG_ASSERT(destination && destination->resource && source && source->resource);
        const rhi::TextureDesc& destinationDesc = destination->resource->getDesc();
        ensureBlitPipeline(destinationDesc.format, destinationScalarClass, sourceScalarClass);
        ComPtr<rhi::ITextureView> destinationView = destination->resource->getDefaultView();
        if (!destinationView)
            fail("HGVM RHI failed to create a texture blit destination view");

        rhi::RenderPassColorAttachment attachment = {};
        attachment.view = destinationView;
        attachment.loadOp = rhi::LoadOp::DontCare;
        attachment.storeOp = rhi::StoreOp::Store;
        rhi::RenderPassDesc passDesc = {};
        passDesc.colorAttachments = &attachment;
        passDesc.colorAttachmentCount = 1;
        rhi::ICommandEncoder* commandEncoder = getPendingCommandEncoder();
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            beginPipelineExecution(
                pendingBatch.executions,
                commandEncoder,
                pendingBatch.trackingId,
                blitPipeline.get(),
                TrackedPipelineKind::Blit));
        rhi::IRenderPassEncoder* pass = commandEncoder->beginRenderPass(passDesc);
        if (!pass)
            fail("HGVM RHI failed to begin a texture blit pass");

        rhi::IShaderObject* root = pass->bindPipeline(blitPipeline);
        uint32_t outputSize[2] = {destinationDesc.size.width, destinationDesc.size.height};
        if (!root)
        {
            pass->end();
            fail("HGVM RHI failed to bind its texture blit pipeline");
        }
        rhi::ShaderCursor cursor(root);
        if (SLANG_FAILED(cursor.getField("source").setBinding(rhi::Binding(source->resource))) ||
            SLANG_FAILED(cursor.getField("outputSize").setData(outputSize, sizeof(outputSize))))
        {
            pass->end();
            fail("HGVM RHI failed to bind a texture blit");
        }

        rhi::RenderState state = {};
        state.viewports[0] = rhi::Viewport::fromSize(float(outputSize[0]), float(outputSize[1]));
        state.viewportCount = 1;
        state.scissorRects[0] = rhi::ScissorRect::fromSize(outputSize[0], outputSize[1]);
        state.scissorRectCount = 1;
        pass->setRenderState(state);
        rhi::DrawArguments draw = {};
        draw.vertexCount = 3;
        pass->draw(draw);
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            trackDraw(
                pendingBatch.executions,
                pendingBatch.trackingId,
                blitPipeline.get(),
                draw.vertexCount,
                draw.instanceCount,
                false));
        pass->end();
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            endDeviceExecution(pendingBatch.executions, commandEncoder));
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            trackBlit(
                pendingBatch.trackingId,
                destination->resource.get(),
                source->resource.get()));

        pendingBatch.holds.textures.add(destination->resource);
        pendingBatch.holds.textures.add(source->resource);
        pendingBatch.holds.renderPipelines.add(blitPipeline);
        finishDeviceOperation();
    }

    /// Clears selected whole subresources in command order, independently of draw state.
    /// Counts use the RHI all-remaining sentinel. Validate before subtracting or asking RHI to
    /// normalize the range, so invalid input is rejected rather than silently clamped.
    void clearTexture(
        Texture2DObject* texture,
        const rhi::ClearValue& value,
        TextureScalarClass scalarClass,
        HGBCTextureAspect aspect,
        rhi::SubresourceRange range)
    {
        const auto& desc = texture->resource->getDesc();
        if (range.mip > desc.mipCount || range.layer > desc.getLayerCount())
            fail("HGVM texture clear subresource range is out of bounds");
        if (range.mipCount == UINT32_MAX)
            range.mipCount = desc.mipCount - range.mip;
        if (range.layerCount == UINT32_MAX)
            range.layerCount = desc.getLayerCount() - range.layer;
        if (range.mipCount > desc.mipCount - range.mip ||
            range.layerCount > desc.getLayerCount() - range.layer)
            fail("HGVM texture clear subresource range is out of bounds");
        if (!range.mipCount || !range.layerCount)
            return;
        if (!rhi::is_set(desc.usage, rhi::TextureUsage::CopyDestination))
            fail("HGVM texture clears require CopyDestination usage");
        const auto& formatInfo = rhi::getRHI()->getFormatInfo(desc.format);
        if (formatInfo.hasDepth || formatInfo.hasStencil)
        {
            bool clearDepth = formatInfo.hasDepth && aspect != HGBCTextureAspect::StencilOnly;
            bool clearStencil = formatInfo.hasStencil && aspect != HGBCTextureAspect::DepthOnly;
            if (!rhi::is_set(desc.usage, rhi::TextureUsage::DepthStencil))
                fail("HGVM depth/stencil clears require DepthStencil usage");
            if (clearDepth && !(value.depthStencil.depth >= 0.f && value.depthStencil.depth <= 1.f))
                fail("HGVM depth clear value must be finite and in [0, 1]");
            if (clearStencil && value.depthStencil.stencil > 255)
                fail("HGVM stencil clear value must be in [0, 255]");
            getPendingCommandEncoder()->clearTextureDepthStencil(
                texture->resource,
                range,
                clearDepth,
                value.depthStencil.depth,
                clearStencil,
                uint8_t(value.depthStencil.stencil));
        }
        else
        {
            bool hasUAV = rhi::is_set(desc.usage, rhi::TextureUsage::UnorderedAccess);
            bool hasRTV = rhi::is_set(desc.usage, rhi::TextureUsage::RenderTarget);
            if (!(hasUAV || (hasRTV && scalarClass == TextureScalarClass::Float)))
                fail(
                    "HGVM color clear requires render-target or compatible unordered-access usage");
            // RHI copies the value into its command stream before this local copy expires.
            rhi::ColorClearValue color = value.color;
            switch (scalarClass)
            {
            case TextureScalarClass::Float:
                getPendingCommandEncoder()->clearTextureFloat(
                    texture->resource,
                    range,
                    color.floatValues);
                break;
            case TextureScalarClass::UInt:
                getPendingCommandEncoder()->clearTextureUint(
                    texture->resource,
                    range,
                    color.uintValues);
                break;
            case TextureScalarClass::Int:
                getPendingCommandEncoder()->clearTextureSint(
                    texture->resource,
                    range,
                    color.intValues);
                break;
            }
        }
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            trackClear(pendingBatch.trackingId, texture->resource.get()));
        pendingBatch.holds.textures.add(texture->resource);
        finishDeviceOperation();
    }

    /// Writes a structured buffer's logical element count and stride into resolved storage.
    ///
    /// For example, a four-element buffer with 16-byte elements writes `{4, 16}` after ensuring
    /// the count fits the current HGBC `uint2` result representation.
    void getStructuredBufferDimensions(StructuredBufferObject* buffer, uint32_t* outDimensions)
        const
    {
        if (buffer->elementCount > std::numeric_limits<uint32_t>::max())
            fail("HGVM structured-buffer element count exceeds uint32");
        outDimensions[0] = uint32_t(buffer->elementCount);
        outDimensions[1] = buffer->elementStride;
    }

    static void validateArrayCopyRange(
        uint64_t totalElementCount,
        uint32_t elementOffset,
        uint32_t elementCount,
        const char* endpointName)
    {
        if (elementOffset > totalElementCount || elementCount > totalElementCount - elementOffset)
        {
            fail(std::string("HGVM array-copy ") + endpointName + " range is out of bounds");
        }
    }

    /// Converts one host range into one contiguous device-layout upload request.
    void copyRuntimeArrayToBuffer(
        StructuredBufferObject* destination,
        uint32_t destinationOffset,
        RuntimeArrayObject* source,
        uint32_t sourceOffset,
        uint32_t elementCount,
        LayoutConverterObject* converter)
    {
        validateArrayCopyRange(
            destination->elementCount,
            destinationOffset,
            elementCount,
            "destination");
        validateArrayCopyRange(source->elementCount, sourceOffset, elementCount, "source");
        if (elementCount == 0)
            return;
        if (!source->elementManagedLeafOffsets.empty())
            fail("HGVM cannot copy managed runtime-array elements into device memory");
        if (!rhi::is_set(destination->resource->getDesc().usage, rhi::BufferUsage::CopyDestination))
        {
            fail("HGVM runtime-array upload requires CopyDestination buffer usage");
        }
        SLANG_ASSERT(converter && converter->sourceLayout && converter->destinationLayout);
        SLANG_ASSERT(converter->sourceLayout->domain == HGBCLayoutDomain::Host);
        SLANG_ASSERT(converter->destinationLayout->domain == HGBCLayoutDomain::DeviceMemory);
        SLANG_ASSERT(converter->sourceLayout->byteStride == source->elementByteStride);
        SLANG_ASSERT(converter->destinationLayout->byteStride == destination->elementStride);

        uint64_t uploadByteSize = uint64_t(elementCount) * destination->elementStride;
        if (uploadByteSize > std::numeric_limits<size_t>::max())
            fail("HGVM array-copy upload is too large for host staging");
        std::vector<uint8_t> uploadBytes(size_t(uploadByteSize), 0);
        for (uint32_t elementIndex = 0; elementIndex < elementCount; ++elementIndex)
        {
            const uint8_t* sourceElement =
                source->payload + uint64_t(sourceOffset + elementIndex) * source->elementByteStride;
            uint8_t* destinationElement =
                uploadBytes.data() + uint64_t(elementIndex) * destination->elementStride;
            for (const LayoutConversionOperation& operation : converter->operations)
            {
                SLANG_ASSERT(
                    operation.sourceByteOffset <= source->elementByteStride &&
                    operation.sourceByteSize <=
                        source->elementByteStride - operation.sourceByteOffset);
                SLANG_ASSERT(
                    operation.destinationByteOffset <= destination->elementStride &&
                    operation.destinationByteSize <=
                        destination->elementStride - operation.destinationByteOffset);
                copyScalarForBinding(
                    operation.scalarType,
                    sourceElement + operation.sourceByteOffset,
                    operation.sourceByteSize,
                    destinationElement + operation.destinationByteOffset,
                    operation.destinationByteSize);
            }
        }
        uploadBufferBytes(
            destination,
            uint64_t(destinationOffset) * destination->elementStride,
            uploadBytes);
    }

    /// Reads one contiguous device range and converts it into canonical host elements.
    void copyBufferToRuntimeArray(
        RuntimeArrayObject* destination,
        uint32_t destinationOffset,
        StructuredBufferObject* source,
        uint32_t sourceOffset,
        uint32_t elementCount,
        LayoutConverterObject* converter)
    {
        validateArrayCopyRange(
            destination->elementCount,
            destinationOffset,
            elementCount,
            "destination");
        validateArrayCopyRange(source->elementCount, sourceOffset, elementCount, "source");
        if (elementCount == 0)
            return;
        if (!destination->elementManagedLeafOffsets.empty())
            fail("HGVM cannot copy device memory into managed runtime-array elements");
        if (!rhi::is_set(source->resource->getDesc().usage, rhi::BufferUsage::CopySource))
            fail("HGVM runtime-array readback requires CopySource buffer usage");
        SLANG_ASSERT(converter && converter->sourceLayout && converter->destinationLayout);
        SLANG_ASSERT(converter->sourceLayout->domain == HGBCLayoutDomain::DeviceMemory);
        SLANG_ASSERT(converter->destinationLayout->domain == HGBCLayoutDomain::Host);
        SLANG_ASSERT(converter->sourceLayout->byteStride == source->elementStride);
        SLANG_ASSERT(converter->destinationLayout->byteStride == destination->elementByteStride);

        uint64_t readbackByteSize = uint64_t(elementCount) * source->elementStride;
        if (readbackByteSize > std::numeric_limits<size_t>::max())
            fail("HGVM array-copy readback is too large for host staging");
        std::vector<uint8_t> readbackBytes(static_cast<size_t>(readbackByteSize));
        readBufferBytes(source, uint64_t(sourceOffset) * source->elementStride, readbackBytes);
        for (uint32_t elementIndex = 0; elementIndex < elementCount; ++elementIndex)
        {
            const uint8_t* sourceElement =
                readbackBytes.data() + uint64_t(elementIndex) * source->elementStride;
            uint8_t* destinationElement =
                destination->payload +
                uint64_t(destinationOffset + elementIndex) * destination->elementByteStride;
            std::memset(destinationElement, 0, destination->elementByteStride);
            for (const LayoutConversionOperation& operation : converter->operations)
            {
                SLANG_ASSERT(
                    operation.sourceByteOffset <= source->elementStride &&
                    operation.sourceByteSize <= source->elementStride - operation.sourceByteOffset);
                SLANG_ASSERT(
                    operation.destinationByteOffset <= destination->elementByteStride &&
                    operation.destinationByteSize <=
                        destination->elementByteStride - operation.destinationByteOffset);
                copyScalarFromDevice(
                    operation.scalarType,
                    sourceElement + operation.sourceByteOffset,
                    operation.sourceByteSize,
                    destinationElement + operation.destinationByteOffset,
                    operation.destinationByteSize);
            }
        }
    }

    /// Records one direct device-buffer range copy without synchronizing the command stream.
    void copyBufferRange(
        StructuredBufferObject* destination,
        uint32_t destinationOffset,
        StructuredBufferObject* source,
        uint32_t sourceOffset,
        uint32_t elementCount,
        LayoutConverterObject* converter)
    {
        validateArrayCopyRange(
            destination->elementCount,
            destinationOffset,
            elementCount,
            "destination");
        validateArrayCopyRange(source->elementCount, sourceOffset, elementCount, "source");
        SLANG_ASSERT(converter && converter->sourceLayout && converter->destinationLayout);
        if (converter->sourceLayout != converter->destinationLayout ||
            source->elementStride != destination->elementStride)
        {
            fail("HGVM device array copy requires identical physical element layouts");
        }

        uint64_t sourceByteOffset = uint64_t(sourceOffset) * source->elementStride;
        uint64_t destinationByteOffset = uint64_t(destinationOffset) * destination->elementStride;
        uint64_t byteSize = uint64_t(elementCount) * source->elementStride;
        if (elementCount == 0 || (source->resource == destination->resource &&
                                  sourceByteOffset == destinationByteOffset))
        {
            return;
        }
        if (source->resource == destination->resource &&
            sourceByteOffset < destinationByteOffset + byteSize &&
            destinationByteOffset < sourceByteOffset + byteSize)
        {
            fail("HGVM array copy does not permit overlapping ranges in one device buffer");
        }
        const rhi::BufferDesc& sourceDesc = source->resource->getDesc();
        const rhi::BufferDesc& destinationDesc = destination->resource->getDesc();
        if (!rhi::is_set(sourceDesc.usage, rhi::BufferUsage::CopySource) ||
            !rhi::is_set(destinationDesc.usage, rhi::BufferUsage::CopyDestination))
        {
            fail("HGVM device array copy requires CopySource and CopyDestination buffer usage");
        }

        getPendingCommandEncoder()->copyBuffer(
            destination->resource,
            destinationByteOffset,
            source->resource,
            sourceByteOffset,
            byteSize);
        pendingBatch.holds.bufferLeases.add(destination);
        pendingBatch.holds.bufferLeases.add(source);
        finishDeviceOperation();
    }

    /// Uploads exact raw bytes into one staged device-buffer range.
    void uploadBufferBytes(
        StructuredBufferObject* buffer,
        uint64_t byteOffset,
        std::span<const uint8_t> bytes)
    {
        const rhi::BufferDesc& desc = buffer->resource->getDesc();
        if (byteOffset > desc.size || bytes.size() > desc.size - byteOffset)
            fail("HGVM byte-stream buffer destination is out of range");
        if (SLANG_FAILED(
                getPendingCommandEncoder()
                    ->uploadBufferData(buffer->resource, byteOffset, bytes.size(), bytes.data())))
        {
            fail("HGVM RHI failed to upload byte-stream data");
        }
        pendingBatch.holds.bufferLeases.add(buffer);
        finishDeviceOperation();
    }

    /// Uploads one tightly packed physical-format payload into a complete 2D texture.
    void uploadTextureBytes(Texture2DObject* texture, std::span<const uint8_t> bytes)
    {
        const rhi::TextureDesc& desc = texture->resource->getDesc();
        const rhi::FormatInfo& formatInfo = rhi::getRHI()->getFormatInfo(desc.format);
        if (desc.type != rhi::TextureType::Texture2D || desc.arrayLength != 1 ||
            desc.mipCount != 1 || desc.sampleCount != 1 || desc.size.depth != 1 ||
            formatInfo.isCompressed || formatInfo.blockWidth != 1 || formatInfo.blockHeight != 1 ||
            formatInfo.pixelsPerBlock != 1 || !formatInfo.blockSizeInBytes)
        {
            fail("HGVM raw texture upload requires one uncompressed 2D subresource");
        }

        uint64_t rowPitch = uint64_t(desc.size.width) * formatInfo.blockSizeInBytes;
        uint64_t expectedByteCount = rowPitch * desc.size.height;
        if (expectedByteCount != bytes.size() || rowPitch > std::numeric_limits<size_t>::max())
        {
            fail("HGVM raw texture payload does not match its physical allocation");
        }

        rhi::SubresourceData data = {};
        data.data = bytes.data();
        data.rowPitch = size_t(rowPitch);
        data.slicePitch = bytes.size();
        if (SLANG_FAILED(getPendingCommandEncoder()->uploadTextureData(
                texture->resource,
                {0, 1, 0, 1},
                {0, 0, 0},
                {desc.size.width, desc.size.height, 1},
                &data,
                1)))
        {
            fail("HGVM RHI failed to upload byte-stream texture data");
        }
        pendingBatch.holds.textures.add(texture->resource);
        finishDeviceOperation();
        pendingRawTextureUploadBytes += bytes.size();
        if (pendingRawTextureUploadBytes >= 256ull * 1024ull * 1024ull)
            submitAndWait();
    }

    /// Reads exact raw bytes from one staged device-buffer range after completing device work.
    void readBufferBytes(
        StructuredBufferObject* buffer,
        uint64_t byteOffset,
        std::span<uint8_t> destination)
    {
        const rhi::BufferDesc& desc = buffer->resource->getDesc();
        if (byteOffset > desc.size || destination.size() > desc.size - byteOffset)
            fail("HGVM byte-stream buffer source is out of range");
        submitAndWait();
        if (SLANG_FAILED(device->readBuffer(
                buffer->resource,
                byteOffset,
                destination.size(),
                destination.data())))
        {
            fail("HGVM RHI failed to read byte-stream buffer data");
        }
    }

    /// Uploads one decoded RGBA32Float image into an existing complete texture subresource.
    void uploadDecodedImage(Texture2DObject* texture, const DecodedTextureImage& image)
    {
        const rhi::TextureDesc& desc = texture->resource->getDesc();
        if (desc.type != rhi::TextureType::Texture2D || desc.format != rhi::Format::RGBA32Float ||
            desc.arrayLength != 1 || desc.mipCount != 1 || desc.sampleCount != 1 ||
            desc.size.depth != 1 || desc.size.width != image.width ||
            desc.size.height != image.height)
        {
            fail("HGVM decoded image does not match its destination texture");
        }

        rhi::SubresourceData data = {};
        data.data = image.rgba.data();
        data.rowPitch = size_t(image.width) * sizeof(float) * 4;
        data.slicePitch = data.rowPitch * image.height;
        if (SLANG_FAILED(getPendingCommandEncoder()->uploadTextureData(
                texture->resource,
                {0, 1, 0, 1},
                {0, 0, 0},
                {image.width, image.height, 1},
                &data,
                1)))
        {
            fail("HGVM RHI failed to upload a decoded image");
        }
        pendingBatch.holds.textures.add(texture->resource);
        finishDeviceOperation();
    }

    /// Describes the actual resource, including backend-resolved mip counts.
    static TextureInfo getTextureInfo(Texture2DObject* texture)
    {
        const auto& desc = texture->resource->getDesc();
        return {
            desc.type,
            desc.format,
            desc.size.width,
            desc.size.height,
            desc.size.depth,
            desc.getLayerCount(),
            desc.mipCount,
            desc.sampleCount};
    }

    /// Reads and encodes exactly one subresource, respecting its padded RHI row layout.
    /// The debugger supplies a byte limit before any GPU work or readback allocation occurs.
    uint64_t encodeTextureMip(
        HGBCImageCodec codec,
        Texture2DObject* texture,
        uint32_t mip,
        SequentialByteWriter& destination,
        uint64_t maxReadbackBytes)
    {
        const auto& desc = texture->resource->getDesc();
        if (desc.type != rhi::TextureType::Texture2D || desc.getLayerCount() != 1 ||
            desc.sampleCount != 1 || desc.size.depth != 1)
            fail("Texture export requires a single-layer, single-sample 2D texture.");
        if (mip >= desc.mipCount)
            fail("Texture mip index out of bounds.");
        if (!rhi::is_set(desc.usage, rhi::TextureUsage::CopySource))
            fail("Texture export requires CopySource usage.");
        rhi::SubresourceLayout layout = {};
        if (SLANG_FAILED(texture->resource->getSubresourceLayout(mip, &layout)))
            fail("Cannot obtain the texture mip layout.");
        if (layout.sizeInBytes > maxReadbackBytes)
            fail("Texture mip readback exceeds the 256 MiB debugger limit; select a smaller mip.");
        submitAndWait();
        ComPtr<ISlangBlob> readback;
        if (SLANG_FAILED(
                device->readTexture(texture->resource, 0, mip, readback.writeRef(), &layout)) ||
            !readback || readback->getBufferSize() != layout.sizeInBytes)
            fail("HGVM RHI failed to read a texture for image encoding");

        TextureInfo info = getTextureInfo(texture);
        info.width = layout.size.width;
        info.height = layout.size.height;
        info.depth = layout.size.depth;
        info.mipCount = 1;
        std::span<const uint8_t> source(
            static_cast<const uint8_t*>(readback->getBufferPointer()),
            readback->getBufferSize());
        uint64_t byteCount = 0;
        std::string error;
        bool succeeded =
            encodeTextureImage(codec, info, layout, source, destination, byteCount, error);
        if (!succeeded)
            fail(error.empty() ? "HGVM failed to encode an image" : error);
        return byteCount;
    }

    /// Reads back and sequentially encodes one complete texture without another RHI resource.
    uint64_t encodeImage(
        HGBCImageCodec codec,
        Texture2DObject* texture,
        SequentialByteWriter& destination)
    {
        const auto& desc = texture->resource->getDesc();
        if (desc.mipCount != 1)
            fail("HGVM image encoding requires one complete 2D texture subresource");
        return encodeTextureMip(codec, texture, 0, destination, UINT64_MAX);
    }

    /// Reads one POD element into interpreter storage after completing pending device writes.
    ///
    /// Consider reading element two of a four-byte buffer: this flushes prior dispatches and asks
    /// RHI to copy exactly four bytes from byte offset eight into the already resolved destination.
    void readBackBufferElement(
        StructuredBufferObject* buffer,
        LayoutConverterObject* converter,
        uint32_t index,
        uint32_t resultByteSize,
        void* destination)
    {
        if (index >= buffer->elementCount)
            fail("HGVM structured-buffer readback is out of range");
        SLANG_ASSERT(converter && converter->sourceLayout && converter->destinationLayout);
        SLANG_ASSERT(converter->sourceLayout->byteStride == buffer->elementStride);
        submitAndWait();
        std::vector<uint8_t> deviceData(buffer->elementStride);
        if (SLANG_FAILED(device->readBuffer(
                buffer->resource,
                uint64_t(index) * buffer->elementStride,
                buffer->elementStride,
                deviceData.data())))
        {
            fail("HGVM RHI failed to read a structured-buffer element");
        }
        auto destinationBytes = static_cast<uint8_t*>(destination);
        for (const LayoutConversionOperation& operation : converter->operations)
        {
            SLANG_ASSERT(
                operation.sourceByteOffset <= deviceData.size() &&
                operation.sourceByteSize <= deviceData.size() - operation.sourceByteOffset);
            SLANG_ASSERT(
                operation.destinationByteOffset <= resultByteSize &&
                operation.destinationByteSize <= resultByteSize - operation.destinationByteOffset);
            copyScalarFromDevice(
                operation.scalarType,
                deviceData.data() + operation.sourceByteOffset,
                operation.sourceByteSize,
                destinationBytes + operation.destinationByteOffset,
                operation.destinationByteSize);
        }
    }

    /// Leases one fixed-size staging allocation from the persistent readback pool.
    ///
    /// Consider two outstanding async reads: each leases a distinct slot. Once either defer is
    /// waited or safely discarded, the next read reuses that slot without allocating an RHI
    /// resource. The pool grows only to the maximum number of simultaneously live reads.
    Index acquireReadbackBuffer()
    {
        retireCompletedSubmissions();
        reclaimCompletedReadbackBuffers();
        for (Index i = 0; i < readbackBufferPool.getCount(); ++i)
        {
            ReadbackBufferSlot& slot = readbackBufferPool[i];
            if (slot.state != ReadbackBufferState::Available)
                continue;
            slot.state = ReadbackBufferState::Leased;
            slot.completionFenceValue = 0;
            return i;
        }

        rhi::BufferDesc desc = {};
        desc.size = kAsyncReadbackBufferByteSize;
        desc.memoryType = rhi::MemoryType::ReadBack;
        desc.usage = rhi::BufferUsage::CopyDestination;
        desc.defaultState = rhi::ResourceState::CopyDestination;
        desc.label = "HGVM pooled readback";

        ReadbackBufferSlot slot;
        SLANG_HGVM_BEGIN_TIMING(eventTracker.get(), creationStart);
        slot.resource = device->createBuffer(desc);
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            trackBufferCreation(
                slot.resource.get(),
                desc,
                TrackedBufferPurpose::Readback,
                slot.resource ? SLANG_OK : SLANG_FAIL,
                creationStart));
        if (!slot.resource)
            fail("HGVM RHI failed to grow the deferred readback pool");
        slot.state = ReadbackBufferState::Leased;
        Index result = readbackBufferPool.getCount();
        readbackBufferPool.add(std::move(slot));
        return result;
    }

    /// Releases an unresolved defer's pool lease without reusing an in-flight allocation.
    ///
    /// For example, overwriting a `Defer<uint>` before `wait()` quarantines its slot until the
    /// recorded completion fence passes. A fence-less backend has already waited during submit,
    /// so its slot becomes available immediately.
    void discardDeferredReadback(DeferredReadbackObject& deferred)
    {
        if (deferred.readbackPoolSlotIndex < 0)
            return;
        SLANG_ASSERT(deferred.deviceState == this);
        SLANG_ASSERT(deferred.readbackPoolSlotIndex < readbackBufferPool.getCount());
        ReadbackBufferSlot& slot = readbackBufferPool[deferred.readbackPoolSlotIndex];
        SLANG_ASSERT(slot.state == ReadbackBufferState::Leased);
        slot.state = !completionFence || slot.completionFenceValue <= lastCompletedFenceValue
                         ? ReadbackBufferState::Available
                         : ReadbackBufferState::AwaitingCompletion;
        deferred.deviceState = nullptr;
        deferred.readbackPoolSlotIndex = -1;
    }

    /// Submits one buffer-element copy and returns immediately with a managed deferred object.
    ///
    /// Consider a dispatch followed by `asyncReadBack(buffer, 0)`: the dispatch and copy are
    /// finished into one command buffer and submitted here, but no host wait or map occurs. The
    /// pending batch retains independent RHI references for the encoded copy, while the returned
    /// object leases the pool slot needed by a later `wait()`.
    DeferredReadbackObject* beginAsyncBufferElementReadback(
        StructuredBufferObject* buffer,
        LayoutConverterObject* converter,
        uint32_t index,
        uint32_t resultByteSize)
    {
        if (index >= buffer->elementCount)
            fail("HGVM structured-buffer async readback is out of range");
        SLANG_ASSERT(converter && converter->sourceLayout && converter->destinationLayout);
        SLANG_ASSERT(converter->sourceLayout->byteStride == buffer->elementStride);
        SLANG_ASSERT(converter->destinationLayout->byteSize == resultByteSize);
        if (buffer->elementStride > kAsyncReadbackBufferByteSize)
            fail("HGVM async readback elements cannot exceed 64 bytes yet");

        Index poolSlotIndex = acquireReadbackBuffer();
        ReadbackBufferSlot& poolSlot = readbackBufferPool[poolSlotIndex];

        rhi::ICommandEncoder* encoder = getPendingCommandEncoder();
        encoder->copyBuffer(
            poolSlot.resource,
            0,
            buffer->resource,
            uint64_t(index) * buffer->elementStride,
            buffer->elementStride);
        pendingBatch.holds.bufferLeases.add(buffer);
        pendingBatch.holds.buffers.add(poolSlot.resource);
        finishDeviceOperation();

        // Async readback is a submission boundary: later host work can overlap this copy, while
        // `wait()` can depend on a command buffer that is already visible to the device.
        poolSlot.completionFenceValue = submitPendingCommands();

        DeferredReadbackObject* deferred =
            allocateHeapObject<DeferredReadbackObject>(SLANG_HGVM_TRACKER(eventTracker));
        deferred->deviceState = this;
        deferred->readbackPoolSlotIndex = poolSlotIndex;
        deferred->converter = converter;
        deferred->resultByteSize = resultByteSize;
        return deferred;
    }

    /// Resolves a deferred copy once and converts its captured device bytes on every wait.
    ///
    /// For example, two aliases of the same `Defer<uint>` share one staging-buffer map. Each
    /// `wait()` still applies the cached converter to its own destination slot, which leaves host
    /// aggregate padding untouched and avoids manufacturing a second cached host representation.
    void waitForDeferredReadback(
        DeferredReadbackObject* deferred,
        uint32_t resultByteSize,
        void* destination)
    {
        SLANG_ASSERT(deferred && deferred->converter);
        SLANG_ASSERT(deferred->resultByteSize == resultByteSize);
        LayoutConverterObject* converter = deferred->converter;
        SLANG_ASSERT(converter->sourceLayout && converter->destinationLayout);

        if (!deferred->resolved)
        {
            if (deferred->deviceState != this || deferred->readbackPoolSlotIndex < 0 ||
                deferred->readbackPoolSlotIndex >= readbackBufferPool.getCount())
            {
                fail("HGVM deferred readback has no pool slot");
            }
            ReadbackBufferSlot& poolSlot = readbackBufferPool[deferred->readbackPoolSlotIndex];
            SLANG_ASSERT(poolSlot.state == ReadbackBufferState::Leased);

            // The readback copy precedes later frame work on the same queue, but its defer owns
            // only this submission dependency. For example, if the copy signals fence seven and
            // the remaining frame signals fence eight, waiting here must leave fence eight in
            // flight so the host can begin preparing the next frame while that work executes.
            waitForSubmission(poolSlot.completionFenceValue);

            void* mappedData = nullptr;
            if (SLANG_FAILED(
                    device->mapBuffer(poolSlot.resource, rhi::CpuAccessMode::Read, &mappedData)) ||
                !mappedData)
            {
                fail("HGVM RHI failed to map a deferred readback");
            }

            deferred->deviceByteSize = converter->sourceLayout->byteStride;
            std::memcpy(deferred->deviceBytes, mappedData, deferred->deviceByteSize);
            if (SLANG_FAILED(device->unmapBuffer(poolSlot.resource)))
                fail("HGVM RHI failed to unmap a deferred readback");
            discardDeferredReadback(*deferred);
            deferred->resolved = true;
        }

        uint8_t* destinationBytes = static_cast<uint8_t*>(destination);
        for (const LayoutConversionOperation& operation : converter->operations)
        {
            SLANG_ASSERT(
                operation.sourceByteOffset <= deferred->deviceByteSize &&
                operation.sourceByteSize <= deferred->deviceByteSize - operation.sourceByteOffset);
            SLANG_ASSERT(
                operation.destinationByteOffset <= resultByteSize &&
                operation.destinationByteSize <= resultByteSize - operation.destinationByteOffset);
            copyScalarFromDevice(
                operation.scalarType,
                deferred->deviceBytes + operation.sourceByteOffset,
                operation.sourceByteSize,
                destinationBytes + operation.destinationByteOffset,
                operation.destinationByteSize);
        }
    }

    /// Executes one staged compute call through the cached converters for its arguments.
    ///
    /// Consider a struct containing POD fields and two textures: the converter's lazy resolution
    /// has already flattened its numeric and resource leaves, so each dispatch applies those cached
    /// rules directly to interpreter storage.
    void executeComputeCall(InterpreterState& interpreter, const StagedRecord::Header& header);
    void executeRayDispatch(InterpreterState& interpreter, const StagedRecord::Header& header);
    List<uint8_t> bindRayRecord(
        InterpreterState& interpreter,
        RayPipelineObject* pipeline,
        uint32_t entryPointIndex,
        ConstArrayView<StagedRecord::RayShaderArgument> arguments,
        HGBCRayStageRole role,
        Dictionary<HeapObject*, DescriptorAccess>& resources);
    ComPtr<rhi::IAccelerationStructure> buildNativeRayAccelerationStructure(
        rhi::ICommandEncoder* encoder,
        const rhi::AccelerationStructureBuildDesc& build,
        rhi::AccelerationStructureKind kind);
    void buildRayAccelerationStructure(
        InterpreterState& interpreter,
        const StagedRecord::BuildRayAccelerationStructure& record);
    ComPtr<rhi::IAccelerationStructure> realizeRayAccelerationStructure(
        RayAccelerationStructureObject* structure,
        uint32_t rayTypeCount);
    void executeGraphicsCall(InterpreterState& interpreter, const StagedRecord::Header& header);

    /// Retires completed submissions before a new invocation starts recording device work.
    ///
    /// Consider a persistent test server: invocation one may return while its final dispatch is
    /// still running. Invocation two performs one non-blocking fence query here and releases any
    /// batches that completed between calls without stalling the host.
    void beginInvocation() { retireCompletedSubmissions(); }

    /// Finishes and submits the current command batch without waiting for GPU completion.
    ///
    /// Consider three consecutive compute calls: this method produces one command buffer, signals
    /// one monotonically increasing fence value, and transfers all independent resource holds to
    /// the corresponding in-flight record. A fence-less backend waits conservatively instead.
    uint64_t submitPendingCommands()
    {
        if (!pendingBatch.encoder)
            return 0;

#if SLANG_ENABLE_HGVM_EVENT_TRACKING
        const uint32_t operationCount = pendingBatch.operationCount;
#endif
        ComPtr<rhi::ICommandBuffer> commandBuffer = pendingBatch.encoder->finish();
        pendingBatch.encoder = nullptr;
        pendingBatch.operationCount = 0;
        pendingRawTextureUploadBytes = 0;
        if (!commandBuffer)
        {
            SLANG_HGVM_TRACK(
                eventTracker.get(),
                trackCommandBatchDiscard(pendingBatch.trackingId, operationCount));
            pendingBatch.holds.clear();
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
            pendingBatch.executions.clear();
#endif
            fail("HGVM RHI failed to finish a command batch");
        }

        ensureCommandQueue();
        tryInitializeCompletionFence();

        InFlightSubmission submission;
        submission.commandBuffer = commandBuffer;
        submission.holds.swapWith(pendingBatch.holds);
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
        submission.executions = std::move(pendingBatch.executions);
#endif
        if (!completionFence)
        {
            SlangResult submitResult = commandQueue->submit(commandBuffer);
            SLANG_HGVM_TRACK(
                eventTracker.get(),
                trackCommandBufferSubmit(
                    pendingBatch.trackingId,
                    commandBuffer.get(),
                    operationCount,
                    0,
                    submitResult));
            if (SLANG_FAILED(submitResult) || SLANG_FAILED(commandQueue->waitOnHost()))
            {
                fail("HGVM RHI failed to submit and complete an unfenced command batch");
            }
            SLANG_HGVM_TRACK(eventTracker.get(), completeDeviceExecutions(submission.executions));
            return 0;
        }

        submission.completionFenceValue = nextCompletionFenceValue++;
        rhi::ICommandBuffer* commandBuffers[] = {commandBuffer};
        rhi::IFence* signalFences[] = {completionFence};
        uint64_t signalValues[] = {submission.completionFenceValue};
        rhi::SubmitDesc submitDesc = {};
        submitDesc.commandBuffers = commandBuffers;
        submitDesc.commandBufferCount = 1;
        submitDesc.signalFences = signalFences;
        submitDesc.signalFenceValues = signalValues;
        submitDesc.signalFenceCount = 1;
        SlangResult submitResult = commandQueue->submit(submitDesc);
        SLANG_HGVM_TRACK(
            eventTracker.get(),
            trackCommandBufferSubmit(
                pendingBatch.trackingId,
                commandBuffer.get(),
                operationCount,
                submission.completionFenceValue,
                submitResult));
        if (SLANG_FAILED(submitResult))
            fail("HGVM RHI failed to submit a command batch");

        uint64_t completionFenceValue = submission.completionFenceValue;
        inFlightSubmissions.add(std::move(submission));
        retireCompletedSubmissions();
        return completionFenceValue;
    }

    /// Submits pending commands and blocks until every earlier HGVM submission has completed.
    ///
    /// For example, `readBack(buffer, 0)` closes the encoder, waits for its fence value, releases
    /// all preceding in-flight resource holds, and only then lets RHI copy bytes to host memory.
    void submitAndWait()
    {
        submitPendingCommands();
        waitForSubmittedCommands();
    }

    /// Submits a normal invocation's final batch while allowing device execution to remain async.
    void completeInvocation() { submitPendingCommands(); }

    /// Drops an incomplete encoder and its independent resource holds after interpreter failure.
    ///
    /// For example, a binding error abandons the current encoder before stack cleanup unwinds
    /// program references. Previously submitted batches remain valid and are retired separately.
    void abandonPendingCommands()
    {
        if (pendingBatch.encoder)
            SLANG_HGVM_TRACK(
                eventTracker.get(),
                trackCommandBatchDiscard(pendingBatch.trackingId, pendingBatch.operationCount));
        pendingBatch.abandon();
        pendingRawTextureUploadBytes = 0;
    }

    /// Drains all device work before the persistent DeviceState and its RHI device are destroyed.
    ///
    /// The public interpreter destructor cannot report an RHI failure, so it invokes this method
    /// behind its existing exception boundary and treats shutdown as best-effort cleanup.
    void shutdown() { submitAndWait(); }

    void trimBufferPool()
    {
        submitAndWait();
        bufferPool->trim();
        if (device && SLANG_FAILED(device->trimMemory()))
            fail("HGVM RHI failed to reclaim unused buffer memory");
    }

    /// Records the successful end of one device operation and applies synchronization policy.
    ///
    /// A synchronous debugger dispatch submits and waits immediately. Normal execution keeps
    /// recording until a semantic boundary such as readback or function completion; operation
    /// count alone never creates an arbitrary submission boundary.
    void finishDeviceOperation()
    {
        ++pendingBatch.operationCount;
        if (synchronousGPUOperations)
            submitAndWait();
    }

private:
    // Lazy layout resolution is part of interpreter execution and may use the same reflection
    // helpers as device binding without exposing them as general DeviceState API surface.
    friend struct InterpreterState;

    /// Returns the persistent graphics queue used by all batches on this DeviceState.
    void ensureCommandQueue()
    {
        if (!device)
            fail("HGVM command recording requires an RHI device");
        if (!commandQueue)
            commandQueue = device->getQueue(rhi::QueueType::Graphics);
        if (!commandQueue)
            fail("HGVM RHI device has no graphics command queue");
    }

    /// Tries once to create the timeline fence used for asynchronous submission retirement.
    ///
    /// Vulkan, D3D12, Metal, CPU, and WebGPU provide this path. An older or restricted backend that
    /// reports the fence unavailable falls back to a queue wait after every submitted batch.
    void tryInitializeCompletionFence()
    {
        if (attemptedCompletionFenceCreation)
            return;
        attemptedCompletionFenceCreation = true;
        rhi::FenceDesc fenceDesc = {};
        fenceDesc.initialValue = 0;
        fenceDesc.label = "HGVM submission completion";
        completionFence = device->createFence(fenceDesc);
    }

    /// Makes discarded readback slots reusable after their device copies have completed.
    ///
    /// Consider a defer dropped at fence seven: its slot remains `AwaitingCompletion` while the
    /// completed value is six, then this method returns it to the free pool when seven is observed.
    void reclaimCompletedReadbackBuffers()
    {
        for (ReadbackBufferSlot& slot : readbackBufferPool)
        {
            if (slot.state == ReadbackBufferState::AwaitingCompletion &&
                (!completionFence || slot.completionFenceValue <= lastCompletedFenceValue))
            {
                slot.state = ReadbackBufferState::Available;
                slot.completionFenceValue = 0;
            }
        }
    }

    /// Releases the FIFO submission prefix known to have completed through one fence value.
    ///
    /// Consider in-flight fences `{6, 7, 8}` and a completed value of seven: this releases the
    /// first two command buffers and their independent resource holds while deliberately retaining
    /// fence eight. Readback waits and non-blocking fence queries share this exact retirement path.
    void retireSubmissionsThrough(uint64_t completedValue)
    {
        lastCompletedFenceValue = std::max(lastCompletedFenceValue, completedValue);
        Index completedCount = 0;
        while (completedCount < inFlightSubmissions.getCount() &&
               inFlightSubmissions[completedCount].completionFenceValue <= completedValue)
        {
            SLANG_HGVM_TRACK(
                eventTracker.get(),
                completeDeviceExecutions(inFlightSubmissions[completedCount].executions));
            inFlightSubmissions[completedCount].release();
            ++completedCount;
        }
        if (completedCount)
            inFlightSubmissions.removeRange(0, completedCount);
        reclaimCompletedReadbackBuffers();
    }

    /// Releases the completed prefix of the FIFO submission list without waiting on the host.
    void retireCompletedSubmissions()
    {
        if (!completionFence || !inFlightSubmissions.getCount())
            return;

        uint64_t completedValue = 0;
        if (SLANG_FAILED(completionFence->getCurrentValue(&completedValue)))
            fail("HGVM RHI failed to query command completion");
        retireSubmissionsThrough(completedValue);
    }

    /// Waits only for one already submitted batch and retains every later submission.
    ///
    /// For example, a deferred copy at fence seven does not depend on the render work at fence
    /// eight. A backend without timeline fences completed the copy synchronously during submit and
    /// records zero, so this method has no remaining work in that case.
    void waitForSubmission(uint64_t targetValue)
    {
        if (!targetValue || targetValue <= lastCompletedFenceValue)
            return;
        SLANG_ASSERT(device && completionFence);
        rhi::IFence* fences[] = {completionFence};
        uint64_t fenceValues[] = {targetValue};
        if (SLANG_FAILED(
                device->waitForFences(1, fences, fenceValues, true, rhi::kTimeoutInfinite)))
        {
            fail("HGVM RHI failed to wait for command completion");
        }
        retireSubmissionsThrough(targetValue);
    }

    /// Waits for the most recent submitted fence and releases every preceding submission.
    void waitForSubmittedCommands()
    {
        if (!inFlightSubmissions.getCount())
            return;
        uint64_t targetValue = inFlightSubmissions.getLast().completionFenceValue;
        waitForSubmission(targetValue);
        SLANG_ASSERT(!inFlightSubmissions.getCount());
    }

    /// Returns the graphics encoder for the current batch and separates dependent operations.
    ///
    /// Consider one shader writing a UAV that the next shader reads: the second call inserts one
    /// coarse global barrier before opening its pass. Staging and execution do not inspect resource
    /// aliases; a future resource-aware scheduler can replace this conservative operation.
    rhi::ICommandEncoder* getPendingCommandEncoder()
    {
        ensureCommandQueue();
        if (!pendingBatch.encoder)
        {
            pendingBatch.encoder = commandQueue->createCommandEncoder();
            if (pendingBatch.encoder)
                SLANG_HGVM_TRACK(
                    eventTracker.get(),
                    trackCommandBatchBegin(pendingBatch.trackingId));
        }
        if (!pendingBatch.encoder)
            fail("HGVM RHI failed to create a command encoder");
        if (pendingBatch.operationCount)
            pendingBatch.encoder->globalBarrier();
        return pendingBatch.encoder;
    }

    /// Returns whether one reflected shader parameter is supplied by GPU execution itself.
    static bool isSystemValueParameter(slang::VariableLayoutReflection* parameter)
    {
        if (!parameter)
            return false;
        const char* semantic = parameter->getSemanticName();
        if (!semantic || !semantic[0] || !semantic[1] || !semantic[2])
            return false;
        char first = char(semantic[0] | 0x20);
        char second = char(semantic[1] | 0x20);
        return (first == 's' && second == 'v' && semantic[2] == '_') ||
               (first == 'n' && second == 'v' && semantic[2] == '_');
    }

    /// Identifies host-supplied storage when validating ray records. This must not be used to
    /// enumerate host arguments: empty uniform parameters have identities but consume no storage.
    static bool shouldBindParameter(slang::VariableLayoutReflection* parameter)
    {
        if (!parameter || isSystemValueParameter(parameter))
            return false;
        slang::ParameterCategory category = parameter->getCategory();
        return category != slang::ParameterCategory::None &&
               category != slang::ParameterCategory::VaryingInput &&
               category != slang::ParameterCategory::VaryingOutput &&
               category != slang::ParameterCategory::RayPayload &&
               category != slang::ParameterCategory::HitAttributes &&
               category != slang::ParameterCategory::CallablePayload;
    }

    /// Binds one staged argument by applying its converter's flattened leaf rules.
    ///
    /// The staged record supplies the argument's base address. Each cached rule supplies the source
    /// dereferences, byte offset, type information, and reflected cursor path for one leaf.
    void bindShaderArgument(
        InterpreterState& interpreter,
        const rhi::ShaderCursor& cursor,
        const StagedRecord::DispatchArgument& argument,
        LayoutConverterObject* converter);

    /// Binds one scalar, vector, matrix, resource, or descriptor leaf selected by a cached rule.
    void bindComputeLeaf(
        InterpreterState& interpreter,
        const rhi::ShaderCursor& cursor,
        Address sourceAddress,
        uint32_t sourceByteOffset,
        const ShaderBindingOperation& operation);

    /// Returns the cached physical descriptor for one managed graph buffer.
    rhi::DescriptorHandle getBufferDescriptorHandle(
        StructuredBufferObject* buffer,
        DescriptorAccess access);
    /// Returns a cached texture descriptor and retains its view through GPU completion.
    rhi::DescriptorHandle getTextureDescriptorHandle(
        Texture2DObject* texture,
        DescriptorAccess access);
    rhi::DescriptorHandle getAccelerationStructureDescriptorHandle(
        RayAccelerationStructureObject* structure);
    void prepareDescriptorResource(
        InterpreterState& interpreter,
        Address sourceAddress,
        const ShaderBindingOperation& operation,
        rhi::ICommandEncoder* encoder,
        Dictionary<HeapObject*, DescriptorAccess>& resources);

    /// Resolves one flattened binding operation's source after any pointer dereferences.
    Address resolveShaderBindingSourceAddress(
        InterpreterState& interpreter,
        Address argumentAddress,
        const ShaderBindingOperation& operation,
        LayoutConverterObject* converter);

    /// Allocates descriptor-backed handles and declares their states before a compute pass begins.
    void prepareDescriptorResources(
        InterpreterState& interpreter,
        ConstArrayView<StagedRecord::DispatchArgument> arguments,
        rhi::ICommandEncoder* encoder);

    /// Flattens aggregate source and cursor traversal while a host-to-shader converter resolves.
    void appendShaderBindingOperations(
        InterpreterState& interpreter,
        slang::TypeLayoutReflection* targetLayout,
        Address typeInfoAddress,
        uint32_t sourceByteOffset,
        std::vector<uint32_t>& sourceDereferenceOffsets,
        std::vector<uint32_t>& cursorPath,
        LayoutConverterObject* converter);

    /// Flattens one reflected device/host conversion into reusable scalar operations.
    void appendLayoutConversionOperations(
        InterpreterState& interpreter,
        slang::TypeLayoutReflection* sourceLayout,
        Address typeInfoAddress,
        uint32_t sourceByteOffset,
        uint32_t destinationByteOffset,
        LayoutConversionDirection direction,
        List<LayoutConversionOperation>& operations);
};

/// Returns an unresolved readback lease when its last program reference disappears.
///
/// Consider `asyncReadBack(buffer, 0);` with no saved result: stack cleanup destroys the defer and
/// reaches this callback. `DeviceState` either frees the pool slot immediately or quarantines it
/// until the already submitted copy completes.
DeferredReadbackObject::~DeferredReadbackObject()
{
    if (deviceState)
        deviceState->discardDeferredReadback(*this);
}

/// Tracks the mutable traversal state for one HGVM invocation.
///
/// Consider hello world's root `main`: `functionAddress` names its immutable function record while
/// `programCounter` advances independently through `OpPrint`, `OpReturn`, and its exit block.
struct InterpreterState
{
    /// Active program and its instance
    RefPtr<StagedProgram> program;
    RefPtr<ProgramInstance> programInstance;

    //
    // Interpreter runtime state
    //

    /// Linear stack for frame-local storage. Function calls just bump the pointer
    List<uint8_t, AlignedAllocator<kStorageBaseAlignment>> stack;

    /// Offset for translating frame-relative addresses into absolute offsets.
    uint32_t currentFrameOffset = 0;

    /// Names the current function record for instruction-relative addresses, returns, and failure
    /// unwinding. Calls save and restore it in `FrameHeader`.
    Address functionAddress;

    /// Currently executing instruction.
    uint32_t programCounter = 0;

    /// Describes whether the current invocation can execute another staged record.
    ///
    /// For example, `FinishFrameExit` changes `Running` to `Completed`. A malformed staged address
    /// throws immediately and is translated to `SLANG_FAIL` at the public interpreter boundary.
    enum class Status : uint8_t
    {
        Ready,
        Running,
        Completed,
    };
    Status status = Status::Ready;
    IStepControl* stepControl = nullptr;
    IDebugHandler* debugHandler = nullptr;
    StepAction stepAction = StepAction::Step;
    bool cancelled = false;

    struct DebugCall
    {
        DebugCallId id;
        uint32_t frameOffset;
        uint32_t function;
    };
    struct DebugMemoryRegion
    {
        const uint8_t* base;
        size_t size;
    };
    List<DebugCall> debugCalls;
    uint64_t* nextDebugCallId = nullptr;
    bool debugStopped = false;
    bool debugHeapReady = false;
    HashSet<HeapObject*> debugObjects;
    List<DebugMemoryRegion> debugMemory;

    void enterDebugCall(DebugInstructionAddress site, Address callee)
    {
        DebugCallId id{++*nextDebugCallId};
        debugCalls.add({id, currentFrameOffset, uint32_t(callee.getUnsignedPayload())});
        debugHandler->onFunctionEnter(site, id);
    }

    const DebugCall* findDebugCall(DebugCallId id) const
    {
        if (debugStopped)
            for (const auto& call : debugCalls)
                if (call.id.value == id.value)
                    return &call;
        return nullptr;
    }

    /// Discovers live objects from real owning slots, never from non-owning debug snapshots.
    /// This runs lazily at a stop, so ordinary allocation and release need no debugger callbacks.
    void collectDebugHeap()
    {
        if (debugHeapReady)
            return;
        debugHeapReady = true;
        List<HeapObject*> pending;
        auto visit = [&](const uint8_t* bytes)
        {
            Address address;
            std::memcpy(&address, bytes, sizeof(address));
            if (address && debugObjects.add(address.getObject()))
                pending.add(address.getObject());
        };
        for (const auto& call : debugCalls)
            if (auto roots = program->debugOwnedSlots.tryGetValue(call.function))
                for (auto root : *roots)
                    visit(stack.getBuffer() + call.frameOffset + root.getUnsignedPayload());
        for (Index i = 0; i < pending.getCount(); ++i)
        {
            auto object = pending[i];
            if (object->kind == HeapObjectKind::Class)
            {
                auto instance = static_cast<ClassObject*>(object);
                const auto& layout = program->staging->findType(instance->typeAddress);
                auto bytes = reinterpret_cast<const uint8_t*>(object);
                debugMemory.add({bytes, layout.allocationByteSize});
                for (auto offset : layout.objectManagedLeafOffsets)
                    visit(bytes + offset);
            }
            else if (object->kind == HeapObjectKind::RuntimeArray)
            {
                auto array = static_cast<RuntimeArrayObject*>(object);
                debugMemory.add({array->payload, array->payloadByteSize});
                for (uint32_t element = 0; element < array->elementCount; ++element)
                    for (auto offset : array->elementManagedLeafOffsets)
                        visit(array->payload + size_t(element) * array->elementByteStride + offset);
            }
            else if (object->kind == HeapObjectKind::Box)
            {
                auto box = static_cast<BoxObject*>(object);
                debugMemory.add({box->payload, box->payloadByteSize});
                for (uint32_t offset : box->managedLeafOffsets)
                    visit(box->payload + offset);
            }
            else if (object->kind == HeapObjectKind::String)
            {
                const auto& bytes = static_cast<StringObject*>(object)->bytes;
                debugMemory.add({reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size()});
            }
        }
    }

    bool readDebugMemory(
        DebugCallId id,
        Address address,
        uint32_t offset,
        void* data,
        uint32_t size)
    {
        const auto* call = findDebugCall(id);
        if (!call)
            return false;
        // Snapshot bytes may be garbage. Inspect the tag without Address::getKind's trusted-IR
        // assertions, then validate the entire range before resolving a native pointer.
        uint64_t baseOffset = address.getUnsignedPayload();
        const uint8_t* base = nullptr;
        uint64_t extent = 0;
        switch (address.value >> kAddressTagShift)
        {
        case uint64_t(AddressKind::FrameRelative):
            base = stack.getBuffer() + call->frameOffset;
            extent = readRecord<StagedRecord::Function>(call->function).frameSize;
            break;
        case uint64_t(AddressKind::FrameAbsolute):
            base = stack.getBuffer();
            extent = uint64_t(stack.getCount());
            break;
        case uint64_t(AddressKind::InstructionAbsolute):
            baseOffset *= sizeof(uint32_t);
            base = program->staging->storage.getBuffer();
            extent = program->staging->storage.getByteSize();
            break;
        case uint64_t(AddressKind::Heap):
            collectDebugHeap();
            for (const auto& region : debugMemory)
            {
                uint64_t start = uint64_t(reinterpret_cast<uintptr_t>(region.base));
                if (baseOffset >= start && baseOffset - start <= region.size &&
                    uint64_t(offset) + size <= region.size - (baseOffset - start))
                {
                    std::memcpy(data, region.base + size_t(baseOffset - start) + offset, size);
                    return true;
                }
            }
            return false;
        default:
            return false;
        }
        if (baseOffset > extent || uint64_t(offset) + size > extent - baseOffset)
            return false;
        std::memcpy(data, base + size_t(baseOffset) + offset, size);
        return true;
    }

    HeapObject* findDebugObject(DebugCallId call, Address address)
    {
        if (!findDebugCall(call) || !address.value || (address.value >> kAddressTagShift) != 0)
            return nullptr;
        collectDebugHeap();
        auto object = reinterpret_cast<HeapObject*>(uintptr_t(address.value));
        return debugObjects.contains(object) ? object : nullptr;
    }

    /// Resolves a buffer only while its owning invocation is stopped and its handle is live.
    DeviceState::StructuredBufferObject* findDebugBuffer(DebugCallId call, Address address)
    {
        auto object = findDebugObject(call, address);
        return object && object->kind == HeapObjectKind::StructuredBuffer
                   ? static_cast<DeviceState::StructuredBufferObject*>(object)
                   : nullptr;
    }

    /// Finds an active-program type ID for an object created by any program in this VM.
    /// Debug descriptors are program-local even though runtime type addresses are shared.
    bool findLocalType(Address typeAddress, HGBCId& type) const
    {
        for (const auto& binding : program->typeBindings)
        {
            if (binding.second.value == typeAddress.value)
            {
                type = binding.first;
                return true;
            }
        }
        return false;
    }

    /// Returns logical element metadata and both strides without submitting device work.
    bool getDebugBufferInfo(DebugCallId call, Address address, DebugBufferInfo& info)
    {
        auto buffer = findDebugBuffer(call, address);
        if (!buffer)
            return false;
        HGBCId elementType;
        if (!findLocalType(buffer->elementLayout->hostTypeInfoAddress, elementType))
            return false;
        const auto& type = readInstructionRecord<StagedRecord::TypeInfo>(
            buffer->elementLayout->hostTypeInfoAddress);
        SLANG_RELEASE_ASSERT(buffer->elementCount <= UINT32_MAX);
        uint64_t hostStride =
            (uint64_t(type.byteSize) + type.byteAlignment - 1) & ~uint64_t(type.byteAlignment - 1);
        SLANG_RELEASE_ASSERT(hostStride <= UINT32_MAX);
        info = {
            elementType,
            uint32_t(buffer->elementCount),
            uint32_t(hostStride),
            buffer->elementStride};
        return true;
    }

    // Conversion plans belong to this invocation's constant layouts, and are built only on a
    // debugger read. Ordinary execution never accesses this dictionary.
    Dictionary<TypeLayoutObject*, List<LayoutConversionOperation>> debugBufferConverters;

    /// Reads a bounded range using the allocation's device layout and canonical host offsets.
    /// For example, a buffer of structs containing bool arrays must compact each device bool
    /// before the ordinary debugger field reader can use the host field offsets.
    void readDebugBufferElements(
        DebugCallId call,
        Address address,
        uint32_t start,
        uint32_t count,
        List<uint8_t>& bytes)
    {
        DebugBufferInfo info;
        if (!getDebugBufferInfo(call, address, info))
            fail("Cannot read this buffer (null, no longer live, or execution is not stopped).");
        if (start > info.elementCount || count > info.elementCount - start)
            fail("Buffer index out of bounds.");
        uint64_t byteCount = uint64_t(count) * info.hostStride;
        if (byteCount > kMaxDebugBufferBytes || info.deviceStride > kMaxDebugBufferBytes)
            fail("Buffer read exceeds the 64 MiB debugger limit; request a smaller range.");
        bytes.clear();
        if (!count)
            return;
        auto buffer = findDebugBuffer(call, address);
        if (!rhi::is_set(buffer->resource->getDesc().usage, rhi::BufferUsage::CopySource))
            fail("Buffer inspection requires CopySource usage.");
        auto layout = buffer->elementLayout;
        const auto& hostType =
            readInstructionRecord<StagedRecord::TypeInfo>(layout->hostTypeInfoAddress);
        if (hostType.managedLeafCount)
            fail("Buffer inspection does not support elements containing managed objects.");
        auto operations = debugBufferConverters.tryGetValue(layout);
        if (!operations)
        {
            List<LayoutConversionOperation> plan;
            deviceState->appendLayoutConversionOperations(
                *this,
                layout->reflectedLayout,
                layout->hostTypeInfoAddress,
                0,
                0,
                LayoutConversionDirection::DeviceToHost,
                plan);
            debugBufferConverters.add(layout, std::move(plan));
            operations = debugBufferConverters.tryGetValue(layout);
        }
        bytes.setCount(Index(byteCount));
        if (byteCount)
            std::memset(bytes.getBuffer(), 0, size_t(byteCount));
        // A page uses one transfer; unusually large requests use bounded temporary storage.
        uint32_t chunkCapacity = Math::Max(1u, (1024u * 1024u) / info.deviceStride);
        List<uint8_t> deviceBytes;
        for (uint32_t first = 0; first < count;)
        {
            uint32_t chunkCount = Math::Min(count - first, chunkCapacity);
            deviceBytes.setCount(Index(uint64_t(chunkCount) * info.deviceStride));
            deviceState->readBufferBytes(
                buffer,
                (uint64_t(start) + first) * info.deviceStride,
                {deviceBytes.getBuffer(), size_t(deviceBytes.getCount())});
            for (uint32_t i = 0; i < chunkCount; ++i)
                for (const auto& operation : *operations)
                {
                    SLANG_RELEASE_ASSERT(
                        uint64_t(operation.sourceByteOffset) + operation.sourceByteSize <=
                            info.deviceStride &&
                        uint64_t(operation.destinationByteOffset) + operation.destinationByteSize <=
                            info.hostStride);
                    copyScalarFromDevice(
                        operation.scalarType,
                        deviceBytes.getBuffer() + uint64_t(i) * info.deviceStride +
                            operation.sourceByteOffset,
                        operation.sourceByteSize,
                        bytes.getBuffer() + uint64_t(first + i) * info.hostStride +
                            operation.destinationByteOffset,
                        operation.destinationByteSize);
                }
            first += chunkCount;
        }
    }

    void executeDebugValue(const StagedRecord::Header& header)
    {
        const auto& record = readRecord<StagedRecord::DebugValue>(programCounter);
        uint64_t offset = record.staticOffset;
        for (const auto& element : record.getElements())
        {
            uint32_t index = readValue<uint32_t>(element.index);
            if (index >= element.count)
            {
                // A debug snapshot must not write outside its slot, even for an invalid source
                // index. The corresponding ordinary access owns program diagnostics.
                advance(header);
                return;
            }
            offset += uint64_t(index) * element.stride;
        }
        SLANG_RELEASE_ASSERT(offset + record.byteSize <= record.destinationSize);
        copyBytes(
            header.resultAddress.addByteOffset(uint32_t(offset)),
            record.source,
            record.byteSize);
        advance(header);
    }

    /// Set when interpreter fails when unwinding a stack frame to
    /// handle an error.
    bool isUnwindingFailure = false;

    /// Latest failure message.
    String failureMessage;

    //
    // Handlers for non-simple operations
    //

    /// Handles OpPrint
    RefPtr<IHGVMOutputSink> outputSink;

    /// Borrows the persistent platform service used by staged window and surface operations.
    IHGVMHostServices* hostServices = nullptr;

    /// Supplies file-backed byte streams without transferring host-service ownership.
    ComPtr<ISlangFileSystem> fileSystem;

    /// Resolves relative stream paths for this interpreter invocation.
    String workingDirectory;

    /// Borrows the interpreter-wide device state used by asynchronous command submissions.
    ///
    /// For example, the root invocation can return after submitting a compute batch while the
    /// persistent `InterpreterCore::Impl` keeps its resources alive until the completion fence
    /// signals.
    DeviceState* deviceState = nullptr;


    /// Converts one trusted logical address plus a byte displacement into a physical pointer.
    ///
    /// Consider a struct stored at an immutable instruction address: resolving offset four and
    /// size four reaches its second scalar field even though instruction addresses themselves are
    /// word-based. Debug assertions diagnose stager/runtime invariant violations, while release
    /// builds reduce successful resolution to address-space selection and offset calculation.
    template<typename T>
    AddressRef<T> resolveAddressAt(Address address, uint32_t relativeByteOffset, uint32_t byteSize)
    {
        size_t byteOffset = 0;
        const void* readBase = nullptr;
        void* writeBase = nullptr;
        size_t storageSize = 0;

        AddressKind addressKind = address.getKind();
        switch (addressKind)
        {
        case AddressKind::Heap:
            writeBase = address.getHeapPointer();
            readBase = writeBase;
            storageSize = std::numeric_limits<size_t>::max();
            break;
        case AddressKind::FrameRelative:
            byteOffset = size_t(currentFrameOffset) + size_t(address.getUnsignedPayload());
            readBase = stack.getBuffer();
            writeBase = stack.getBuffer();
            storageSize = size_t(stack.getCount());
            break;
        case AddressKind::FrameAbsolute:
            byteOffset = size_t(address.getUnsignedPayload());
            readBase = stack.getBuffer();
            writeBase = stack.getBuffer();
            storageSize = size_t(stack.getCount());
            break;
        case AddressKind::InstructionRelative:
        case AddressKind::InstructionAbsolute:
            {
                uint32_t wordOffset = 0;
                if (addressKind == AddressKind::InstructionRelative)
                {
                    SLANG_ASSERT(functionAddress.getKind() == AddressKind::InstructionAbsolute);
                    int64_t resolved = int64_t(functionAddress.getUnsignedPayload()) +
                                       address.getInstructionDisplacement();
                    SLANG_ASSERT(resolved >= 0 && resolved < kInstructionPayloadLimit);
                    wordOffset = uint32_t(resolved);
                }
                else
                {
                    wordOffset = uint32_t(address.getUnsignedPayload());
                }
                byteOffset = size_t(wordOffset) * sizeof(uint32_t);
                readBase = program->staging->storage.getBuffer();
                storageSize = size_t(program->staging->storage.getWordCount()) * sizeof(uint32_t);
                if constexpr (!std::is_const_v<T>)
                    SLANG_ASSERT(!"HGVM instructions cannot write into the instruction stack");
                break;
            }
        case AddressKind::ConstHeap:
            {
                SLANG_ASSERT(programInstance);
                byteOffset = size_t(address.getUnsignedPayload());
                readBase = programInstance->constHeap.getBuffer();
                storageSize = size_t(programInstance->constHeap.getCount());
                if constexpr (!std::is_const_v<T>)
                    SLANG_ASSERT(!"HGVM program operations cannot write into the const heap");
                break;
            }
        case AddressKind::Invalid:
        case AddressKind::Forbidden:
            SLANG_ASSERT(!"HGVM attempted to resolve an invalid address");
            break;
        }

        SLANG_ASSERT(byteOffset <= storageSize);
        SLANG_ASSERT(relativeByteOffset <= storageSize - byteOffset);
        byteOffset += relativeByteOffset;
        SLANG_ASSERT(byteOffset <= storageSize && byteSize <= storageSize - byteOffset);

        AddressRef<T> result;
        if constexpr (std::is_const_v<T>)
        {
            result.pointer =
                reinterpret_cast<T*>(static_cast<const uint8_t*>(readBase) + byteOffset);
        }
        else
        {
            result.pointer = reinterpret_cast<T*>(static_cast<uint8_t*>(writeBase) + byteOffset);
        }
        SLANG_ASSERT(
            reinterpret_cast<uintptr_t>(result.pointer) % alignof(std::remove_const_t<T>) == 0);
        result.byteSize = byteSize;
        return result;
    }

    /// Converts one trusted logical address into a physical pointer for an explicit byte extent.
    ///
    /// For example, `resolveAddress<const uint8_t>(textAddress, 5)` returns five staged string
    /// bytes. This common zero-displacement form shares the same address-space bottleneck as
    /// aggregate field and array-element resolution.
    template<typename T>
    AddressRef<T> resolveAddress(Address address, uint32_t byteSize)
    {
        return resolveAddressAt<T>(address, 0, byteSize);
    }

    /// Loads one fixed-width interpreter value through the common address bottleneck.
    ///
    /// For example, reading a two-byte `Float16Value` copies exactly two bytes from either
    /// immutable literal storage or the current frame without asking the storage for its type.
    template<typename T>
    T readValue(Address address)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        AddressRef<const T> source = resolveAddress<const T>(address, uint32_t(sizeof(T)));
        T result;
        std::memcpy(&result, source.get(), sizeof(T));
        return result;
    }

    /// Loads one fixed-width interpreter value at a byte displacement from a logical address.
    ///
    /// Consider the second field of `{ bool enabled; uint value; }`: its staged source offset is
    /// four, so `readValueAt<uint32_t>(base, 4)` loads it without manufacturing a second logical
    /// address or assuming that the base resides in the frame stack.
    template<typename T>
    T readValueAt(Address address, uint32_t relativeByteOffset)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        AddressRef<const uint8_t> source =
            resolveAddressAt<const uint8_t>(address, relativeByteOffset, uint32_t(sizeof(T)));
        T result;
        std::memcpy(&result, source.get(), sizeof(T));
        return result;
    }

    /// Stores one fixed-width result through the common writable-address bottleneck.
    ///
    /// For example, an `OpAdd Float64` will copy eight result bytes into its frame-relative result
    /// address; debug builds assert if the trusted stager ever supplies read-only storage.
    template<typename T>
    void writeValue(Address address, const T& value)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        AddressRef<T> destination = resolveAddress<T>(address, uint32_t(sizeof(T)));
        std::memcpy(destination.get(), &value, sizeof(T));
    }

    /// Stores one fixed-width value at a staged byte displacement from an aggregate base.
    ///
    /// Consider the `key` field of an input event: staging embeds its struct offset, and execution
    /// writes the four-byte host key code here without constructing another logical address.
    template<typename T>
    void writeValueAt(Address address, uint32_t relativeByteOffset, const T& value)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        AddressRef<uint8_t> destination =
            resolveAddressAt<uint8_t>(address, relativeByteOffset, uint32_t(sizeof(T)));
        std::memcpy(destination.get(), &value, sizeof(T));
    }

    /// Copies an explicit byte extent without inferring a value size from either address.
    ///
    /// For example, a two-byte Float16 `OpStore` and a future eight-byte scalar store use the same
    /// helper with different staged `byteSize` operands, including across address spaces.
    void copyBytes(Address destinationAddress, Address sourceAddress, uint32_t byteSize)
    {
        AddressRef<const uint8_t> source = resolveAddress<const uint8_t>(sourceAddress, byteSize);
        AddressRef<uint8_t> destination = resolveAddress<uint8_t>(destinationAddress, byteSize);
        std::memcpy(destination.get(), source.get(), byteSize);
    }

    /// Reads a heap handle and returns its already selected concrete interpreter object.
    ///
    /// For example, `readHeapObject<DeviceState::StructuredBufferObject>(address)` derives
    /// `StructuredBuffer`
    /// from the wrapper's `kKind`. A caller therefore cannot repeat—and accidentally mismatch—
    /// the C++ type and runtime tag.
    template<typename T>
    T* readHeapObject(Address address)
    {
        return readHeapObjectAt<T>(address, 0);
    }

    /// Reads a heap handle at a byte displacement and derives its runtime tag from `T`.
    ///
    /// Consider a texture stored after an ordinary scalar in a struct: the recursive binder reads
    /// the texture's heap `Address` at its staged field offset, while this helper still proves in a
    /// debug build that the selected wrapper is `DeviceState::Texture2DObject`.
    template<typename T>
    T* readHeapObjectAt(Address address, uint32_t relativeByteOffset)
    {
        static_assert(std::is_base_of_v<HeapObject, T>);
        Address heapAddress = readValueAt<Address>(address, relativeByteOffset);
        HeapObject* object = heapAddress.getObject();
        SLANG_ASSERT(object && object->kind == T::kKind);
        return static_cast<T*>(object);
    }

    /// Checks a direct class handle and its staged nominal type before object-relative field
    /// access.
    void validateClassHandle(Address handle, Address expectedTypeAddress)
    {
        if (!handle)
            fail("HGVM class field access through a null object");
        if (handle.getKind() != AddressKind::Heap)
            fail("HGVM class value does not contain a heap address");
        HeapObject* object = handle.getObject();
        if (!object || object->kind != HeapObjectKind::Class)
            fail("HGVM class value contains a different heap-object kind");
        ClassObject* classObject = static_cast<ClassObject*>(object);
        if (classObject->typeAddress.value != expectedTypeAddress.value)
            fail("HGVM class value contains a different class type");
    }

    /// Resolves a runtime-array handle and checks its staged element specialization.
    RuntimeArrayObject* getRuntimeArrayObject(Address handle, Address expectedTypeAddress)
    {
        if (!handle)
            fail("HGVM runtime array access through a null object");
        if (handle.getKind() != AddressKind::Heap)
            fail("HGVM runtime array value does not contain a heap address");
        HeapObject* object = handle.getObject();
        if (!object || object->kind != HeapObjectKind::RuntimeArray)
            fail("HGVM runtime array value contains a different heap-object kind");
        RuntimeArrayObject* arrayObject = static_cast<RuntimeArrayObject*>(object);
        if (arrayObject->typeAddress.value != expectedTypeAddress.value)
            fail("HGVM runtime array value contains a different element type");
        return arrayObject;
    }

    /// Reads one immutable staged byte range as a Slang `String`.
    ///
    /// For example, lazy entry-point realization converts the staged `"writeValue"` literal into
    /// the null-terminated string expected by the Slang reflection API.
    String readLiteralString(Address address, uint32_t byteSize)
    {
        AddressRef<const char> bytes = resolveAddress<const char>(address, byteSize);
        return String(UnownedStringSlice(bytes.get(), bytes.get() + byteSize));
    }

    /// Resolves either branch of the fixed-width source-language String representation.
    ///
    /// Consider a helper receiving `"literal"` on one call and `format("{}", 3)` on another. The
    /// first value resolves immutable instruction bytes; the second borrows its managed object's
    /// immutable `std::string`. Call execution copied the same 24-byte value in both cases.
    std::string_view readStringView(Address address)
    {
        StringValue value = readValue<StringValue>(address);
        if (value.owner)
        {
            HeapObject* baseObject = value.owner.getObject();
            SLANG_ASSERT(baseObject && baseObject->kind == StringObject::kKind);
            StringObject* object = static_cast<StringObject*>(baseObject);
            SLANG_ASSERT(object->bytes.size() == value.byteCount);
            return std::string_view(object->bytes.data(), object->bytes.size());
        }

        if (!value.byteCount)
            return {};
        SLANG_ASSERT(value.literalBytes.getKind() == AddressKind::InstructionAbsolute);
        AddressRef<const char> bytes =
            resolveAddress<const char>(value.literalBytes, value.byteCount);
        return std::string_view(bytes.get(), value.byteCount);
    }

    /// Resolves a const address directly to its concrete in-place object and realizes it if needed.
    ///
    /// For example, `readConstObject<ComputeDispatcherObject>(address)` performs one offset from
    /// the const-arena base to the dispatcher itself. The `const_cast` is confined to this lazy
    /// path, which is the only interpreter component allowed to mutate const storage.
    template<typename T>
    T* readConstObject(Address address)
    {
        static_assert(std::is_base_of_v<LazyObject, T>);
        SLANG_ASSERT(address.getKind() == AddressKind::ConstHeap);
        AddressRef<const T> storage = resolveAddress<const T>(address, uint32_t(sizeof(T)));
        T* object = const_cast<T*>(storage.get());
        SLANG_ASSERT(object->kind == T::kKind);
        ensureLazyObject(object);
        return object;
    }

    /// Loads one embedded module from its staged name, binary, and module dependencies.
    void evaluateSlangModule(SlangModuleObject* object, const StagedRecord::LazyObject& record)
    {
        object->dependencies.clear();
        for (Address dependencyAddress : record.getDependencies())
        {
            SlangModuleObject* dependency = readConstObject<SlangModuleObject>(dependencyAddress);
            object->dependencies.add(dependency);
        }

        String name = readLiteralString(record.firstOperandAddress, record.firstOperandByteSize);
        AddressRef<const uint8_t> binary = resolveAddress<const uint8_t>(
            record.secondOperandAddress,
            record.secondOperandByteSize);
        deviceState->loadSlangModule(
            object,
            programInstance->slangSession,
            name,
            binary.get(),
            record.secondOperandByteSize);
    }

    /// Returns the cached reflection component used to resolve embedded-program type names.
    ///
    /// Consider a kernel from module `kernels` specialized with `types::Payload`: HGBC embeds both
    /// modules even though the base entry point directly references only `kernels`. The first
    /// specialization or device-memory layout realizes every embedded module and composes them
    /// once; later lookups reuse the program-instance component.
    slang::IComponentType* getDispatchTypeLookupComponent()
    {
        if (programInstance->typeLookupComponent)
            return programInstance->typeLookupComponent;

        std::vector<slang::IComponentType*> modules;
        for (const ConstObjectLayout& layout : program->constObjectLayouts)
        {
            if (layout.kind != LazyObjectKind::SlangModule)
                continue;
            SlangModuleObject* module =
                readConstObject<SlangModuleObject>(Address::makeConstHeap(layout.byteOffset));
            slang::IComponentType* component = module->module;
            if (std::find(modules.begin(), modules.end(), component) == modules.end())
                modules.push_back(component);
        }

        SLANG_ASSERT(!modules.empty() && programInstance->slangSession);
        if (modules.size() == 1)
        {
            programInstance->typeLookupComponent = modules.front();
            return programInstance->typeLookupComponent;
        }

        ComPtr<ISlangBlob> diagnostics;
        SlangResult result = programInstance->slangSession->createCompositeComponentType(
            modules.data(),
            SlangInt(modules.size()),
            programInstance->typeLookupComponent.writeRef(),
            diagnostics.writeRef());
        if (SLANG_FAILED(result) || !programInstance->typeLookupComponent)
            fail("HGVM failed to compose embedded modules for specialization type lookup");
        return programInstance->typeLookupComponent;
    }

    /// Resolves an embedded entry-point declaration without compiling its pipeline yet.
    void evaluateSlangEntryPoint(
        SlangEntryPointObject* object,
        const StagedRecord::LazyObject& record)
    {
        object->module = readConstObject<SlangModuleObject>(record.firstOperandAddress);
        object->name = readLiteralString(record.secondOperandAddress, record.secondOperandByteSize);
    }

    /// Resolves a specialized entry point while retaining its generic arguments for pipeline use.
    ///
    /// Consider `dispatch(fill<float, 4>, groups)`: this inherits `fill`'s module and name, points
    /// at the program-wide type lookup component, and reads two compact trailing argument records.
    /// The actual Slang specialization remains lazy until its dispatcher first creates a pipeline.
    void evaluateSpecializedSlangEntryPoint(
        SlangEntryPointObject* object,
        const StagedRecord::LazyObject& record)
    {
        SlangEntryPointObject* base =
            readConstObject<SlangEntryPointObject>(record.firstOperandAddress);
        object->module = base->module;
        object->name = base->name;
        object->typeLookupComponent = getDispatchTypeLookupComponent();
        object->specializationArguments.clear();

        for (const auto& argument : record.getSpecializationArguments())
        {
            /// TODO: Looks like we are passing values as string ("4" instead of 4)
            /// TODO: Also looks like since we represent them as just strings, we don't hold onto
            /// the source module that the type came from, so right now we default to loading all of
            /// them. We should canonicalize this by lowering a proper representation (e.g. a
            /// combination of type and module reference)
            object->specializationArguments.add(
                {argument.kind, readLiteralString(argument.valueAddress, argument.valueByteSize)});
        }
    }

    /// Links and creates the RHI compute pipeline associated with one lazy dispatcher.
    void evaluateComputeDispatcher(
        ComputeDispatcherObject* object,
        const StagedRecord::LazyObject& record)
    {
        SlangEntryPointObject* entryPoint =
            readConstObject<SlangEntryPointObject>(record.firstOperandAddress);
        deviceState->createComputeDispatcher(
            object,
            entryPoint,
            programInstance->slangSession,
            program->debugMap);
    }

    /// Returns the shared HGBC-to-Slang type spelling or aborts lazy layout resolution.
    std::string getSlangTypeName(HGBCId typeId)
    {
        auto name = program->sourceProgram->getSlangTypeName(typeId);
        if (!name || name->empty())
            fail("HGVM cannot reflect this HGBC type in device memory");
        return *name;
    }

    /// Resolves a logical type in exactly the domain named by its staged descriptor.
    ///
    /// Consider a shader-parameter layout: resolving its dispatcher creates the linked program,
    /// then `contextIndex` selects the original reflected parameter, including empty parameters.
    /// The compiler already excluded system values from the host call and retained this identity
    /// through legalization. A device-memory layout instead uses `DefaultStructuredBuffer` rules.
    void evaluateTypeLayout(TypeLayoutObject* object, const StagedRecord::TypeLayoutObject& record)
    {
        object->logicalTypeId = record.logicalTypeId;
        object->domain = record.domain;
        object->usage = record.usage;
        object->hostTypeInfoAddress = record.hostTypeInfoAddress;
        object->contextAddress = record.contextAddress;
        object->contextIndex = record.contextIndex;
        object->entryPointIndex = record.entryPointIndex;
        object->graphicsContext = record.graphicsContext;

        switch (record.domain)
        {
        case HGBCLayoutDomain::Host:
            {
                StagedRecord::TypeInfo typeInfo =
                    readValue<StagedRecord::TypeInfo>(record.hostTypeInfoAddress);
                object->byteSize = typeInfo.byteSize;
                object->byteAlignment = typeInfo.byteAlignment;
                uint64_t byteStride = (uint64_t(typeInfo.byteSize) + typeInfo.byteAlignment - 1) &
                                      ~uint64_t(typeInfo.byteAlignment - 1);
                SLANG_ASSERT(byteStride <= std::numeric_limits<uint32_t>::max());
                object->byteStride = uint32_t(byteStride);
                return;
            }
        case HGBCLayoutDomain::DeviceMemory:
            {
                slang::IComponentType* lookupComponent = getDispatchTypeLookupComponent();
                slang::ProgramLayout* lookupLayout =
                    lookupComponent ? lookupComponent->getLayout() : nullptr;
                std::string typeName = getSlangTypeName(record.logicalTypeId);
                slang::TypeReflection* type =
                    lookupLayout ? lookupLayout->findTypeByName(typeName.c_str()) : nullptr;
                if (!type || !programInstance->slangSession)
                    fail("HGVM could not find a device-memory layout type");
                ComPtr<ISlangBlob> diagnostics;
                object->reflectedLayout = programInstance->slangSession->getTypeLayout(
                    type,
                    0,
                    slang::LayoutRules::DefaultStructuredBuffer,
                    diagnostics.writeRef());
                if (!object->reflectedLayout)
                    fail("HGVM failed to resolve a structured-buffer layout");
            }
            break;
        case HGBCLayoutDomain::ShaderObject:
            {
                slang::IComponentType* component =
                    record.graphicsContext
                        ? readConstObject<GraphicsPipelineObject>(record.contextAddress)
                              ->linkedComponent.get()
                        : readConstObject<ComputeDispatcherObject>(record.contextAddress)
                              ->linkedComponent.get();
                slang::ProgramLayout* programLayout = component->getLayout();
                slang::EntryPointReflection* entryPoint =
                    programLayout && record.entryPointIndex < programLayout->getEntryPointCount()
                        ? programLayout->getEntryPointByIndex(record.entryPointIndex)
                        : nullptr;
                if (entryPoint && record.contextIndex < entryPoint->getParameterCount())
                    object->reflectedLayout =
                        entryPoint->getParameterByIndex(record.contextIndex)->getTypeLayout();
                if (!object->reflectedLayout)
                    fail("HGVM could not resolve a shader-parameter layout");
            }
            break;
        case HGBCLayoutDomain::Serialized:
            fail("HGVM serialized layouts are not implemented yet");
        }

        size_t reflectedSize = object->reflectedLayout->getSize();
        size_t reflectedStride = object->reflectedLayout->getStride();
        int32_t reflectedAlignment = object->reflectedLayout->getAlignment();
        if (reflectedSize != SLANG_UNKNOWN_SIZE && reflectedSize != SLANG_UNBOUNDED_SIZE &&
            reflectedSize <= std::numeric_limits<uint32_t>::max())
            object->byteSize = uint32_t(reflectedSize);
        if (reflectedStride != SLANG_UNKNOWN_SIZE && reflectedStride != SLANG_UNBOUNDED_SIZE &&
            reflectedStride <= std::numeric_limits<uint32_t>::max())
            object->byteStride = uint32_t(reflectedStride);
        if (reflectedAlignment > 0)
            object->byteAlignment = uint32_t(reflectedAlignment);
    }

    /// Resolves both layouts and constructs the reusable conversion representation between them.
    void evaluateLayoutConverter(
        LayoutConverterObject* object,
        const StagedRecord::LayoutConverterObject& record)
    {
        object->sourceLayout = readConstObject<TypeLayoutObject>(record.sourceLayoutAddress);
        object->destinationLayout =
            readConstObject<TypeLayoutObject>(record.destinationLayoutAddress);
        SLANG_ASSERT(
            object->sourceLayout->hostTypeInfoAddress.value ==
            object->destinationLayout->hostTypeInfoAddress.value);
        object->operations.clear();
        object->shaderBindingOperations.clear();
        object->sourceDereferenceOffsets.clear();
        object->cursorPathIndices.clear();

        if (object->sourceLayout->domain == object->destinationLayout->domain)
        {
            if (object->sourceLayout != object->destinationLayout)
                fail("HGVM same-domain layout copies require one physical layout");
            return;
        }

        switch (object->sourceLayout->domain)
        {
        case HGBCLayoutDomain::DeviceMemory:
            switch (object->destinationLayout->domain)
            {
            case HGBCLayoutDomain::Host:
                deviceState->appendLayoutConversionOperations(
                    *this,
                    object->sourceLayout->reflectedLayout,
                    object->destinationLayout->hostTypeInfoAddress,
                    0,
                    0,
                    LayoutConversionDirection::DeviceToHost,
                    object->operations);
                return;
            default:
                break;
            }
            break;
        case HGBCLayoutDomain::Host:
            switch (object->destinationLayout->domain)
            {
            case HGBCLayoutDomain::DeviceMemory:
                deviceState->appendLayoutConversionOperations(
                    *this,
                    object->destinationLayout->reflectedLayout,
                    object->sourceLayout->hostTypeInfoAddress,
                    0,
                    0,
                    LayoutConversionDirection::HostToDevice,
                    object->operations);
                return;
            case HGBCLayoutDomain::ShaderObject:
                {
                    std::vector<uint32_t> sourceDereferenceOffsets;
                    std::vector<uint32_t> cursorPath;
                    deviceState->appendShaderBindingOperations(
                        *this,
                        object->destinationLayout->reflectedLayout,
                        object->sourceLayout->hostTypeInfoAddress,
                        0,
                        sourceDereferenceOffsets,
                        cursorPath,
                        object);
                    return;
                }
            default:
                break;
            }
            break;
        case HGBCLayoutDomain::ShaderObject:
        case HGBCLayoutDomain::Serialized:
            break;
        }
        fail("HGVM does not implement this layout-conversion direction");
    }

    /// Ensures one already constructed const-heap object has realized its expensive payload.
    ///
    /// The compiler guarantees an acyclic embedded dependency graph. `Resolving` is retained as a
    /// debug invariant and for exception-safe retry, not as a runtime graph-validation pass.
    void ensureLazyObject(LazyObject* object)
    {
        if (object->state == LazyState::Resolved)
            return;
        SLANG_ASSERT(object->state == LazyState::Unresolved);
        object->state = LazyState::Resolving;
        try
        {
            switch (object->kind)
            {
            case LazyObjectKind::SlangModule:
                {
                    const StagedRecord::LazyObject& record =
                        readRecord<StagedRecord::LazyObject>(object->descriptorWordOffset);
                    evaluateSlangModule(static_cast<SlangModuleObject*>(object), record);
                }
                break;
            case LazyObjectKind::SlangEntryPoint:
                {
                    const StagedRecord::LazyObject& record =
                        readRecord<StagedRecord::LazyObject>(object->descriptorWordOffset);
                    if (HGBCOpcode(record.header.opcodeAndFlags & StagedRecord::kOpcodeMask) ==
                        HGBCOpcode::SpecializeSlangEntryPoint)
                    {
                        evaluateSpecializedSlangEntryPoint(
                            static_cast<SlangEntryPointObject*>(object),
                            record);
                    }
                    else
                    {
                        SLANG_ASSERT(
                            HGBCOpcode(record.header.opcodeAndFlags & StagedRecord::kOpcodeMask) ==
                            HGBCOpcode::SlangEntryPoint);
                        evaluateSlangEntryPoint(
                            static_cast<SlangEntryPointObject*>(object),
                            record);
                    }
                }
                break;
            case LazyObjectKind::ComputeDispatcher:
                {
                    const StagedRecord::LazyObject& record =
                        readRecord<StagedRecord::LazyObject>(object->descriptorWordOffset);
                    evaluateComputeDispatcher(
                        static_cast<ComputeDispatcherObject*>(object),
                        record);
                }
                break;
            case LazyObjectKind::TypeLayout:
                evaluateTypeLayout(
                    static_cast<TypeLayoutObject*>(object),
                    readRecord<StagedRecord::TypeLayoutObject>(object->descriptorWordOffset));
                break;
            case LazyObjectKind::GraphicsPipeline:
                {
                    const auto& record =
                        readRecord<StagedRecord::GraphicsPipeline>(object->descriptorWordOffset);
                    auto vertex = readConstObject<SlangEntryPointObject>(record.vertexEntryPoint);
                    auto fragment =
                        readConstObject<SlangEntryPointObject>(record.fragmentEntryPoint);
                    deviceState->createGraphicsPipeline(
                        static_cast<GraphicsPipelineObject*>(object),
                        record,
                        vertex,
                        fragment,
                        programInstance->slangSession,
                        program->debugMap);
                }
                break;
            case LazyObjectKind::LayoutConverter:
                evaluateLayoutConverter(
                    static_cast<LayoutConverterObject*>(object),
                    readRecord<StagedRecord::LayoutConverterObject>(object->descriptorWordOffset));
                break;
            }
            object->state = LazyState::Resolved;
        }
        catch (...)
        {
            object->state = LazyState::Unresolved;
            throw;
        }
    }

    /// Borrows a typed record from the immutable program at its word address.
    template<typename T>
    const T& readRecord(uint32_t recordProgramCounter)
    {
        return *program->staging->storage.getRecord<T>(recordProgramCounter);
    }

    /// Borrows immutable type metadata together with its trailing field table.
    template<typename T>
    const T& readInstructionRecord(Address address)
    {
        SLANG_RELEASE_ASSERT(address.getKind() == AddressKind::InstructionAbsolute);
        return readRecord<T>(uint32_t(address.getUnsignedPayload()));
    }

    /// Reads the common record prefix at the current program counter.
    ///
    /// For example, an eight-word print advances from the extent written by the trusted stager. The
    /// assertions document internal construction invariants; release execution does not revalidate
    /// the immutable staged program.
    const StagedRecord::Header& readCurrentHeader()
    {
        const StagedRecord::Header& result = readRecord<StagedRecord::Header>(programCounter);
        SLANG_ASSERT(
            result.recordWordCount >= StagedRecord::getRecordWordCount<StagedRecord::Header>());
        SLANG_ASSERT(
            size_t(programCounter) + result.recordWordCount <=
            size_t(program->staging->storage.getWordCount()));
        return result;
    }

    /// Advances to the record immediately following a validated current record.
    ///
    /// For example, executing an eight-word print at PC 20 sets the next PC to 28 without decoding
    /// any HGBC operand metadata.
    void advance(const StagedRecord::Header& header) { programCounter += header.recordWordCount; }

    /// Materializes one variable's canonical absolute-frame address.
    ///
    /// For example, relative storage offset 24 in a frame beginning at byte 64 becomes absolute
    /// address 88. Later operations can pass that eight-byte address without retaining frame-local
    /// interpretation rules.
    void executeVar(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::Var>());
        const auto& record = readRecord<StagedRecord::Var>(programCounter);
        uint64_t absoluteOffset = uint64_t(currentFrameOffset) + record.storageByteOffset;
        SLANG_ASSERT(absoluteOffset < kFramePayloadLimit);
        writeValue(
            record.header.resultAddress,
            Address::makeAbsoluteFrame(uint32_t(absoluteOffset)));
        advance(header);
    }

    /// Executes an `OpLoad` as one fixed-size byte copy.
    ///
    /// For example, a staged 16-byte struct load reads its destination `Address` token and copies
    /// 16 bytes into the result slot; the runtime never resolves a type descriptor.
    void executeLoad(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::Load>());
        const auto& record = readRecord<StagedRecord::Load>(programCounter);
        Address sourceAddress = readValue<Address>(record.pointerAddress);
        copyBytes(record.header.resultAddress, sourceAddress, record.byteSize);
        advance(header);
    }

    /// Executes a resource-to-descriptor-handle conversion as one managed-address copy.
    void executeResourceToDescriptorHandle(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::ResourceToDescriptorHandle>());
        const auto& record = readRecord<StagedRecord::ResourceToDescriptorHandle>(programCounter);
        writeValue(record.header.resultAddress, readValue<Address>(record.resourceAddress));
        advance(header);
    }

    /// Executes a descriptor-handle dereference as one managed-address copy.
    void executeDescriptorHandleToResource(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::DescriptorHandleToResource>());
        const auto& record = readRecord<StagedRecord::DescriptorHandleToResource>(programCounter);
        writeValue(record.header.resultAddress, readValue<Address>(record.handleAddress));
        advance(header);
    }

    /// Executes an `OpStore` as one fixed-size byte copy.
    ///
    /// For example, whole-struct assignment copies the staged padded extent from the source value
    /// into the storage named by the destination address token.
    void executeStore(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::Store>());
        const auto& record = readRecord<StagedRecord::Store>(programCounter);
        Address destinationAddress = readValue<Address>(record.pointerAddress);
        copyBytes(destinationAddress, record.valueAddress, record.byteSize);
        advance(header);
    }

    /// Executes an `OpFieldAddress` as an address read and precomputed byte-offset addition.
    ///
    /// For example, offset four added to an absolute address for `sample` yields an absolute
    /// address for `sample.weight`; nested field-address operations repeat the same two steps.
    void executeFieldAddress(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::FieldAddress>());
        const auto& record = readRecord<StagedRecord::FieldAddress>(programCounter);
        Address baseAddress = readValue<Address>(record.baseAddress);
        if (record.classTypeAddress)
        {
            Address classHandle = readValue<Address>(baseAddress);
            validateClassHandle(classHandle, record.classTypeAddress);
            baseAddress = classHandle;
        }
        Address fieldAddress = baseAddress.addByteOffset(record.fieldByteOffset);
        writeValue(record.header.resultAddress, fieldAddress);
        advance(header);
    }

    /// Executes an `OpFieldExtract` as one bounded copy from a staged byte offset.
    ///
    /// For example, extracting four bytes at offset 12 resolves 16 source bytes, advances the
    /// physical source pointer by 12, and writes four bytes into the inline result slot.
    void executeFieldExtract(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::FieldExtract>());
        const auto& record = readRecord<StagedRecord::FieldExtract>(programCounter);
        Address sourceAddress = record.baseAddress;
        if (record.classTypeAddress)
        {
            Address classHandle = readValue<Address>(record.baseAddress);
            validateClassHandle(classHandle, record.classTypeAddress);
            sourceAddress = classHandle;
        }
        uint64_t sourceExtent = uint64_t(record.fieldByteOffset) + record.byteSize;
        SLANG_ASSERT(sourceExtent <= std::numeric_limits<uint32_t>::max());
        AddressRef<const uint8_t> source =
            resolveAddress<const uint8_t>(sourceAddress, uint32_t(sourceExtent));
        AddressRef<uint8_t> destination =
            resolveAddress<uint8_t>(record.header.resultAddress, record.byteSize);
        std::memcpy(destination.get(), source.get() + record.fieldByteOffset, record.byteSize);
        advance(header);
    }

    /// Calculates one array element's byte offset from execution-ready staged metadata.
    ///
    /// Consider index two into an array with eight-byte stride: this returns 16 after a runtime
    /// bounds check and a debug-only overflow invariant. Both address formation and value
    /// extraction share this path.
    uint32_t getElementByteOffset(
        Address indexAddress,
        ScalarType indexScalarType,
        uint32_t elementByteStride,
        uint32_t elementCount)
    {
        uint32_t index = 0;
        if (indexScalarType == ScalarType::Int32)
        {
            int32_t signedIndex = readValue<int32_t>(indexAddress);
            if (signedIndex < 0)
                fail("HGVM array index is negative");
            index = uint32_t(signedIndex);
        }
        else
        {
            SLANG_ASSERT(indexScalarType == ScalarType::UInt32);
            index = readValue<uint32_t>(indexAddress);
        }
        if (index >= elementCount)
            fail("HGVM array index is out of range");
        uint64_t byteOffset = uint64_t(index) * elementByteStride;
        SLANG_ASSERT(byteOffset <= UINT32_MAX);
        return uint32_t(byteOffset);
    }

    /// Executes `OpElementAddress` as an address read followed by one scaled offset addition.
    ///
    /// Consider `&values[2]` with stride eight: execution reads the canonical base address, adds
    /// 16 bytes, and writes another canonical address. It performs no HGBC or type lookup.
    void executeElementAddress(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::ElementAddress>());
        const auto& record = readRecord<StagedRecord::ElementAddress>(programCounter);
        Address baseAddress;
        uint32_t elementCount = record.elementCount;
        if (record.runtimeArrayTypeAddress)
        {
            Address handle = readValue<Address>(record.baseAddress);
            RuntimeArrayObject* object =
                getRuntimeArrayObject(handle, record.runtimeArrayTypeAddress);
            SLANG_ASSERT(object->elementByteStride == record.elementByteStride);
            baseAddress = Address::fromHeapPointer(object->payload);
            elementCount = object->elementCount;
        }
        else
        {
            baseAddress = readValue<Address>(record.baseAddress);
        }
        uint32_t byteOffset = getElementByteOffset(
            record.indexAddress,
            record.indexScalarType,
            record.elementByteStride,
            elementCount);
        Address elementAddress = baseAddress.addByteOffset(byteOffset);
        writeValue(record.header.resultAddress, elementAddress);
        advance(header);
    }

    /// Executes `OpExtractElement` as one fixed-size copy from a scaled array offset.
    ///
    /// Consider element two of an inline `Array<Pair, 3>`: execution forms the precomputed byte
    /// offset and copies one opaque `Pair` into the result slot.
    void executeElementExtract(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::ElementExtract>());
        const auto& record = readRecord<StagedRecord::ElementExtract>(programCounter);
        uint32_t byteOffset = getElementByteOffset(
            record.indexAddress,
            ScalarType::UInt32,
            record.elementByteStride,
            record.elementCount);
        Address elementAddress = record.baseAddress.addByteOffset(byteOffset);
        copyBytes(record.header.resultAddress, elementAddress, record.elementByteSize);
        advance(header);
    }

    /// Executes aggregate construction using its trailing precomputed member-copy entries.
    ///
    /// Consider both `{ true, 1.0f }` and an array containing those structs: frame creation has
    /// already initialized the result slot, so execution only performs the opaque member copies.
    /// Top-level padding retains that initial value. A nested member copy or a later whole-value
    /// load or store includes the copied aggregate's padded extent.
    void executeMakeAggregate(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount >=
            StagedRecord::getRecordWordCount<StagedRecord::MakeAggregate>());
        const auto& record = readRecord<StagedRecord::MakeAggregate>(programCounter);
        SLANG_ASSERT(header.operandCount == record.elementCount);
        SLANG_ASSERT((
            header.recordWordCount == StagedRecord::getByteSize<
                                          StagedRecord::MakeAggregate,
                                          StagedRecord::AggregateElementCopy>(record.elementCount) /
                                          sizeof(uint32_t)));

        AddressRef<uint8_t> destination =
            resolveAddress<uint8_t>(record.header.resultAddress, record.byteSize);
        for (uint32_t elementIndex = 0; elementIndex < record.elementCount; ++elementIndex)
        {
            const auto& copy = *record.getElementCopy(elementIndex);
            SLANG_ASSERT(
                copy.destinationByteOffset <= record.byteSize &&
                copy.byteSize <= record.byteSize - copy.destinationByteOffset);
            AddressRef<const uint8_t> source =
                resolveAddress<const uint8_t>(copy.sourceAddress, copy.byteSize);
            std::memcpy(
                destination.get() + copy.destinationByteOffset,
                source.get(),
                copy.byteSize);
        }
        advance(header);
    }

    /// Constructs one vector or matrix from its staged scalar source addresses.
    ///
    /// Consider `float2x2(a, b, c, d)`: `Float32` is dispatched once, the destination's complete
    /// sixteen-byte range is resolved once, and each trailing address contributes one packed lane.
    void executeMakeNumeric(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount >=
            StagedRecord::getRecordWordCount<StagedRecord::MakeNumeric>());
        const auto& record = readRecord<StagedRecord::MakeNumeric>(programCounter);
        SLANG_ASSERT(header.operandCount == record.laneCount);
        SLANG_ASSERT(
            (header.recordWordCount ==
             StagedRecord::getByteSize<StagedRecord::MakeNumeric, Address>(record.laneCount) /
                 sizeof(uint32_t)));

        dispatchScalarType(
            record.scalarType,
            [&]<typename T>()
            {
                uint32_t byteSize = record.laneCount * uint32_t(sizeof(T));
                AddressRef<T> destination =
                    resolveAddress<T>(record.header.resultAddress, byteSize);
                for (uint32_t laneIndex = 0; laneIndex < record.laneCount; ++laneIndex)
                {
                    Address sourceAddress = *record.getLane(laneIndex);
                    destination.get()[laneIndex] = readValue<T>(sourceAddress);
                }
            });
        advance(header);
    }

    /// Appends one staged printable value using only its compact execution classification.
    ///
    /// Consider both `print(value)` and `format("{}", value)`: they reach this same routine with a
    /// direct address and either `String` or a scalar code. A float format such as `:.2f` is parsed
    /// here because it depends on runtime pattern bytes; all type classification happened earlier.
    void appendFormattedValue(
        std::string& output,
        ValueKind kind,
        ScalarType scalarType,
        Address valueAddress,
        std::string_view formatSpec)
    {
        if (kind == ValueKind::String)
        {
            if (!formatSpec.empty())
                fail("A format was applied to a non-float value");
            std::string_view text = readStringView(valueAddress);
            if (!text.empty())
                output.append(text.data(), text.size());
            return;
        }

        SLANG_ASSERT(kind == ValueKind::Scalar);
        if (!formatSpec.empty() && scalarType != ScalarType::Float16 &&
            scalarType != ScalarType::Float32 && scalarType != ScalarType::Float64)
            fail("A format was applied to a non-float value");
        switch (scalarType)
        {
        case ScalarType::Bool:
            output += readValue<uint8_t>(valueAddress) ? "true" : "false";
            return;
        case ScalarType::Int32:
            output += std::to_string(readValue<int32_t>(valueAddress));
            return;
        case ScalarType::UInt32:
            output += std::to_string(readValue<uint32_t>(valueAddress));
            return;
        case ScalarType::Float16:
        case ScalarType::Float32:
        case ScalarType::Float64:
            {
                double value = scalarType == ScalarType::Float16
                                   ? double(readValue<Float16Value>(valueAddress))
                               : scalarType == ScalarType::Float32
                                   ? readValue<float>(valueAddress)
                                   : readValue<double>(valueAddress);
                if (formatSpec.empty())
                {
                    output += std::to_string(value);
                    return;
                }
                if (formatSpec.size() < 4 || formatSpec[0] != ':' || formatSpec[1] != '.' ||
                    formatSpec.back() != 'f')
                {
                    fail("Unsupported float format");
                }
                uint32_t precision = 0;
                for (size_t digitIndex = 2; digitIndex + 1 < formatSpec.size(); ++digitIndex)
                {
                    char digit = formatSpec[digitIndex];
                    if (digit < '0' || digit > '9' || precision > 9)
                        fail("Invalid float precision");
                    precision = precision * 10 + uint32_t(digit - '0');
                }
                if (precision > 9)
                    fail("Float precision is too large");
                char buffer[96];
                std::snprintf(buffer, sizeof(buffer), "%.*f", int(precision), double(value));
                output += buffer;
                return;
            }
        default:
            SLANG_UNREACHABLE("unsupported staged printable scalar type");
        }
    }

    /// Executes one staged format pattern and transfers a new managed String into its result slot.
    ///
    /// Consider `format("{{{0}}}: {:.1f}", name, rate)`: escaped braces are copied literally,
    /// explicit and automatic indices select trailing descriptors, and only the selected compact
    /// scalar/String code controls replacement formatting.
    void executeFormatString(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount >=
            StagedRecord::getRecordWordCount<StagedRecord::FormatString>());
        const auto& record = readRecord<StagedRecord::FormatString>(programCounter);
        SLANG_ASSERT(header.operandCount == record.argumentCount + 1);
        SLANG_ASSERT(
            (header.recordWordCount ==
             StagedRecord::getByteSize<StagedRecord::FormatString, StagedRecord::FormatArgument>(
                 record.argumentCount) /
                 sizeof(uint32_t)));

        std::string_view pattern = readStringView(record.patternAddress);
        std::string text;
        size_t automaticIndex = 0;
        for (size_t cursor = 0; cursor < pattern.size();)
        {
            char character = pattern[cursor];
            if (character == '{' && cursor + 1 < pattern.size() && pattern[cursor + 1] == '{')
            {
                text.push_back('{');
                cursor += 2;
                continue;
            }
            if (character == '}' && cursor + 1 < pattern.size() && pattern[cursor + 1] == '}')
            {
                text.push_back('}');
                cursor += 2;
                continue;
            }
            if (character == '}')
                fail("Pattern has an unmatched closing brace");
            if (character != '{')
            {
                text.push_back(character);
                ++cursor;
                continue;
            }

            size_t fieldEnd = pattern.find('}', cursor + 1);
            if (fieldEnd == std::string_view::npos)
                fail("Pattern has an unmatched opening brace");
            std::string_view field = pattern.substr(cursor + 1, fieldEnd - cursor - 1);
            size_t specOffset = field.find(':');
            std::string_view indexText = field.substr(0, specOffset);
            std::string_view formatSpec = specOffset == std::string_view::npos
                                              ? std::string_view()
                                              : field.substr(specOffset);

            size_t argumentIndex = automaticIndex++;
            if (!indexText.empty())
            {
                argumentIndex = 0;
                for (char digit : indexText)
                {
                    if (digit < '0' || digit > '9' ||
                        argumentIndex > (std::numeric_limits<size_t>::max() - 9) / 10)
                    {
                        fail("Invalid argument index");
                    }
                    argumentIndex = argumentIndex * 10 + size_t(digit - '0');
                }
            }
            if (argumentIndex >= record.argumentCount)
                fail("Argument index is out of range");

            const auto& argument = *record.getArgument(uint32_t(argumentIndex));
            appendFormattedValue(
                text,
                argument.kind,
                argument.scalarType,
                argument.valueAddress,
                formatSpec);
            cursor = fieldEnd + 1;
        }

        if (text.size() > std::numeric_limits<uint32_t>::max())
            fail("Formatted string exceeds the supported byte count");
        StringObject* object = allocateHeapObject<StringObject>(
            SLANG_HGVM_TRACKER(deviceState->eventTracker),
            std::move(text));
        StringValue result;
        result.byteCount = uint32_t(object->bytes.size());
        result.owner = Address::fromObject(object);
        writeValue(record.header.resultAddress, result);
        advance(header);
    }

    /// Executes one byte-count query from the String value itself.
    void executeStringGetByteCount(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::StringOperation>());
        StagedRecord::StringOperation record =
            readRecord<StagedRecord::StringOperation>(programCounter);
        StringValue value = readValue<StringValue>(record.stringAddress);
        writeValue(record.header.resultAddress, value.byteCount);
        advance(header);
    }

    /// Executes one bounds-checked UTF-8 byte read without decoding Unicode code points.
    void executeStringReadByte(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::StringReadByte>());
        StagedRecord::StringReadByte record =
            readRecord<StagedRecord::StringReadByte>(programCounter);
        uint32_t index = readValue<uint32_t>(record.indexAddress);
        std::string_view text = readStringView(record.stringAddress);
        if (index >= text.size())
            fail("String byte index is out of range");
        writeValue(record.header.resultAddress, uint32_t(uint8_t(text[index])));
        advance(header);
    }

    /// Executes creation, elapsed-time measurement, or reset for one staged timer.
    ///
    /// For example, elapsed measurement converts `steady_clock` duration to the HGBC Float32
    /// result directly; timer aliases continue to share the same reset point through the managed
    /// object.
    void executeTimerOperation(HGBCOpcode opcode, const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::TimerOperation>());
        StagedRecord::TimerOperation record =
            readRecord<StagedRecord::TimerOperation>(programCounter);
        switch (opcode)
        {
        case HGBCOpcode::CreateTimer:
            {
                TimerObject* timer =
                    allocateHeapObject<TimerObject>(SLANG_HGVM_TRACKER(deviceState->eventTracker));
                writeValue(record.header.resultAddress, Address::fromObject(timer));
            }
            break;
        case HGBCOpcode::TimerElapsed:
            {
                TimerObject* timer = readHeapObject<TimerObject>(record.timerAddress);
                float seconds = float(std::chrono::duration<double>(
                                          std::chrono::steady_clock::now() - timer->resetTime)
                                          .count());
                writeValue(record.header.resultAddress, seconds);
            }
            break;
        case HGBCOpcode::ResetTimer:
            readHeapObject<TimerObject>(record.timerAddress)->resetTime =
                std::chrono::steady_clock::now();
            break;
        default:
            SLANG_UNREACHABLE("non-timer opcode reached the staged timer evaluator");
        }
        advance(header);
    }

    /// Rejects borrowed pointers before a value is stored beyond the current call.
    /// The aggregate walk follows canonical type fields, not the shape of lambda syntax.
    void validateOwnedRayArgument(Address typeInfoAddress)
    {
        const auto& type = readInstructionRecord<StagedRecord::TypeInfo>(typeInfoAddress);
        switch (type.kind)
        {
        case TypeKind::Address:
            fail("HGVM registered ray arguments cannot retain borrowed host addresses");
        case TypeKind::TaggedUnion:
            fail("HGVM registered ray arguments do not support boxed union captures");
        case TypeKind::Struct:
            for (const auto& field : type.getFields())
                validateOwnedRayArgument(field.typeInfoAddress);
            break;
        case TypeKind::Array:
            validateOwnedRayArgument(type.elementTypeInfoAddress);
            break;
        case TypeKind::HeapObject:
            if (type.heapObjectKind == HeapObjectKind::Class ||
                type.heapObjectKind == HeapObjectKind::RuntimeArray ||
                type.heapObjectKind == HeapObjectKind::Box)
            {
                fail("HGVM registered ray arguments do not yet support managed class or "
                     "runtime-array or boxed captures");
            }
            break;
        default:
            break;
        }
    }

    /// Copies an already lowered argument and retains its canonical managed leaves.
    /// A tracker captured by its own registration is an internal ownership edge, so it does
    /// not retain itself. Other tracker edges require the later multi-tracker graph contract.
    RefPtr<RayShaderArgumentValue> retainRayArgument(
        const StagedRecord::RayShaderArgument& argument,
        RaytracingPipelineObject* owner)
    {
        validateOwnedRayArgument(argument.typeInfoAddress);
        const auto& type = readInstructionRecord<StagedRecord::TypeInfo>(argument.typeInfoAddress);
        RefPtr<RayShaderArgumentValue> result = new RayShaderArgumentValue;
        result->typeInfoAddress = argument.typeInfoAddress;
        result->parameterIndex = argument.parameterIndex;
        result->stage = argument.stage;
        result->typeId = argument.typeId;
        result->bytes = allocateAlignedStorage(
            type.byteSize ? type.byteSize : 1,
            Math::Max(size_t(type.byteAlignment), alignof(void*)));
        if (!result->bytes)
            fail("HGVM could not allocate registered ray arguments");
        auto source = resolveAddress<const uint8_t>(argument.valueAddress, type.byteSize);
        std::memcpy(result->bytes, source.get(), type.byteSize);
        if (type.managedLeafCount)
        {
            auto offsets = resolveAddress<const uint32_t>(
                type.managedLeafOffsetsAddress,
                type.managedLeafCount * uint32_t(sizeof(uint32_t)));
            for (uint32_t i = 0; i < type.managedLeafCount; ++i)
            {
                Address handle;
                std::memcpy(
                    &handle,
                    static_cast<const uint8_t*>(result->bytes) + offsets.get()[i],
                    sizeof(handle));
                if (!handle)
                    continue;
                HeapObject* object = handle.getObject();
                if (object->kind == HeapObjectKind::RaytracingPipeline)
                {
                    if (object != owner)
                        fail("HGVM ray registration supports one tracker; a callback captured "
                             "another tracker");
                    continue;
                }
                result->resources.add(object);
            }
        }
        return result;
    }

    void executeRayShaderOperation(HGBCOpcode opcode, const StagedRecord::Header& header)
    {
        if (opcode == HGBCOpcode::MakeRaytracingPipeline)
        {
            const auto& record = readRecord<StagedRecord::MakeRaytracingPipeline>(programCounter);
            uint32_t count = readValue<uint32_t>(record.rayTypeCountAddress);
            if (!count || count > 15)
                fail("HGVM ray trackers require between one and fifteen ray slots");
            auto* tracker = allocateHeapObject<RaytracingPipelineObject>(
                SLANG_HGVM_TRACKER(deviceState->eventTracker),
                count);
            writeValue(record.header.resultAddress, Address::fromObject(tracker));
            advance(header);
            return;
        }

        const auto& record = readRecord<StagedRecord::SetRayShader>(programCounter);
        Address handle = readValue<Address>(record.pipelineAddress);
        if (!handle)
            fail("HGVM cannot register a shader on a null ray tracker");
        auto* tracker = readHeapObject<RaytracingPipelineObject>(record.pipelineAddress);
        uint32_t raySlot = readValue<uint32_t>(record.raySlotAddress);
        if (raySlot >= tracker->rayTypeCount)
            fail("HGVM ray shader registration has an out-of-range ray slot");
        RefPtr<RayShaderRegistration> registration = new RayShaderRegistration;
        registration->programInstance = programInstance;
        registration->payloadType = record.payloadType;
        registration->attributeType = record.attributeType;
        registration->geometryKind = record.geometryKind;
        for (uint32_t stage = 0; stage < 3; ++stage)
            registration->entryPoints[stage] = record.entryPoints[stage];
        for (const auto& argument : record.getArguments())
            registration->arguments.add(retainRayArgument(argument, tracker));

        if (opcode == HGBCOpcode::SetRayMiss)
            tracker->misses[raySlot] = registration;
        else
        {
            uint32_t geometrySlot = readValue<uint32_t>(record.geometrySlotAddress);
            tracker->hits[(uint64_t(geometrySlot) << 32) | raySlot] = registration;
        }
        ++tracker->version;
        advance(header);
    }

    /// Creates one managed stream object from a host file-system path.
    ByteStreamObject* createByteStream(HGBCByteStreamAccess access, Address pathAddress)
    {
        if (!fileSystem)
            fail("HGVM byte streams require a file system");
        std::string_view pathText = readStringView(pathAddress);
        String path(pathText.data(), pathText.data() + pathText.size());
        if (workingDirectory.getLength() && !Path::isAbsolute(path))
            path = Path::combine(workingDirectory, path);

        ByteStreamObject* object =
            allocateHeapObject<ByteStreamObject>(SLANG_HGVM_TRACKER(deviceState->eventTracker));
        object->path = path.getBuffer();
        bool isDirectOSFileSystem = fileSystem == OSFileSystem::getLoadSingleton() ||
                                    fileSystem == OSFileSystem::getExtSingleton() ||
                                    fileSystem == OSFileSystem::getMutableSingleton();
        if (access == HGBCByteStreamAccess::Read)
        {
            object->access = ByteStreamObject::Access::Read;
            RefPtr<FileStream> directStream = new FileStream;
            if (isDirectOSFileSystem && SLANG_SUCCEEDED(directStream->init(
                                            path,
                                            FileMode::Open,
                                            FileAccess::Read,
                                            FileShare::ReadWrite)))
            {
                object->stream = directStream;
            }
            else if (
                SLANG_FAILED(
                    fileSystem->loadFile(object->path.c_str(), object->inputContents.writeRef())) ||
                !object->inputContents)
            {
                object->releaseReference();
                fail("HGVM failed to open input byte stream: " + object->path);
            }
            else
            {
                object->stream = new MemoryStreamBase(
                    FileAccess::Read,
                    object->inputContents->getBufferPointer(),
                    object->inputContents->getBufferSize());
            }
        }
        else
        {
            object->access = ByteStreamObject::Access::Write;
            RefPtr<FileStream> directStream = new FileStream;
            if (isDirectOSFileSystem && SLANG_SUCCEEDED(directStream->init(
                                            path,
                                            FileMode::Create,
                                            FileAccess::Write,
                                            FileShare::ReadWrite)))
            {
                object->stream = directStream;
            }
            else
            {
                auto mutableFileSystem = static_cast<ISlangMutableFileSystem*>(
                    fileSystem->castAs(ISlangMutableFileSystem::getTypeGuid()));
                if (!mutableFileSystem)
                {
                    object->releaseReference();
                    fail("HGVM output byte streams require a mutable file system");
                }
                object->mutableFileSystem = mutableFileSystem;
                object->output = new OwnedMemoryStream(FileAccess::Write);
                object->stream = object->output;
            }
        }
        return object;
    }

    /// Flushes one output stream to its host file-system destination and closes either mode.
    void closeByteStream(ByteStreamObject* object)
    {
        if (object->closed)
            return;
        if (object->access == ByteStreamObject::Access::Write)
        {
            if (object->output)
            {
                if (!object->mutableFileSystem)
                    fail("HGVM output byte stream has no writable storage");
                ConstArrayView<uint8_t> contents = object->output->getContents();
                if (SLANG_FAILED(object->mutableFileSystem->saveFile(
                        object->path.c_str(),
                        contents.getBuffer(),
                        size_t(contents.getCount()))))
                {
                    fail("HGVM failed to save output byte stream: " + object->path);
                }
            }
            else if (!object->stream || SLANG_FAILED(object->stream->flush()))
            {
                fail("HGVM failed to flush output byte stream: " + object->path);
            }
        }
        if (object->stream)
            object->stream->close();
        object->closed = true;
        object->inputContents.setNull();
        object->stream = nullptr;
        object->output = nullptr;
        object->mutableFileSystem.setNull();
    }

    /// Reads an exact field from a potentially short sequential transport.
    void readStreamExactly(ByteStreamObject* stream, void* destination, size_t byteCount)
    {
        uint8_t* output = static_cast<uint8_t*>(destination);
        size_t remaining = byteCount;
        while (remaining)
        {
            size_t readCount = 0;
            if (SLANG_FAILED(stream->read(output, remaining, readCount)) || !readCount ||
                readCount > remaining)
            {
                fail("HGVM byte-stream read reached the end of: " + stream->path);
            }
            output += readCount;
            remaining -= readCount;
        }
    }

    /// Executes `OpByteStreamFromFile` and transfers the initial raw program reference.
    void executeByteStreamFromFile(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::ByteStreamFromFile>());
        StagedRecord::ByteStreamFromFile record =
            readRecord<StagedRecord::ByteStreamFromFile>(programCounter);
        ByteStreamObject* object = createByteStream(record.access, record.pathAddress);
        writeValue(record.header.resultAddress, Address::fromObject(object));
        advance(header);
    }

    /// Executes explicit close or the retained file-backed size query.
    void executeByteStreamOperation(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::CloseByteStream>());
        StagedRecord::CloseByteStream record =
            readRecord<StagedRecord::CloseByteStream>(programCounter);
        ByteStreamObject* stream = readHeapObject<ByteStreamObject>(record.streamAddress);
        HGBCOpcode opcode = HGBCOpcode(header.opcodeAndFlags & StagedRecord::kOpcodeMask);
        if (opcode == HGBCOpcode::CloseByteStream)
        {
            closeByteStream(stream);
        }
        else
        {
            if (stream->closed || !stream->stream)
                fail("HGVM byte stream is closed");
            Int64 position = stream->stream->getPosition();
            if (position < 0 || SLANG_FAILED(stream->stream->seek(SeekOrigin::End, 0)))
                fail("HGVM byte stream has an invalid position");
            Int64 endPosition = stream->stream->getPosition();
            if (endPosition < 0 || SLANG_FAILED(stream->stream->seek(SeekOrigin::Start, position)))
                fail("HGVM byte stream has an invalid size");
            uint64_t byteCount = uint64_t(endPosition);
            if (byteCount > std::numeric_limits<uint32_t>::max())
                fail("HGVM byte-stream size exceeds uint32");
            writeValue(record.header.resultAddress, uint32_t(byteCount));
        }
        advance(header);
    }

    /// Executes one exact raw stream transfer after its endpoint kind was selected by staging.
    void executeByteStreamTransfer(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::ByteStreamTransfer>());
        StagedRecord::ByteStreamTransfer record =
            readRecord<StagedRecord::ByteStreamTransfer>(programCounter);
        ByteStreamObject* stream = readHeapObject<ByteStreamObject>(record.streamAddress);
        uint32_t byteOffset = readValue<uint32_t>(record.endpointByteOffsetAddress);
        uint32_t byteCount = readValue<uint32_t>(record.byteCountAddress);
        bool isRead = HGBCOpcode(header.opcodeAndFlags & StagedRecord::kOpcodeMask) ==
                      HGBCOpcode::ByteStreamRead;

        if (record.endpointKind == HGBCByteStreamEndpointKind::HostAddress)
        {
            if (byteOffset > record.hostByteExtent ||
                byteCount > record.hostByteExtent - byteOffset)
            {
                fail("HGVM byte-stream host transfer is out of range");
            }
            Address hostAddress = readValue<Address>(record.endpointAddress);
            if (isRead)
            {
                AddressRef<uint8_t> destination =
                    resolveAddressAt<uint8_t>(hostAddress, byteOffset, byteCount);
                readStreamExactly(stream, destination.get(), byteCount);
            }
            else
            {
                AddressRef<const uint8_t> source =
                    resolveAddressAt<const uint8_t>(hostAddress, byteOffset, byteCount);
                if (SLANG_FAILED(stream->write({source.get(), byteCount})))
                    fail("HGVM failed to write output byte stream: " + stream->path);
            }
        }
        else if (record.endpointKind == HGBCByteStreamEndpointKind::RuntimeArray)
        {
            RuntimeArrayObject* array = getRuntimeArrayObject(
                readValue<Address>(record.endpointAddress),
                record.runtimeArrayTypeAddress);
            if (byteOffset > array->payloadByteSize ||
                byteCount > array->payloadByteSize - byteOffset)
            {
                fail("HGVM byte-stream runtime-array transfer is out of range");
            }
            uint8_t* bytes = array->payload + byteOffset;
            if (isRead)
            {
                readStreamExactly(stream, bytes, byteCount);
            }
            else if (SLANG_FAILED(stream->write({bytes, byteCount})))
            {
                fail("HGVM failed to write output byte stream: " + stream->path);
            }
        }
        else if (record.endpointKind == HGBCByteStreamEndpointKind::DeviceBuffer)
        {
            DeviceState::StructuredBufferObject* buffer =
                readHeapObject<DeviceState::StructuredBufferObject>(record.endpointAddress);
            if (isRead)
            {
                std::vector<uint8_t> bytes(byteCount);
                readStreamExactly(stream, bytes.data(), bytes.size());
                deviceState->uploadBufferBytes(buffer, byteOffset, bytes);
            }
            else
            {
                std::vector<uint8_t> bytes(byteCount);
                deviceState->readBufferBytes(buffer, byteOffset, bytes);
                if (SLANG_FAILED(stream->write(bytes)))
                    fail("HGVM failed to write output byte stream: " + stream->path);
            }
        }
        else
        {
            SLANG_ASSERT(
                record.endpointKind == HGBCByteStreamEndpointKind::DeviceTexture2D && isRead);
            if (byteOffset != 0)
                fail("HGVM byte-stream texture uploads require a zero byte offset");
            DeviceState::Texture2DObject* texture =
                readHeapObject<DeviceState::Texture2DObject>(record.endpointAddress);
            std::vector<uint8_t> bytes(byteCount);
            readStreamExactly(stream, bytes.data(), bytes.size());
            deviceState->uploadTextureBytes(texture, bytes);
        }
        writeValue(record.header.resultAddress, byteCount);
        advance(header);
    }

    /// Executes one general image codec transform over the currently supported endpoint pair.
    void executeImageOperation(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::ImageOperation>());
        StagedRecord::ImageOperation record =
            readRecord<StagedRecord::ImageOperation>(programCounter);
        if (record.encodedEndpointKind != HGBCEncodedImageEndpointKind::ByteStream ||
            record.pixelEndpointKind != HGBCPixelImageEndpointKind::DeviceTexture2D)
        {
            fail("HGVM does not support this staged image endpoint pair yet");
        }
        ByteStreamObject* stream = readHeapObject<ByteStreamObject>(record.encodedEndpointAddress);
        DeviceState::Texture2DObject* texture =
            readHeapObject<DeviceState::Texture2DObject>(record.pixelEndpointAddress);

        uint64_t byteCount = 0;
        HGBCOpcode opcode = HGBCOpcode(header.opcodeAndFlags & StagedRecord::kOpcodeMask);
        if (opcode == HGBCOpcode::ImageDecode)
        {
            DecodedTextureImage image;
            std::string error;
            if (!decodeTextureImage(record.codec, *stream, image, byteCount, error))
                fail(error.empty() ? "HGVM failed to decode an image" : error);
            deviceState->uploadDecodedImage(texture, image);
        }
        else
        {
            byteCount = deviceState->encodeImage(record.codec, texture, *stream);
        }
        if (byteCount > std::numeric_limits<uint32_t>::max())
            fail("HGVM encoded image byte count exceeds uint32");
        writeValue(record.header.resultAddress, uint32_t(byteCount));
        advance(header);
    }

    /// Begins ordinary exceptional unwinding with the complete staged String as its message.
    void executeFail(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::StringOperation>());
        StagedRecord::StringOperation record =
            readRecord<StagedRecord::StringOperation>(programCounter);
        std::string_view message = readStringView(record.stringAddress);
        fail(message.empty() ? std::string() : std::string(message.data(), message.size()));
    }

    /// Executes one staged scalar or String print without resolving an HGBC type or value ID.
    void executePrint(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::Print>());
        StagedRecord::Print record = readRecord<StagedRecord::Print>(programCounter);
        std::string line;
        appendFormattedValue(line, record.kind, record.scalarType, record.valueAddress, {});

        // Use the host's sink when supplied. Without one, output goes to stdout.
        if (outputSink)
            outputSink->writeLine(line);
        else
            std::fprintf(stdout, "%.*s\n", int(line.size()), line.c_str());
        advance(header);
    }

    /// Executes a direct unconditional branch without any SSA parameter transfer.
    ///
    /// For example, `OpBranch %merge` assigns the already-patched merge PC. Any source-level phi
    /// transfer reached HGBC earlier as explicit variable stores and loads.
    void executeBranch(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(header.operandCount == 0);
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::Branch>());
        StagedRecord::Branch record = readRecord<StagedRecord::Branch>(programCounter);
        programCounter = record.targetProgramCounter;
    }

    /// Executes a conditional branch by reading one canonical staged boolean value.
    ///
    /// For example, a comparison result containing one selects its direct true PC without a block
    /// table lookup; zero selects the direct false PC.
    void executeConditionalBranch(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::BranchConditional>());
        StagedRecord::BranchConditional record =
            readRecord<StagedRecord::BranchConditional>(programCounter);
        uint8_t condition = readValue<uint8_t>(record.conditionAddress);
        programCounter = condition ? record.trueProgramCounter : record.falseProgramCounter;
    }

    /// Dispatches one staged scalar code to its present C++ interpreter representation.
    ///
    /// For example, `UInt32` instantiates the same templated operation body with `uint32_t` that a
    /// future `UInt64` case will instantiate with `uint64_t`; operation logic is not duplicated.
    template<typename Callable>
    void dispatchScalarType(ScalarType type, Callable&& callable)
    {
        switch (type)
        {
        case ScalarType::Bool:
            callable.template operator()<uint8_t>();
            return;
        case ScalarType::Int32:
            callable.template operator()<int32_t>();
            return;
        case ScalarType::UInt32:
            callable.template operator()<uint32_t>();
            return;
        case ScalarType::Float16:
            callable.template operator()<Float16Value>();
            return;
        case ScalarType::Float32:
            callable.template operator()<float>();
            return;
        case ScalarType::Float64:
            callable.template operator()<double>();
            return;
        default:
            SLANG_ASSERT(!"HGVM cannot execute this staged scalar type yet");
            return;
        }
    }

    /// Applies the value semantics shared by every pair of native scalar representations.
    ///
    /// Consider `bool(256u)`: converting directly to the one-byte Boolean storage type would
    /// truncate it to zero, so a Boolean destination is explicitly canonicalized through `!= 0`.
    /// Other pairs use the corresponding C++ numeric conversion, including truncation toward zero
    /// for an in-range floating-point-to-integer cast.
    template<typename DestinationT, typename SourceT>
    static DestinationT castScalarValue(ScalarType destinationType, SourceT value)
    {
        if constexpr (std::is_same_v<SourceT, Float16Value>)
        {
            if constexpr (std::is_same_v<DestinationT, Float16Value>)
            {
                return value;
            }
            else
            {
                if (destinationType == ScalarType::Bool)
                    return DestinationT(float(value) != 0.0f);
                return static_cast<DestinationT>(float(value));
            }
        }
        else
        {
            if (destinationType == ScalarType::Bool)
                return DestinationT(value != SourceT(0));
            if constexpr (std::is_same_v<DestinationT, Float16Value>)
                return Float16Value(value);
            else
                return static_cast<DestinationT>(value);
        }
    }

    /// Executes one scalar cast after the source scalar code has selected its C++ representation.
    ///
    /// For example, an `Int32` source is read once as `int32_t`; destination dispatch then chooses
    /// `float` for `Float32` and writes directly to the result slot allocated during staging.
    template<typename SourceT>
    void executeScalarTypeCastFromSource(const StagedRecord::ScalarTypeCast& record)
    {
        SourceT source = readValue<SourceT>(record.sourceAddress);
        dispatchScalarType(
            record.destinationType,
            [&]<typename DestinationT>()
            {
                writeValue(
                    record.header.resultAddress,
                    castScalarValue<DestinationT>(record.destinationType, source));
            });
    }

    /// Applies a binary operator while preserving modulo semantics for signed integers.
    ///
    /// Consider `int8_t(127) + int8_t(1)`: performing the addition as signed C++ would overflow.
    /// This helper applies the supplied operator to the corresponding unsigned bit patterns and
    /// bit-casts the low-width result back to `T`. Unsigned integers and floating-point values use
    /// their ordinary C++ representations.
    template<typename T, typename Operator>
    static T applyBinaryArithmeticOperator(T left, T right, Operator operation)
    {
        if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
        {
            using UnsignedT = std::make_unsigned_t<T>;
            UnsignedT leftBits = bitCast<UnsignedT>(left);
            UnsignedT rightBits = bitCast<UnsignedT>(right);
            UnsignedT resultBits = UnsignedT(operation(leftBits, rightBits));
            return bitCast<T>(resultBits);
        }
        else
        {
            return T(operation(left, right));
        }
    }

    /// Adds two values using the shared signed-overflow-safe arithmetic path.
    ///
    /// For example, adding one to `INT32_MAX` operates on its unsigned bits and returns the wrapped
    /// `INT32_MIN` representation; floating-point inputs use their ordinary native addition.
    template<typename T>
    static T addValues(T left, T right)
    {
        return applyBinaryArithmeticOperator(
            left,
            right,
            [](auto lhs, auto rhs) { return lhs + rhs; });
    }

    /// Subtracts two values using the shared signed-overflow-safe arithmetic path.
    ///
    /// For example, `INT32_MIN - 1` is computed through unsigned bits, while `Float32` continues
    /// to use the platform's IEEE floating-point subtraction.
    template<typename T>
    static T subtractValues(T left, T right)
    {
        return applyBinaryArithmeticOperator(
            left,
            right,
            [](auto lhs, auto rhs) { return lhs - rhs; });
    }

    /// Multiplies two values using the shared signed-overflow-safe arithmetic path.
    ///
    /// For example, an overflowing signed product retains its low-width bits, while a floating
    /// product follows the selected floating-point representation's multiplication.
    template<typename T>
    static T multiplyValues(T left, T right)
    {
        return applyBinaryArithmeticOperator(
            left,
            right,
            [](auto lhs, auto rhs) { return lhs * rhs; });
    }

    /// Shifts an integer's bit representation without invoking undefined C++ behavior.
    ///
    /// HGBC validation restricts this operation to integer types. A shift at least as wide as the
    /// value has an undefined Slang result, so HGVM chooses zero while keeping execution defined.
    template<typename T>
    static T shiftLeftValue(T value, T amount)
    {
        static_assert(std::is_integral_v<T>);
        using UnsignedT = std::make_unsigned_t<T>;
        UnsignedT shift = UnsignedT(amount);
        if (shift >= std::numeric_limits<UnsignedT>::digits)
            return T(0);

        UnsignedT valueBits;
        if constexpr (std::is_signed_v<T>)
            valueBits = bitCast<UnsignedT>(value);
        else
            valueBits = value;
        UnsignedT resultBits = UnsignedT(valueBits << shift);
        if constexpr (std::is_signed_v<T>)
            return bitCast<T>(resultBits);
        else
            return resultBits;
    }

    /// Applies one already-selected binary operation to every packed numeric lane.
    ///
    /// Consider `float4 + float4`: address resolution happens three times for the complete ranges,
    /// then this loop performs four native additions without another opcode or type dispatch.
    template<typename T, typename Operation>
    void executeBinaryLanes(const StagedRecord::Binary& record, Operation operation)
    {
        uint32_t byteSize = record.laneCount * uint32_t(sizeof(T));
        AddressRef<const T> left = resolveAddress<const T>(record.leftAddress, byteSize);
        AddressRef<const T> right = resolveAddress<const T>(record.rightAddress, byteSize);
        AddressRef<T> result = resolveAddress<T>(record.header.resultAddress, byteSize);
        for (uint32_t laneIndex = 0; laneIndex < record.laneCount; ++laneIndex)
            result.get()[laneIndex] = operation(left.get()[laneIndex], right.get()[laneIndex]);
    }

    /// Applies one already-selected comparison to every packed lane and writes Boolean bytes.
    ///
    /// For example, comparing two `float3` values reads twelve bytes from each input and writes
    /// three canonical zero-or-one bytes to the `bool3` result.
    template<typename T, typename Operation>
    void executeComparisonLanes(const StagedRecord::Binary& record, Operation operation)
    {
        uint32_t inputByteSize = record.laneCount * uint32_t(sizeof(T));
        AddressRef<const T> left = resolveAddress<const T>(record.leftAddress, inputByteSize);
        AddressRef<const T> right = resolveAddress<const T>(record.rightAddress, inputByteSize);
        AddressRef<uint8_t> result =
            resolveAddress<uint8_t>(record.header.resultAddress, record.laneCount);
        for (uint32_t laneIndex = 0; laneIndex < record.laneCount; ++laneIndex)
        {
            result.get()[laneIndex] =
                operation(left.get()[laneIndex], right.get()[laneIndex]) ? 1 : 0;
        }
    }

    /// Applies one already-selected unary operation to every packed numeric lane.
    ///
    /// Consider `sin(float3)`: the complete input and output ranges are resolved once and the
    /// supplied operation is invoked three times with no type-record access.
    template<typename T, typename Operation>
    void executeUnaryLanes(const StagedRecord::Unary& record, Operation operation)
    {
        uint32_t byteSize = record.laneCount * uint32_t(sizeof(T));
        AddressRef<const T> operand = resolveAddress<const T>(record.operandAddress, byteSize);
        AddressRef<T> result = resolveAddress<T>(record.header.resultAddress, byteSize);
        for (uint32_t laneIndex = 0; laneIndex < record.laneCount; ++laneIndex)
            result.get()[laneIndex] = operation(operand.get()[laneIndex]);
    }

    /// Executes an arithmetic opcode after the scalar representation has selected `T`.
    ///
    /// The opcode switch precedes the lane loop, so `float4` addition pays one type dispatch and
    /// one opcode dispatch rather than repeating either decision four times.
    template<typename T>
    void executeTypedArithmetic(const StagedRecord::Binary& record)
    {
        HGBCOpcode opcode = HGBCOpcode(record.header.opcodeAndFlags & StagedRecord::kOpcodeMask);
        switch (opcode)
        {
        case HGBCOpcode::Add:
            executeBinaryLanes<T>(record, [](T left, T right) { return addValues(left, right); });
            return;
        case HGBCOpcode::Sub:
            executeBinaryLanes<T>(
                record,
                [](T left, T right) { return subtractValues(left, right); });
            return;
        case HGBCOpcode::Mul:
            executeBinaryLanes<T>(
                record,
                [](T left, T right) { return multiplyValues(left, right); });
            return;
        case HGBCOpcode::Min:
            executeBinaryLanes<T>(
                record,
                [](T left, T right)
                {
                    // Promote exactly to double, including binary16, without losing Float64 bits.
                    // fmin/fmax return the numeric operand when only one operand is NaN.
                    if constexpr (kIsInterpreterFloatingPoint<T>)
                        return T(std::fmin(double(left), double(right)));
                    else
                        return left < right ? left : right;
                });
            return;
        case HGBCOpcode::Max:
            executeBinaryLanes<T>(
                record,
                [](T left, T right)
                {
                    if constexpr (kIsInterpreterFloatingPoint<T>)
                        return T(std::fmax(double(left), double(right)));
                    else
                        return left > right ? left : right;
                });
            return;
        case HGBCOpcode::IDiv:
            if constexpr (std::is_integral_v<T>)
            {
                executeBinaryLanes<T>(
                    record,
                    [](T left, T right)
                    {
                        if (!right)
                            fail("Integer division by zero");
                        if constexpr (std::is_signed_v<T>)
                        {
                            if (left == std::numeric_limits<T>::min() && right == T(-1))
                                fail("Signed integer division overflow");
                        }
                        return T(left / right);
                    });
                return;
            }
            SLANG_UNREACHABLE("validated OpIDiv has a non-integer staged type");
        case HGBCOpcode::IRem:
            if constexpr (std::is_integral_v<T>)
            {
                executeBinaryLanes<T>(
                    record,
                    [](T left, T right)
                    {
                        if (!right)
                            fail("Integer remainder by zero");
                        if constexpr (std::is_signed_v<T>)
                        {
                            if (left == std::numeric_limits<T>::min() && right == T(-1))
                                fail("Signed integer remainder overflow");
                        }
                        return T(left % right);
                    });
                return;
            }
            SLANG_UNREACHABLE("validated OpIRem has a non-integer staged type");
        case HGBCOpcode::Lsh:
            if constexpr (std::is_integral_v<T>)
            {
                executeBinaryLanes<T>(
                    record,
                    [](T value, T amount) { return shiftLeftValue(value, amount); });
                return;
            }
            SLANG_UNREACHABLE("validated OpLsh has a non-integer staged type");
        case HGBCOpcode::FDiv:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeBinaryLanes<T>(record, [](T left, T right) { return T(left / right); });
                return;
            }
            SLANG_UNREACHABLE("validated OpFDiv has a non-floating staged type");
        case HGBCOpcode::Pow:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeBinaryLanes<T>(
                    record,
                    [](T left, T right)
                    { return T(std::pow(NativeFloat<T>(left), NativeFloat<T>(right))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpPow has a non-floating staged type");
        case HGBCOpcode::Atan2:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeBinaryLanes<T>(
                    record,
                    [](T y, T x) { return T(std::atan2(float(y), float(x))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpAtan2 has a non-floating staged type");
        default:
            SLANG_UNREACHABLE("non-arithmetic opcode reached arithmetic evaluator");
        }
    }

    /// Executes one comparison opcode after the operand representation has selected `T`.
    template<typename T>
    void executeTypedComparison(const StagedRecord::Binary& record)
    {
        HGBCOpcode opcode = HGBCOpcode(record.header.opcodeAndFlags & StagedRecord::kOpcodeMask);
        switch (opcode)
        {
        case HGBCOpcode::Eql:
            executeComparisonLanes<T>(record, [](T left, T right) { return left == right; });
            return;
        case HGBCOpcode::Neq:
            executeComparisonLanes<T>(record, [](T left, T right) { return left != right; });
            return;
        case HGBCOpcode::Less:
            executeComparisonLanes<T>(record, [](T left, T right) { return left < right; });
            return;
        case HGBCOpcode::Greater:
            executeComparisonLanes<T>(record, [](T left, T right) { return left > right; });
            return;
        case HGBCOpcode::Leq:
            executeComparisonLanes<T>(record, [](T left, T right) { return left <= right; });
            return;
        case HGBCOpcode::Geq:
            executeComparisonLanes<T>(record, [](T left, T right) { return left >= right; });
            return;
        default:
            SLANG_UNREACHABLE("non-comparison opcode reached comparison evaluator");
        }
    }

    /// Executes one unary opcode after the scalar representation has selected `T`.
    template<typename T>
    void executeTypedUnary(const StagedRecord::Unary& record)
    {
        HGBCOpcode opcode = HGBCOpcode(record.header.opcodeAndFlags & StagedRecord::kOpcodeMask);
        switch (opcode)
        {
        case HGBCOpcode::Not:
            executeUnaryLanes<T>(record, [](T value) { return T(!value); });
            return;
        case HGBCOpcode::Neg:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(record, [](T value) { return T(-value); });
                return;
            }
            else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
            {
                executeUnaryLanes<T>(record, [](T value) { return subtractValues(T(0), value); });
                return;
            }
            SLANG_UNREACHABLE("validated OpNeg has an unsigned staged type");
        case HGBCOpcode::Sin:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(
                    record,
                    [](T value) { return T(std::sin(NativeFloat<T>(value))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpSin has a non-floating staged type");
        case HGBCOpcode::Cos:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(
                    record,
                    [](T value) { return T(std::cos(NativeFloat<T>(value))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpCos has a non-floating staged type");
        case HGBCOpcode::Tan:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(
                    record,
                    [](T value) { return T(std::tan(NativeFloat<T>(value))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpTan has a non-floating staged type");
        case HGBCOpcode::Asin:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(
                    record,
                    [](T value) { return T(std::asin(NativeFloat<T>(value))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpAsin has a non-floating staged type");
        case HGBCOpcode::Sqrt:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(
                    record,
                    [](T value) { return T(std::sqrt(NativeFloat<T>(value))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpSqrt has a non-floating staged type");
        case HGBCOpcode::Ceil:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(
                    record,
                    [](T value) { return T(std::ceil(NativeFloat<T>(value))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpCeil has a non-floating staged type");
        case HGBCOpcode::Floor:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(
                    record,
                    [](T value) { return T(std::floor(NativeFloat<T>(value))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpFloor has a non-floating staged type");
        case HGBCOpcode::Exp:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(
                    record,
                    [](T value) { return T(std::exp(NativeFloat<T>(value))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpExp has a non-floating staged type");
        case HGBCOpcode::Exp2:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(
                    record,
                    [](T value) { return T(std::exp2(NativeFloat<T>(value))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpExp2 has a non-floating staged type");
        case HGBCOpcode::Log:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(
                    record,
                    [](T value) { return T(std::log(NativeFloat<T>(value))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpLog has a non-floating staged type");
        case HGBCOpcode::Log2:
            if constexpr (kIsInterpreterFloatingPoint<T>)
            {
                executeUnaryLanes<T>(
                    record,
                    [](T value) { return T(std::log2(NativeFloat<T>(value))); });
                return;
            }
            SLANG_UNREACHABLE("validated OpLog2 has a non-floating staged type");
        default:
            SLANG_UNREACHABLE("non-unary opcode reached unary evaluator");
        }
    }

    /// Reads and dispatches one common staged arithmetic record.
    void executeArithmetic(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::Binary>());
        StagedRecord::Binary record = readRecord<StagedRecord::Binary>(programCounter);
        dispatchScalarType(
            record.scalarType,
            [&]<typename T>() { executeTypedArithmetic<T>(record); });
        advance(header);
    }

    /// Reads and dispatches one common staged comparison record.
    void executeComparison(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::Binary>());
        StagedRecord::Binary record = readRecord<StagedRecord::Binary>(programCounter);
        dispatchScalarType(
            record.scalarType,
            [&]<typename T>() { executeTypedComparison<T>(record); });
        advance(header);
    }

    /// Reads and dispatches one common staged unary record.
    void executeUnary(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::Unary>());
        StagedRecord::Unary record = readRecord<StagedRecord::Unary>(programCounter);
        dispatchScalarType(record.scalarType, [&]<typename T>() { executeTypedUnary<T>(record); });
        advance(header);
    }

    /// Reduces all packed operand lanes to one canonical Boolean byte.
    template<typename T>
    void executeTypedAny(const StagedRecord::Unary& record)
    {
        uint32_t byteSize = record.laneCount * uint32_t(sizeof(T));
        AddressRef<const T> operand = resolveAddress<const T>(record.operandAddress, byteSize);
        uint8_t result = 0;
        for (uint32_t laneIndex = 0; laneIndex < record.laneCount; ++laneIndex)
        {
            if (castScalarValue<uint8_t>(ScalarType::Bool, operand.get()[laneIndex]))
            {
                result = 1;
                break;
            }
        }
        writeValue(record.header.resultAddress, result);
    }

    /// Reads and dispatches one staged scalar Boolean reduction.
    void executeAny(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::Unary>());
        StagedRecord::Unary record = readRecord<StagedRecord::Unary>(programCounter);
        dispatchScalarType(record.scalarType, [&]<typename T>() { executeTypedAny<T>(record); });
        advance(header);
    }

    /// Executes one fully staged cast with one source and one destination scalar dispatch.
    ///
    /// Consider `uint(floatValue)`: this record already contains `Float32`, `UInt32`, and both
    /// addresses. Neither the HGBC type section nor a runtime value tag is consulted here.
    void executeScalarTypeCast(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::ScalarTypeCast>());
        StagedRecord::ScalarTypeCast record =
            readRecord<StagedRecord::ScalarTypeCast>(programCounter);
        dispatchScalarType(
            record.sourceType,
            [&]<typename SourceT>() { executeScalarTypeCastFromSource<SourceT>(record); });
        advance(header);
    }

    /// Reads one positive extent from the HGBC-required `uint` representation.
    ///
    /// For example, the unused height of a buffer allocation is the literal one. The validator
    /// has already established the value type, so execution reads four bytes directly and checks
    /// only the runtime value.
    uint32_t readPositiveResourceExtent(Address address)
    {
        uint32_t value = readValue<uint32_t>(address);
        if (!value)
            fail("HGVM resource extent must be positive");
        return value;
    }

    /// Allocates a class with zeroed inline fields and transfers its initial reference to the
    /// result.
    void executeAllocateObject(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount >=
            StagedRecord::getRecordWordCount<StagedRecord::AllocateObject>());
        const auto& record = readRecord<StagedRecord::AllocateObject>(programCounter);
        size_t expectedWordCount =
            StagedRecord::getByteSize<StagedRecord::AllocateObject, uint32_t>(
                record.managedLeafCount) /
            sizeof(uint32_t);
        SLANG_ASSERT(header.recordWordCount == expectedWordCount);
        ClassObject* object =
            ClassObject::create(SLANG_HGVM_TRACKER(deviceState->eventTracker), record);
        writeValue(record.header.resultAddress, Address::fromObject(object));
        advance(header);
    }

    /// Checks the allocation's canonical type before interpreting any erased payload bytes.
    BoxObject* getBoxObject(Address handle, Address payloadTypeAddress)
    {
        if (!handle)
            fail("HGVM cannot extract an empty box");
        SLANG_RELEASE_ASSERT(handle.getKind() == AddressKind::Heap);
        HeapObject* object = handle.getObject();
        SLANG_RELEASE_ASSERT(object->kind == HeapObjectKind::Box);
        auto box = static_cast<BoxObject*>(object);
        if (box->payloadTypeAddress.value != payloadTypeAddress.value)
            fail("HGVM boxed payload has a different concrete type");
        return box;
    }

    /// Replaces one owning box slot, retaining the replacement before an aliased old owner dies.
    void replaceBoxHandle(Address destination, Address value)
    {
        if (value)
            value.getObject()->addReference();
        Address previous = readValue<Address>(destination);
        writeValue(destination, value);
        if (previous)
            previous.getObject()->releaseReference();
    }

    /// Replaces the entire discriminant/payload pair after construction or conversion succeeds.
    void replaceTaggedUnion(Address destination, const TaggedUnionValue& value)
    {
        if (value.payload)
            value.payload.getObject()->addReference();
        TaggedUnionValue previous = readValue<TaggedUnionValue>(destination);
        writeValue(destination, value);
        if (previous.payload)
            previous.payload.getObject()->releaseReference();
    }

    /// Reads a semantic tagged value; zeroed storage never denotes an active first case.
    TaggedUnionValue readTaggedUnion(Address source)
    {
        TaggedUnionValue value = readValue<TaggedUnionValue>(source);
        if (!value.caseIndexPlusOne)
            fail("HGVM cannot access an uninitialized tagged union");
        return value;
    }

    /// Copies a concrete value into a self-contained immutable allocation before replacing a slot.
    void executeBox(const StagedRecord::Header& header)
    {
        const auto& record = readRecord<StagedRecord::Box>(programCounter);
        auto source = resolveAddress<const uint8_t>(record.valueAddress, record.payloadByteSize);
        BoxObject* object = allocateHeapObject<BoxObject>(
            SLANG_HGVM_TRACKER(deviceState->eventTracker),
            SLANG_HGVM_TRACKER(deviceState->eventTracker),
            program.Ptr(),
            record,
            source.get());
        Address previous = readValue<Address>(record.header.resultAddress);
        writeValue(record.header.resultAddress, Address::fromObject(object));
        if (previous)
            previous.getObject()->releaseReference();
        advance(header);
    }

    /// Returns an owned concrete copy only after the case and actual allocation type match.
    void executeUnbox(HGBCOpcode opcode, const StagedRecord::Header& header)
    {
        const auto& record = readRecord<StagedRecord::Unbox>(programCounter);
        Address handle;
        if (opcode == HGBCOpcode::ExtractTaggedUnion)
        {
            TaggedUnionValue value = readTaggedUnion(record.sourceAddress);
            if (value.caseTag != record.caseTag)
                fail("HGVM tagged union has a different active case");
            handle = value.payload;
        }
        else
            handle = readValue<Address>(record.sourceAddress);

        if (!record.payloadTypeAddress)
        {
            SLANG_RELEASE_ASSERT(opcode == HGBCOpcode::ExtractTaggedUnion);
            if (handle)
                fail("HGVM payload-free tagged union case contains a box");
            advance(header);
            return;
        }

        // A concrete result may itself own the source box (for example after repeated loop
        // execution). Keep its immutable bytes alive until every copied reference is retained.
        RefPtr<BoxObject> box = getBoxObject(handle, record.payloadTypeAddress);
        SLANG_RELEASE_ASSERT(box->payloadByteSize == record.payloadByteSize);
        auto destination =
            resolveAddress<uint8_t>(record.header.resultAddress, record.payloadByteSize);
        for (uint32_t offset : record.getManagedLeafOffsets())
        {
            Address child;
            std::memcpy(&child, box->payload + offset, sizeof(child));
            if (child)
                child.getObject()->addReference();
        }
        for (uint32_t offset : record.getManagedLeafOffsets())
        {
            Address previous;
            std::memcpy(&previous, destination.get() + offset, sizeof(previous));
            if (previous)
                previous.getObject()->releaseReference();
        }
        std::memcpy(destination.get(), box->payload, record.payloadByteSize);
        advance(header);
    }

    /// Checks the dynamic case and concrete box type before publishing their combined value.
    void executeMakeTaggedUnion(const StagedRecord::Header& header)
    {
        const auto& record = readRecord<StagedRecord::MakeTaggedUnion>(programCounter);
        uint32_t caseTag = readValue<uint32_t>(record.caseAddress);
        Address payload = readValue<Address>(record.payloadAddress);
        auto cases = record.getCases();
        for (Index i = 0; i < cases.getCount(); ++i)
        {
            if (cases[i].caseTag != caseTag)
                continue;
            if (cases[i].payloadTypeAddress)
                getBoxObject(payload, cases[i].payloadTypeAddress);
            else if (payload)
                fail("HGVM payload-free tagged union case contains a box");
            replaceTaggedUnion(record.header.resultAddress, {uint32_t(i + 1), caseTag, payload});
            advance(header);
            return;
        }
        fail("HGVM tagged union construction uses an invalid case");
    }

    /// Reads an initialized case or creates an independently owning handle to its erased payload.
    void executeTaggedUnionAccess(HGBCOpcode opcode, const StagedRecord::Header& header)
    {
        const auto& record = readRecord<StagedRecord::TaggedUnionAccess>(programCounter);
        TaggedUnionValue value = readTaggedUnion(record.sourceAddress);
        if (opcode == HGBCOpcode::GetTaggedUnionCase)
            writeValue(record.header.resultAddress, value.caseTag);
        else
            replaceBoxHandle(record.header.resultAddress, value.payload);
        advance(header);
    }

    /// Applies a checked case mapping while sharing the original immutable payload allocation.
    void executeConvertTaggedUnion(const StagedRecord::Header& header)
    {
        const auto& record = readRecord<StagedRecord::ConvertTaggedUnion>(programCounter);
        TaggedUnionValue value = readTaggedUnion(record.sourceAddress);
        for (const auto& mapping : record.getMappings())
        {
            if (mapping.sourceCaseTag != value.caseTag)
                continue;
            value.caseTag = mapping.destinationCaseTag;
            value.caseIndexPlusOne = mapping.destinationCaseIndexPlusOne;
            replaceTaggedUnion(record.header.resultAddress, value);
            advance(header);
            return;
        }
        fail("HGVM tagged union conversion excludes the active case");
    }

    /// Allocates one zeroed runtime-array payload and transfers its initial reference to the
    /// result.
    void executeAllocateRuntimeArray(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount >=
            StagedRecord::getRecordWordCount<StagedRecord::AllocateRuntimeArray>());
        const auto& record = readRecord<StagedRecord::AllocateRuntimeArray>(programCounter);
        size_t expectedWordCount =
            StagedRecord::getByteSize<StagedRecord::AllocateRuntimeArray, uint32_t>(
                record.elementManagedLeafCount) /
            sizeof(uint32_t);
        SLANG_ASSERT(header.recordWordCount == expectedWordCount);

        uint32_t elementCount = 0;
        if (record.countScalarType == ScalarType::Int32)
        {
            int32_t signedCount = readValue<int32_t>(record.elementCountAddress);
            if (signedCount < 0)
                fail("HGVM runtime array element count is negative");
            elementCount = uint32_t(signedCount);
        }
        else
        {
            SLANG_ASSERT(record.countScalarType == ScalarType::UInt32);
            elementCount = readValue<uint32_t>(record.elementCountAddress);
            if (elementCount > uint32_t(std::numeric_limits<int32_t>::max()))
                fail("HGVM runtime array element count exceeds getCount() range");
        }
        if (uint64_t(elementCount) * record.elementByteStride >
            std::numeric_limits<uint32_t>::max())
        {
            fail("HGVM runtime array allocation exceeds its byte-size limit");
        }

        auto managedLeafOffsets = record.getManagedLeafOffsets();
        RuntimeArrayObject* object = allocateHeapObject<RuntimeArrayObject>(
            SLANG_HGVM_TRACKER(deviceState->eventTracker),
            SLANG_HGVM_TRACKER(deviceState->eventTracker),
            record.runtimeArrayTypeAddress,
            elementCount,
            record.elementByteStride,
            record.elementByteAlignment,
            std::span<const uint32_t>(
                managedLeafOffsets.getBuffer(),
                managedLeafOffsets.getCount()));
        writeValue(record.header.resultAddress, Address::fromObject(object));
        advance(header);
    }

    /// Returns the immutable allocation count from one runtime-array object.
    void executeRuntimeArrayGetCount(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::RuntimeArrayGetCount>());
        StagedRecord::RuntimeArrayGetCount record =
            readRecord<StagedRecord::RuntimeArrayGetCount>(programCounter);
        Address handle = readValue<Address>(record.arrayAddress);
        RuntimeArrayObject* object = getRuntimeArrayObject(handle, record.runtimeArrayTypeAddress);
        SLANG_ASSERT(object->elementCount <= uint32_t(std::numeric_limits<int32_t>::max()));
        writeValue(record.header.resultAddress, int32_t(object->elementCount));
        advance(header);
    }

    void copyRuntimeArrayRange(
        RuntimeArrayObject* destination,
        uint32_t destinationOffset,
        RuntimeArrayObject* source,
        uint32_t sourceOffset,
        uint32_t elementCount)
    {
        DeviceState::validateArrayCopyRange(
            destination->elementCount,
            destinationOffset,
            elementCount,
            "destination");
        DeviceState::validateArrayCopyRange(
            source->elementCount,
            sourceOffset,
            elementCount,
            "source");
        if (destination->elementByteStride != source->elementByteStride ||
            destination->elementManagedLeafOffsets != source->elementManagedLeafOffsets)
        {
            fail("HGVM runtime-array copy requires identical element layouts");
        }
        if (elementCount == 0 || (destination == source && destinationOffset == sourceOffset))
        {
            return;
        }
        if (destination == source &&
            uint64_t(sourceOffset) < uint64_t(destinationOffset) + elementCount &&
            uint64_t(destinationOffset) < uint64_t(sourceOffset) + elementCount)
        {
            fail("HGVM array copy does not permit overlapping ranges in one runtime array");
        }

        uint64_t byteCount = uint64_t(elementCount) * source->elementByteStride;
        uint8_t* destinationBytes =
            destination->payload + uint64_t(destinationOffset) * destination->elementByteStride;
        const uint8_t* sourceBytes =
            source->payload + uint64_t(sourceOffset) * source->elementByteStride;
        if (source->elementManagedLeafOffsets.empty())
        {
            std::memmove(destinationBytes, sourceBytes, size_t(byteCount));
            return;
        }

        for (uint32_t elementIndex = 0; elementIndex < elementCount; ++elementIndex)
        {
            uint8_t* destinationElement =
                destinationBytes + uint64_t(elementIndex) * destination->elementByteStride;
            const uint8_t* sourceElement =
                sourceBytes + uint64_t(elementIndex) * source->elementByteStride;
            for (uint32_t leafOffset : source->elementManagedLeafOffsets)
            {
                Address handle;
                std::memcpy(&handle, sourceElement + leafOffset, sizeof(handle));
                if (handle)
                    handle.getObject()->addReference();
            }
            for (uint32_t leafOffset : destination->elementManagedLeafOffsets)
            {
                Address handle;
                std::memcpy(&handle, destinationElement + leafOffset, sizeof(handle));
                if (handle)
                    handle.getObject()->releaseReference();
            }
            std::memcpy(destinationElement, sourceElement, destination->elementByteStride);
        }
    }

    /// Dispatches one staged bulk copy by the endpoint spaces fixed during staging.
    void executeCopyArrayRange(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::CopyArrayRange>());
        StagedRecord::CopyArrayRange record =
            readRecord<StagedRecord::CopyArrayRange>(programCounter);
        uint32_t destinationOffset = readValue<uint32_t>(record.destinationOffsetAddress);
        uint32_t sourceOffset = readValue<uint32_t>(record.sourceOffsetAddress);
        uint32_t elementCount = readValue<uint32_t>(record.elementCountAddress);
        LayoutConverterObject* converter =
            readConstObject<LayoutConverterObject>(record.converterAddress);

        if (record.destinationKind == HeapObjectKind::RuntimeArray)
        {
            RuntimeArrayObject* destination = getRuntimeArrayObject(
                readValue<Address>(record.destinationAddress),
                record.destinationRuntimeArrayTypeAddress);
            if (record.sourceKind == HeapObjectKind::RuntimeArray)
            {
                RuntimeArrayObject* source = getRuntimeArrayObject(
                    readValue<Address>(record.sourceAddress),
                    record.sourceRuntimeArrayTypeAddress);
                copyRuntimeArrayRange(
                    destination,
                    destinationOffset,
                    source,
                    sourceOffset,
                    elementCount);
            }
            else
            {
                SLANG_ASSERT(record.sourceKind == HeapObjectKind::StructuredBuffer);
                DeviceState::StructuredBufferObject* source =
                    readHeapObject<DeviceState::StructuredBufferObject>(record.sourceAddress);
                deviceState->copyBufferToRuntimeArray(
                    destination,
                    destinationOffset,
                    source,
                    sourceOffset,
                    elementCount,
                    converter);
            }
        }
        else
        {
            SLANG_ASSERT(record.destinationKind == HeapObjectKind::StructuredBuffer);
            DeviceState::StructuredBufferObject* destination =
                readHeapObject<DeviceState::StructuredBufferObject>(record.destinationAddress);
            if (record.sourceKind == HeapObjectKind::RuntimeArray)
            {
                RuntimeArrayObject* source = getRuntimeArrayObject(
                    readValue<Address>(record.sourceAddress),
                    record.sourceRuntimeArrayTypeAddress);
                deviceState->copyRuntimeArrayToBuffer(
                    destination,
                    destinationOffset,
                    source,
                    sourceOffset,
                    elementCount,
                    converter);
            }
            else
            {
                SLANG_ASSERT(record.sourceKind == HeapObjectKind::StructuredBuffer);
                DeviceState::StructuredBufferObject* source =
                    readHeapObject<DeviceState::StructuredBufferObject>(record.sourceAddress);
                deviceState->copyBufferRange(
                    destination,
                    destinationOffset,
                    source,
                    sourceOffset,
                    elementCount,
                    converter);
            }
        }
        advance(header);
    }

    /// Executes one resource allocation using only the explicit staged descriptor.
    ///
    /// Consider `allocateTexture2D<float4>(8, 4)`: execution reads `{8, 4, 1}`, passes the staged
    /// `Texture2D`/`RGBA32Float`/usage descriptor to `DeviceState`, and transfers the returned
    /// wrapper reference to the already owning result slot.
    void executeAllocateResource(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::AllocateResource>());
        StagedRecord::AllocateResource record =
            readRecord<StagedRecord::AllocateResource>(programCounter);
        uint32_t widthOrElementCount =
            readPositiveResourceExtent(record.widthOrElementCountAddress);
        uint32_t height = readPositiveResourceExtent(record.heightAddress);
        uint32_t depth = readPositiveResourceExtent(record.depthAddress);
        TypeLayoutObject* layout = readConstObject<TypeLayoutObject>(record.layoutAddress);
        HeapObject* object =
            deviceState->allocateResource(record, layout, widthOrElementCount, height, depth);
        writeValue(record.header.resultAddress, Address::fromObject(object));
        advance(header);
    }

    /// Executes a texture-dimensions query from the RHI descriptor retained by its wrapper.
    void executeTextureDimensions(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::TextureDimensions>());
        StagedRecord::TextureDimensions record =
            readRecord<StagedRecord::TextureDimensions>(programCounter);
        DeviceState::Texture2DObject* texture =
            readHeapObject<DeviceState::Texture2DObject>(record.textureAddress);
        AddressRef<uint32_t> result =
            resolveAddress<uint32_t>(record.header.resultAddress, 2 * sizeof(uint32_t));
        deviceState->getTextureDimensions(texture, result.get());
        advance(header);
    }

    /// Converts a host-service failure into the HGVM exceptional path with its supplied detail.
    void requireHostSuccess(const char* action, SlangResult result, const String& detail)
    {
        if (SLANG_SUCCEEDED(result))
            return;
        std::string message(action);
        if (detail.getLength())
        {
            message += ": ";
            message.append(detail.getBuffer(), size_t(detail.getLength()));
        }
        fail(message);
    }

    /// Creates one managed window wrapper from fully staged scalar and String operands.
    void executeCreateWindow(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::CreateWindow>());
        if (!hostServices)
            fail("HGVM window creation requires host services");
        StagedRecord::CreateWindow record = readRecord<StagedRecord::CreateWindow>(programCounter);
        RefPtr<HGVMHostObject> hostWindow;
        String hostError;
        SlangResult result = hostServices->createWindow(
            readStringView(record.titleAddress),
            readValue<uint32_t>(record.widthAddress),
            readValue<uint32_t>(record.heightAddress),
            hostWindow,
            hostError);
        requireHostSuccess("HGVM host window creation failed", result, hostError);
        if (!hostWindow || hostWindow->getKind() != HGVMHostObjectKind::Window ||
            hostWindow->getOwner() != hostServices)
        {
            fail("HGVM host returned an invalid Window object");
        }
        WindowObject* window =
            allocateHeapObject<WindowObject>(SLANG_HGVM_TRACKER(deviceState->eventTracker));
        window->object = std::move(hostWindow);
        writeValue(record.header.resultAddress, Address::fromObject(window));
        advance(header);
    }

    /// Processes pending events or requests closure for one staged window.
    void executeWindowOperation(HGBCOpcode opcode, const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::WindowOperation>());
        if (!hostServices)
            fail("HGVM window operation requires host services");
        StagedRecord::WindowOperation record =
            readRecord<StagedRecord::WindowOperation>(programCounter);
        WindowObject* window = readHeapObject<WindowObject>(record.windowAddress);
        String hostError;
        SlangResult result = SLANG_FAIL;
        switch (opcode)
        {
        case HGBCOpcode::ProcessWindowEvents:
            {
                bool open = false;
                result = hostServices->processWindowEvents(window->object, open, hostError);
                requireHostSuccess("HGVM host event processing failed", result, hostError);
                writeValue(record.header.resultAddress, uint8_t(open ? 1 : 0));
            }
            break;
        case HGBCOpcode::CloseWindow:
            result = hostServices->closeWindow(window->object, hostError);
            requireHostSuccess("HGVM host window close failed", result, hostError);
            break;
        default:
            SLANG_UNREACHABLE("non-window operation reached the staged window evaluator");
        }
        advance(header);
    }

    /// Applies one staged String title to a host window.
    void executeSetWindowTitle(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::SetWindowTitle>());
        if (!hostServices)
            fail("HGVM window title update requires host services");
        StagedRecord::SetWindowTitle record =
            readRecord<StagedRecord::SetWindowTitle>(programCounter);
        WindowObject* window = readHeapObject<WindowObject>(record.windowAddress);
        String hostError;
        SlangResult result = hostServices->setWindowTitle(
            window->object,
            readStringView(record.titleAddress),
            hostError);
        requireHostSuccess("HGVM host window title update failed", result, hostError);
        advance(header);
    }

    /// Polls one canonical host event and writes it into the staged source-language struct layout.
    void executePollInputEvent(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::PollInputEvent>());
        if (!hostServices)
            fail("HGVM input polling requires host services");
        StagedRecord::PollInputEvent record =
            readRecord<StagedRecord::PollInputEvent>(programCounter);
        WindowObject* window = readHeapObject<WindowObject>(record.windowAddress);
        HGVMInputEvent event = {};
        String hostError;
        SlangResult result = hostServices->pollInputEvent(window->object, event, hostError);
        requireHostSuccess("HGVM host input polling failed", result, hostError);
        writeValueAt(record.header.resultAddress, record.fieldByteOffsets[0], uint32_t(event.type));
        writeValueAt(record.header.resultAddress, record.fieldByteOffsets[1], event.key);
        writeValueAt(record.header.resultAddress, record.fieldByteOffsets[2], event.button);
        writeValueAt(record.header.resultAddress, record.fieldByteOffsets[3], event.position[0]);
        writeValueAt(
            record.header.resultAddress,
            record.fieldByteOffsets[3] + sizeof(float),
            event.position[1]);
        writeValueAt(record.header.resultAddress, record.fieldByteOffsets[4], event.scroll[0]);
        writeValueAt(
            record.header.resultAddress,
            record.fieldByteOffsets[4] + sizeof(float),
            event.scroll[1]);
        writeValueAt(record.header.resultAddress, record.fieldByteOffsets[5], event.size[0]);
        writeValueAt(
            record.header.resultAddress,
            record.fieldByteOffsets[5] + sizeof(uint32_t),
            event.size[1]);
        advance(header);
    }

    /// Creates a typed surface using the logical RHI format embedded during staging.
    void executeCreateSurface(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::CreateSurface>());
        if (!hostServices)
            fail("HGVM surface creation requires host services");
        StagedRecord::CreateSurface record =
            readRecord<StagedRecord::CreateSurface>(programCounter);
        WindowObject* window = readHeapObject<WindowObject>(record.windowAddress);
        RefPtr<HGVMHostObject> hostSurface;
        String hostError;
        SlangResult result = hostServices->createSurface(
            window->object,
            record.requestedFormat,
            readValue<uint32_t>(record.imageCountAddress),
            hostSurface,
            hostError);
        requireHostSuccess("HGVM host surface creation failed", result, hostError);
        if (!hostSurface || hostSurface->getKind() != HGVMHostObjectKind::Surface ||
            hostSurface->getOwner() != hostServices)
        {
            fail("HGVM host returned an invalid Surface object");
        }
        SurfaceObject* surface =
            allocateHeapObject<SurfaceObject>(SLANG_HGVM_TRACKER(deviceState->eventTracker));
        surface->object = std::move(hostSurface);
        writeValue(record.header.resultAddress, Address::fromObject(surface));
        advance(header);
    }

    /// Submits device work before entering either potentially blocking host surface operation.
    ///
    /// Consider an offscreen render followed by `acquireSurfaceTexture()`: acquisition can wait
    /// for VSync to release a swap-chain image. Submit the independent render first so the GPU can
    /// execute it during that wait. The acquired-image blit starts a second batch which is
    /// submitted before `presentSurface()` enters the host presentation path.
    void executeSurfaceOperation(HGBCOpcode opcode, const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::SurfaceOperation>());
        if (!hostServices)
            fail("HGVM surface operation requires host services");
        StagedRecord::SurfaceOperation record =
            readRecord<StagedRecord::SurfaceOperation>(programCounter);
        SurfaceObject* surface = readHeapObject<SurfaceObject>(record.surfaceAddress);
        String hostError;
        switch (opcode)
        {
        case HGBCOpcode::AcquireSurfaceTexture:
            {
                deviceState->submitPendingCommands();
                ComPtr<rhi::ITexture> texture;
                SlangResult result =
                    hostServices->acquireSurfaceTexture(surface->object, texture, hostError);
                requireHostSuccess("HGVM host surface acquisition failed", result, hostError);
                DeviceState::Texture2DObject* wrapper =
                    deviceState->wrapTexture(std::move(texture));
                writeValue(record.header.resultAddress, Address::fromObject(wrapper));
            }
            break;
        case HGBCOpcode::PresentSurface:
        case HGBCOpcode::TryPresentSurface:
            {
                deviceState->submitPendingCommands();
                SlangResult result = hostServices->presentSurface(surface->object, hostError);
                if (opcode != HGBCOpcode::TryPresentSurface || result != SLANG_E_NOT_AVAILABLE)
                    requireHostSuccess("HGVM host surface presentation failed", result, hostError);
                if (opcode == HGBCOpcode::TryPresentSurface)
                    writeValue(record.header.resultAddress, result != SLANG_E_NOT_AVAILABLE);
            }
            break;
        default:
            SLANG_UNREACHABLE("non-surface operation reached the staged surface evaluator");
        }
        advance(header);
    }

    /// Reports a temporarily unavailable surface without constructing a null texture wrapper.
    void executeTryAcquireSurfaceTexture(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::TryAcquireSurfaceTexture>());
        if (!hostServices)
            fail("HGVM surface acquisition requires host services");
        auto record = readRecord<StagedRecord::TryAcquireSurfaceTexture>(programCounter);
        auto surface = readHeapObject<SurfaceObject>(record.surfaceAddress);
        Address destination = readValue<Address>(record.destinationPointerAddress);
        deviceState->submitPendingCommands();
        ComPtr<rhi::ITexture> texture;
        String hostError;
        SlangResult result =
            hostServices->acquireSurfaceTexture(surface->object, texture, hostError);
        Address image;
        if (result != SLANG_E_NOT_AVAILABLE)
        {
            requireHostSuccess("HGVM host surface acquisition failed", result, hostError);
            image = Address::fromObject(deviceState->wrapTexture(std::move(texture)));
        }
        writeValue(destination, image);
        writeValue(record.header.resultAddress, result != SLANG_E_NOT_AVAILABLE);
        advance(header);
    }

    /// Retires HGVM's submissions before handing explicit reconfiguration to the native host.
    /// The host owns its queue and swapchain, so it can finish their work without calling back.
    void executeResizeSurface(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::ResizeSurface>());
        if (!hostServices)
            fail("HGVM surface resizing requires host services");
        auto record = readRecord<StagedRecord::ResizeSurface>(programCounter);
        auto surface = readHeapObject<SurfaceObject>(record.surfaceAddress);
        deviceState->submitAndWait();
        String hostError;
        SlangResult result = hostServices->resizeSurface(
            surface->object,
            readValue<uint32_t>(record.widthAddress),
            readValue<uint32_t>(record.heightAddress),
            hostError);
        if (result != SLANG_E_NOT_AVAILABLE)
            requireHostSuccess("HGVM host surface resize failed", result, hostError);
        writeValue(record.header.resultAddress, result != SLANG_E_NOT_AVAILABLE);
        advance(header);
    }

    /// Records one staged complete-texture blit in the persistent DeviceState batch.
    void executeBlitTexture(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::BlitTexture>());
        StagedRecord::BlitTexture record = readRecord<StagedRecord::BlitTexture>(programCounter);
        DeviceState::Texture2DObject* destination =
            readHeapObject<DeviceState::Texture2DObject>(record.destinationAddress);
        DeviceState::Texture2DObject* source =
            readHeapObject<DeviceState::Texture2DObject>(record.sourceAddress);
        deviceState->blitTexture(
            destination,
            source,
            record.destinationScalarClass,
            record.sourceScalarClass);
        advance(header);
    }

    /// Reads dynamic clear bounds and typed payloads from the staged addresses.
    void executeClearTexture(const StagedRecord::Header& header)
    {
        auto record = readRecord<StagedRecord::ClearTexture>(programCounter);
        auto texture = readHeapObject<DeviceState::Texture2DObject>(record.textureAddress);
        rhi::SubresourceRange range = {
            readValue<uint32_t>(record.layerAddress),
            readValue<uint32_t>(record.layerCountAddress),
            readValue<uint32_t>(record.mipAddress),
            readValue<uint32_t>(record.mipCountAddress)};
        rhi::ClearValue value = {};
        if (isHGBCDepthFormat(record.format))
        {
            if (record.aspect != HGBCTextureAspect::StencilOnly)
                value.depthStencil.depth = readValue<float>(record.valueAddress);
            if (hasHGBCStencilAspect(record.format) &&
                record.aspect != HGBCTextureAspect::DepthOnly)
            {
                value.depthStencil.stencil = readValue<uint32_t>(
                    record.aspect == HGBCTextureAspect::StencilOnly ? record.valueAddress
                                                                    : record.stencilAddress);
            }
        }
        else
        {
            auto bytes = resolveAddress<const uint8_t>(record.valueAddress, record.valueByteSize);
            std::memcpy(&value.color, bytes.get(), record.valueByteSize);
        }
        deviceState->clearTexture(texture, value, record.scalarClass, record.aspect, range);
        advance(header);
    }

    /// Executes the synchronous POD element readback path.
    ///
    /// For example, a four-byte `uint` result reads exactly four bytes from `index * stride` into
    /// its already allocated scalar result slot.
    void executeReadBackBufferElement(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::ReadBackBufferElement>());
        StagedRecord::ReadBackBufferElement record =
            readRecord<StagedRecord::ReadBackBufferElement>(programCounter);
        DeviceState::StructuredBufferObject* buffer =
            readHeapObject<DeviceState::StructuredBufferObject>(record.bufferAddress);
        uint32_t index = readValue<uint32_t>(record.indexAddress);
        AddressRef<uint8_t> destination =
            resolveAddress<uint8_t>(record.header.resultAddress, record.resultByteSize);
        LayoutConverterObject* converter =
            readConstObject<LayoutConverterObject>(record.converterAddress);
        deviceState->readBackBufferElement(
            buffer,
            converter,
            index,
            record.resultByteSize,
            destination.get());
        advance(header);
    }

    /// Starts one staged readback and transfers its initial program reference into the result.
    ///
    /// For example, `Defer<uint> pending = asyncReadBack(buffer, 0)` submits the device copy here
    /// and writes only the returned object's eight-byte heap handle into `pending`.
    void executeAsyncReadBackBufferElement(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::ReadBackBufferElement>());
        StagedRecord::ReadBackBufferElement record =
            readRecord<StagedRecord::ReadBackBufferElement>(programCounter);
        DeviceState::StructuredBufferObject* buffer =
            readHeapObject<DeviceState::StructuredBufferObject>(record.bufferAddress);
        LayoutConverterObject* converter =
            readConstObject<LayoutConverterObject>(record.converterAddress);
        DeferredReadbackObject* deferred = deviceState->beginAsyncBufferElementReadback(
            buffer,
            converter,
            readValue<uint32_t>(record.indexAddress),
            record.resultByteSize);
        writeValue(record.header.resultAddress, Address::fromObject(deferred));
        advance(header);
    }

    /// Blocks on one deferred transfer and converts its captured bytes into the result slot.
    ///
    /// Consider `uint value = pending.wait()`: the deferred object is borrowed rather than
    /// consumed, so another alias can wait again and receive the same captured value.
    void executeWaitDefer(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::WaitDefer>());
        StagedRecord::WaitDefer record = readRecord<StagedRecord::WaitDefer>(programCounter);
        DeferredReadbackObject* deferred =
            readHeapObject<DeferredReadbackObject>(record.deferredAddress);
        AddressRef<uint8_t> destination =
            resolveAddress<uint8_t>(record.header.resultAddress, record.resultByteSize);
        deviceState->waitForDeferredReadback(deferred, record.resultByteSize, destination.get());
        advance(header);
    }

    /// Executes one buffer-dimensions query from wrapper metadata.
    void executeStructuredBufferDimensions(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount ==
            StagedRecord::getRecordWordCount<StagedRecord::StructuredBufferDimensions>());
        StagedRecord::StructuredBufferDimensions record =
            readRecord<StagedRecord::StructuredBufferDimensions>(programCounter);
        DeviceState::StructuredBufferObject* buffer =
            readHeapObject<DeviceState::StructuredBufferObject>(record.bufferAddress);
        AddressRef<uint32_t> result =
            resolveAddress<uint32_t>(record.header.resultAddress, 2 * sizeof(uint32_t));
        deviceState->getStructuredBufferDimensions(buffer, result.get());
        advance(header);
    }

    /// Converts an embedded caller-frame destination to its canonical absolute address.
    ///
    /// For example, a call result slot at relative offset 40 in a caller frame beginning at byte
    /// 64 becomes absolute-frame address 104 before it is saved in the callee's frame header.
    Address getAbsoluteFrameAddress(Address address) const
    {
        if (address.getKind() == AddressKind::FrameAbsolute)
            return address;
        SLANG_ASSERT(address.getKind() == AddressKind::FrameRelative);
        uint64_t byteOffset = uint64_t(currentFrameOffset) + address.getUnsignedPayload();
        SLANG_ASSERT(byteOffset < kFramePayloadLimit);
        return Address::makeAbsoluteFrame(uint32_t(byteOffset));
    }

    /// Enters one staged direct function without using the native C++ call stack.
    ///
    /// Consider `callee(value, inoutAddress)`: this reserves and zeroes the callee frame, copies
    /// each caller slot according to the corresponding parameter's byte size, saves traversal in
    /// `FrameHeader`, and redirects the interpreter cursor to the callee body. Both arguments use
    /// the same raw-copy loop even though the second value happens to contain an address.
    void executeMakeGraphicsDraw(const StagedRecord::Header& header)
    {
        auto record = readRecord<StagedRecord::MakeGraphicsDraw>(programCounter);
        GraphicsDrawValue value;
        value.count = readValue<uint32_t>(record.count);
        value.first = readValue<uint32_t>(record.first);
        value.instanceCount = readValue<uint32_t>(record.instanceCount);
        value.firstInstance = readValue<uint32_t>(record.firstInstance);
        value.baseVertex = readValue<int32_t>(record.baseVertex);
        if (record.indices)
        {
            value.indices = readValue<Address>(record.indices);
            readHeapObject<DeviceState::StructuredBufferObject>(record.indices)->addReference();
        }
        writeValue(header.resultAddress, value);
        advance(header);
    }

    void executeMakeGraphicsDrawParameters(const StagedRecord::Header& header)
    {
        auto record = readRecord<StagedRecord::MakeGraphicsDrawParameters>(programCounter);
        GraphicsDrawParametersValue value;
        value.useDefaultViewport = readValue<bool>(record.operands[0]);
        value.useDefaultScissor = readValue<bool>(record.operands[1]);
        value.viewport.originX = readValue<float>(record.operands[2]);
        value.viewport.originY = readValue<float>(record.operands[3]);
        value.viewport.extentX = readValue<float>(record.operands[4]);
        value.viewport.extentY = readValue<float>(record.operands[5]);
        value.viewport.minZ = readValue<float>(record.operands[6]);
        value.viewport.maxZ = readValue<float>(record.operands[7]);
        value.scissor.minX = readValue<uint32_t>(record.operands[8]);
        value.scissor.minY = readValue<uint32_t>(record.operands[9]);
        value.scissor.maxX = readValue<uint32_t>(record.operands[10]);
        value.scissor.maxY = readValue<uint32_t>(record.operands[11]);
        value.stencilRef = readValue<uint32_t>(record.operands[12]);
        writeValue(header.resultAddress, value);
        advance(header);
    }

    /// Copies a parameter into a freshly zeroed frame and acquires its owning references.
    /// Both host invocations and VM calls use the staged parameter transfer description.
    void initializeParameter(
        uint32_t frameOffset,
        const StagedRecord::Parameter& parameter,
        const void* source)
    {
        SLANG_ASSERT(parameter.header.resultAddress.getKind() == AddressKind::FrameRelative);
        uint64_t offset = frameOffset + parameter.header.resultAddress.getUnsignedPayload();
        SLANG_ASSERT(offset < kFramePayloadLimit);
        Address destination = Address::makeAbsoluteFrame(uint32_t(offset));
        auto storage = resolveAddress<uint8_t>(destination, parameter.transferByteSize);
        std::memcpy(storage.get(), source, parameter.transferByteSize);
        for (uint32_t leafOffset : parameter.getManagedLeafOffsets())
        {
            Address object = readValue<Address>(destination.addByteOffset(leafOffset));
            if (object)
                object.getObject()->addReference();
        }
    }

    void executeCall(const StagedRecord::Header& header)
    {
        if (header.opcodeAndFlags & StagedRecord::kGraphicsCallBit)
        {
            deviceState->executeGraphicsCall(*this, header);
            return;
        }
        if (header.opcodeAndFlags & StagedRecord::kComputeCallBit)
        {
            deviceState->executeComputeCall(*this, header);
            return;
        }
        SLANG_ASSERT(
            header.recordWordCount >= StagedRecord::getRecordWordCount<StagedRecord::Call>());
        const auto& record = readRecord<StagedRecord::Call>(programCounter);
        SLANG_ASSERT(record.functionAddress.getKind() == AddressKind::InstructionAbsolute);
        const auto& callee = readRecord<StagedRecord::Function>(
            uint32_t(record.functionAddress.getUnsignedPayload()));
        SLANG_ASSERT(
            (callee.header.opcodeAndFlags & StagedRecord::kOpcodeMask) ==
            uint32_t(HGBCOpcode::Func));
        SLANG_ASSERT(header.operandCount == callee.parameterCount);
        SLANG_ASSERT(
            callee.frameAlignment && !(callee.frameAlignment & (callee.frameAlignment - 1)));
        SLANG_ASSERT(callee.frameAlignment <= kStorageBaseAlignment);

        uint32_t callerStackTop = uint32_t(stack.getCount());
        uint64_t calleeFrameOffset = (uint64_t(callerStackTop) + callee.frameAlignment - 1) &
                                     ~uint64_t(callee.frameAlignment - 1);
        uint64_t calleeStackTop = calleeFrameOffset + callee.frameSize;
        if (calleeStackTop >= kFramePayloadLimit ||
            calleeStackTop > std::numeric_limits<Index>::max())
        {
            fail("HGVM call stack exceeds its address limit");
        }

        uint32_t returnProgramCounter = programCounter + header.recordWordCount;
        Address returnDestinationAddress;
        if (record.header.resultAddress.value)
            returnDestinationAddress = getAbsoluteFrameAddress(record.header.resultAddress);

        // Grow before resolving any caller or callee stack address because List growth can move
        // its native allocation. Geometric capacity growth avoids reallocating the complete stack
        // at every newly reached call depth. All staged logical addresses remain valid across a
        // move, and the explicit memset below initializes only the active callee frame.
        stack.growToCount(Index(calleeStackTop));
        std::memset(stack.getBuffer() + size_t(calleeFrameOffset), 0, callee.frameSize);

        FrameHeader calleeHeader;
        calleeHeader.callerFrameOffset = currentFrameOffset;
        calleeHeader.previousStackTop = callerStackTop;
        calleeHeader.returnProgramCounter = returnProgramCounter;
        calleeHeader.callerFunctionAddress = functionAddress;
        calleeHeader.returnDestinationAddress = returnDestinationAddress;
        std::memcpy(
            stack.getBuffer() + size_t(calleeFrameOffset),
            &calleeHeader,
            sizeof(calleeHeader));

        const StagedRecord::Parameter* parameterPointer = callee.getFirstParameter();
        for (uint32_t parameterIndex = 0; parameterIndex < callee.parameterCount; ++parameterIndex)
        {
            Address argumentAddress = *record.getArgument(parameterIndex);
            const auto& parameter = *parameterPointer;
            SLANG_ASSERT(
                parameter.header.recordWordCount >=
                StagedRecord::getRecordWordCount<StagedRecord::Parameter>());
            auto source =
                resolveAddress<const uint8_t>(argumentAddress, parameter.transferByteSize);
            initializeParameter(uint32_t(calleeFrameOffset), parameter, source.get());
            if (parameterIndex + 1 < callee.parameterCount)
                parameterPointer = parameter.getNext();
        }

        currentFrameOffset = uint32_t(calleeFrameOffset);
        if (debugHandler)
            enterDebugCall(
                {program->debugMap->getProgramId(), programCounter},
                record.functionAddress);
        functionAddress = record.functionAddress;
        programCounter = callee.entryProgramCounter;
    }

    /// Redirects an ordinary source return into the current function's mandatory exit block.
    ///
    /// For example, hello world's `OpReturn` does not complete the invocation directly; it sets
    /// the PC to `StagedRecord::Function::exitProgramCounter` so the current cleanup records for
    /// resource-owning values cannot be bypassed.
    void executeReturn(const StagedRecord::Header& header)
    {
        SLANG_ASSERT(header.operandCount == 0);
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::Header>());
        SLANG_ASSERT(functionAddress.getKind() == AddressKind::InstructionAbsolute);
        StagedRecord::Function functionRecord =
            readRecord<StagedRecord::Function>(uint32_t(functionAddress.getUnsignedPayload()));
        programCounter = functionRecord.exitProgramCounter;
    }

    /// Resolves the heap-handle leaf named by one explicit ownership record.
    ///
    /// A cleanup record starts from a direct relative frame address. A pointer store starts by
    /// loading an absolute address from its pointer slot. Both then add the staged flattened leaf
    /// offset and reach the same eight-byte handle representation.
    Address getReferenceTargetAddress(const StagedRecord::Reference& record)
    {
        Address result =
            record.isIndirect ? readValue<Address>(record.targetAddress) : record.targetAddress;
        if (record.leafByteOffset)
            result = result.addByteOffset(record.leafByteOffset);
        return result;
    }

    /// Executes one explicit program-reference increment or release.
    void executeReferenceOperation(
        StagedRecord::InternalOpcode opcode,
        const StagedRecord::Header& header)
    {
        SLANG_ASSERT(
            header.recordWordCount == StagedRecord::getRecordWordCount<StagedRecord::Reference>());
        StagedRecord::Reference record = readRecord<StagedRecord::Reference>(programCounter);
        Address targetAddress = getReferenceTargetAddress(record);
        Address heapAddress = readValue<Address>(targetAddress);
        if (heapAddress)
        {
            if (opcode == StagedRecord::InternalOpcode::IncRef)
            {
                heapAddress.getObject()->addReference();
            }
            else
            {
                writeValue(targetAddress, Address{});
                heapAddress.getObject()->releaseReference();
            }
        }
        advance(header);
    }

    /// Executes the VM-only record at the current program counter.
    ///
    /// For example, `FinishFrameExit` restores the caller's frame, function address, and return PC
    /// after a helper call, or completes execution when the exiting frame is the root.
    void executeInternal(const StagedRecord::Header& header)
    {
        StagedRecord::InternalOpcode opcode =
            StagedRecord::InternalOpcode(header.opcodeAndFlags & StagedRecord::kOpcodeMask);
        switch (opcode)
        {
        case StagedRecord::InternalOpcode::IncRef:
        case StagedRecord::InternalOpcode::MaybeDecRefAndRelease:
            executeReferenceOperation(opcode, header);
            return;
        case StagedRecord::InternalOpcode::FinishFrameExit:
            SLANG_ASSERT(
                header.recordWordCount ==
                StagedRecord::getRecordWordCount<StagedRecord::FinishFrameExit>());
            {
                FrameHeader frame =
                    readValue<FrameHeader>(Address::makeAbsoluteFrame(currentFrameOffset));
                SLANG_ASSERT(Index(frame.previousStackTop) <= stack.getCount());
                stack.unsafeShrinkToCount(Index(frame.previousStackTop));
                if (debugHandler)
                {
                    debugHandler->onFunctionExit();
                    debugCalls.removeLast();
                }
                if (frame.callerFrameOffset == kInvalidStackOffset)
                {
                    if (isUnwindingFailure && !cancelled)
                        throw Exception(failureMessage.getBuffer());
                    status = Status::Completed;
                    return;
                }
                currentFrameOffset = frame.callerFrameOffset;
                functionAddress = frame.callerFunctionAddress;
                if (isUnwindingFailure)
                {
                    StagedRecord::Function caller = readRecord<StagedRecord::Function>(
                        uint32_t(functionAddress.getUnsignedPayload()));
                    programCounter = caller.exitProgramCounter;
                }
                else
                {
                    programCounter = frame.returnProgramCounter;
                }
            }
            return;
        }
        SLANG_ASSERT(!"HGVM encountered an unknown internal opcode");
    }

#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    /// Captures the current record address only when an event is emitted.
    static EventOrigin captureEventOrigin(void* context)
    {
        auto& state = *static_cast<InterpreterState*>(context);
        EventOrigin origin;
        origin.programId = state.program->debugMap->getProgramId();
        origin.recordWordOffset = state.programCounter;
        return origin;
    }
#endif

    /// Runs the fast staged-record loop until the root function completes or throws.
    ///
    /// For example, ordinary hello world visits `OpPrint`, `OpReturn`, and `FinishFrameExit`.
    /// The ordinary specialization contains no per-record debugger checks. The debug specialization
    /// consumes compact scope directives and invokes step control only at requested stops.
    template<bool debugging>
    void run()
    {
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
        EventTracker::ContextScope eventContext(
            deviceState->eventTracker,
            captureEventOrigin,
            this);
#endif
        status = Status::Running;
        while (status == Status::Running)
        {
            try
            {
                StagedRecord::Header header = readCurrentHeader();
                if constexpr (debugging)
                {
                    if (!isUnwindingFailure &&
                        (header.opcodeAndFlags & StagedRecord::kDebugRecordBit))
                    {
                        if ((header.opcodeAndFlags & StagedRecord::kOpcodeMask) ==
                            uint32_t(HGBCOpcode::DebugValue))
                        {
                            executeDebugValue(header);
                            continue;
                        }
                        if (debugHandler)
                        {
                            const auto& scope =
                                readRecord<StagedRecord::DebugScope>(programCounter);
                            debugHandler->onScope(scope.scopeIndex, scope.inlineSiteCount);
                        }
                        advance(header);
                        continue;
                    }
                    // A source instruction can lower to release/copy/retain records. Observe
                    // debugger requests only before the whole operation, when all slots own
                    // exactly the references represented by their bytes.
                    bool instructionBoundary =
                        !(header.opcodeAndFlags & StagedRecord::kInstructionContinuationBit);
                    bool breakpointHit =
                        (header.opcodeAndFlags & StagedRecord::kBreakpointBit) != 0;
                    bool controlRequested = instructionBoundary && !isUnwindingFailure &&
                                            stepControl && stepControl->takeControlRequest();
                    if (instructionBoundary && !isUnwindingFailure && stepControl &&
                        (controlRequested || stepAction == StepAction::Step || breakpointHit))
                    {
                        debugStopped = true;
                        DebugInstructionAddress address = {
                            program->debugMap->getProgramId(),
                            programCounter};
                        stepAction =
                            controlRequested
                                ? stepControl->onControl(address, stepAction, breakpointHit)
                                : stepControl->onStep(address, breakpointHit);
                        debugStopped = false;
                        debugHeapReady = false;
                        debugObjects.clear();
                        debugMemory.clear();
                        if (stepAction == StepAction::Stop)
                        {
                            cancelled = true;
                            isUnwindingFailure = true;
                            programCounter = readRecord<StagedRecord::Function>(
                                                 uint32_t(functionAddress.getUnsignedPayload()))
                                                 .exitProgramCounter;
                            continue;
                        }
                    }
                }
                if (header.opcodeAndFlags & StagedRecord::kInternalOpcodeBit)
                {
                    executeInternal(header);
                    continue;
                }

                HGBCOpcode opcode = HGBCOpcode(header.opcodeAndFlags & StagedRecord::kOpcodeMask);
                switch (opcode)
                {
                case HGBCOpcode::Add:
                case HGBCOpcode::Sub:
                case HGBCOpcode::Mul:
                case HGBCOpcode::IDiv:
                case HGBCOpcode::IRem:
                case HGBCOpcode::Lsh:
                case HGBCOpcode::FDiv:
                case HGBCOpcode::Pow:
                case HGBCOpcode::Min:
                case HGBCOpcode::Max:
                case HGBCOpcode::Atan2:
                    executeArithmetic(header);
                    break;
                case HGBCOpcode::Eql:
                case HGBCOpcode::Neq:
                case HGBCOpcode::Less:
                case HGBCOpcode::Greater:
                case HGBCOpcode::Leq:
                case HGBCOpcode::Geq:
                    executeComparison(header);
                    break;
                case HGBCOpcode::Neg:
                case HGBCOpcode::Not:
                case HGBCOpcode::Sin:
                case HGBCOpcode::Cos:
                case HGBCOpcode::Tan:
                case HGBCOpcode::Asin:
                case HGBCOpcode::Sqrt:
                case HGBCOpcode::Ceil:
                case HGBCOpcode::Floor:
                case HGBCOpcode::Exp:
                case HGBCOpcode::Exp2:
                case HGBCOpcode::Log:
                case HGBCOpcode::Log2:
                    executeUnary(header);
                    break;
                case HGBCOpcode::Any:
                    executeAny(header);
                    break;
                case HGBCOpcode::ScalarTypeCast:
                    executeScalarTypeCast(header);
                    break;
                case HGBCOpcode::Var:
                    executeVar(header);
                    break;
                case HGBCOpcode::Load:
                    executeLoad(header);
                    break;
                case HGBCOpcode::ResourceToDescriptorHandle:
                    executeResourceToDescriptorHandle(header);
                    break;
                case HGBCOpcode::DescriptorHandleToResource:
                    executeDescriptorHandleToResource(header);
                    break;
                case HGBCOpcode::Store:
                    executeStore(header);
                    break;
                case HGBCOpcode::FieldAddress:
                    executeFieldAddress(header);
                    break;
                case HGBCOpcode::ElementAddress:
                    executeElementAddress(header);
                    break;
                case HGBCOpcode::FieldExtract:
                    executeFieldExtract(header);
                    break;
                case HGBCOpcode::ExtractElement:
                    executeElementExtract(header);
                    break;
                case HGBCOpcode::MakeStruct:
                case HGBCOpcode::MakeArray:
                    executeMakeAggregate(header);
                    break;
                case HGBCOpcode::MakeVector:
                case HGBCOpcode::MakeMatrix:
                    executeMakeNumeric(header);
                    break;
                case HGBCOpcode::Call:
                    executeCall(header);
                    break;
                case HGBCOpcode::FormatString:
                    executeFormatString(header);
                    break;
                case HGBCOpcode::Print:
                    executePrint(header);
                    break;
                case HGBCOpcode::Fail:
                    executeFail(header);
                    break;
                case HGBCOpcode::StringGetByteCount:
                    executeStringGetByteCount(header);
                    break;
                case HGBCOpcode::StringReadByte:
                    executeStringReadByte(header);
                    break;
                case HGBCOpcode::CreateTimer:
                case HGBCOpcode::TimerElapsed:
                case HGBCOpcode::ResetTimer:
                    executeTimerOperation(opcode, header);
                    break;
                case HGBCOpcode::MakeRaytracingPipeline:
                case HGBCOpcode::SetRayMiss:
                case HGBCOpcode::SetRayHitGroup:
                    executeRayShaderOperation(opcode, header);
                    break;
                case HGBCOpcode::DispatchRays:
                    deviceState->executeRayDispatch(*this, header);
                    break;
                case HGBCOpcode::AllocAccelerationStructure:
                    {
                        const auto& record =
                            readRecord<StagedRecord::AllocRayAccelerationStructure>(programCounter);
                        auto* structure =
                            allocateHeapObject<DeviceState::RayAccelerationStructureObject>(
                                SLANG_HGVM_TRACKER(deviceState->eventTracker),
                                record.topLevel);
                        writeValue(record.header.resultAddress, Address::fromObject(structure));
                        advance(header);
                    }
                    break;
                case HGBCOpcode::BuildAccelerationStructure:
                    deviceState->buildRayAccelerationStructure(
                        *this,
                        readRecord<StagedRecord::BuildRayAccelerationStructure>(programCounter));
                    advance(header);
                    break;
                case HGBCOpcode::Return:
                    executeReturn(header);
                    break;
                case HGBCOpcode::Branch:
                    executeBranch(header);
                    break;
                case HGBCOpcode::BranchConditional:
                    executeConditionalBranch(header);
                    break;
                case HGBCOpcode::AllocateObject:
                    executeAllocateObject(header);
                    break;
                case HGBCOpcode::Box:
                    executeBox(header);
                    break;
                case HGBCOpcode::Unbox:
                case HGBCOpcode::ExtractTaggedUnion:
                    executeUnbox(opcode, header);
                    break;
                case HGBCOpcode::MakeTaggedUnion:
                    executeMakeTaggedUnion(header);
                    break;
                case HGBCOpcode::GetTaggedUnionCase:
                case HGBCOpcode::GetTaggedUnionPayload:
                    executeTaggedUnionAccess(opcode, header);
                    break;
                case HGBCOpcode::ConvertTaggedUnion:
                    executeConvertTaggedUnion(header);
                    break;
                case HGBCOpcode::AllocateRuntimeArray:
                    executeAllocateRuntimeArray(header);
                    break;
                case HGBCOpcode::RuntimeArrayGetCount:
                    executeRuntimeArrayGetCount(header);
                    break;
                case HGBCOpcode::CopyArrayRange:
                    executeCopyArrayRange(header);
                    break;
                case HGBCOpcode::AllocateResource:
                    executeAllocateResource(header);
                    break;
                case HGBCOpcode::MakeGraphicsDraw:
                    executeMakeGraphicsDraw(header);
                    break;
                case HGBCOpcode::MakeGraphicsDrawParameters:
                    executeMakeGraphicsDrawParameters(header);
                    break;
                case HGBCOpcode::ReadBackBufferElement:
                    executeReadBackBufferElement(header);
                    break;
                case HGBCOpcode::AsyncReadBackBufferElement:
                    executeAsyncReadBackBufferElement(header);
                    break;
                case HGBCOpcode::WaitDefer:
                    executeWaitDefer(header);
                    break;
                case HGBCOpcode::StructuredBufferGetDimensions:
                    executeStructuredBufferDimensions(header);
                    break;
                case HGBCOpcode::TextureGetDimensions:
                    executeTextureDimensions(header);
                    break;
                case HGBCOpcode::BlitTexture2D:
                    executeBlitTexture(header);
                    break;
                case HGBCOpcode::ClearTexture:
                    executeClearTexture(header);
                    break;
                case HGBCOpcode::CreateWindow:
                    executeCreateWindow(header);
                    break;
                case HGBCOpcode::ProcessWindowEvents:
                case HGBCOpcode::CloseWindow:
                    executeWindowOperation(opcode, header);
                    break;
                case HGBCOpcode::PollInputEvent:
                    executePollInputEvent(header);
                    break;
                case HGBCOpcode::SetWindowTitle:
                    executeSetWindowTitle(header);
                    break;
                case HGBCOpcode::CreateSurface:
                    executeCreateSurface(header);
                    break;
                case HGBCOpcode::AcquireSurfaceTexture:
                case HGBCOpcode::PresentSurface:
                case HGBCOpcode::TryPresentSurface:
                    executeSurfaceOperation(opcode, header);
                    break;
                case HGBCOpcode::TryAcquireSurfaceTexture:
                    executeTryAcquireSurfaceTexture(header);
                    break;
                case HGBCOpcode::ResizeSurface:
                    executeResizeSurface(header);
                    break;
                case HGBCOpcode::ByteStreamFromFile:
                    executeByteStreamFromFile(header);
                    break;
                case HGBCOpcode::CloseByteStream:
                case HGBCOpcode::ByteStreamGetSize:
                    executeByteStreamOperation(header);
                    break;
                case HGBCOpcode::ByteStreamRead:
                case HGBCOpcode::ByteStreamWrite:
                    executeByteStreamTransfer(header);
                    break;
                case HGBCOpcode::ImageDecode:
                case HGBCOpcode::ImageEncode:
                    executeImageOperation(header);
                    break;
                default:
                    {
                        SLANG_ASSERT(!"HGVM encountered an unstaged HGBC opcode");
                        return;
                    }
                }
            }
            catch (const Exception& exception)
            {
                if (isUnwindingFailure)
                    throw;
                isUnwindingFailure = true;
                failureMessage = exception.Message;
                deviceState->abandonPendingCommands();
                StagedRecord::Function functionRecord = readRecord<StagedRecord::Function>(
                    uint32_t(functionAddress.getUnsignedPayload()));
                programCounter = functionRecord.exitProgramCounter;
            }
        }
    }
};

/// Narrows one trusted reflected byte extent after ruling out Slang's sentinel values.
///
/// Consider a `float3` lane layout: reflection reports four bytes, which becomes the bounded
/// extent used by the numeric marshaller. Unknown, unbounded, and oversized values indicate an
/// internal compiler/RHI contract violation and are assertions rather than runtime validation.
uint32_t getReflectedByteSize(size_t byteSize)
{
    SLANG_ASSERT(
        byteSize != SLANG_UNKNOWN_SIZE && byteSize != SLANG_UNBOUNDED_SIZE &&
        byteSize <= std::numeric_limits<uint32_t>::max());
    return uint32_t(byteSize);
}

/// Copies one canonical interpreter scalar into its reflected device representation.
///
/// Consider an HGVM `bool`: it occupies one byte in interpreter storage but commonly occupies
/// four bytes in shader uniform storage. Other scalar kinds preserve their staged bit pattern and
/// width because Slang has already checked that the source and shader types agree.
void copyScalarForBinding(
    ScalarType scalarType,
    const uint8_t* source,
    uint32_t sourceByteSize,
    uint8_t* destination,
    uint32_t destinationByteSize)
{
    if (scalarType != ScalarType::Bool)
    {
        SLANG_ASSERT(sourceByteSize == destinationByteSize);
        std::memcpy(destination, source, sourceByteSize);
        return;
    }

    SLANG_ASSERT(sourceByteSize == sizeof(uint8_t));
    bool value = source[0] != 0;
    switch (destinationByteSize)
    {
    case sizeof(uint8_t):
        {
            uint8_t bits = value ? 1 : 0;
            std::memcpy(destination, &bits, sizeof(bits));
        }
        return;
    case sizeof(uint16_t):
        {
            uint16_t bits = value ? 1 : 0;
            std::memcpy(destination, &bits, sizeof(bits));
        }
        return;
    case sizeof(uint32_t):
        {
            uint32_t bits = value ? 1 : 0;
            std::memcpy(destination, &bits, sizeof(bits));
        }
        return;
    case sizeof(uint64_t):
        {
            uint64_t bits = value ? 1 : 0;
            std::memcpy(destination, &bits, sizeof(bits));
        }
        return;
    default:
        SLANG_ASSERT(!"HGVM encountered an unsupported reflected bool width");
        return;
    }
}

/// Copies one reflected device scalar back into its canonical interpreter representation.
///
/// This is the inverse of `copyScalarForBinding`. For example, a four-byte device boolean becomes
/// one canonical byte, while integer and floating-point leaves retain their checked bit width.
void copyScalarFromDevice(
    ScalarType scalarType,
    const uint8_t* source,
    uint32_t sourceByteSize,
    uint8_t* destination,
    uint32_t destinationByteSize)
{
    if (scalarType != ScalarType::Bool)
    {
        SLANG_ASSERT(sourceByteSize == destinationByteSize);
        std::memcpy(destination, source, sourceByteSize);
        return;
    }

    SLANG_ASSERT(destinationByteSize == sizeof(uint8_t));
    bool value = false;
    switch (sourceByteSize)
    {
    case sizeof(uint8_t):
        value = *source != 0;
        break;
    case sizeof(uint16_t):
        {
            uint16_t bits = 0;
            std::memcpy(&bits, source, sizeof(bits));
            value = bits != 0;
        }
        break;
    case sizeof(uint32_t):
        {
            uint32_t bits = 0;
            std::memcpy(&bits, source, sizeof(bits));
            value = bits != 0;
        }
        break;
    case sizeof(uint64_t):
        {
            uint64_t bits = 0;
            std::memcpy(&bits, source, sizeof(bits));
            value = bits != 0;
        }
        break;
    default:
        SLANG_ASSERT(!"HGVM encountered an unsupported reflected bool width");
        return;
    }
    *destination = value ? 1 : 0;
}

/// Finds the reflected spelling of one compact HGBC struct field.
///
/// Consider `struct S { float value; Empty tag; }`. HGBC optimization may retain only `value`,
/// while device reflection still reports both declarations. Resolving by the serialized field
/// name finds the correct reflected index without recreating the source struct's physical shape.
static slang::VariableLayoutReflection* findReflectedStructField(
    InterpreterState& interpreter,
    slang::TypeLayoutReflection* targetLayout,
    const StagedRecord::StructFieldInfo& sourceField,
    uint32_t* outTargetFieldIndex = nullptr)
{
    SLANG_ASSERT(targetLayout && targetLayout->getKind() == slang::TypeReflection::Kind::Struct);
    SLANG_RELEASE_ASSERT(interpreter.program && interpreter.program->sourceProgram);
    String fieldName =
        interpreter.readLiteralString(sourceField.fieldNameAddress, sourceField.fieldNameByteCount);

    SlangInt targetFieldIndex = targetLayout->findFieldIndexByName(
        fieldName.getBuffer(),
        fieldName.getBuffer() + fieldName.getLength());
    if (targetFieldIndex < 0)
        return nullptr;
    SLANG_RELEASE_ASSERT(uint64_t(targetFieldIndex) <= std::numeric_limits<uint32_t>::max());
    uint32_t reflectedIndex = uint32_t(targetFieldIndex);
    if (outTargetFieldIndex)
        *outTargetFieldIndex = reflectedIndex;
    return targetLayout->getFieldByIndex(reflectedIndex);
}

/// Flattens one reflected structured-buffer element into scalar host/device operations.
///
/// Consider `Array<struct { Array<bool, 3> flags; uint value; }, 2>`. Reflection supplies the
/// device array/field offsets, while staged type descriptors supply the compact interpreter
/// offsets. Recursing once when the converter is first read produces a flat plan used by every
/// later readback, even when the buffer has never been bound to a shader.
void DeviceState::appendLayoutConversionOperations(
    InterpreterState& interpreter,
    slang::TypeLayoutReflection* targetLayout,
    Address typeInfoAddress,
    uint32_t deviceByteOffset,
    uint32_t interpreterByteOffset,
    LayoutConversionDirection direction,
    List<LayoutConversionOperation>& operations)
{
    SLANG_ASSERT(targetLayout);
    const auto& typeInfo =
        interpreter.readInstructionRecord<StagedRecord::TypeInfo>(typeInfoAddress);
    switch (typeInfo.kind)
    {
    case TypeKind::Scalar:
        {
            LayoutConversionOperation operation;
            operation.scalarType = typeInfo.numericLayout.scalarType;
            uint32_t deviceByteSize = getReflectedByteSize(targetLayout->getSize());
            if (direction == LayoutConversionDirection::DeviceToHost)
            {
                operation.sourceByteOffset = deviceByteOffset;
                operation.destinationByteOffset = interpreterByteOffset;
                operation.sourceByteSize = deviceByteSize;
                operation.destinationByteSize = typeInfo.byteSize;
            }
            else
            {
                operation.sourceByteOffset = interpreterByteOffset;
                operation.destinationByteOffset = deviceByteOffset;
                operation.sourceByteSize = typeInfo.byteSize;
                operation.destinationByteSize = deviceByteSize;
            }
            operations.add(operation);
        }
        break;
    case TypeKind::Vector:
        {
            SLANG_ASSERT(targetLayout->getKind() == slang::TypeReflection::Kind::Vector);
            SLANG_ASSERT(targetLayout->getElementCount() == typeInfo.elementCount);
            slang::TypeLayoutReflection* targetElementLayout = targetLayout->getElementTypeLayout();
            SLANG_ASSERT(targetElementLayout);
            uint32_t targetElementStride = getReflectedByteSize(
                targetLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM));
            for (uint32_t laneIndex = 0; laneIndex < typeInfo.elementCount; ++laneIndex)
            {
                appendLayoutConversionOperations(
                    interpreter,
                    targetElementLayout,
                    typeInfo.elementTypeInfoAddress,
                    deviceByteOffset + laneIndex * targetElementStride,
                    interpreterByteOffset + laneIndex * typeInfo.elementByteStride,
                    direction,
                    operations);
            }
        }
        break;
    case TypeKind::Matrix:
        {
            SLANG_ASSERT(targetLayout->getKind() == slang::TypeReflection::Kind::Matrix);
            bool columnMajor =
                targetLayout->getMatrixLayoutMode() == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
            uint32_t majorCount =
                columnMajor ? typeInfo.numericLayout.columnCount : typeInfo.numericLayout.rowCount;
            uint32_t minorCount =
                columnMajor ? typeInfo.numericLayout.rowCount : typeInfo.numericLayout.columnCount;
            uint32_t targetByteSize =
                getReflectedByteSize(targetLayout->getStride(SLANG_PARAMETER_CATEGORY_UNIFORM));
            SLANG_ASSERT(majorCount && targetByteSize % majorCount == 0);
            uint32_t targetMajorStride = targetByteSize / majorCount;
            slang::TypeLayoutReflection* targetVectorLayout = targetLayout->getElementTypeLayout();
            slang::TypeLayoutReflection* targetElementLayout =
                targetVectorLayout ? targetVectorLayout->getElementTypeLayout() : nullptr;
            SLANG_ASSERT(targetElementLayout);
            uint32_t targetElementSize = getReflectedByteSize(targetElementLayout->getSize());
            for (uint32_t majorIndex = 0; majorIndex < majorCount; ++majorIndex)
            {
                for (uint32_t minorIndex = 0; minorIndex < minorCount; ++minorIndex)
                {
                    uint32_t rowIndex = columnMajor ? minorIndex : majorIndex;
                    uint32_t columnIndex = columnMajor ? majorIndex : minorIndex;
                    uint32_t interpreterLane =
                        rowIndex * typeInfo.numericLayout.columnCount + columnIndex;
                    appendLayoutConversionOperations(
                        interpreter,
                        targetElementLayout,
                        typeInfo.elementTypeInfoAddress,
                        deviceByteOffset + majorIndex * targetMajorStride +
                            minorIndex * targetElementSize,
                        interpreterByteOffset +
                            interpreterLane * typeInfo.numericLayout.laneByteStride,
                        direction,
                        operations);
                }
            }
        }
        break;
    case TypeKind::Array:
        {
            SLANG_ASSERT(targetLayout->getKind() == slang::TypeReflection::Kind::Array);
            SLANG_ASSERT(targetLayout->getElementCount() == typeInfo.elementCount);
            slang::TypeLayoutReflection* targetElementLayout = targetLayout->getElementTypeLayout();
            SLANG_ASSERT(targetElementLayout);
            uint32_t targetElementStride = getReflectedByteSize(
                targetLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM));
            for (uint32_t elementIndex = 0; elementIndex < typeInfo.elementCount; ++elementIndex)
            {
                appendLayoutConversionOperations(
                    interpreter,
                    targetElementLayout,
                    typeInfo.elementTypeInfoAddress,
                    deviceByteOffset + elementIndex * targetElementStride,
                    interpreterByteOffset + elementIndex * typeInfo.elementByteStride,
                    direction,
                    operations);
            }
        }
        break;
    case TypeKind::Struct:
        {
            SLANG_ASSERT(targetLayout->getKind() == slang::TypeReflection::Kind::Struct);
            if (!typeInfo.fieldCount)
                break;
            auto fields = typeInfo.getFields();
            for (uint32_t fieldIndex = 0; fieldIndex < typeInfo.fieldCount; ++fieldIndex)
            {
                const StagedRecord::StructFieldInfo& field = fields[fieldIndex];
                slang::VariableLayoutReflection* targetField =
                    findReflectedStructField(interpreter, targetLayout, field);
                if (!targetField || !targetField->getTypeLayout())
                    fail("HGVM could not match a structured-buffer field to device reflection");
                uint32_t targetFieldOffset =
                    getReflectedByteSize(targetField->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM));
                appendLayoutConversionOperations(
                    interpreter,
                    targetField->getTypeLayout(),
                    field.typeInfoAddress,
                    deviceByteOffset + targetFieldOffset,
                    interpreterByteOffset + field.sourceByteOffset,
                    direction,
                    operations);
            }
        }
        break;
    case TypeKind::Address:
    case TypeKind::String:
        SLANG_UNREACHABLE("non-POD type reached host-device layout conversion");
    case TypeKind::HeapObject:
        if (typeInfo.heapObjectKind == HeapObjectKind::Box)
            fail("HGVM boxed values cannot be converted to raw device storage");
        SLANG_UNREACHABLE("non-POD type reached host-device layout conversion");
    case TypeKind::TaggedUnion:
        fail("HGVM boxed union values cannot be converted to raw device storage");
    }
}

/// Flattens one host type and its reflected shader layout into leaf binding rules.
///
/// Consider `struct S { Array<float, 2> a; Empty tag; RWStructuredBuffer<uint> b; }`. If host
/// optimization removes `tag`, field-name lookup still records reflected cursor paths `{0,0}`,
/// `{0,1}`, and `{2}` plus the corresponding staged source offsets. A later dispatch follows those
/// integer paths directly and never asks whether `S` is a struct or array.
void DeviceState::appendShaderBindingOperations(
    InterpreterState& interpreter,
    slang::TypeLayoutReflection* targetLayout,
    Address typeInfoAddress,
    uint32_t sourceByteOffset,
    std::vector<uint32_t>& sourceDereferenceOffsets,
    std::vector<uint32_t>& cursorPath,
    LayoutConverterObject* converter)
{
    SLANG_ASSERT(targetLayout && converter);
    const auto& typeInfo =
        interpreter.readInstructionRecord<StagedRecord::TypeInfo>(typeInfoAddress);

    switch (typeInfo.kind)
    {
    case TypeKind::TaggedUnion:
        fail("HGVM boxed union values cannot be bound directly to device shaders");
    case TypeKind::Address:
        {
            sourceDereferenceOffsets.push_back(sourceByteOffset);
            appendShaderBindingOperations(
                interpreter,
                targetLayout,
                typeInfo.valueTypeInfoAddress,
                0,
                sourceDereferenceOffsets,
                cursorPath,
                converter);
            sourceDereferenceOffsets.pop_back();
        }
        break;
    case TypeKind::Array:
        {
            SLANG_ASSERT(targetLayout->getKind() == slang::TypeReflection::Kind::Array);
            SLANG_ASSERT(targetLayout->getElementCount() == typeInfo.elementCount);
            slang::TypeLayoutReflection* targetElementLayout = targetLayout->getElementTypeLayout();
            SLANG_ASSERT(targetElementLayout);
            for (uint32_t elementIndex = 0; elementIndex < typeInfo.elementCount; ++elementIndex)
            {
                uint64_t childByteOffset = uint64_t(sourceByteOffset) +
                                           uint64_t(elementIndex) * typeInfo.elementByteStride;
                SLANG_ASSERT(childByteOffset <= std::numeric_limits<uint32_t>::max());
                cursorPath.push_back(elementIndex);
                appendShaderBindingOperations(
                    interpreter,
                    targetElementLayout,
                    typeInfo.elementTypeInfoAddress,
                    uint32_t(childByteOffset),
                    sourceDereferenceOffsets,
                    cursorPath,
                    converter);
                cursorPath.pop_back();
            }
        }
        break;
    case TypeKind::Struct:
        {
            SLANG_ASSERT(targetLayout->getKind() == slang::TypeReflection::Kind::Struct);
            auto fields = typeInfo.getFields();
            for (uint32_t fieldIndex = 0; fieldIndex < typeInfo.fieldCount; ++fieldIndex)
            {
                const StagedRecord::StructFieldInfo& field = fields[fieldIndex];
                uint32_t targetFieldIndex = 0;
                slang::VariableLayoutReflection* targetField =
                    findReflectedStructField(interpreter, targetLayout, field, &targetFieldIndex);
                // Device specialization can remove a field that the host value still carries.
                // It has no shader cursor and therefore needs no binding operation.
                if (!targetField)
                    continue;
                SLANG_ASSERT(targetField->getTypeLayout());
                uint64_t childByteOffset = uint64_t(sourceByteOffset) + field.sourceByteOffset;
                SLANG_ASSERT(childByteOffset <= std::numeric_limits<uint32_t>::max());
                cursorPath.push_back(targetFieldIndex);
                appendShaderBindingOperations(
                    interpreter,
                    targetField->getTypeLayout(),
                    field.typeInfoAddress,
                    uint32_t(childByteOffset),
                    sourceDereferenceOffsets,
                    cursorPath,
                    converter);
                cursorPath.pop_back();
            }
        }
        break;
    case TypeKind::Scalar:
    case TypeKind::Vector:
    case TypeKind::Matrix:
    case TypeKind::HeapObject:
        {
            ShaderBindingOperation operation;
            operation.typeInfoAddress = typeInfoAddress;
            if (typeInfo.kind == TypeKind::HeapObject)
            {
                if (typeInfo.heapObjectKind == HeapObjectKind::Box)
                    fail("HGVM boxed values cannot be bound directly to device shaders");
                operation.mode = ShaderBindingMode::Resource;
                if (typeInfo.heapObjectKind == HeapObjectKind::RaytracingPipeline)
                    operation.mode = ShaderBindingMode::Data;
                if (typeInfo.descriptorAccess != DescriptorAccess::None)
                {
                    if (targetLayout->getKind() == slang::TypeReflection::Kind::Resource)
                    {
                        if (typeInfo.heapObjectKind != HeapObjectKind::StructuredBuffer)
                            fail("HGVM texture and acceleration-structure handles require a "
                                 "bindless device "
                                 "layout");
                        SlangResourceAccess expectedAccess =
                            typeInfo.descriptorAccess == DescriptorAccess::Read
                                ? SLANG_RESOURCE_ACCESS_READ
                                : SLANG_RESOURCE_ACCESS_READ_WRITE;
                        slang::BindingType expectedBinding =
                            typeInfo.descriptorAccess == DescriptorAccess::Read
                                ? slang::BindingType::RawBuffer
                                : slang::BindingType::MutableRawBuffer;
                        if ((targetLayout->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) !=
                                SLANG_STRUCTURED_BUFFER ||
                            targetLayout->getResourceAccess() != expectedAccess ||
                            targetLayout->getBindingRangeCount() != 1 ||
                            targetLayout->getBindingRangeType(0) != expectedBinding)
                        {
                            fail("HGVM native descriptor-handle reflection does not match its "
                                 "buffer type");
                        }
                    }
                    else
                    {
                        if (targetLayout->getBindingRangeCount() != 0)
                            fail("HGVM descriptor-handle data unexpectedly consumes a resource "
                                 "binding");
                        operation.mode = ShaderBindingMode::DescriptorHandle;
                    }
                }
            }
            operation.sourceByteOffset = sourceByteOffset;
            operation.sourceDereferenceOffset =
                uint32_t(converter->sourceDereferenceOffsets.getCount());
            operation.sourceDereferenceCount = uint32_t(sourceDereferenceOffsets.size());
            operation.cursorPathOffset = uint32_t(converter->cursorPathIndices.getCount());
            operation.cursorPathCount = uint32_t(cursorPath.size());
            for (uint32_t offset : sourceDereferenceOffsets)
                converter->sourceDereferenceOffsets.add(offset);
            for (uint32_t index : cursorPath)
                converter->cursorPathIndices.add(index);
            converter->shaderBindingOperations.add(operation);
        }
        break;
    case TypeKind::String:
        SLANG_UNREACHABLE("string reached shader binding conversion");
    }
}

/// Returns the RHI descriptor heap entry corresponding to one typed HGVM handle.
rhi::DescriptorHandle DeviceState::getBufferDescriptorHandle(
    StructuredBufferObject* buffer,
    DescriptorAccess access)
{
    SLANG_ASSERT(buffer && access != DescriptorAccess::None);
    if (!device || !device->hasFeature(rhi::Feature::Bindless))
        fail("HGVM RHI device does not support bindless descriptors");

    rhi::DescriptorHandle descriptor = {};
    rhi::DescriptorHandleAccess rhiAccess = access == DescriptorAccess::Read
                                                ? rhi::DescriptorHandleAccess::Read
                                                : rhi::DescriptorHandleAccess::ReadWrite;
    if (SLANG_FAILED(buffer->resource->getDescriptorHandle(
            rhiAccess,
            rhi::Format::Undefined,
            rhi::kEntireBuffer,
            &descriptor)))
    {
        fail("HGVM RHI failed to allocate a buffer descriptor handle");
    }
    return descriptor;
}

/// Resolves a typed texture handle to a cached native view rather than a resource-binding slot.
/// Consider a mipmapped Texture2D<float>: its descriptor can load every mip, while an
/// RWTexture2D<float> descriptor writes only mip zero. The native view owns the descriptor heap
/// entry, so retaining just the texture would let a dropped graph handle recycle that entry
/// while a queued dispatch still uses it.
rhi::DescriptorHandle DeviceState::getTextureDescriptorHandle(
    Texture2DObject* texture,
    DescriptorAccess access)
{
    SLANG_ASSERT(texture && access != DescriptorAccess::None);
    if (!device || !device->hasFeature(rhi::Feature::Bindless))
        fail("HGVM RHI device does not support bindless descriptors");
    bool readOnly = access == DescriptorAccess::Read;
    auto usage = readOnly ? rhi::TextureUsage::ShaderResource : rhi::TextureUsage::UnorderedAccess;
    if (!rhi::is_set(texture->resource->getDesc().usage, usage))
        fail("HGVM texture descriptor access does not match its resource usage");
    auto& view = readOnly ? texture->readDescriptorView : texture->writeDescriptorView;
    if (!view)
    {
        rhi::TextureViewDesc desc;
        // A float depth texture samples depth even when its storage also contains stencil.
        if (rhi::getFormatInfo(texture->resource->getDesc().format).hasDepth)
            desc.aspect = rhi::TextureAspect::DepthOnly;
        if (!readOnly)
            desc.subresourceRange.mipCount = 1;
        view = texture->resource->createView(desc);
        if (!view)
            fail("HGVM RHI failed to create a texture descriptor view");
    }
    rhi::DescriptorHandle descriptor = {};
    auto rhiAccess =
        readOnly ? rhi::DescriptorHandleAccess::Read : rhi::DescriptorHandleAccess::ReadWrite;
    if (SLANG_FAILED(view->getDescriptorHandle(rhiAccess, &descriptor)))
        fail("HGVM RHI failed to allocate a texture descriptor handle");
    pendingBatch.holds.textureViews.add(view);
    return descriptor;
}

/// Resolves the source address shared by descriptor preflight and ordinary shader binding.
Address DeviceState::resolveShaderBindingSourceAddress(
    InterpreterState& interpreter,
    Address argumentAddress,
    const ShaderBindingOperation& operation,
    LayoutConverterObject* converter)
{
    Address sourceAddress = argumentAddress;
    for (uint32_t dereferenceIndex = 0; dereferenceIndex < operation.sourceDereferenceCount;
         ++dereferenceIndex)
    {
        uint32_t offset =
            converter
                ->sourceDereferenceOffsets[operation.sourceDereferenceOffset + dereferenceIndex];
        sourceAddress = interpreter.readValueAt<Address>(sourceAddress, offset);
    }
    return sourceAddress;
}

/// Resolves a scene handle using the current dispatch's geometry stride.
rhi::DescriptorHandle DeviceState::getAccelerationStructureDescriptorHandle(
    RayAccelerationStructureObject* structure)
{
    if (!device || !device->hasFeature(rhi::Feature::Bindless))
        fail("HGVM RHI device does not support bindless descriptors");
    auto resource = realizeRayAccelerationStructure(structure, rayBindingStride);
    rhi::DescriptorHandle descriptor;
    if (SLANG_FAILED(resource->getDescriptorHandle(&descriptor)))
        fail("HGVM RHI failed to allocate an acceleration-structure descriptor handle");
    pendingBatch.holds.accelerationStructures.add(resource);
    for (const auto& bottomLevel : structure->bottomLevels)
        pendingBatch.holds.accelerationStructures.add(bottomLevel);
    return descriptor;
}

/// Declares one explicit descriptor's use before any pass binds the shared descriptor heap.
/// The same preflight handles compute arguments and deferred shader-record arguments.
void DeviceState::prepareDescriptorResource(
    InterpreterState& interpreter,
    Address sourceAddress,
    const ShaderBindingOperation& operation,
    rhi::ICommandEncoder* encoder,
    Dictionary<HeapObject*, DescriptorAccess>& resources)
{
    if (operation.mode != ShaderBindingMode::DescriptorHandle)
        return;
    const auto& typeInfo =
        interpreter.readInstructionRecord<StagedRecord::TypeInfo>(operation.typeInfoAddress);
    Address handle = interpreter.readValueAt<Address>(sourceAddress, operation.sourceByteOffset);
    if (!handle)
        return;
    auto* object = handle.getObject();
    SLANG_RELEASE_ASSERT(object->kind == typeInfo.heapObjectKind);
    if (object->kind == HeapObjectKind::RayAccelerationStructure)
    {
        getAccelerationStructureDescriptorHandle(
            static_cast<RayAccelerationStructureObject*>(object));
        return;
    }
    auto found = resources.tryGetValue(object);
    if (found && *found != typeInfo.descriptorAccess)
        fail("HGVM cannot bind one resource through conflicting descriptor access modes");
    resources[object] = typeInfo.descriptorAccess;
    // Resolving an earlier AS may submit a synchronous build and replace the batch encoder.
    encoder = getPendingCommandEncoder();
    auto state = typeInfo.descriptorAccess == DescriptorAccess::Read
                     ? rhi::ResourceState::ShaderResource
                     : rhi::ResourceState::UnorderedAccess;
    if (object->kind == HeapObjectKind::Texture2D)
    {
        auto* texture = static_cast<Texture2DObject*>(object);
        getTextureDescriptorHandle(texture, typeInfo.descriptorAccess);
        encoder->setTextureState(texture->resource, state);
    }
    else
    {
        SLANG_RELEASE_ASSERT(object->kind == HeapObjectKind::StructuredBuffer);
        auto* buffer = static_cast<StructuredBufferObject*>(object);
        getBufferDescriptorHandle(buffer, typeInfo.descriptorAccess);
        encoder->setBufferState(buffer->resource, state);
        pendingBatch.holds.bufferLeases.add(buffer);
    }
}

/// Prepares every descriptor-backed resource before entering a compute pass.
///
/// Descriptor handles deliberately carry IDs rather than RHI bindings, so RHI cannot infer
/// resource states from `setDescriptorHandle`. Resolve the referenced graph resources here,
/// allocate their cached descriptors, and declare their access in this dispatch. This also avoids
/// updating a backend descriptor heap after the pass has already bound it.
void DeviceState::prepareDescriptorResources(
    InterpreterState& interpreter,
    ConstArrayView<StagedRecord::DispatchArgument> arguments,
    rhi::ICommandEncoder* encoder)
{
    Dictionary<HeapObject*, DescriptorAccess> resources;
    for (const auto& argument : arguments)
    {
        LayoutConverterObject* converter =
            interpreter.readConstObject<LayoutConverterObject>(argument.converterAddress);
        for (const ShaderBindingOperation& operation : converter->shaderBindingOperations)
        {
            Address sourceAddress = resolveShaderBindingSourceAddress(
                interpreter,
                argument.valueAddress,
                operation,
                converter);
            prepareDescriptorResource(interpreter, sourceAddress, operation, encoder, resources);
        }
    }
}

/// Applies one pre-flattened numeric or resource binding rule.
///
/// Consider the `output` buffer leaf in `struct Args { bool enabled; Buffer output; }`: converter
/// resolution has already selected its host offset and cursor path, so this function only reads
/// the heap handle and binds it. Aggregate kinds cannot reach this execution path.
void DeviceState::bindComputeLeaf(
    InterpreterState& interpreter,
    const rhi::ShaderCursor& cursor,
    Address sourceAddress,
    uint32_t sourceByteOffset,
    const ShaderBindingOperation& operation)
{
    SLANG_ASSERT(cursor.isValid());
    const auto& typeInfo =
        interpreter.readInstructionRecord<StagedRecord::TypeInfo>(operation.typeInfoAddress);

    switch (typeInfo.kind)
    {
    case TypeKind::HeapObject:
        {
            SlangResult result = SLANG_FAIL;
            switch (typeInfo.heapObjectKind)
            {
            case HeapObjectKind::StructuredBuffer:
                {
                    Address heapAddress =
                        interpreter.readValueAt<Address>(sourceAddress, sourceByteOffset);
                    StructuredBufferObject* buffer = nullptr;
                    if (heapAddress)
                    {
                        HeapObject* object = heapAddress.getObject();
                        SLANG_ASSERT(object && object->kind == HeapObjectKind::StructuredBuffer);
                        buffer = static_cast<StructuredBufferObject*>(object);
                    }

                    if (operation.mode == ShaderBindingMode::DescriptorHandle)
                    {
                        SLANG_ASSERT(typeInfo.descriptorAccess != DescriptorAccess::None);
                        rhi::DescriptorHandle descriptor =
                            buffer ? getBufferDescriptorHandle(buffer, typeInfo.descriptorAccess)
                                   : rhi::DescriptorHandle{};
                        result = cursor.setDescriptorHandle(descriptor);
                    }
                    else
                    {
                        SLANG_ASSERT(operation.mode == ShaderBindingMode::Resource);
                        if (!buffer && typeInfo.descriptorAccess == DescriptorAccess::None)
                            fail("HGVM cannot bind a null structured-buffer resource");
                        result = cursor.setBinding(
                            rhi::Binding(buffer ? buffer->resource.get() : nullptr));
                    }
                    if (buffer)
                        pendingBatch.holds.bufferLeases.add(buffer);
                }
                break;
            case HeapObjectKind::Texture2D:
                {
                    Address handle =
                        interpreter.readValueAt<Address>(sourceAddress, sourceByteOffset);
                    SLANG_RELEASE_ASSERT(
                        !handle || handle.getObject()->kind == HeapObjectKind::Texture2D);
                    auto* texture =
                        handle ? static_cast<Texture2DObject*>(handle.getObject()) : nullptr;
                    if (operation.mode == ShaderBindingMode::DescriptorHandle)
                    {
                        result = cursor.setDescriptorHandle(
                            texture ? getTextureDescriptorHandle(texture, typeInfo.descriptorAccess)
                                    : rhi::DescriptorHandle{});
                    }
                    else
                    {
                        SLANG_ASSERT(operation.mode == ShaderBindingMode::Resource);
                        if (!texture)
                            fail("HGVM cannot bind a null texture resource");
                        result = cursor.setBinding(rhi::Binding(texture->resource));
                        pendingBatch.holds.textures.add(texture->resource);
                    }
                }
                break;
            case HeapObjectKind::RayAccelerationStructure:
                {
                    Address handle =
                        interpreter.readValueAt<Address>(sourceAddress, sourceByteOffset);
                    if (operation.mode != ShaderBindingMode::DescriptorHandle)
                        fail("HGVM ray resources must use getDescriptorHandle() instead of "
                             "resource slots");
                    auto* structure =
                        handle ? static_cast<RayAccelerationStructureObject*>(handle.getObject())
                               : nullptr;
                    result = cursor.setDescriptorHandle(
                        structure ? getAccelerationStructureDescriptorHandle(structure)
                                  : rhi::DescriptorHandle{});
                }
                break;
            case HeapObjectKind::RaytracingPipeline:
                {
                    Address handle =
                        interpreter.readValueAt<Address>(sourceAddress, sourceByteOffset);
                    if (!handle || handle.getObject() != rayBindingTracker)
                        fail("HGVM ray dispatch supports exactly one captured tracker");
                    // Multi-tracker dispatch assigns each tracker its own column offset here.
                    const uint32_t offset = 0;
                    result = cursor.getElement(0).setData(&offset, sizeof(offset));
                }
                break;
            case HeapObjectKind::ByteStream:
            case HeapObjectKind::String:
            case HeapObjectKind::DeferredReadback:
            case HeapObjectKind::Timer:
            case HeapObjectKind::Window:
            case HeapObjectKind::Surface:
            case HeapObjectKind::Class:
            case HeapObjectKind::RuntimeArray:
            case HeapObjectKind::Box:
                SLANG_UNREACHABLE("non-device heap object reached compute binding");
            }
            if (SLANG_FAILED(result))
                fail("HGVM RHI failed to bind a compute resource");
        }
        break;
    case TypeKind::Scalar:
        {
            slang::TypeLayoutReflection* targetLayout = cursor.getTypeLayout();
            SLANG_ASSERT(targetLayout);
            AddressRef<const uint8_t> source = interpreter.resolveAddressAt<const uint8_t>(
                sourceAddress,
                sourceByteOffset,
                typeInfo.byteSize);
            uint32_t targetByteSize = getReflectedByteSize(targetLayout->getSize());
            uint8_t data[sizeof(uint64_t)];
            SLANG_ASSERT(targetByteSize <= sizeof(data));
            copyScalarForBinding(
                typeInfo.numericLayout.scalarType,
                source.get(),
                typeInfo.byteSize,
                data,
                targetByteSize);
            if (SLANG_FAILED(cursor.setData(data, targetByteSize)))
                fail("HGVM RHI failed to bind a compute scalar");
        }
        break;
    case TypeKind::Vector:
        {
            slang::TypeLayoutReflection* targetLayout = cursor.getTypeLayout();
            SLANG_ASSERT(targetLayout);
            AddressRef<const uint8_t> source = interpreter.resolveAddressAt<const uint8_t>(
                sourceAddress,
                sourceByteOffset,
                typeInfo.byteSize);
            static constexpr uint32_t kMaximumShaderNumericByteSize = 4 * 4 * sizeof(uint64_t);
            SLANG_ASSERT(targetLayout->getKind() == slang::TypeReflection::Kind::Vector);
            SLANG_ASSERT(targetLayout->getElementCount() == typeInfo.elementCount);
            slang::TypeLayoutReflection* targetElementLayout = targetLayout->getElementTypeLayout();
            SLANG_ASSERT(targetElementLayout);
            uint32_t targetElementByteSize = getReflectedByteSize(targetElementLayout->getSize());
            uint32_t targetElementByteStride = getReflectedByteSize(
                targetLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM));
            SLANG_ASSERT(targetElementByteSize <= targetElementByteStride);
            uint32_t targetByteSize =
                typeInfo.elementCount
                    ? (typeInfo.elementCount - 1) * targetElementByteStride + targetElementByteSize
                    : 0;
            SLANG_ASSERT(targetByteSize <= kMaximumShaderNumericByteSize);
            uint8_t data[kMaximumShaderNumericByteSize] = {};
            for (uint32_t laneIndex = 0; laneIndex < typeInfo.elementCount; ++laneIndex)
            {
                copyScalarForBinding(
                    typeInfo.numericLayout.scalarType,
                    source.get() + laneIndex * typeInfo.numericLayout.laneByteStride,
                    typeInfo.numericLayout.laneByteStride,
                    data + laneIndex * targetElementByteStride,
                    targetElementByteSize);
            }
            if (SLANG_FAILED(cursor.setData(data, targetByteSize)))
                fail("HGVM RHI failed to bind a compute vector");
        }
        break;
    case TypeKind::Matrix:
        {
            slang::TypeLayoutReflection* targetLayout = cursor.getTypeLayout();
            SLANG_ASSERT(targetLayout);
            AddressRef<const uint8_t> source = interpreter.resolveAddressAt<const uint8_t>(
                sourceAddress,
                sourceByteOffset,
                typeInfo.byteSize);
            static constexpr uint32_t kMaximumShaderNumericByteSize = 4 * 4 * sizeof(uint64_t);
            SLANG_ASSERT(targetLayout->getKind() == slang::TypeReflection::Kind::Matrix);
            SLANG_ASSERT(targetLayout->getRowCount() == typeInfo.numericLayout.rowCount);
            SLANG_ASSERT(targetLayout->getColumnCount() == typeInfo.numericLayout.columnCount);
            bool columnMajor =
                targetLayout->getMatrixLayoutMode() == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
            uint32_t majorCount =
                columnMajor ? typeInfo.numericLayout.columnCount : typeInfo.numericLayout.rowCount;
            uint32_t minorCount =
                columnMajor ? typeInfo.numericLayout.rowCount : typeInfo.numericLayout.columnCount;
            uint32_t targetByteSize =
                getReflectedByteSize(targetLayout->getStride(SLANG_PARAMETER_CATEGORY_UNIFORM));
            SLANG_ASSERT(majorCount && targetByteSize % majorCount == 0);
            uint32_t targetMajorByteStride = targetByteSize / majorCount;
            slang::TypeLayoutReflection* targetVectorLayout = targetLayout->getElementTypeLayout();
            slang::TypeLayoutReflection* targetElementLayout =
                targetVectorLayout ? targetVectorLayout->getElementTypeLayout() : nullptr;
            SLANG_ASSERT(targetElementLayout);
            uint32_t targetElementByteSize = getReflectedByteSize(targetElementLayout->getSize());
            SLANG_ASSERT(minorCount * targetElementByteSize <= targetMajorByteStride);
            SLANG_ASSERT(targetByteSize <= kMaximumShaderNumericByteSize);

            // Reflected matrix rows or columns can contain device padding. Unlike interpreter stack
            // padding, these bytes are included in one contiguous RHI upload, so initialize them
            // before crossing that API boundary.
            uint8_t data[kMaximumShaderNumericByteSize] = {};
            for (uint32_t majorIndex = 0; majorIndex < majorCount; ++majorIndex)
            {
                for (uint32_t minorIndex = 0; minorIndex < minorCount; ++minorIndex)
                {
                    uint32_t rowIndex = columnMajor ? minorIndex : majorIndex;
                    uint32_t columnIndex = columnMajor ? majorIndex : minorIndex;
                    uint32_t sourceLaneIndex =
                        rowIndex * typeInfo.numericLayout.columnCount + columnIndex;
                    copyScalarForBinding(
                        typeInfo.numericLayout.scalarType,
                        source.get() + sourceLaneIndex * typeInfo.numericLayout.laneByteStride,
                        typeInfo.numericLayout.laneByteStride,
                        data + majorIndex * targetMajorByteStride +
                            minorIndex * targetElementByteSize,
                        targetElementByteSize);
                }
            }
            if (SLANG_FAILED(cursor.setData(data, targetByteSize)))
                fail("HGVM RHI failed to bind a compute matrix");
        }
        break;
    case TypeKind::Address:
    case TypeKind::Array:
    case TypeKind::Struct:
    case TypeKind::String:
    case TypeKind::TaggedUnion:
        SLANG_UNREACHABLE("aggregate type reached a flattened shader binding rule");
    }
}

/// Binds one staged compute argument with direct access to interpreter storage and heap wrappers.
///
/// Consider a top-level struct containing an array and a buffer. The converter already contains
/// one rule per leaf; this function follows their packed integer cursor paths and source address
/// dereferences without revisiting either aggregate layout.
void DeviceState::bindShaderArgument(
    InterpreterState& interpreter,
    const rhi::ShaderCursor& cursor,
    const StagedRecord::DispatchArgument& argument,
    LayoutConverterObject* converter)
{
    SLANG_ASSERT(converter);
    SLANG_ASSERT(converter->sourceLayout->domain == HGBCLayoutDomain::Host);
    SLANG_ASSERT(converter->destinationLayout->domain == HGBCLayoutDomain::ShaderObject);
    SLANG_ASSERT(converter->destinationLayout->reflectedLayout == cursor.getTypeLayout());
    for (const ShaderBindingOperation& operation : converter->shaderBindingOperations)
    {
        Address sourceAddress = resolveShaderBindingSourceAddress(
            interpreter,
            argument.valueAddress,
            operation,
            converter);

        rhi::ShaderCursor leafCursor = cursor;
        for (uint32_t pathIndex = 0; pathIndex < operation.cursorPathCount; ++pathIndex)
        {
            uint32_t elementIndex =
                converter->cursorPathIndices[operation.cursorPathOffset + pathIndex];
            leafCursor = leafCursor.getElement(elementIndex);
        }
        bindComputeLeaf(
            interpreter,
            leafCursor,
            sourceAddress,
            operation.sourceByteOffset,
            operation);
    }
}

/// Executes a staged compute call without flattening its arguments at the interpreter/device edge.
///
/// Consider an entry point with a system thread ID followed by `(params, texture)`: this method
/// skips the system parameter through reflection, then applies the argument converter's cached
/// leaf rules. Reflection identifies each top-level cursor, but aggregate traversal and host
/// offsets were already resolved on the converter's first use.
/// Records a draw using a pipeline that was prepared before host execution. Load/Store is
/// unconditional here; clears are separate commands and cannot accidentally recur per draw.
void DeviceState::executeGraphicsCall(
    InterpreterState& interpreter,
    const StagedRecord::Header& header)
{
    const auto& record =
        interpreter.readRecord<StagedRecord::GraphicsCall>(interpreter.programCounter);
    auto pipeline = interpreter.readConstObject<GraphicsPipelineObject>(record.pipelineAddress);
    auto draw = interpreter.readValue<GraphicsDrawValue>(record.drawAddress);
    auto parameters = interpreter.readValue<GraphicsDrawParametersValue>(record.parametersAddress);
    auto color = interpreter.readHeapObject<Texture2DObject>(record.colorAddress);
    const auto& colorDesc = color->resource->getDesc();
    if (colorDesc.format != pipeline->colorFormat || colorDesc.sampleCount != 1 ||
        !is_set(colorDesc.usage, rhi::TextureUsage::RenderTarget))
        fail("HGVM color attachment is incompatible with the prepared graphics pipeline");
    Texture2DObject* depth = nullptr;
    if (record.depthAddress)
    {
        depth = interpreter.readHeapObject<Texture2DObject>(record.depthAddress);
        const auto& depthDesc = depth->resource->getDesc();
        if (depthDesc.format != pipeline->depthFormat ||
            depthDesc.sampleCount != colorDesc.sampleCount ||
            depthDesc.size.width != colorDesc.size.width ||
            depthDesc.size.height != colorDesc.size.height ||
            !is_set(depthDesc.usage, rhi::TextureUsage::DepthStencil))
            fail("HGVM depth attachment does not match the draw's color attachment and pipeline");
    }
    if (!parameters.useDefaultScissor && !pipeline->scissorEnable)
        fail("HGVM custom scissor rectangles require scissorEnable in the generic rasterizer "
             "policy");
    if (parameters.useDefaultViewport)
        parameters
            .viewport = {0, 0, float(colorDesc.size.width), float(colorDesc.size.height), 0, 1};
    const auto& viewport = parameters.viewport;
    if (!std::isfinite(viewport.originX) || !std::isfinite(viewport.originY) ||
        !std::isfinite(viewport.extentX) || !std::isfinite(viewport.extentY) ||
        !std::isfinite(viewport.minZ) || !std::isfinite(viewport.maxZ) || viewport.extentX < 0 ||
        viewport.extentY < 0 || viewport.minZ < 0 || viewport.maxZ > 1 ||
        viewport.minZ > viewport.maxZ)
        fail("HGVM viewport values are outside the supported graphics contract");
    if (parameters.useDefaultScissor)
        parameters.scissor = {0, 0, colorDesc.size.width, colorDesc.size.height};
    auto& scissor = parameters.scissor;
    if (scissor.maxX < scissor.minX || scissor.maxY < scissor.minY)
        fail("HGVM scissor maximum must not precede its minimum");
    scissor.minX = std::min(scissor.minX, colorDesc.size.width);
    scissor.minY = std::min(scissor.minY, colorDesc.size.height);
    scissor.maxX = std::min(scissor.maxX, colorDesc.size.width);
    scissor.maxY = std::min(scissor.maxY, colorDesc.size.height);
    if (!draw.count || !draw.instanceCount || !viewport.extentX || !viewport.extentY ||
        (pipeline->scissorEnable && (scissor.minX == scissor.maxX || scissor.minY == scissor.maxY)))
    {
        interpreter.advance(header);
        return;
    }
    if (draw.firstInstance > UINT32_MAX - (draw.instanceCount - 1))
        fail("HGVM instance range overflows uint32 IDs");
    StructuredBufferObject* indices = nullptr;
    if (draw.indices)
    {
        indices = interpreter.readHeapObjectAt<StructuredBufferObject>(
            record.drawAddress,
            offsetof(GraphicsDrawValue, indices));
        if (draw.first > indices->elementCount || draw.count > indices->elementCount - draw.first)
            fail("HGVM index range exceeds its index buffer");
        if (draw.baseVertex < 0)
            fail("HGVM negative base vertices require signed RHI indexed-draw arguments");
    }
    else if (draw.first > UINT32_MAX - (draw.count - 1))
        fail("HGVM vertex range overflows uint32 IDs");

    auto encoder = getPendingCommandEncoder();
    prepareDescriptorResources(interpreter, record.getArguments(), encoder);
    encoder = getPendingCommandEncoder();
    ComPtr<rhi::ITextureView> colorView = color->resource->getDefaultView();
    ComPtr<rhi::ITextureView> depthView = depth ? depth->resource->getDefaultView() : nullptr;
    rhi::RenderPassColorAttachment colorAttachment;
    colorAttachment.view = colorView;
    colorAttachment.loadOp = rhi::LoadOp::Load;
    colorAttachment.storeOp = rhi::StoreOp::Store;
    rhi::RenderPassDepthStencilAttachment depthAttachment;
    depthAttachment.view = depthView;
    depthAttachment.depthLoadOp = rhi::LoadOp::Load;
    depthAttachment.depthStoreOp = rhi::StoreOp::Store;
    depthAttachment.stencilLoadOp = rhi::LoadOp::Load;
    depthAttachment.stencilStoreOp = rhi::StoreOp::Store;
    rhi::RenderPassDesc passDescription;
    passDescription.colorAttachments = &colorAttachment;
    passDescription.colorAttachmentCount = 1;
    passDescription.depthStencilAttachment = depth ? &depthAttachment : nullptr;
    SLANG_HGVM_TRACK(
        eventTracker.get(),
        beginPipelineExecution(
            pendingBatch.executions,
            encoder,
            pendingBatch.trackingId,
            pipeline->pipeline.get(),
            TrackedPipelineKind::Graphics));
    auto pass = encoder->beginRenderPass(passDescription);
    if (!pass)
        fail("HGVM failed to begin the graphics render pass");
    auto root = pass->bindPipeline(pipeline->pipeline);
    if (!root)
    {
        pass->end();
        fail("HGVM failed to bind the prepared graphics pipeline");
    }
    for (uint32_t i = 0; i < record.argumentCount; ++i)
    {
        const auto& argument = *record.getArgument(i);
        auto converter =
            interpreter.readConstObject<LayoutConverterObject>(argument.converterAddress);
        auto destination = converter->destinationLayout;
        ComPtr<rhi::IShaderObject> stageObject = root->getEntryPoint(destination->entryPointIndex);
        bindShaderArgument(
            interpreter,
            rhi::ShaderCursor(stageObject).getElement(destination->contextIndex),
            argument,
            converter);
    }
    rhi::RenderState renderState;
    renderState.viewports[0] = parameters.viewport;
    renderState.viewportCount = 1;
    renderState.scissorRects[0] = parameters.scissor;
    renderState.scissorRectCount = 1;
    renderState.stencilRef = parameters.stencilRef;
    rhi::DrawArguments arguments;
    arguments.vertexCount = draw.count;
    arguments.instanceCount = draw.instanceCount;
    arguments.startInstanceLocation = draw.firstInstance;
    if (indices)
    {
        renderState.indexBuffer = {indices->resource, 0};
        renderState.indexFormat = rhi::IndexFormat::Uint32;
        arguments.startIndexLocation = draw.first;
        arguments.startVertexLocation = uint32_t(draw.baseVertex);
        pendingBatch.holds.buffers.add(indices->resource);
    }
    else
        arguments.startVertexLocation = draw.first;
    pass->setRenderState(renderState);
    if (indices)
        pass->drawIndexed(arguments);
    else
        pass->draw(arguments);
    SLANG_HGVM_TRACK(
        eventTracker.get(),
        trackDraw(
            pendingBatch.executions,
            pendingBatch.trackingId,
            pipeline->pipeline.get(),
            arguments.vertexCount,
            arguments.instanceCount,
            indices != nullptr));
    pass->end();
    SLANG_HGVM_TRACK(eventTracker.get(), endDeviceExecution(pendingBatch.executions, encoder));
    pendingBatch.holds.renderPipelines.add(pipeline->pipeline);
    pendingBatch.holds.textures.add(color->resource);
    if (depth)
        pendingBatch.holds.textures.add(depth->resource);
    finishDeviceOperation();
    interpreter.advance(header);
}

void DeviceState::executeComputeCall(
    InterpreterState& interpreter,
    const StagedRecord::Header& header)
{
    const auto& record =
        interpreter.readRecord<StagedRecord::ComputeCall>(interpreter.programCounter);
    SLANG_ASSERT(record.argumentCount == header.operandCount);
    ComputeDispatcherObject* dispatcher =
        interpreter.readConstObject<ComputeDispatcherObject>(record.dispatcherAddress);
    AddressRef<const uint32_t> groups =
        interpreter.resolveAddress<const uint32_t>(record.groupCountAddress, 3 * sizeof(uint32_t));

    rhi::ICommandEncoder* commandEncoder = getPendingCommandEncoder();
    prepareDescriptorResources(interpreter, record.getArguments(), commandEncoder);
    commandEncoder = getPendingCommandEncoder();

    SLANG_HGVM_TRACK(
        eventTracker.get(),
        beginPipelineExecution(
            pendingBatch.executions,
            commandEncoder,
            pendingBatch.trackingId,
            dispatcher->pipeline.get(),
            TrackedPipelineKind::Compute));
    rhi::IComputePassEncoder* pass = commandEncoder->beginComputePass();
    if (!pass)
        fail("HGVM RHI failed to begin a compute pass");
    rhi::IShaderObject* rootObject = pass->bindPipeline(dispatcher->pipeline);
    if (!rootObject)
    {
        pass->end();
        fail("HGVM RHI failed to bind a compute pipeline");
    }
    ComPtr<rhi::IShaderObject> entryPointObject = rootObject->getEntryPoint(0);
    slang::ProgramLayout* programLayout = dispatcher->linkedComponent->getLayout();
    slang::EntryPointReflection* entryPointLayout =
        programLayout && programLayout->getEntryPointCount()
            ? programLayout->getEntryPointByIndex(0)
            : nullptr;
    if (!entryPointObject || !entryPointLayout)
    {
        pass->end();
        fail("HGVM compute pipeline has no reflected entry point");
    }

    rhi::ShaderCursor entryPointCursor(entryPointObject);
    for (uint32_t argumentIndex = 0; argumentIndex < record.argumentCount; ++argumentIndex)
    {
        const auto& argument = *record.getArgument(argumentIndex);
        LayoutConverterObject* converter =
            interpreter.readConstObject<LayoutConverterObject>(argument.converterAddress);
        bindShaderArgument(
            interpreter,
            entryPointCursor.getElement(converter->destinationLayout->contextIndex),
            argument,
            converter);
    }

    pendingBatch.holds.computePipelines.add(dispatcher->pipeline);
    pass->dispatchCompute(groups.get()[0], groups.get()[1], groups.get()[2]);
    SLANG_HGVM_TRACK(
        eventTracker.get(),
        trackDispatch(
            pendingBatch.executions,
            pendingBatch.trackingId,
            dispatcher->pipeline.get(),
            groups.get()[0],
            groups.get()[1],
            groups.get()[2]));
    pass->end();
    SLANG_HGVM_TRACK(
        eventTracker.get(),
        endDeviceExecution(pendingBatch.executions, commandEncoder));
    finishDeviceOperation();
    interpreter.advance(header);
}

#include "slang-hgvm-ray-acceleration.inl"
#include "slang-hgvm-ray-dispatch.inl"

struct NativeValueState;
struct NativeFunctionState;

/// Shares VM validity and handle bookkeeping without retaining the execution core or programs.
/// Values and bound functions keep this block alive after close so later access and destruction
/// remain safe even after the VM's execution resources have been released.
struct VMControlBlock : RefObject
{
    bool closed = false;
    bool operationActive = false;
    InterpreterCore* core = nullptr; // Borrowed until close, cleared before teardown callbacks.
    NativeValueState* values = nullptr;
    NativeFunctionState* functions = nullptr;
};

/// Covers host allocation, staging, execution, and teardown callbacks as well as the VM loop.
/// Retaining the small control block keeps close's guard valid after the core is released.
struct NativeOperationScope
{
    explicit NativeOperationScope(VMControlBlock* owner)
        : lifetime(owner), wasActive(owner->operationActive)
    {
        owner->operationActive = true;
    }

    ~NativeOperationScope() { lifetime->operationActive = wasActive; }

    RefPtr<VMControlBlock> lifetime;
    bool wasActive;
};

struct NativeValueState : RefObject
{
    explicit NativeValueState(VMControlBlock* owner)
        : lifetime(owner)
    {
        next = owner->values;
        if (next)
            next->previous = this;
        owner->values = this;
    }

    ~NativeValueState() override
    {
        NativeOperationScope operation(lifetime);
        if (previous)
            previous->next = next;
        else
            lifetime->values = next;
        if (next)
            next->previous = previous;
        // PayloadFree events are synchronous. Keep the guard until storage has released its
        // bytes, including when the last external Value is destroyed between VM operations.
        storage.clear();
    }

    RefPtr<VMControlBlock> lifetime;
    NativeValueState* previous = nullptr;
    NativeValueState* next = nullptr;
    Address typeAddress;
    HostValueType type;
    OwnedPayload storage;
};

struct NativeFunctionState : RefObject
{
    explicit NativeFunctionState(VMControlBlock* owner)
        : lifetime(owner)
    {
        next = owner->functions;
        if (next)
            next->previous = this;
        owner->functions = this;
    }

    ~NativeFunctionState() override
    {
        if (previous)
            previous->next = next;
        else
            lifetime->functions = next;
        if (next)
            next->previous = previous;
    }

    RefPtr<VMControlBlock> lifetime;
    NativeFunctionState* previous = nullptr;
    NativeFunctionState* next = nullptr;
    ProgramInstance* programInstance = nullptr; // Owned by the live interpreter.
    Address entry;
    String name;
    List<HostValueType> parameters;
    List<Address> parameterTypes;
    Address resultTypeAddress;
    HostValueType resultType;
    uint32_t resultAlignment = 1;
    List<uint32_t> resultManagedLeafOffsets;
};

SlangResult hostInvocationError(String& error, const char* message, SlangResult code = SLANG_FAIL)
{
    error = message;
    return code;
}

} // namespace

const String& BoundFunction::getName() const
{
    return static_cast<NativeFunctionState*>(m_state.Ptr())->name;
}

ConstArrayView<HostValueType> BoundFunction::getParameterTypes() const
{
    return static_cast<NativeFunctionState*>(m_state.Ptr())->parameters.getArrayView();
}

const HostValueType& BoundFunction::getResultType() const
{
    return static_cast<NativeFunctionState*>(m_state.Ptr())->resultType;
}

bool BoundFunction::isValid() const
{
    return m_state && !static_cast<NativeFunctionState*>(m_state.Ptr())->lifetime->closed;
}

SlangResult BoundFunction::operator()(
    ConstArrayView<Argument> arguments,
    Value& outResult,
    String& outError) const
{
    if (!isValid())
        return hostInvocationError(outError, "The bound function's HGVM VM is closed");
    auto& lifetime = *static_cast<NativeFunctionState*>(m_state.Ptr())->lifetime;
    SLANG_RELEASE_ASSERT(lifetime.core);
    return lifetime.core->invoke(this, arguments, outResult, outError);
}

bool Value::isValid() const
{
    return m_state && !static_cast<NativeValueState*>(m_state.Ptr())->lifetime->closed;
}

bool Value::isVoid() const
{
    return !m_state;
}

const HostValueType& Value::getType() const
{
    static const HostValueType voidType;
    return m_state ? static_cast<NativeValueState*>(m_state.Ptr())->type : voidType;
}

SlangResult Value::getScalarData(ScalarType type, void* destination, size_t size, String& outError)
    const
{
    outError = "";
    if (!m_state)
        return hostInvocationError(
            outError,
            "A void value has no scalar data",
            SLANG_E_INVALID_ARG);
    const auto& state = *static_cast<NativeValueState*>(m_state.Ptr());
    if (state.lifetime->closed)
        return hostInvocationError(outError, "The HGVM VM is closed");
    if (state.type.kind != HostValueType::Kind::Scalar || state.type.scalarType != type ||
        state.type.byteSize != size || !destination)
        return hostInvocationError(
            outError,
            "Scalar result type or storage does not match the value",
            SLANG_E_INVALID_ARG);
    std::memcpy(destination, state.storage.payload, size);
    return SLANG_OK;
}

/// Owns staged programs and the services shared by sequential HGVM invocations.
///
/// For example, the second invocation of the same source `main` finds its `Program*` in `programs`
/// and creates only a small fresh `InterpreterState`.
struct InterpreterCore::Impl
{
    RefPtr<VMControlBlock> lifetime = new VMControlBlock;
    RefPtr<SharedStagingContext> staging = new SharedStagingContext;
    InterpreterState* activeState = nullptr;
    uint64_t nextDebugCallId = 0;
    IStepControl* stepControl = nullptr;
    IDebugHandler* debugHandler = nullptr;
    // Keep maps available during resource destruction and after executable programs are released.
    Dictionary<uint64_t, RefPtr<DebugMap>> debugMaps;
    DeviceState deviceState;
    RefPtr<IHGVMOutputSink> outputSink;
    RefPtr<IHGVMHostServices> hostServices;
    ComPtr<ISlangFileSystem> fileSystem;
    String workingDirectory;
    std::unordered_map<HGVM::Program*, RefPtr<ProgramInstance>> programs;

    /// Returns a cached staged program or creates it from validated HGBC.
    ///
    /// For example, the first call stages all strings and functions, while the second call returns
    /// the same immutable instruction stack without walking HGBC records.
    RefPtr<ProgramInstance> getOrStageProgram(HGVM::Program* sourceProgram)
    {
        auto found = programs.find(sourceProgram);
        if (found != programs.end())
            return found->second;

        Stager stager;
        RefPtr<ProgramInstance> instance = new ProgramInstance;
        instance->program = stager.stage(sourceProgram, debugHandler != nullptr, staging);
        auto debugMap = instance->program->debugMap;
        instance->initializeConstObjects(stager.getBuilder());
        try
        {
            debugMaps.add(debugMap->getProgramId(), debugMap);
            programs.emplace(sourceProgram, instance);
            stager.commit();
        }
        catch (...)
        {
            programs.erase(sourceProgram);
            debugMaps.remove(debugMap->getProgramId());
            throw;
        }
        return instance;
    }

    StagedProgram* findProgram(uint64_t id) const
    {
        for (const auto& pair : programs)
            if (pair.second->program->debugMap->getProgramId() == id)
                return pair.second->program;
        SLANG_RELEASE_ASSERT(!"HGVM debug address refers to a released program");
        return nullptr;
    }
};

/// Initializes the shared HGVM cache and output service references.
///
/// For example, constructing with a test-server sink makes every later hello-world invocation use
/// that sink without storing it in the immutable staged program.
InterpreterCore::InterpreterCore()
    : InterpreterCore(Desc{})
{
}

InterpreterCore::InterpreterCore(const Desc& desc)
    : m_impl(new Impl)
{
    m_impl->lifetime->core = this;
    m_impl->stepControl = desc.stepControl;
    m_impl->debugHandler = desc.debugHandler;
    m_impl->deviceState.device = desc.device;
    m_impl->fileSystem = desc.fileSystem;
    m_impl->workingDirectory = desc.workingDirectory ? desc.workingDirectory : "";
    m_impl->outputSink = desc.outputSink;
    m_impl->hostServices = desc.hostServices;
    m_impl->deviceState.synchronousGPUOperations =
        desc.gpuExecution == GPUExecutionMode::Synchronous;
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    m_impl->deviceState.eventTracker =
        new EventTracker(desc.enableEventTracking, desc.eventSink, desc.device, this);
#endif
    m_impl->deviceState.bufferPool = new BufferPool(
        desc.bufferPoolByteLimit,
        SLANG_HGVM_TRACKER(m_impl->deviceState.eventTracker));
}

/// Releases the implementation cache after execution has stopped.
///
/// For example, destruction drains the last asynchronously submitted batch before dropping cached
/// staged programs and the shared RHI device.
InterpreterCore::~InterpreterCore()
{
    String ignored;
    close(ignored);
}

SlangResult InterpreterCore::close(String& outError)
{
    outError = "";
    if (!m_impl)
        return SLANG_OK;
    if (m_impl->lifetime->operationActive)
        return hostInvocationError(outError, "HGVM cannot close during an active VM operation");
    NativeOperationScope operation(m_impl->lifetime);

    m_impl->lifetime->closed = true;
    m_impl->lifetime->core = nullptr;
    for (auto value = m_impl->lifetime->values; value;)
    {
        // An event callback may drop a wrapper. Retain the current state while its payload is
        // cleared, then follow the registry links updated by any other wrapper destruction.
        RefPtr<NativeValueState> retained = value;
        retained->storage.clear();
        value = retained->next;
    }
    for (auto function = m_impl->lifetime->functions; function; function = function->next)
        function->programInstance = nullptr;

    SlangResult result = SLANG_OK;
    try
    {
        m_impl->deviceState.shutdown();
    }
    catch (const Exception& exception)
    {
        outError = exception.Message;
        result = SLANG_FAIL;
        m_impl->deviceState.abandonPendingCommands();
    }
    // Const heaps and resource objects may borrow device and event services during destruction.
    m_impl->programs.clear();
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    if (m_impl->deviceState.eventTracker)
        m_impl->deviceState.eventTracker->m_interpreter = nullptr;
#endif
    m_impl.reset();
    return result;
}

SlangResult InterpreterCore::bindFunction(
    Program* sourceProgram,
    HGBCId functionId,
    RefPtr<BoundFunction>& outFunction,
    String& outError)
{
    outError = "";
    if (!m_impl)
        return hostInvocationError(outError, "The HGVM VM is closed");
    if (m_impl->lifetime->operationActive)
        return hostInvocationError(outError, "HGVM cannot bind during an active VM operation");
    NativeOperationScope operation(m_impl->lifetime);
    if (!sourceProgram)
        return hostInvocationError(
            outError,
            "HGVM binding requires a program",
            SLANG_E_INVALID_ARG);
    FunctionSignature signature;
    SLANG_RETURN_ON_FAIL(sourceProgram->getFunctionSignature(functionId, signature, outError));
    try
    {
        auto instance = m_impl->getOrStageProgram(sourceProgram);
        auto program = instance->program;
        RefPtr<NativeFunctionState> state = new NativeFunctionState(m_impl->lifetime);
        state->programInstance = instance;
        state->entry = program->functionAddresses.at(functionId);
        state->name = String(functionId);
        for (const auto& exported : sourceProgram->getIndex().exports)
            if (exported.second == functionId)
            {
                state->name = exported.first.c_str();
                break;
            }
        for (HGBCId typeId : signature.parameterTypes)
        {
            auto binding = program->typeBindings.find(typeId);
            if (binding == program->typeBindings.end())
                return hostInvocationError(
                    outError,
                    "HGVM host parameters require supported value types",
                    SLANG_E_INVALID_ARG);
            const auto& layout = program->staging->findType(binding->second);
            if (!layout.hostValueSupported &&
                layout.hostType.kind != HostValueType::Kind::HostObject)
                return hostInvocationError(
                    outError,
                    "HGVM host parameters require supported owning values or host objects",
                    SLANG_E_INVALID_ARG);
            state->parameters.add(layout.hostType);
            state->parameterTypes.add(binding->second);
        }
        auto resultBinding = program->typeBindings.find(signature.resultType);
        if (resultBinding != program->typeBindings.end())
        {
            const auto& layout = program->staging->findType(resultBinding->second);
            if (!layout.hostValueSupported)
                return hostInvocationError(
                    outError,
                    "HGVM host results require supported owning values without borrowed references",
                    SLANG_E_INVALID_ARG);
            state->resultTypeAddress = resultBinding->second;
            state->resultType = layout.hostType;
            state->resultAlignment = layout.byteAlignment;
            state->resultManagedLeafOffsets.addRange(
                layout.managedLeafOffsets.data(),
                Index(layout.managedLeafOffsets.size()));
        }
        else
        {
            size_t resultSize = 0;
            SLANG_RETURN_ON_FAIL(
                sourceProgram->getHostValueSize(signature.resultType, resultSize, outError));
            if (resultSize)
                return hostInvocationError(outError, "HGVM result has no staged type binding");
        }
        RefPtr<BoundFunction> function = new BoundFunction;
        function->m_state = state;
        outFunction = function;
        return SLANG_OK;
    }
    catch (const Exception& exception)
    {
        outError = exception.Message;
        return SLANG_FAIL;
    }
}

SlangResult InterpreterCore::invoke(
    const BoundFunction* function,
    ConstArrayView<Argument> arguments,
    Value& outResult,
    String& outError)
{
    outError = "";
    if (!m_impl)
        return hostInvocationError(outError, "The HGVM VM is closed");
    if (m_impl->lifetime->operationActive)
        return hostInvocationError(outError, "HGVM does not support reentrant invocation");
    NativeOperationScope operation(m_impl->lifetime);
    if (!function || !function->m_state)
        return hostInvocationError(
            outError,
            "HGVM invocation requires a bound function",
            SLANG_E_INVALID_ARG);
    RefPtr<NativeFunctionState> bound = static_cast<NativeFunctionState*>(function->m_state.Ptr());
    if (bound->lifetime->closed)
        return hostInvocationError(outError, "The bound function's HGVM VM is closed");
    if (bound->lifetime != m_impl->lifetime)
        return hostInvocationError(
            outError,
            "The function belongs to another HGVM VM",
            SLANG_E_INVALID_ARG);
    if (arguments.getCount() != bound->parameters.getCount())
        return hostInvocationError(
            outError,
            "HGVM argument count does not match the signature",
            SLANG_E_INVALID_ARG);

    List<ValueView> transfers;
    List<Address> hostAddresses;
    List<RefPtr<WindowObject>> hostWindows;
    hostAddresses.setCount(arguments.getCount());
    hostWindows.setCount(arguments.getCount());
    for (Index i = 0; i < arguments.getCount(); ++i)
    {
        const auto& argument = arguments[i];
        const auto& expected = bound->parameters[i];
        const void* bytes = nullptr;
        if (argument.kind == Argument::Kind::Scalar)
        {
            if (expected.kind != HostValueType::Kind::Scalar ||
                argument.scalarType != expected.scalarType)
                return hostInvocationError(
                    outError,
                    "HGVM scalar argument type does not match the signature",
                    SLANG_E_INVALID_ARG);
            bytes = argument.scalarBytes;
            if (expected.scalarType == ScalarType::Bool && argument.scalarBytes[0] > 1)
                return hostInvocationError(
                    outError,
                    "HGVM boolean arguments must contain zero or one",
                    SLANG_E_INVALID_ARG);
        }
        else if (argument.kind == Argument::Kind::Value)
        {
            if (!argument.valueStorage.m_state)
                return hostInvocationError(
                    outError,
                    "A void value cannot be an HGVM argument",
                    SLANG_E_INVALID_ARG);
            const auto& value =
                *static_cast<NativeValueState*>(argument.valueStorage.m_state.Ptr());
            if (value.lifetime->closed)
                return hostInvocationError(outError, "The argument's HGVM VM is closed");
            if (value.lifetime != m_impl->lifetime)
                return hostInvocationError(
                    outError,
                    "The value belongs to another HGVM VM",
                    SLANG_E_INVALID_ARG);
            if (value.typeAddress.value != bound->parameterTypes[i].value)
                return hostInvocationError(
                    outError,
                    "HGVM value type does not match the signature",
                    SLANG_E_INVALID_ARG);
            bytes = value.storage.payload;
        }
        else if (argument.kind == Argument::Kind::HostObject)
        {
            if (expected.kind != HostValueType::Kind::HostObject || !argument.hostObject ||
                argument.hostObject->getKind() != expected.hostObjectKind ||
                argument.hostObject->getOwner() != m_impl->hostServices)
                return hostInvocationError(
                    outError,
                    "HGVM host object type or owner does not match the invocation",
                    SLANG_E_INVALID_ARG);
            SLANG_RELEASE_ASSERT(expected.hostObjectKind == HGVMHostObjectKind::Window);
            hostWindows[i].attach(allocateHeapObject<WindowObject>(
                SLANG_HGVM_TRACKER(m_impl->deviceState.eventTracker)));
            hostWindows[i]->object = argument.hostObject;
            hostAddresses[i] = Address::fromObject(hostWindows[i].Ptr());
            bytes = &hostAddresses[i];
        }
        else
            return hostInvocationError(outError, "Invalid HGVM argument kind", SLANG_E_INVALID_ARG);
        transfers.add({0, bytes, expected.byteSize});
    }

    try
    {
        Value result;
        MutableValueView output;
        if (bound->resultType.kind != HostValueType::Kind::Void)
        {
            RefPtr<NativeValueState> state = new NativeValueState(m_impl->lifetime);
            state->type = bound->resultType;
            state->typeAddress = bound->resultTypeAddress;
            state->storage.initialize(
                SLANG_HGVM_TRACKER(m_impl->deviceState.eventTracker),
                uint32_t(state->type.byteSize),
                bound->resultAlignment,
                bound->resultManagedLeafOffsets.getArrayView());
            output = {0, state->storage.payload, state->type.byteSize};
            result.m_state = state;
        }
        SLANG_RETURN_ON_FAIL(execute(
            bound->programInstance,
            bound->entry.value,
            transfers.getArrayView(),
            output,
            bound->resultManagedLeafOffsets.getArrayView(),
            outError));
        // Input handles remain retained through execution; output replacement happens last.
        outResult = std::move(result);
        return SLANG_OK;
    }
    catch (const Exception& exception)
    {
        outError = exception.Message;
        return SLANG_FAIL;
    }
}

/// Stages and runs a host invocation behind the HGVM exception boundary.
///
/// For example, invoking source function `%main` finds its absolute staged function address and
/// starts traversal at that function record's precomputed entry PC. Any exceptional staging or
/// execution failure is caught here and converted to the surrounding HGVM `SlangResult` API.
SlangResult InterpreterCore::execute(
    RefObject* context,
    uint64_t entryAddressBits,
    ConstArrayView<ValueView> arguments,
    MutableValueView result,
    ConstArrayView<uint32_t> resultManagedLeafOffsets,
    String& outError)
{
    if (m_impl->activeState)
    {
        outError = "HGVM does not support reentrant invocation";
        return SLANG_FAIL;
    }
    try
    {
        m_impl->deviceState.beginInvocation();
        RefPtr<ProgramInstance> programInstance = static_cast<ProgramInstance*>(context);
        RefPtr<StagedProgram> program = programInstance->program;

        InterpreterState state;
        // Keep the borrowed execution state valid only inside this synchronous invocation.
        struct ActiveStateScope
        {
            InterpreterState*& active;
            ~ActiveStateScope() { active = nullptr; }
        } activeScope{m_impl->activeState};
        m_impl->activeState = &state;
        state.program = program;
        state.programInstance = programInstance;
        state.outputSink = m_impl->outputSink;
        state.hostServices = m_impl->hostServices;
        state.fileSystem = m_impl->fileSystem;
        state.workingDirectory = m_impl->workingDirectory;
        state.deviceState = &m_impl->deviceState;
        state.stepControl = m_impl->stepControl;
        state.debugHandler = m_impl->debugHandler;
        state.nextDebugCallId = &m_impl->nextDebugCallId;
        state.functionAddress.value = entryAddressBits;
        const auto& functionRecord = state.readRecord<StagedRecord::Function>(
            uint32_t(state.functionAddress.getUnsignedPayload()));

        // The host owns a result slot before the root frame. Normal return lowering writes
        // through rootHeader.returnDestinationAddress, and normal frame exit preserves the prefix.
        // Logical frame addresses remain valid even if a nested call reallocates the stack.
        SLANG_RELEASE_ASSERT(functionRecord.frameAlignment <= kStorageBaseAlignment);
        SLANG_RELEASE_ASSERT(functionRecord.frameSize >= sizeof(FrameHeader));
        uint64_t prefixSize = (uint64_t(result.size) + functionRecord.frameAlignment - 1) &
                              ~uint64_t(functionRecord.frameAlignment - 1);
        if (prefixSize >= kFramePayloadLimit)
            fail("HGVM call result exceeds its address limit");
        uint32_t rootOffset = uint32_t(prefixSize);
        uint64_t stackSize = prefixSize + functionRecord.frameSize;
        if (stackSize >= kFramePayloadLimit || stackSize > std::numeric_limits<Index>::max())
            fail("HGVM call stack exceeds its address limit");
        state.stack.setCount(Index(stackSize));
        std::memset(state.stack.getBuffer(), 0, size_t(stackSize));

        // Ordinary return lowering owns this prefix before the root frame exits. Keep it
        // guarded through backend submission as well: a late failure must release the result.
        // Resolve its bytes at cleanup because nested calls may grow and relocate the stack.
        struct RootResultScope
        {
            InterpreterState& state;
            ConstArrayView<uint32_t> offsets;
            ~RootResultScope() { releaseManagedFields(state.stack.getBuffer(), offsets); }
        } rootResult{state, resultManagedLeafOffsets};

        // A graphics definition is independent of host resources and branches. Prepare every
        // specialization before the first host instruction, including the binding converters.
        for (const auto& layout : program->constObjectLayouts)
            if (layout.kind == LazyObjectKind::GraphicsPipeline)
                state.readConstObject<GraphicsPipelineObject>(
                    Address::makeConstHeap(layout.byteOffset));
        for (const auto& layout : program->constObjectLayouts)
            if (layout.kind == LazyObjectKind::LayoutConverter)
            {
                auto converterRecord = state.readRecord<StagedRecord::LayoutConverterObject>(
                    layout.descriptorWordOffset);
                auto destination = state.resolveAddress<const TypeLayoutObject>(
                    converterRecord.destinationLayoutAddress,
                    sizeof(TypeLayoutObject));
                if (destination.get()->graphicsContext)
                    state.readConstObject<LayoutConverterObject>(
                        Address::makeConstHeap(layout.byteOffset));
            }

        FrameHeader rootHeader;
        rootHeader.previousStackTop = rootOffset;
        if (result.size)
            rootHeader.returnDestinationAddress = Address::makeAbsoluteFrame(0);
        std::memcpy(state.stack.getBuffer() + rootOffset, &rootHeader, sizeof(rootHeader));
        state.currentFrameOffset = rootOffset;
        SLANG_RELEASE_ASSERT(functionRecord.parameterCount == arguments.getCount());
        const auto* parameter = functionRecord.getFirstParameter();
        for (Index i = 0; i < arguments.getCount(); ++i)
        {
            SLANG_RELEASE_ASSERT(parameter->transferByteSize == arguments[i].size);
            state.initializeParameter(rootOffset, *parameter, arguments[i].data);
            if (i + 1 < arguments.getCount())
                parameter = parameter->getNext();
        }
        state.programCounter = functionRecord.entryProgramCounter;
        if (state.debugHandler)
            state.enterDebugCall(
                {program->debugMap->getProgramId(), state.programCounter},
                state.functionAddress);
        if (state.stepControl || state.debugHandler)
            state.run<true>();
        else
            state.run<false>();
        if (state.cancelled)
        {
            m_impl->deviceState.abandonPendingCommands();
            return SLANG_E_ABORT;
        }
        m_impl->deviceState.completeInvocation();
        if (result.size)
        {
            // Move the root slot's owning references into the already allocated native value.
            // No retain is needed: clearing the source transfers, rather than duplicates, them.
            std::memcpy(result.data, state.stack.getBuffer(), result.size);
            std::memset(state.stack.getBuffer(), 0, result.size);
        }
        return SLANG_OK;
    }
    catch (const Exception& exception)
    {
        m_impl->deviceState.abandonPendingCommands();
        outError = exception.Message;
        return SLANG_FAIL;
    }
}

/// Retains programs until close so canonical metadata and bound code remain valid.
void InterpreterCore::releaseProgram(HGVM::Program* sourceProgram)
{
    // Canonical records and bound program contexts are retained until close. Individual program
    // reclamation requires tracking dependencies across the shared staging arena.
    SLANG_UNUSED(sourceProgram);
}

DebugMap* InterpreterCore::getDebugMap(uint64_t programId) const
{
    if (!m_impl)
        return nullptr;
    auto map = m_impl->debugMaps.tryGetValue(programId);
    return map ? map->get() : nullptr;
}

bool InterpreterCore::readDebugMemory(
    DebugCallId call,
    Address address,
    uint32_t offset,
    void* data,
    uint32_t size)
{
    return m_impl && m_impl->activeState &&
           m_impl->activeState->readDebugMemory(call, address, offset, data, size);
}

bool InterpreterCore::getDebugObject(
    DebugCallId call,
    Address address,
    HGBCId& type,
    Address& data,
    uint32_t& count)
{
    auto state = m_impl ? m_impl->activeState : nullptr;
    auto object = state ? state->findDebugObject(call, address) : nullptr;
    if (!object)
        return false;
    // Debug descriptors use the active program's HGBC IDs. An object from another program can
    // be inspected with any local binding to its canonical type; unbound types stay opaque.
    if (object->kind == HeapObjectKind::Class)
    {
        if (!state->findLocalType(static_cast<ClassObject*>(object)->typeAddress, type))
            return false;
        data = address;
        count = 0;
        return true;
    }
    if (object->kind == HeapObjectKind::RuntimeArray)
    {
        auto array = static_cast<RuntimeArrayObject*>(object);
        if (!state->findLocalType(array->typeAddress, type))
            return false;
        data = Address::fromHeapPointer(array->payload);
        count = array->elementCount;
        return true;
    }
    if (object->kind == HeapObjectKind::Box)
    {
        auto box = static_cast<BoxObject*>(object);
        if (!state->findLocalType(box->payloadTypeAddress, type))
            return false;
        data = Address::fromHeapPointer(box->payload);
        count = 0;
        return true;
    }
    return false;
}

bool InterpreterCore::getDebugBufferInfo(DebugCallId call, Address object, DebugBufferInfo& info)
{
    auto state = m_impl ? m_impl->activeState : nullptr;
    return state && state->getDebugBufferInfo(call, object, info);
}

SlangResult InterpreterCore::readDebugBufferElements(
    DebugCallId call,
    Address object,
    uint32_t start,
    uint32_t count,
    List<uint8_t>& bytes,
    String& error)
{
    bytes.clear();
    error = {};
    try
    {
        auto state = m_impl ? m_impl->activeState : nullptr;
        if (!state)
            fail("Buffer inspection requires stopped execution.");
        state->readDebugBufferElements(call, object, start, count, bytes);
        return SLANG_OK;
    }
    catch (const Exception& exception)
    {
        bytes.clear();
        error = exception.Message;
        return SLANG_FAIL;
    }
}

bool InterpreterCore::getDebugTextureInfo(DebugCallId call, Address address, TextureInfo& info)
{
    auto state = m_impl ? m_impl->activeState : nullptr;
    auto object = state ? state->findDebugObject(call, address) : nullptr;
    if (!object || object->kind != HeapObjectKind::Texture2D)
        return false;
    info = DeviceState::getTextureInfo(static_cast<DeviceState::Texture2DObject*>(object));
    return true;
}

SlangResult InterpreterCore::exportDebugTextureMip(
    DebugCallId call,
    Address address,
    uint32_t mip,
    SequentialByteWriter& destination,
    String& error)
{
    error = {};
    try
    {
        auto state = m_impl ? m_impl->activeState : nullptr;
        auto object = state ? state->findDebugObject(call, address) : nullptr;
        if (!object || object->kind != HeapObjectKind::Texture2D)
            fail("Cannot read this texture (null or no longer live).");
        m_impl->deviceState.encodeTextureMip(
            HGBCImageCodec::EXR,
            static_cast<DeviceState::Texture2DObject*>(object),
            mip,
            destination,
            256ull * 1024 * 1024);
        return SLANG_OK;
    }
    catch (const Exception& exception)
    {
        error = exception.Message;
        return SLANG_FAIL;
    }
}

bool InterpreterCore::getDebugObjectProperties(
    DebugCallId call,
    Address address,
    HeapObjectKind expectedKind,
    List<DebugObjectProperty>& properties)
{
    properties.clear();
    auto state = m_impl ? m_impl->activeState : nullptr;
    auto object = state ? state->findDebugObject(call, address) : nullptr;
    if (!object || object->kind != expectedKind)
        return false;
    switch (object->kind)
    {
    case HeapObjectKind::Box:
        {
            auto box = static_cast<BoxObject*>(object);
            const auto& layout = state->program->staging->findType(box->payloadTypeAddress);
            properties.add({"payloadType", layout.hostType.name});
            properties.add({"bytes", String(box->payloadByteSize)});
            break;
        }
    case HeapObjectKind::StructuredBuffer:
        {
            auto buffer = static_cast<DeviceState::StructuredBufferObject*>(object);
            const auto& desc = buffer->resource->getDesc();
            properties.add({"count", String(buffer->elementCount)});
            properties.add({"stride", String(buffer->elementStride)});
            properties.add({"bytes", String(desc.size)});
            if (desc.format != rhi::Format::Undefined)
                properties.add({"format", rhi::getFormatInfo(desc.format).name});
            properties.add({"usage", getBufferUsageName(desc.usage)});
            break;
        }
    case HeapObjectKind::Texture2D:
        {
            const auto& desc =
                static_cast<DeviceState::Texture2DObject*>(object)->resource->getDesc();
            String size = String(desc.size.width) + " x " + String(desc.size.height);
            if (desc.size.depth > 1)
                size = size + " x " + String(desc.size.depth);
            properties.add({"size", size});
            properties.add({"format", rhi::getFormatInfo(desc.format).name});
            properties.add({"mips", String(desc.mipCount)});
            properties.add({"layers", String(desc.getLayerCount())});
            properties.add({"samples", String(desc.sampleCount)});
            properties.add({"usage", getTextureUsageName(desc.usage)});
            break;
        }
    case HeapObjectKind::Window:
    case HeapObjectKind::Surface:
        {
            HGVMHostObject* hostObject = object->kind == HeapObjectKind::Window
                                             ? static_cast<WindowObject*>(object)->object.get()
                                             : static_cast<SurfaceObject*>(object)->object.get();
            hostObject->getOwner()->getDebugProperties(hostObject, properties);
            break;
        }
    case HeapObjectKind::ByteStream:
        {
            auto stream = static_cast<ByteStreamObject*>(object);
            properties.add(
                {"access", stream->access == ByteStreamObject::Access::Read ? "read" : "write"});
            properties.add({"closed", stream->closed ? "true" : "false"});
            break;
        }
    case HeapObjectKind::DeferredReadback:
        properties.add(
            {"resolved",
             static_cast<DeferredReadbackObject*>(object)->resolved ? "true" : "false"});
        break;
    default:
        break;
    }
    return true;
}

bool InterpreterCore::readDebugString(
    DebugCallId call,
    Address address,
    uint32_t offset,
    String& text)
{
    auto state = m_impl ? m_impl->activeState : nullptr;
    StringValue value;
    if (!state || !state->readDebugMemory(call, address, offset, &value, sizeof(value)))
        return false;
    Address bytes = value.literalBytes;
    uint32_t count = value.byteCount;
    if (value.owner)
    {
        auto object = state->findDebugObject(call, value.owner);
        if (!object || object->kind != HeapObjectKind::String)
            return false;
        const auto& string = static_cast<StringObject*>(object)->bytes;
        bytes = Address::fromHeapPointer(const_cast<char*>(string.data()));
        count = uint32_t(string.size());
    }
    char buffer[256];
    uint32_t length = count < sizeof(buffer) ? count : sizeof(buffer);
    if (length && !state->readDebugMemory(call, bytes, 0, buffer, length))
        return false;
    text = String(UnownedStringSlice(buffer, length));
    if (count > length)
        text = text + "...";
    return true;
}

SlangResult InterpreterCore::prepareProgram(Program* program, uint64_t& id, String& error)
{
    error = "";
    if (!m_impl)
        return hostInvocationError(error, "The HGVM VM is closed");
    if (m_impl->lifetime->operationActive)
        return hostInvocationError(error, "HGVM cannot load during an active VM operation");
    NativeOperationScope operation(m_impl->lifetime);
    if (!program)
        return hostInvocationError(error, "HGVM loading requires a program", SLANG_E_INVALID_ARG);
    try
    {
        id = m_impl->getOrStageProgram(program)->program->debugMap->getProgramId();
        return SLANG_OK;
    }
    catch (const Exception& exception)
    {
        error = exception.Message;
        return SLANG_FAIL;
    }
}

void InterpreterCore::setBreakpoint(DebugInstructionAddress address, bool enabled)
{
    if (!m_impl)
        return;
    auto program = m_impl->findProgram(address.programId);
    SLANG_RELEASE_ASSERT(
        address.wordOffset >= program->firstWordOffset &&
        address.wordOffset < program->endWordOffset);
    SLANG_RELEASE_ASSERT(
        !(program->staging->storage.getRecord<StagedRecord::Header>(address.wordOffset)
              ->opcodeAndFlags &
          StagedRecord::kInstructionContinuationBit));
    program->staging->storage.setBreakpoint(address.wordOffset, enabled);
}

const StagedRecord::Header* InterpreterCore::getRecord(DebugInstructionAddress address) const
{
    if (!m_impl)
        return nullptr;
    auto program = m_impl->findProgram(address.programId);
    SLANG_RELEASE_ASSERT(
        address.wordOffset >= program->firstWordOffset &&
        address.wordOffset < program->endWordOffset);
    return program->staging->storage.getRecord<StagedRecord::Header>(address.wordOffset);
}

BufferPoolStats InterpreterCore::getBufferPoolStats() const
{
    if (!m_impl)
        return {};
    return m_impl->deviceState.bufferPool->getStats();
}

SlangResult InterpreterCore::trimBufferPool(String& outError)
{
    if (!m_impl)
        return hostInvocationError(outError, "The HGVM VM is closed");
    if (m_impl->lifetime->operationActive)
        return hostInvocationError(
            outError,
            "HGVM cannot trim buffers during an active VM operation");
    NativeOperationScope operation(m_impl->lifetime);
    try
    {
        m_impl->deviceState.trimBufferPool();
        return SLANG_OK;
    }
    catch (const Exception& exception)
    {
        outError = exception.Message;
        return SLANG_FAIL;
    }
}

} // namespace Slang::HGVM
