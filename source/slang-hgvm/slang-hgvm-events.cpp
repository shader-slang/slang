#include "slang-hgvm-events.h"

#include "slang-hgvm-debug-map.h"
#include "slang-hgvm-interpreter.h"
#include "slang-hgvm-types.h"

#include <algorithm>
#include <atomic>
#include <chrono>

namespace Slang::HGVM
{
EventTracker::EventTracker(
    bool enabled,
    IEventSink* sink,
    rhi::IDevice* device,
    InterpreterCore* interpreter)
    : m_interpreter(interpreter), m_enabled(enabled && sink), m_sink(sink), m_device(device)
{
    if (m_enabled && device && device->hasFeature(rhi::Feature::TimestampQuery))
        m_timestampFrequency = device->getInfo().timestampFrequency;
}

/// Uses the same monotonic clock for event timestamps and operation durations.
static uint64_t getTimestampNanoseconds() noexcept
{
    return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::steady_clock::now().time_since_epoch())
                        .count());
}

void EventTracker::beginOperation(uint64_t& outStartTimeNanoseconds) noexcept
{
    if (!m_enabled)
        return;
    outStartTimeNanoseconds = getTimestampNanoseconds();
}

EventTracker::ContextScope::ContextScope(
    EventTracker* tracker,
    CaptureOrigin capture,
    void* context)
    : m_tracker(tracker && tracker->isEnabled() ? tracker : nullptr)
{
    if (!m_tracker)
        return;
    m_previousCapture = m_tracker->m_captureOrigin;
    m_previousContext = m_tracker->m_context;
    m_tracker->m_captureOrigin = capture;
    m_tracker->m_context = context;
}

EventTracker::ContextScope::~ContextScope()
{
    if (m_tracker)
    {
        m_tracker->m_captureOrigin = m_previousCapture;
        m_tracker->m_context = m_previousContext;
    }
}

EventOrigin EventTracker::captureOrigin() const
{
    return m_enabled && m_captureOrigin ? m_captureOrigin(m_context) : EventOrigin();
}

void EventTracker::emit(TrackingEvent& event, const EventOrigin* origin) noexcept
{
    event.origin = origin ? *origin : captureOrigin();
    event.sequence = ++m_sequence;
    event.tracker = this;
    event.timestampNanoseconds = getTimestampNanoseconds();
    m_sink->onEvent(event);
}

void EventTracker::trackHeapAlloc(HeapObject* object, uint64_t byteSize) noexcept
{
    if (!m_enabled)
        return;
    EventOrigin origin = captureOrigin();
    auto debugMap = m_interpreter ? m_interpreter->getDebugMap(origin.programId) : nullptr;
    HGBCId typeId = debugMap ? debugMap->findRecordType(origin.recordWordOffset) : 0;
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    object->eventTracker = this;
    object->trackedByteSize = byteSize;
    object->trackedTypeId = typeId;
    object->allocationOrigin = origin;
#endif
    TrackingEvent event{};
    event.kind = TrackingEventKind::HeapAlloc;
    event.heap = {object, byteSize, object->kind, typeId};
    event.allocationOrigin = origin;
    emit(event, &origin);
}

void EventTracker::trackHeapFree(const HeapObject* object, uint64_t byteSize) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::HeapFree;
    event.heap = {object, byteSize, object->kind};
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    event.heap.typeId = object->trackedTypeId;
    event.allocationOrigin = object->allocationOrigin;
#endif
    emit(event);
}

#if SLANG_ENABLE_HGVM_EVENT_TRACKING
HeapObject::~HeapObject()
{
    SLANG_HGVM_TRACK(eventTracker.get(), trackHeapFree(this, trackedByteSize));
}
#endif

void EventTracker::trackPayloadAlloc(
    const void* address,
    uint64_t byteSize,
    uint32_t alignment) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::PayloadAlloc;
    event.payload = {address, byteSize, alignment};
    emit(event);
}

void EventTracker::trackPayloadFree(const void* address) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::PayloadFree;
    event.payload = {address, 0, 0};
    emit(event);
}

void EventTracker::trackModuleLoad(
    const void* module,
    const char* name,
    uint64_t byteSize,
    TrackedModuleFormat format,
    SlangResult result,
    uint64_t startTimeNanoseconds) noexcept
{
    if (!m_enabled)
        return;
    uint64_t duration = getTimestampNanoseconds() - startTimeNanoseconds;
    TrackingEvent event{};
    event.kind = TrackingEventKind::ModuleLoad;
    event.module = {module, name, byteSize, format, result, duration};
    emit(event);
}

void EventTracker::trackPipelineCreation(
    const void* pipeline,
    TrackedPipelineKind kind,
    SlangResult result,
    uint64_t startTimeNanoseconds,
    EventOrigin pipelineOrigin,
    List<TrackedPipelineShader> shaders) noexcept
{
    if (!m_enabled)
        return;
    uint64_t duration = getTimestampNanoseconds() - startTimeNanoseconds;
    if (pipeline)
    {
        // A dynamically assembled RT pipeline has no static descriptor. Give every creation a
        // process-wide identity so snapshots never confuse variants or reused native handles.
        static std::atomic<uint64_t> nextPipelineId{1};
        RefPtr<TrackedPipelineDescription> description = new TrackedPipelineDescription;
        description->id = nextPipelineId.fetch_add(1, std::memory_order_relaxed);
        description->origin = pipelineOrigin;
        description->shaders = std::move(shaders);
        m_pipelineDescriptions[pipeline] = description;
    }
    TrackingEvent event{};
    event.kind = TrackingEventKind::PipelineCreation;
    event.pipeline = {pipeline, kind, result, duration};
    emit(event);
}

const EventOrigin* EventTracker::findPipelineOrigin(const void* pipeline) const
{
    auto description = findPipelineDescription(pipeline);
    return description && description->origin.programId ? &description->origin : nullptr;
}

TrackedPipelineDescription* EventTracker::findPipelineDescription(const void* pipeline) const
{
    auto description = m_pipelineDescriptions.tryGetValue(pipeline);
    return description ? description->get() : nullptr;
}

void EventTracker::trackBufferCreation(
    const void* resource,
    const rhi::BufferDesc& desc,
    TrackedBufferPurpose purpose,
    SlangResult result,
    uint64_t startTimeNanoseconds) noexcept
{
    if (!m_enabled)
        return;
    uint64_t duration = getTimestampNanoseconds() - startTimeNanoseconds;
    TrackingEvent event{};
    event.kind = TrackingEventKind::BufferCreation;
    event.buffer = {resource, desc.size, desc.elementSize, desc.usage, purpose, result, duration};
    emit(event);
}

void EventTracker::trackTextureCreation(
    const void* resource,
    const rhi::TextureDesc& desc,
    SlangResult result) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::TextureCreation;
    event.texture = {
        resource,
        desc.size.width,
        desc.size.height,
        desc.size.depth,
        desc.mipCount,
        desc.sampleCount,
        desc.format,
        result};
    emit(event);
}

void EventTracker::trackBufferReuse(const void* resource, uint64_t byteSize) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::BufferReuse;
    event.buffer = {resource, byteSize};
    emit(event);
}

void EventTracker::trackBufferRecycle(const void* resource, uint64_t byteSize) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::BufferRecycle;
    event.buffer = {resource, byteSize};
    emit(event);
}

void EventTracker::trackBufferEviction(const void* resource, uint64_t byteSize) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::BufferEviction;
    event.buffer = {resource, byteSize};
    emit(event);
}

void EventTracker::trackCommandBatchBegin(uint64_t& outId) noexcept
{
    if (!m_enabled)
        return;
    outId = m_nextBatchId++;
    TrackingEvent event{};
    event.kind = TrackingEventKind::CommandBatchBegin;
    event.batch = {outId};
    emit(event);
}

void EventTracker::trackCommandBatchDiscard(uint64_t id, uint32_t operationCount) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::CommandBatchDiscard;
    event.batch = {id, nullptr, operationCount};
    emit(event);
}

void EventTracker::trackCommandBufferSubmit(
    uint64_t id,
    const void* commandBuffer,
    uint32_t operationCount,
    uint64_t completionFenceValue,
    SlangResult result) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::CommandBufferSubmit;
    event.batch = {id, commandBuffer, operationCount, completionFenceValue, result};
    emit(event);
}

void EventTracker::trackDraw(
    const DeviceExecutionBatch& executions,
    uint64_t batchId,
    const void* pipeline,
    uint32_t count,
    uint32_t instanceCount,
    bool indexed) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::Draw;
    const auto& completion = executions.executions.getLast().completion;
    SLANG_ASSERT(completion.kind == TrackingEventKind::PipelineExecutionComplete);
    const auto& execution = completion.pipelineExecution;
    SLANG_ASSERT(execution.batchId == batchId && execution.pipeline == pipeline);
    event.draw = {execution.executionId, batchId, pipeline, count, instanceCount, indexed};
    emit(event, &completion.origin);
}

const char* getTrackedPipelineKindName(TrackedPipelineKind kind)
{
    switch (kind)
    {
    case TrackedPipelineKind::Compute:
        return "compute";
    case TrackedPipelineKind::Graphics:
        return "graphics";
    case TrackedPipelineKind::Blit:
        return "blit";
    case TrackedPipelineKind::Raytracing:
        return "ray-tracing";
    }
    SLANG_UNREACHABLE("Unexpected pipeline kind");
}

void EventTracker::trackDispatch(
    const DeviceExecutionBatch& executions,
    uint64_t batchId,
    const void* pipeline,
    uint32_t x,
    uint32_t y,
    uint32_t z) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::Dispatch;
    const auto& completion = executions.executions.getLast().completion;
    SLANG_ASSERT(completion.kind == TrackingEventKind::PipelineExecutionComplete);
    const auto& execution = completion.pipelineExecution;
    SLANG_ASSERT(execution.batchId == batchId && execution.pipeline == pipeline);
    event.dispatch = {execution.executionId, batchId, pipeline, x, y, z};
    emit(event, &completion.origin);
}

void EventTracker::trackBlit(uint64_t batchId, const void* destination, const void* source) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::Blit;
    event.textureOperation = {batchId, destination, source};
    emit(event);
}

void EventTracker::trackClear(uint64_t batchId, const void* texture) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent event{};
    event.kind = TrackingEventKind::Clear;
    event.textureOperation = {batchId, texture, nullptr};
    emit(event);
}

void EventTracker::beginPipelineExecution(
    DeviceExecutionBatch& batch,
    rhi::ICommandEncoder* encoder,
    uint64_t batchId,
    const void* pipeline,
    TrackedPipelineKind kind) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent completion{};
    completion.kind = TrackingEventKind::PipelineExecutionComplete;
    completion.pipelineExecution = {m_nextExecutionId++, batchId, pipeline, kind, false, 0};
    beginDeviceExecution(batch, encoder, completion);
}

void EventTracker::beginAccelerationStructureBuild(
    DeviceExecutionBatch& batch,
    rhi::ICommandEncoder* encoder,
    uint64_t batchId,
    const void* accelerationStructure,
    rhi::AccelerationStructureKind kind) noexcept
{
    if (!m_enabled)
        return;
    TrackingEvent completion{};
    completion.kind = TrackingEventKind::AccelerationStructureBuildComplete;
    completion.accelerationStructureBuild =
        {m_nextExecutionId++, batchId, accelerationStructure, kind, false, 0};
    beginDeviceExecution(batch, encoder, completion);
    TrackingEvent event = batch.executions.getLast().completion;
    event.kind = TrackingEventKind::AccelerationStructureBuild;
    emit(event, &event.origin);
}

void EventTracker::beginDeviceExecution(
    DeviceExecutionBatch& batch,
    rhi::ICommandEncoder* encoder,
    TrackingEvent completion) noexcept
{
    SLANG_ASSERT(!batch.executions.getCount() || batch.executions.getLast().ended);
    Index index = batch.executions.getCount();
    completion.origin = captureOrigin();
    batch.executions.add({completion});
    if (!m_timestampFrequency)
        return;

    uint32_t queryIndex = uint32_t(index % kExecutionsPerQueryPool) * 2;
    if (!queryIndex)
    {
        ComPtr<rhi::IQueryPool> pool;
        if (m_freeQueryPools.getCount())
        {
            pool = std::move(m_freeQueryPools.getLast());
            m_freeQueryPools.removeLast();
        }
        else
        {
            rhi::QueryPoolDesc desc{};
            desc.type = rhi::QueryType::Timestamp;
            desc.count = kExecutionsPerQueryPool * 2;
            desc.label = "HGVM device execution timestamps";
            if (SLANG_FAILED(m_device->createQueryPool(desc, pool.writeRef())))
                pool = nullptr;
        }
        // A missing pool preserves the page's indices and still allows completion events.
        batch.queryPools.add(std::move(pool));
    }
    auto pool = batch.queryPools[index / kExecutionsPerQueryPool].get();
    if (pool)
        encoder->writeTimestamp(pool, queryIndex);
}

void EventTracker::endDeviceExecution(
    DeviceExecutionBatch& batch,
    rhi::ICommandEncoder* encoder) noexcept
{
    if (!m_enabled)
        return;
    Index index = batch.executions.getCount() - 1;
    auto& execution = batch.executions[index];
    SLANG_ASSERT(!execution.ended);
    execution.ended = true;
    if (!m_timestampFrequency)
        return;
    auto pool = batch.queryPools[index / kExecutionsPerQueryPool].get();
    if (pool)
        encoder->writeTimestamp(pool, uint32_t(index % kExecutionsPerQueryPool) * 2 + 1);
}

void EventTracker::completeDeviceExecutions(DeviceExecutionBatch& batch) noexcept
{
    if (!m_enabled)
        return;
    for (Index offset = 0; offset < batch.executions.getCount(); offset += kExecutionsPerQueryPool)
    {
        uint32_t count = uint32_t(
            std::min(Index(kExecutionsPerQueryPool), batch.executions.getCount() - offset));
        rhi::IQueryPool* pool = m_timestampFrequency
                                    ? batch.queryPools[offset / kExecutionsPerQueryPool].get()
                                    : nullptr;
        uint64_t ticks[kExecutionsPerQueryPool * 2];
        rhi::QueryResultState state = rhi::QueryResultState::Reset;
        bool resolved = pool && SLANG_SUCCEEDED(pool->getResultState(0, count * 2, &state)) &&
                        state == rhi::QueryResultState::Resolved;
        // Read a whole page at once to avoid a driver readback call for every execution.
        bool hasResults = resolved && SLANG_SUCCEEDED(pool->getResult(0, count * 2, ticks));
        for (uint32_t i = 0; i < count; ++i)
        {
            const auto& execution = batch.executions[offset + i];
            SLANG_ASSERT(execution.ended);
            TrackingEvent event = execution.completion;
            bool hasDuration = hasResults && ticks[i * 2 + 1] >= ticks[i * 2];
            uint64_t duration = hasDuration ? uint64_t(
                                                  double(ticks[i * 2 + 1] - ticks[i * 2]) * 1e9 /
                                                  double(m_timestampFrequency))
                                            : 0;
            switch (event.kind)
            {
            case TrackingEventKind::PipelineExecutionComplete:
                event.pipelineExecution.hasDuration = hasDuration;
                event.pipelineExecution.durationNanoseconds = duration;
                break;
            case TrackingEventKind::AccelerationStructureBuildComplete:
                event.accelerationStructureBuild.hasDuration = hasDuration;
                event.accelerationStructureBuild.durationNanoseconds = duration;
                break;
            default:
                SLANG_UNEXPECTED("Unexpected device execution completion kind");
            }
            emit(event, &event.origin);
        }
        // Never reset a pool whose readback is still pending, even after queue completion.
        if (resolved && m_freeQueryPools.getCount() < kMaxCachedQueryPools &&
            SLANG_SUCCEEDED(pool->reset()))
            m_freeQueryPools.add(std::move(batch.queryPools[offset / kExecutionsPerQueryPool]));
    }
    batch.clear();
}
} // namespace Slang::HGVM
