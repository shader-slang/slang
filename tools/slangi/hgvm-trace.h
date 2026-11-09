// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

#include "hgvm-event-display.h"
#include "slang-com-ptr.h"
#include "slang-hgvm/slang-hgvm-debug-map.h"
#include "slang-hgvm/slang-hgvm-events.h"

namespace SlangITool
{
/// Writes the first eventLimit host events to the supplied writer, or all events for UINT64_MAX.
class HGVMTraceSink : public Slang::HGVM::IEventSink
{
public:
    HGVMTraceSink(ISlangWriter* writer, const EventOutputOptions& options);
    void onEvent(const Slang::HGVM::TrackingEvent& event) noexcept override;

private:
    void appendPipelineName(
        EventText& line,
        const Slang::HGVM::EventTracker* tracker,
        const void* pipeline) const;
    Slang::ComPtr<ISlangWriter> m_writer;
    uint64_t m_remainingEvents;
    EventTerminal m_terminal;
    HGVMEventDisplay m_display;
};
} // namespace SlangITool
