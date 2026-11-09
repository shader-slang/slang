// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "hgvm-stats.h"

#include "slang-hgvm/slang-hgvm-interpreter.h"

namespace SlangITool
{
using namespace Slang;
using namespace Slang::HGVM;

void HGVMStatsValues::addDuration(uint64_t nanoseconds)
{
    ++timedCount;
    durationNanoseconds += nanoseconds;
    maxDurationNanoseconds = Math::Max(maxDurationNanoseconds, nanoseconds);
}

void HGVMStatsValues::merge(const HGVMStatsValues& other)
{
    count += other.count;
    bytes += other.bytes;
    timedCount += other.timedCount;
    durationNanoseconds += other.durationNanoseconds;
    maxDurationNanoseconds = Math::Max(maxDurationNanoseconds, other.maxDurationNanoseconds);
}

void HGVMStatsSnapshot::merge(const HGVMStatsSnapshot& other)
{
    for (const auto& entry : other.events)
        events[entry.first].merge(entry.second);
    for (const auto& entry : other.allocations)
        allocations[entry.first].merge(entry.second);
    for (const auto& entry : other.pipelines)
        pipelines[entry.first].merge(entry.second);
    for (const auto& entry : other.pipelineDescriptions)
        pipelineDescriptions[entry.first] = entry.second;
    for (const auto& entry : other.debugMaps)
        debugMaps[entry.first] = entry.second;
}

HGVMStatsValues HGVMStatsSnapshot::get(TrackingEventKind kind) const
{
    auto value = events.tryGetValue(kind);
    return value ? *value : HGVMStatsValues{};
}

void HGVMStatsSink::retainDebugMap(const TrackingEvent& event, uint64_t programId)
{
    if (!programId || m_pending.debugMaps.containsKey(programId))
        return;
    auto interpreter = event.tracker->getInterpreter();
    if (auto map = interpreter ? interpreter->getDebugMap(programId) : nullptr)
        m_pending.debugMaps.add(programId, map);
}

void HGVMStatsSink::onEvent(const TrackingEvent& event) noexcept
{
    if (event.kind == TrackingEventKind::PayloadAlloc ||
        event.kind == TrackingEventKind::PayloadFree)
        return;

    std::lock_guard lock(m_mutex);
    auto& value = m_pending.events[event.kind];
    ++value.count;
    switch (event.kind)
    {
    case TrackingEventKind::HeapAlloc:
        {
            uint64_t bytes = event.heap.byteSize;
            auto origin = event.allocationOrigin;
            value.bytes += bytes;
            auto& site = m_pending.allocations[{origin.programId, origin.recordWordOffset}];
            ++site.count;
            site.bytes += bytes;
            retainDebugMap(event, origin.programId);
            break;
        }
    case TrackingEventKind::HeapFree:
        value.bytes += event.heap.byteSize;
        break;
    case TrackingEventKind::ModuleLoad:
        value.addDuration(event.module.durationNanoseconds);
        break;
    case TrackingEventKind::PipelineCreation:
        value.addDuration(event.pipeline.durationNanoseconds);
        break;
    case TrackingEventKind::BufferCreation:
        if (SLANG_SUCCEEDED(event.buffer.result))
            value.bytes += event.buffer.byteSize;
        value.addDuration(event.buffer.durationNanoseconds);
        break;
    case TrackingEventKind::PipelineExecutionComplete:
        {
            const auto& execution = event.pipelineExecution;
            HGVMStatsKey key;
            key.pipelineKind = execution.kind;
            if (auto origin = event.tracker->findPipelineOrigin(execution.pipeline))
            {
                key.programId = origin->programId;
                key.wordOffset = origin->recordWordOffset;
                retainDebugMap(event, key.programId);
            }
            if (auto description = event.tracker->findPipelineDescription(execution.pipeline))
            {
                // Static compute/graphics descriptors keep their existing aggregation. An RT
                // variant is identified by its assembly, not by its raygen or dispatch site.
                if (!key.programId)
                    key.dynamicPipelineId = description->id;
                m_pending.pipelineDescriptions[key] = description;
                for (const auto& shader : description->shaders)
                    retainDebugMap(event, shader.entryPoint.programId);
            }
            auto& pipeline = m_pending.pipelines[key];
            ++pipeline.count;
            if (execution.hasDuration)
                pipeline.addDuration(execution.durationNanoseconds);
            break;
        }
    case TrackingEventKind::AccelerationStructureBuildComplete:
        if (event.accelerationStructureBuild.hasDuration)
            value.addDuration(event.accelerationStructureBuild.durationNanoseconds);
        break;
    default:
        break;
    }
}

HGVMStatsSnapshot HGVMStatsSink::takeSnapshot()
{
    HGVMStatsSnapshot result;
    {
        // Only transfer ownership while holding the callback lock. Sorting, merging, and releasing
        // old maps happen on the reader's thread after this lock has been released.
        std::lock_guard lock(m_mutex);
        std::swap(result, m_pending);
    }
    return result;
}
} // namespace SlangITool
