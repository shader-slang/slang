// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "hgvm-stats-console.h"

#include "core/slang-string-util.h"

#if SLANG_WINDOWS_FAMILY
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace SlangITool
{
using namespace Slang;
using namespace Slang::HGVM;

HGVMStatsConsole::HGVMStatsConsole(
    HGVMStatsSink* sink,
    ISlangWriter* output,
    ISlangWriter* error,
    const EventOutputOptions& options)
    : m_sink(sink)
    , m_output(output)
    , m_error(error)
    , m_top(options.statsTop)
    , m_terminal(error, SLANG_WRITER_CHANNEL_STD_ERROR, options.color)
    , m_display(options.fullNames)
{
    m_live = m_terminal.canRedraw();
    if (m_live)
        m_thread = std::thread([this] { run(); });
}

HGVMStatsConsole::~HGVMStatsConsole()
{
    finish();
}

void HGVMStatsConsole::finish()
{
    if (m_finished)
        return;
    {
        std::lock_guard lock(m_stopMutex);
        m_stop = true;
    }
    m_wake.notify_one();
    if (m_thread.joinable())
        m_thread.join();
    refresh(true);
    m_finished = true;
}

void HGVMStatsConsole::run()
{
    std::unique_lock lock(m_stopMutex);
    while (!m_wake.wait_for(lock, std::chrono::seconds(1), [this] { return m_stop; }))
    {
        lock.unlock();
        refresh(false);
        lock.lock();
    }
}

void HGVMStatsConsole::writeLine(std::string_view text)
{
    std::lock_guard lock(m_displayMutex);
    // A print may share the terminal with stderr. Leave it above the next dashboard instead of
    // allowing a later redraw to erase it. Redirected stdout never needs screen manipulation.
    if (m_output->isConsole())
        clearDisplay();
    m_output->write(text.data(), text.size());
    m_output->write("\n", 1);
    m_output->flush();
}

void HGVMStatsConsole::getTerminalSize(int& width, int& height) const
{
    width = 80;
    height = 24;
#if SLANG_WINDOWS_FAMILY
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &info))
    {
        width = info.srWindow.Right - info.srWindow.Left + 1;
        height = info.srWindow.Bottom - info.srWindow.Top + 1;
    }
#else
    winsize size = {};
    if (ioctl(STDERR_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col && size.ws_row)
    {
        width = size.ws_col;
        height = size.ws_row;
    }
#endif
}

void HGVMStatsConsole::clearDisplay()
{
    if (!m_displayLines)
        return;
    int width, height;
    getTerminalSize(width, height);
    // Resizing can reflow previous rows. Start a fresh dashboard rather than erase unrelated
    // scrollback using stale cursor coordinates.
    if (width == m_displayWidth)
    {
        for (int i = 0; i < Math::Min(m_displayLines, height - 1); ++i)
            m_error->write("\x1b[1A\r\x1b[2K", 9);
    }
    m_error->flush();
    m_displayLines = 0;
}

void HGVMStatsConsole::refresh(bool final)
{
    auto window = m_sink->takeSnapshot();
    auto now = Clock::now();
    double seconds = std::chrono::duration<double>(now - m_previous).count();
    m_previous = now;
    m_totals.merge(window);

    int width, height;
    getTerminalSize(width, height);
    EventText text = format(window, seconds, final, Math::Max(0, height - 1));
    std::lock_guard lock(m_displayMutex);
    clearDisplay();
    if (final)
    {
        String rendered = text.render(m_terminal.useColor());
        m_error->write(rendered.getBuffer(), rendered.getLength());
    }
    else
    {
        auto remaining = text.getText();
        Index offset = 0;
        while (remaining.getLength() && m_displayLines < height - 1)
        {
            Index end = remaining.indexOf('\n');
            if (end < 0)
                end = remaining.getLength();
            String line =
                text.render(m_terminal.useColor(), offset, offset + end, Math::Max(0, width - 1));
            m_error->write(line.getBuffer(), line.getLength());
            m_error->write("\n", 1);
            ++m_displayLines;
            Index consumed = Math::Min(end + 1, remaining.getLength());
            remaining = remaining.tail(consumed);
            offset += consumed;
        }
        m_displayWidth = width;
    }
    m_error->flush();
}

struct SourceLineKey
{
    String path;
    uint32_t line = 0;
    bool operator==(const SourceLineKey& other) const = default;
    HashCode getHashCode() const
    {
        return combineHash(Slang::getHashCode(path), Slang::getHashCode(line));
    }
};

struct AllocationRow
{
    SourceLineKey source;
    HGVMStatsValues value;
};

struct PipelineRow
{
    HGVMStatsKey key;
    HGVMStatsValues value;
};

/// Combines different allocation records on the same source line after the callback has returned.
static List<AllocationRow> getAllocationRows(
    const Dictionary<HGVMStatsKey, HGVMStatsValues>& sites,
    const Dictionary<uint64_t, RefPtr<DebugMap>>& maps,
    uint64_t& unknown)
{
    Dictionary<SourceLineKey, HGVMStatsValues> lines;
    unknown = 0;
    for (const auto& entry : sites)
    {
        auto map = maps.tryGetValue(entry.first.programId);
        auto source = map ? (*map)->findSourceLocation(entry.first.wordOffset) : nullptr;
        if (source && source->line && source->path.getLength())
            lines[{source->path, source->line}].merge(entry.second);
        else
            unknown += entry.second.count;
    }
    List<AllocationRow> result;
    for (const auto& entry : lines)
        result.add({entry.first, entry.second});
    result.sort(
        [](const AllocationRow& a, const AllocationRow& b)
        {
            if (a.value.count != b.value.count)
                return a.value.count > b.value.count;
            int order = compare(a.source.path.getUnownedSlice(), b.source.path.getUnownedSlice());
            return order ? order < 0 : a.source.line < b.source.line;
        });
    return result;
}

EventText HGVMStatsConsole::format(
    const HGVMStatsSnapshot& window,
    double seconds,
    bool final,
    int maxLines)
{
    double elapsed = std::chrono::duration<double>(m_previous - m_start).count();
    const auto& data = final ? m_totals : window;
    using Style = EventTextStyle;
    EventText text;
    text << Style::Label << "HGVM Live Statistics" << Style::Plain;
    if (final)
        text << " | final";
    else
    {
        text << " | last " << Style::Duration;
        text.appendFormat("%.2f s", seconds);
    }
    text << Style::Plain << " | elapsed " << Style::Duration;
    text.appendFormat("%.2f s\n", elapsed);

    text << Style::Dim << "----------------------------------------\n";
    text << Style::Label << "CPU Heap Statistics\n";
    text << Style::Plain << "    Allocs: " << Style::Number
         << data.get(TrackingEventKind::HeapAlloc).count << '\n';
    text << Style::Plain << "    Frees: " << Style::Number
         << data.get(TrackingEventKind::HeapFree).count << '\n';
    text << Style::Plain << "    Traffic: " << Style::Number;
    text.appendFormat("%.1f KiB\n", data.get(TrackingEventKind::HeapAlloc).bytes / 1024.0);
    if (final)
        text << '\n';

    text << Style::Dim << "----------------------------------------\n";
    text << Style::Label << "Device Statistics\n";
    text << Style::Plain << "    Buffers: create " << Style::Number
         << data.get(TrackingEventKind::BufferCreation).count << Style::Plain << "; reuse "
         << Style::Number << data.get(TrackingEventKind::BufferReuse).count << Style::Plain
         << "; recycle " << Style::Number << data.get(TrackingEventKind::BufferRecycle).count
         << Style::Plain << "; evict " << Style::Number
         << data.get(TrackingEventKind::BufferEviction).count << '\n';
    text << Style::Plain << "    Textures: create " << Style::Number
         << data.get(TrackingEventKind::TextureCreation).count << '\n';
    text << Style::Plain << "    Commands: submit " << Style::Number
         << data.get(TrackingEventKind::CommandBufferSubmit).count << Style::Plain << "; draw "
         << Style::Number << data.get(TrackingEventKind::Draw).count << Style::Plain
         << "; dispatch " << Style::Number << data.get(TrackingEventKind::Dispatch).count
         << Style::Plain << "; blit " << Style::Number << data.get(TrackingEventKind::Blit).count
         << Style::Plain << "; clear " << Style::Number << data.get(TrackingEventKind::Clear).count
         << '\n';
    auto builds = data.get(TrackingEventKind::AccelerationStructureBuildComplete);
    text << Style::Plain << "    Acceleration structures: build " << Style::Number
         << data.get(TrackingEventKind::AccelerationStructureBuild).count << Style::Plain
         << "; complete " << Style::Number << builds.count << Style::Plain << "; timed "
         << Style::Number << builds.timedCount << Style::Plain << " (";
    if (builds.timedCount)
    {
        text << Style::Duration;
        text.appendFormat("%.3f GPU ms", builds.durationNanoseconds / 1e6);
    }
    else
        text << "GPU time unavailable";
    text << Style::Plain << ")\n";
    auto modules = data.get(TrackingEventKind::ModuleLoad);
    auto pipelines = data.get(TrackingEventKind::PipelineCreation);
    text << Style::Plain << "    Programs: slang-module-load " << Style::Number << modules.count
         << Style::Plain << " (" << Style::Duration;
    text.appendFormat("%.2f ms", modules.durationNanoseconds / 1e6);
    text << Style::Plain << "); pipeline-create " << Style::Number << pipelines.count
         << Style::Plain << " (" << Style::Duration;
    text.appendFormat("%.2f ms", pipelines.durationNanoseconds / 1e6);
    text << Style::Plain << ");\n";
    if (final)
        text << '\n';

    uint64_t unknown;
    auto allocationRows = getAllocationRows(data.allocations, m_totals.debugMaps, unknown);
    text << Style::Dim << "----------------------------------------\n";
    text << Style::Label << "Top allocation lines by count" << Style::Plain
         << " (no source: " << Style::Number << unknown << Style::Plain << ")\n";
    text << Style::Label << "    Count       KiB  Source\n";
    // Reserve the fixed dashboard rows and give composite pipelines the larger share of the
    // remaining height. The final report is unrestricted vertically.
    Index allocationCount = Math::Min(Index(m_top), allocationRows.getCount());
    Index allocationLimit =
        final ? allocationCount
              : Math::Min(allocationCount, Index(Math::Max(0, (maxLines - 19) / 3)));
    bool truncated = allocationLimit < allocationCount;
    for (Index i = 0; i < allocationLimit; ++i)
    {
        auto& row = allocationRows[i];
        text << Style::Number;
        text.appendFormat(
            "  %7llu %9.1f  ",
            (unsigned long long)row.value.count,
            row.value.bytes / 1024.0);
        text << Style::Plain << m_display.getSourcePath(row.source.path) << ':' << row.source.line
             << '\n';
    }
    if (final)
        text << '\n';

    List<PipelineRow> rows;
    for (const auto& entry : data.pipelines)
        if (entry.second.timedCount)
            rows.add({entry.first, entry.second});
    rows.sort(
        [](const PipelineRow& a, const PipelineRow& b)
        {
            if (a.value.durationNanoseconds != b.value.durationNanoseconds)
                return a.value.durationNanoseconds > b.value.durationNanoseconds;
            if (a.key.programId != b.key.programId)
                return a.key.programId < b.key.programId;
            if (a.key.wordOffset != b.key.wordOffset)
                return a.key.wordOffset < b.key.wordOffset;
            if (a.key.dynamicPipelineId != b.key.dynamicPipelineId)
                return a.key.dynamicPipelineId < b.key.dynamicPipelineId;
            return a.key.pipelineKind < b.key.pipelineKind;
        });
    text << Style::Dim << "----------------------------------------\n";
    text << Style::Label << "Top pipelines by measured GPU time\n";
    text << "    Count   Total ms   Mean ms  Pipeline\n";
    int linesLeft = maxLines - 1; // Reserve the live truncation notice.
    for (char c : text.getText())
        if (c == '\n')
            --linesLeft;
    for (Index i = 0; i < Math::Min(Index(m_top), rows.getCount()); ++i)
    {
        if (!final && linesLeft <= 0)
        {
            truncated = true;
            break;
        }
        auto& row = rows[i];
        text << Style::Number;
        text.appendFormat("  %7llu ", (unsigned long long)row.value.count);
        text << Style::Duration;
        text.appendFormat(
            "%10.3f %9.3f  ",
            row.value.durationNanoseconds / 1e6,
            row.value.durationNanoseconds / (1e6 * row.value.timedCount));
        auto description = m_totals.pipelineDescriptions.tryGetValue(row.key);
        bool composite = description && (*description)->shaders.getCount() &&
                         row.key.pipelineKind != TrackedPipelineKind::Compute;
        text << Style::Plain << getTrackedPipelineKindName(row.key.pipelineKind);
        if (!composite)
        {
            auto map = m_totals.debugMaps.tryGetValue(row.key.programId);
            auto name = map ? (*map)->findPipelineName(row.key.wordOffset) : nullptr;
            text << ' ' << Style::Type;
            if (name)
                text << m_display.getDisplayName(*map, *name);
            else
                text
                    << (row.key.pipelineKind == TrackedPipelineKind::Blit ? "<internal>"
                                                                          : "<unknown>");
        }
        text << '\n';
        --linesLeft;
        if (composite)
        {
            for (const auto& shader : (*description)->shaders)
            {
                if (!final && linesLeft <= 0)
                {
                    truncated = true;
                    break;
                }
                auto map = m_totals.debugMaps.tryGetValue(shader.entryPoint.programId);
                text << Style::Plain << "            ";
                m_display.appendPipelineShader(text, shader, map ? map->get() : nullptr);
                text << '\n';
                --linesLeft;
            }
        }
    }
    if (final)
        text << '\n';
    if (!final && truncated)
        text << Style::Plain
             << "  Live rankings clipped; final summary includes all requested entries.\n";
    return text;
}
} // namespace SlangITool
