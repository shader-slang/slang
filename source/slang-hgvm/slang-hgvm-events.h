#pragma once

#include "core/slang-basic.h"
#include "slang-hgbc-container.h"

#include <slang-rhi.h>

#ifndef SLANG_ENABLE_HGVM_EVENT_TRACKING
#define SLANG_ENABLE_HGVM_EVENT_TRACKING 0
#endif

// Discard the complete expression in builds without instrumentation, including its arguments.
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
#define SLANG_HGVM_TRACK(tracker, operation)    \
    do                                          \
    {                                           \
        if (auto* hgvmEventTracker = (tracker)) \
            hgvmEventTracker->operation;        \
    } while (false)
#define SLANG_HGVM_TRACKER(tracker) ((tracker).get())
// Declare a start time in the caller's scope without reading the clock when tracking is disabled.
#define SLANG_HGVM_BEGIN_TIMING(tracker, name) \
    uint64_t name = 0;                         \
    SLANG_HGVM_TRACK(tracker, beginOperation(name))
#else
#define SLANG_HGVM_TRACK(tracker, operation) ((void)0)
#define SLANG_HGVM_TRACKER(tracker) nullptr
#define SLANG_HGVM_BEGIN_TIMING(tracker, name) ((void)0)
#endif

namespace Slang::HGVM
{
struct HeapObject;
enum class HeapObjectKind : uint32_t;
class EventTracker;
class InterpreterCore;

/// Identifies one staged record without retaining or decoding its debug map.
struct EventOrigin
{
    uint64_t programId = 0;
    uint32_t recordWordOffset = UINT32_MAX;
};

enum class TrackingEventKind
{
    HeapAlloc,
    HeapFree,
    PayloadAlloc,
    PayloadFree,
    ModuleLoad,
    PipelineCreation,
    BufferCreation,
    TextureCreation,
    BufferReuse,
    BufferRecycle,
    BufferEviction,
    CommandBatchBegin,
    CommandBatchDiscard,
    CommandBufferSubmit,
    Draw,
    Dispatch,
    PipelineExecutionComplete,
    AccelerationStructureBuild,
    AccelerationStructureBuildComplete,
    Blit,
    Clear,
};

enum class TrackedPipelineKind
{
    Compute,
    Graphics,
    Blit,
    Raytracing
};

/// Returns the shared presentation name used by event traces and pipeline statistics.
const char* getTrackedPipelineKindName(TrackedPipelineKind kind);

/// Identifies a shader in the native pipeline inventory, independently of SBT record contents.
struct TrackedPipelineShader
{
    SlangStage stage = SLANG_STAGE_NONE;
    EventOrigin entryPoint;
    /// Native hit-group index; UINT32_MAX for shaders outside hit groups.
    uint32_t hitGroupIndex = UINT32_MAX;
};

/// Published once at creation. Sinks may retain this metadata without keeping GPU objects alive.
struct TrackedPipelineDescription : RefObject
{
    uint64_t id = 0;
    EventOrigin origin;
    List<TrackedPipelineShader> shaders;
};

enum class TrackedModuleFormat
{
    IR,
    Source
};
enum class TrackedBufferPurpose
{
    Program,
    Readback
};

/// Describes a host-side observation, with optional GPU elapsed time on execution completion.
/// Allocation events do not represent physical GPU memory accounting.
/// Pointer identities are valid only for the corresponding object's lifetime. A sink that retains
/// module names must copy them during onEvent. Debug maps belong to the interpreter; receivers
/// that need to decode events after its destruction must retain the maps themselves.
struct TrackingEvent
{
    TrackingEventKind kind;
    uint64_t sequence;
    uint64_t timestampNanoseconds;
    const EventTracker* tracker;
    EventOrigin origin;
    /// Heap events preserve allocation identity even when free occurs outside an invocation.
    EventOrigin allocationOrigin;

    struct Heap
    {
        const void* address;
        uint64_t byteSize;
        HeapObjectKind objectKind;
        /// Resolves in allocationOrigin.programId's map; zero means no known source-visible type.
        HGBCId typeId;
    };
    struct Payload
    {
        const void* address;
        uint64_t byteSize;
        uint32_t alignment;
    };
    struct Module
    {
        const void* module;
        const char* name;
        uint64_t byteSize;
        TrackedModuleFormat format;
        SlangResult result;
        uint64_t durationNanoseconds;
    };
    struct Pipeline
    {
        const void* pipeline;
        TrackedPipelineKind kind;
        SlangResult result;
        uint64_t durationNanoseconds;
    };
    struct Buffer
    {
        const void* resource;
        uint64_t byteSize;
        uint32_t elementSize;
        rhi::BufferUsage usage;
        TrackedBufferPurpose purpose;
        SlangResult result;
        uint64_t durationNanoseconds;
    };
    struct Texture
    {
        const void* resource;
        uint32_t width, height, depth;
        uint32_t mipCount, sampleCount;
        rhi::Format format;
        SlangResult result;
    };
    struct Batch
    {
        uint64_t id;
        const void* commandBuffer;
        uint32_t operationCount;
        uint64_t completionFenceValue;
        SlangResult result;
    };
    struct Draw
    {
        uint64_t executionId;
        uint64_t batchId;
        const void* pipeline;
        uint32_t count, instanceCount;
        bool indexed;
    };
    struct Dispatch
    {
        uint64_t executionId;
        uint64_t batchId;
        const void* pipeline;
        uint32_t x, y, z;
    };
    struct PipelineExecution
    {
        uint64_t executionId;
        uint64_t batchId;
        const void* pipeline;
        TrackedPipelineKind kind;
        bool hasDuration;
        uint64_t durationNanoseconds;
    };
    struct AccelerationStructureBuild
    {
        uint64_t executionId;
        uint64_t batchId;
        const void* accelerationStructure;
        rhi::AccelerationStructureKind kind;
        /// Only completion events may have a GPU duration.
        bool hasDuration;
        uint64_t durationNanoseconds;
    };
    struct TextureOperation
    {
        uint64_t batchId;
        const void* destination;
        const void* source;
    };
    union
    {
        Heap heap;
        Payload payload;
        Module module;
        Pipeline pipeline;
        Buffer buffer;
        Texture texture;
        Batch batch;
        Draw draw;
        Dispatch dispatch;
        PipelineExecution pipelineExecution;
        AccelerationStructureBuild accelerationStructureBuild;
        TextureOperation textureOperation;
    };
};

/// Receives events synchronously on the calling HGVM thread. Callbacks may read debug maps,
/// but must not start execution or mutate the interpreter.
/// A sink shared by multiple interpreters must serialize its own output; sequence numbers and
/// batch and execution IDs belong to one tracker. Tracking owns its query pools but never retains
/// the observed pipelines, acceleration structures, or heap objects.
class IEventSink : public RefObject
{
public:
    virtual void onEvent(const TrackingEvent& event) noexcept = 0;
};

/// Moves query storage and execution identities with one command batch until GPU completion.
/// An abandoned batch drops this storage without reporting successful completion.
struct DeviceExecutionBatch
{
    struct Execution
    {
        /// Captures the command identity and origin at recording time. Completion only adds
        /// its optional GPU duration; it must not recapture a later invocation's context.
        TrackingEvent completion;
        bool ended = false;
    };
    List<Execution> executions;
    List<ComPtr<rhi::IQueryPool>> queryPools;

    void clear()
    {
        executions.clearAndDeallocate();
        queryPools.clearAndDeallocate();
    }
};

/// Owns a fixed tracking configuration for one interpreter, including its delayed destruction.
/// Every public tracking method checks the enabled flag before constructing or delivering events.
class EventTracker : public RefObject
{
public:
    /// Borrows the device during command recording. A null device disables GPU timestamps.
    EventTracker(
        bool enabled,
        IEventSink* sink,
        rhi::IDevice* device = nullptr,
        InterpreterCore* interpreter = nullptr);

    /// Borrows the owning interpreter during callbacks, or returns null after its destruction.
    InterpreterCore* getInterpreter() const { return m_interpreter; }

    /// Resolves a live pipeline handle to its staged descriptor without decoding its name.
    const EventOrigin* findPipelineOrigin(const void* pipeline) const;
    /// Borrows immutable metadata until this handle is reused or the tracker is destroyed.
    TrackedPipelineDescription* findPipelineDescription(const void* pipeline) const;

    bool isEnabled() const { return m_enabled; }
    using CaptureOrigin = EventOrigin (*)(void*);
    /// Borrows an invocation only while it is running. Events capture offsets on demand.
    class ContextScope
    {
    public:
        ContextScope(EventTracker* tracker, CaptureOrigin capture, void* context);
        ~ContextScope();
        ContextScope(const ContextScope&) = delete;
        ContextScope& operator=(const ContextScope&) = delete;

    private:
        EventTracker* m_tracker;
        CaptureOrigin m_previousCapture = nullptr;
        void* m_previousContext = nullptr;
    };

    /// Captures the start of one synchronous operation only when tracking is enabled.
    /// Pass this value to the corresponding completion event immediately after the call returns.
    void beginOperation(uint64_t& outStartTimeNanoseconds) noexcept;

    void trackHeapAlloc(HeapObject* object, uint64_t byteSize) noexcept;
    void trackHeapFree(const HeapObject* object, uint64_t byteSize) noexcept;
    void trackPayloadAlloc(const void* address, uint64_t byteSize, uint32_t alignment) noexcept;
    void trackPayloadFree(const void* address) noexcept;
    void trackModuleLoad(
        const void* module,
        const char* name,
        uint64_t byteSize,
        TrackedModuleFormat format,
        SlangResult result,
        uint64_t startTimeNanoseconds) noexcept;
    void trackPipelineCreation(
        const void* pipeline,
        TrackedPipelineKind kind,
        SlangResult result,
        uint64_t startTimeNanoseconds,
        EventOrigin pipelineOrigin = {},
        List<TrackedPipelineShader> shaders = {}) noexcept;
    void trackBufferCreation(
        const void* resource,
        const rhi::BufferDesc& desc,
        TrackedBufferPurpose purpose,
        SlangResult result,
        uint64_t startTimeNanoseconds) noexcept;
    void trackTextureCreation(
        const void* resource,
        const rhi::TextureDesc& desc,
        SlangResult result) noexcept;
    void trackBufferReuse(const void* resource, uint64_t byteSize) noexcept;
    void trackBufferRecycle(const void* resource, uint64_t byteSize) noexcept;
    void trackBufferEviction(const void* resource, uint64_t byteSize) noexcept;
    void trackCommandBatchBegin(uint64_t& outId) noexcept;
    void trackCommandBatchDiscard(uint64_t id, uint32_t operationCount) noexcept;
    void trackCommandBufferSubmit(
        uint64_t id,
        const void* commandBuffer,
        uint32_t operationCount,
        uint64_t completionFenceValue,
        SlangResult result) noexcept;
    void trackDraw(
        const DeviceExecutionBatch& executions,
        uint64_t batchId,
        const void* pipeline,
        uint32_t count,
        uint32_t instanceCount,
        bool indexed) noexcept;
    void trackDispatch(
        const DeviceExecutionBatch& executions,
        uint64_t batchId,
        const void* pipeline,
        uint32_t x,
        uint32_t y,
        uint32_t z) noexcept;
    void trackBlit(uint64_t batchId, const void* destination, const void* source) noexcept;
    void trackClear(uint64_t batchId, const void* texture) noexcept;

    /// Records timestamps outside the pass so Vulkan query resets never occur inside rendering.
    void beginPipelineExecution(
        DeviceExecutionBatch& executions,
        rhi::ICommandEncoder* encoder,
        uint64_t batchId,
        const void* pipeline,
        TrackedPipelineKind kind) noexcept;
    /// Reports one native build, including TLAS realizations recorded lazily at binding time.
    void beginAccelerationStructureBuild(
        DeviceExecutionBatch& executions,
        rhi::ICommandEncoder* encoder,
        uint64_t batchId,
        const void* accelerationStructure,
        rhi::AccelerationStructureKind kind) noexcept;
    void endDeviceExecution(
        DeviceExecutionBatch& executions,
        rhi::ICommandEncoder* encoder) noexcept;
    /// Emits completions only after the caller has established queue completion. Query errors
    /// omit the optional duration; they never fail the interpreted program or force a GPU wait.
    void completeDeviceExecutions(DeviceExecutionBatch& executions) noexcept;

private:
    friend class InterpreterCore;
    /// Reserves a timestamp pair and preserves the completion metadata for any device command.
    void beginDeviceExecution(
        DeviceExecutionBatch& executions,
        rhi::ICommandEncoder* encoder,
        TrackingEvent completion) noexcept;
    InterpreterCore* m_interpreter;
    EventOrigin captureOrigin() const;
    void emit(TrackingEvent& event, const EventOrigin* origin = nullptr) noexcept;
    CaptureOrigin m_captureOrigin = nullptr;
    void* m_context = nullptr;
    const bool m_enabled;
    RefPtr<IEventSink> m_sink;
    uint64_t m_sequence = 0;
    uint64_t m_nextBatchId = 1;
    uint64_t m_nextExecutionId = 1;
    Dictionary<const void*, RefPtr<TrackedPipelineDescription>> m_pipelineDescriptions;
    rhi::IDevice* m_device;
    uint64_t m_timestampFrequency = 0;
    List<ComPtr<rhi::IQueryPool>> m_freeQueryPools;
    static constexpr uint32_t kExecutionsPerQueryPool = 128;
    static constexpr Index kMaxCachedQueryPools = 4;
};
} // namespace Slang::HGVM
