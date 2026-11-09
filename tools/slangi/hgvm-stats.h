// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

#include "slang-hgvm/slang-hgvm-debug-map.h"
#include "slang-hgvm/slang-hgvm-events.h"

#include <mutex>

namespace SlangITool
{
/// Identifies a static record, or a runtime pipeline when no staged descriptor exists.
struct HGVMStatsKey
{
    uint64_t programId = 0;
    uint32_t wordOffset = UINT32_MAX;
    Slang::HGVM::TrackedPipelineKind pipelineKind = Slang::HGVM::TrackedPipelineKind::Compute;
    uint64_t dynamicPipelineId = 0;

    bool operator==(const HGVMStatsKey& other) const = default;
    Slang::HashCode getHashCode() const
    {
        return Slang::combineHash(
            Slang::getHashCode(programId),
            Slang::getHashCode(wordOffset),
            Slang::combineHash(
                Slang::getHashCode(pipelineKind),
                Slang::getHashCode(dynamicPipelineId)));
    }
};

struct HGVMStatsValues
{
    uint64_t count = 0;
    uint64_t bytes = 0;
    uint64_t timedCount = 0;
    uint64_t durationNanoseconds = 0;
    uint64_t maxDurationNanoseconds = 0;

    void addDuration(uint64_t nanoseconds);
    void merge(const HGVMStatsValues& other);
};

/// Owns an interval's aggregates, never individual events or borrowed runtime pointers.
struct HGVMStatsSnapshot
{
    Slang::Dictionary<Slang::HGVM::TrackingEventKind, HGVMStatsValues> events;
    Slang::Dictionary<HGVMStatsKey, HGVMStatsValues> allocations;
    Slang::Dictionary<HGVMStatsKey, HGVMStatsValues> pipelines;
    Slang::Dictionary<HGVMStatsKey, Slang::RefPtr<Slang::HGVM::TrackedPipelineDescription>>
        pipelineDescriptions;
    Slang::Dictionary<uint64_t, Slang::RefPtr<Slang::HGVM::DebugMap>> debugMaps;

    void merge(const HGVMStatsSnapshot& other);
    HGVMStatsValues get(Slang::HGVM::TrackingEventKind kind) const;
};

/// Collects events from one interpreter. Decoding and presentation belong to the snapshot reader.
class HGVMStatsSink : public Slang::HGVM::IEventSink
{
public:
    void onEvent(const Slang::HGVM::TrackingEvent& event) noexcept override;
    HGVMStatsSnapshot takeSnapshot();

private:
    void retainDebugMap(const Slang::HGVM::TrackingEvent& event, uint64_t programId);

    std::mutex m_mutex;
    HGVMStatsSnapshot m_pending;
};
} // namespace SlangITool
