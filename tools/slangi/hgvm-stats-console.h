// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

#include "hgvm-event-display.h"
#include "hgvm-stats.h"
#include "slang-com-ptr.h"
#include "slang-hgvm/slang-hgvm.h"

#include <chrono>
#include <condition_variable>
#include <thread>

namespace SlangITool
{
/// Refreshes a terminal dashboard while preserving program output. The interpreter must finish
/// destruction before finish() so the final snapshot includes resource teardown and GPU completion.
class HGVMStatsConsole : public Slang::HGVM::IHGVMOutputSink
{
public:
    HGVMStatsConsole(
        HGVMStatsSink* sink,
        ISlangWriter* output,
        ISlangWriter* error,
        const EventOutputOptions& options);
    ~HGVMStatsConsole();
    void writeLine(std::string_view text) override;
    void finish();

private:
    using Clock = std::chrono::steady_clock;

    void run();
    void refresh(bool final);
    EventText format(const HGVMStatsSnapshot& window, double seconds, bool final, int maxLines);
    void clearDisplay();
    void getTerminalSize(int& width, int& height) const;

    Slang::RefPtr<HGVMStatsSink> m_sink;
    Slang::ComPtr<ISlangWriter> m_output;
    Slang::ComPtr<ISlangWriter> m_error;
    uint32_t m_top;
    EventTerminal m_terminal;
    HGVMEventDisplay m_display;
    HGVMStatsSnapshot m_totals;
    Clock::time_point m_start = Clock::now();
    Clock::time_point m_previous = m_start;
    bool m_live = false;
    bool m_finished = false;
    int m_displayLines = 0;
    int m_displayWidth = 0;
    std::mutex m_displayMutex;
    std::mutex m_stopMutex;
    std::condition_variable m_wake;
    bool m_stop = false;
    std::thread m_thread;
};
} // namespace SlangITool
