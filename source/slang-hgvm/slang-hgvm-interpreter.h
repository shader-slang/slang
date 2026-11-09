// slang-hgvm-interpreter.h
#pragma once

#include "core/slang-basic.h"
#include "core/slang-string.h"
#include "slang-hgbc-container.h"
#include "slang-hgvm-debug.h"
#include "slang-hgvm-execution-mode.h"
#include "slang-hgvm-invocation.h"

#include <memory>

namespace rhi
{
class IDevice;
}

namespace Slang::HGVM
{
class IHGVMOutputSink;
class IHGVMHostServices;
class IEventSink;
class SequentialByteWriter;
struct TextureInfo;
class DebugMap;
class Program;
struct Address;
struct BufferPoolStats;
enum class HeapObjectKind : uint32_t;
namespace StagedRecord
{
struct Header;
}

/// Runs validated HGBC through the staged HGVM execution core.
///
/// Consider this host function:
///
/// ```slang
/// [host] void main() { print("hello"); }
/// ```
///
/// The first call stages its HGBC records into an immutable instruction stack. Later calls reuse
/// that staged program instead of resolving HGBC IDs again.
class InterpreterCore
{
public:
    /// Supplies the interpreter-side services used by staged host operations.
    ///
    /// For example, a test server sets `outputSink` so `print("hello")` is captured by FileCheck
    /// instead of being written directly to the process's standard output.
    struct Desc
    {
        rhi::IDevice* device = nullptr;
        ISlangFileSystem* fileSystem = nullptr;
        const char* workingDirectory = nullptr;
        IHGVMOutputSink* outputSink = nullptr;
        IHGVMHostServices* hostServices = nullptr;
        GPUExecutionMode gpuExecution = GPUExecutionMode::Default;
        uint64_t bufferPoolByteLimit = uint64_t(256) * 1024 * 1024;
        bool enableEventTracking = false;
        IEventSink* eventSink = nullptr;
        IStepControl* stepControl = nullptr;
        IDebugHandler* debugHandler = nullptr;
    };

    /// Creates an HGVM interpreter core with no active program or invocation.
    ///
    /// For example, `InterpreterCore({sink})` creates an execution core whose `OpPrint` records
    /// write through `sink`.
    InterpreterCore();
    explicit InterpreterCore(const Desc& desc);

    /// Destroys cached staged programs after all active execution has finished.
    ///
    /// For example, destroying the owning `HGVM::Interpreter` also destroys this cache without
    /// requiring every caller to release staged programs individually.
    ~InterpreterCore();

    SlangResult bindFunction(
        Program* program,
        HGBCId functionId,
        RefPtr<BoundFunction>& outFunction,
        String& outError);
    SlangResult invoke(
        const BoundFunction* function,
        ConstArrayView<Argument> arguments,
        Value& outResult,
        String& outError);
    SlangResult close(String& outError);
    bool isClosed() const { return !m_impl; }

    /// Compatibility hook. Shared staged records and program contexts remain owned until close.
    void releaseProgram(HGVM::Program* sourceProgram);

    /// Returns a staged program's debug map, or null for an unknown program ID.
    /// Maps remain available until close, including after releaseProgram.
    DebugMap* getDebugMap(uint64_t programId) const;

    SlangResult prepareProgram(Program* program, uint64_t& outProgramId, String& outError);
    void setBreakpoint(DebugInstructionAddress address, bool enabled);
    const StagedRecord::Header* getRecord(DebugInstructionAddress address) const;
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
    bool getDebugBufferInfo(DebugCallId call, Address object, DebugBufferInfo& info);
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
    /// Validates a live handle before inspecting cached native state. Never reads GPU memory.
    bool getDebugObjectProperties(
        DebugCallId call,
        Address object,
        HeapObjectKind expectedKind,
        List<DebugObjectProperty>& properties);

    BufferPoolStats getBufferPoolStats() const;
    SlangResult trimBufferPool(String& outError);

private:
    /// Executes an already-bound entry using its live program context. No source-ID resolution
    /// occurs here; host validation has completed before this common frame-transfer path.
    SlangResult execute(
        RefObject* programInstance,
        uint64_t entryAddressBits,
        ConstArrayView<ValueView> arguments,
        MutableValueView result,
        ConstArrayView<uint32_t> resultManagedLeafOffsets,
        String& outError);

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Slang::HGVM
