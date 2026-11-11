#pragma once

#include "core/slang-basic.h"
#include "slang-hgvm-execution-mode.h"
#include "slang-hgvm-invocation.h"

#include <memory>
#include <optional>
#include <slang-com-ptr.h>
#include <slang-hgbc-container.h>
#include <slang-rhi.h>
#include <string>
#include <string_view>
#include <vector>

namespace Slang::HGVM
{

using HGBC::Block;
using HGBC::Function;
using HGBC::Record;

/// Owns and indexes a validated HGBC container.
class Program : public RefObject
{
public:
    static SlangResult load(
        const void* data,
        size_t size,
        std::string name,
        RefPtr<Program>& outProgram,
        String& outError);

    const std::string& getName() const { return m_name; }
    const HGBC::Program& getDecodedProgram() const { return m_decodedProgram; }
    const HGBC::BasicIndex& getIndex() const { return m_decodedProgram.basicIndex; }

    /// Describes a function without requiring execution or debug information.
    SlangResult getFunctionSignature(
        HGBCId functionId,
        FunctionSignature& outSignature,
        String& outError) const;
    bool tryGetScalarType(HGBCId typeId, ScalarType& outType) const;
    /// Reports the canonical size of a supported host value, or zero for void.
    SlangResult getHostValueSize(HGBCId typeId, size_t& outSize, String& outError) const;

    const Function* findFunction(HGBCId id) const;
    const Function* findFunctionExport(std::string_view name) const;
    const HGBC::TypeSizeDescriptor* findTypeSize(HGBCId typeId) const;
    const HGBC::StructTypeDescriptor* findStructType(HGBCId typeId) const;
    const HGBC::ClassTypeDescriptor* findClassType(HGBCId typeId) const;
    const HGBC::TaggedUnionTypeDescriptor* findTaggedUnionType(HGBCId typeId) const;
    const std::string* findString(HGBCId id) const;
    std::optional<std::string> findStringDecoration(HGBCId targetId, HGBCDecorationKind kind) const;
    std::optional<std::string> getSlangTypeName(HGBCId typeId) const;

private:
    std::string m_name;
    std::vector<uint8_t> m_data;
    HGBC::Program m_decodedProgram;
};

class IHGVMHostServices;
struct DebugObjectProperty;
enum class HeapObjectKind : uint32_t;

enum class HGVMInputEventType : uint32_t
{
    None = 0,
    KeyPress = 1,
    KeyRelease = 2,
    MouseButtonDown = 3,
    MouseButtonUp = 4,
    MouseMove = 5,
    MouseScroll = 6,
    WindowResize = 7,
    MouseEnter = 8,
    MouseLeave = 9,
    WindowFocusGained = 10,
    WindowFocusLost = 11,
    MouseCaptureLost = 12,
    // Replaces MouseButtonDown for the second press; MouseButtonUp still follows it.
    MouseDoubleClick = 13,
};

/// The canonical host representation returned by OpPollInputEvent.
struct HGVMInputEvent
{
    HGVMInputEventType type = HGVMInputEventType::None;
    uint32_t key = 0;
    uint32_t button = 3;
    float position[2] = {};
    float scroll[2] = {};
    /// Latest client-area extent for WindowResize; zero while minimized.
    uint32_t size[2] = {};
};

/// Supplies host-owned window and presentation behavior for HGVM platform instructions.
class IHGVMHostServices : public RefObject
{
public:
    /// Describes an owned object while stopped. Implementations must not pump events, acquire
    /// images, wait for the GPU, or otherwise advance the program to obtain these properties.
    virtual void getDebugProperties(HGVMHostObject* object, List<DebugObjectProperty>& properties)
    {
    }

    virtual SlangResult createWindow(
        std::string_view title,
        uint32_t width,
        uint32_t height,
        RefPtr<HGVMHostObject>& outWindow,
        String& outError);
    virtual SlangResult processWindowEvents(
        HGVMHostObject* window,
        bool& outOpen,
        String& outError);
    virtual SlangResult pollInputEvent(
        HGVMHostObject* window,
        HGVMInputEvent& outEvent,
        String& outError);
    virtual SlangResult closeWindow(HGVMHostObject* window, String& outError);
    virtual SlangResult setWindowTitle(
        HGVMHostObject* window,
        std::string_view title,
        String& outError);
    virtual SlangResult createSurface(
        HGVMHostObject* window,
        rhi::Format requestedFormat,
        uint32_t imageCount,
        RefPtr<HGVMHostObject>& outSurface,
        String& outError);
    /// Reconfigures a surface after HGVM has completed and retired its submissions.
    /// The host waits for its native queue and replaces the swapchain. Graph-owned image
    /// references must already be released. Zero drawable size returns SLANG_E_NOT_AVAILABLE.
    virtual SlangResult resizeSurface(
        HGVMHostObject* surface,
        uint32_t width,
        uint32_t height,
        String& outError);
    /// Reports configured image dimensions without acquiring a texture. Unconfigured surfaces
    /// return zero dimensions so callers can allocate offscreen targets before acquisition.
    virtual SlangResult getSurfaceDimensions(
        HGVMHostObject* surface,
        uint32_t& outWidth,
        uint32_t& outHeight,
        String& outError);
    /// Waits for image availability without exposing a texture or activating surface submission
    /// synchronization. A host may reserve an image until acquisition or reconfiguration.
    /// Return SLANG_E_NOT_AVAILABLE when the drawable area is temporarily unavailable.
    virtual SlangResult waitForSurface(HGVMHostObject* surface, String& outError);
    /// Return SLANG_E_NOT_AVAILABLE without a texture when the surface is temporarily invalid.
    /// Acquisition must not resize or replace the surface's images.
    virtual SlangResult acquireSurfaceTexture(
        HGVMHostObject* surface,
        ComPtr<rhi::ITexture>& outTexture,
        String& outError);
    /// Return SLANG_E_NOT_AVAILABLE if a window change discards the acquired frame.
    virtual SlangResult presentSurface(HGVMHostObject* surface, String& outError);
    virtual SlangResult blitTexture2D(
        rhi::ITexture* destination,
        rhi::ITexture* source,
        String& outError);
};

/// Receives complete lines written by OpPrint during an interpreter invocation.
class IHGVMOutputSink : public RefObject
{
public:
    virtual void writeLine(std::string_view text) = 0;
};

/// Describes a texture value without retaining its RHI resource.
struct TextureInfo
{
    rhi::TextureType type = rhi::TextureType::Texture2D;
    rhi::Format format = rhi::Format::Undefined;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    uint32_t layerCount = 0;
    uint32_t mipCount = 0;
    uint32_t sampleCount = 0;
};

/// Counts whole-buffer reuse. Byte counts describe requested buffer sizes, not backend padding
/// or CUDA heap pages. Leased bytes include buffers retained by pending GPU commands.
struct BufferPoolStats
{
    uint64_t hits = 0;
    uint64_t misses = 0;
    uint64_t evictions = 0;
    uint64_t cachedBytes = 0;
    uint64_t leasedBytes = 0;
};

class IEventSink;
class SequentialByteWriter;
class DebugMap;
class IStepControl;
class IDebugHandler;
struct DebugInstructionAddress;
struct DebugCallId;
struct DebugBufferInfo;
struct Address;
namespace StagedRecord
{
struct Header;
}

/// Supplies services and configuration used by the HGVM interpreter.
struct Desc
{
    rhi::IDevice* device = nullptr;
    ISlangFileSystem* fileSystem = nullptr;
    /// Resolves relative runtime file paths without changing the host process's working directory.
    const char* workingDirectory = nullptr;
    /// Controls submission and completion of GPU operations, including dispatches and copies.
    GPUExecutionMode gpuExecution = GPUExecutionMode::Default;
    /// Maximum idle whole-buffer cache size. Zero disables caching; live allocations are uncapped.
    uint64_t bufferPoolByteLimit = uint64_t(256) * 1024 * 1024;
    IHGVMHostServices* hostServices = nullptr;
    IHGVMOutputSink* outputSink = nullptr;
    /// Enables host-side events when compiled with SLANG_ENABLE_HGVM_EVENT_TRACKING.
    /// Configuration is fixed for this interpreter's lifetime; the tracker retains the sink.
    bool enableEventTracking = false;
    IEventSink* eventSink = nullptr;
    /// Borrowed for this interpreter's lifetime; callbacks execute synchronously on its thread.
    IStepControl* stepControl = nullptr;
    IDebugHandler* debugHandler = nullptr;
};

/// Stages and executes host graph code using the HGVM interpreter.
class Interpreter : public RefObject
{
public:
    explicit Interpreter(const Desc& desc = {});
    ~Interpreter();

    /// Resolves a concrete host function and its canonical signature once. The VM retains its
    /// loaded execution context until close, independently of module and function facades.
    /// Reuses a prepared program, or stages it lazily if prepareProgram was not called.
    SlangResult bindFunction(
        Program* program,
        HGBCId functionId,
        RefPtr<BoundFunction>& outFunction,
        String& outError);

    /// Invokes a bound function with native scalars and owned values from this VM. All checks
    /// precede execution. Failure preserves outResult; it may also be one of the input values.
    SlangResult invoke(
        const BoundFunction* function,
        ConstArrayView<Argument> arguments,
        Value& outResult,
        String& outError);

    /// Invalidates surviving functions/values and promptly releases their native payloads.
    /// Calls, loading, and close must be serialized. Repeated close is harmless.
    SlangResult close(String& outError);
    bool isClosed() const;

    /// Invokes a concrete function with typed host storage. Type and function IDs belong to
    /// `program`. Calls on an interpreter must be serialized; reentrant execution is rejected.
    /// Arguments are copied before execution, and result storage is untouched on failure.
    SlangResult invoke(
        Program* program,
        HGBCId functionId,
        ConstArrayView<ValueView> arguments,
        MutableValueView result,
        String& outError);

    /// Executes one exported, parameterless host function and discards its owned result.
    SlangResult call(Program* program, std::string_view name, String& outError);

    /// Stages a program without executing it, making its debug map and addresses available.
    /// Call this after Program::load to report staging errors before binding any functions.
    SlangResult prepareProgram(Program* program, uint64_t& outProgramId, String& outError);
    DebugMap* getDebugMap(uint64_t programId) const;
    /// Changes only the break bit. The address must identify the first executable record of a
    /// source instruction, not an ownership continuation, data, or a scope directive. Edits are
    /// allowed before execution or inside a stopped callback.
    void setBreakpoint(DebugInstructionAddress address, bool enabled = true);
    /// Borrows an immutable view until the next program load or close. Retain the logical
    /// DebugInstructionAddress rather than this pointer across loads.
    const StagedRecord::Header* getRecord(DebugInstructionAddress address) const;

    /// Copies memory only during a stopped callback and while the call remains active.
    /// Heap reads are limited to live owned objects; no native stack pointers escape.
    bool readDebugMemory(
        DebugCallId call,
        Address address,
        uint32_t offset,
        void* data,
        uint32_t size);
    bool getDebugObject(
        DebugCallId call,
        Address object,
        HGBCId& type,
        Address& data,
        uint32_t& count);
    bool readDebugString(DebugCallId call, Address address, uint32_t offset, String& text);
    /// Describes a live buffer during a stopped callback without submitting GPU work.
    bool getDebugBufferInfo(DebugCallId call, Address object, DebugBufferInfo& info);
    /// Completes preceding GPU work and reads a range into canonical host-layout elements.
    /// Read failures are reported to the debugger without aborting the running program.
    SlangResult readDebugBufferElements(
        DebugCallId call,
        Address object,
        uint32_t start,
        uint32_t count,
        List<uint8_t>& bytes,
        String& error);
    /// Describes a live texture without submitting GPU work.
    bool getDebugTextureInfo(DebugCallId call, Address object, TextureInfo& info);
    /// Completes pending GPU work and streams one mip as EXR. Errors do not abort the program.
    SlangResult exportDebugTextureMip(
        DebugCallId call,
        Address object,
        uint32_t mip,
        SequentialByteWriter& destination,
        String& error);
    bool getDebugObjectProperties(
        DebugCallId call,
        Address object,
        HeapObjectKind expectedKind,
        List<DebugObjectProperty>& properties);

    /// Compatibility hook. Shared records, modules, and pipelines remain owned until close.
    void releaseProgram(Program* program);

    BufferPoolStats getBufferPoolStats() const;
    /// Drains this interpreter's commands, drops idle buffers and reclaims unused RHI memory.
    /// This explicit memory-pressure operation may wait; ordinary cache hits never do.
    SlangResult trimBufferPool(String& outError);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Slang::HGVM

namespace Slang
{

/// Public host-graph virtual machine.
using HGVirtualMachine = HGVM::Interpreter;

} // namespace Slang
