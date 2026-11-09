// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

#include "core/slang-string-util.h"
#include "slang-hgvm/slang-hgvm-debug-map.h"
#include "slang-hgvm/slang-hgvm-events.h"
#include "slangi-options.h"

namespace SlangITool
{
enum class EventTextStyle
{
    Plain,
    Label,
    Type,
    String,
    Duration,
    Number,
    Dim,
};

/// Stores text and semantic style spans. ANSI encoding happens only after clipping plain text.
class EventText
{
public:
    EventText& operator<<(EventTextStyle style)
    {
        m_style = style;
        return *this;
    }
    template<typename T>
    EventText& operator<<(const T& value)
    {
        auto start = m_text.getLength();
        m_text << value;
        addSpan(start);
        return *this;
    }
    template<typename... Args>
    void appendFormat(const char* format, Args... args)
    {
        auto start = m_text.getLength();
        Slang::StringUtil::appendFormat(m_text, format, args...);
        addSpan(start);
    }
    template<typename T>
    void field(const char* name, const T& value, EventTextStyle style = EventTextStyle::Number)
    {
        *this << EventTextStyle::Plain << ' ' << name << '=' << style << value;
    }
    Slang::UnownedStringSlice getText() const { return m_text.getUnownedSlice(); }
    /// Renders a byte range on character boundaries; maxColumns < 0 leaves it unclipped.
    Slang::String render(bool color, Slang::Index start, Slang::Index end, int maxColumns = -1)
        const;
    Slang::String render(bool color) const { return render(color, 0, m_text.getLength()); }

private:
    struct Span
    {
        Slang::Index start;
        Slang::Index end;
        EventTextStyle style;
    };
    void addSpan(Slang::Index start);
    Slang::StringBuilder m_text;
    Slang::List<Span> m_spans;
    EventTextStyle m_style = EventTextStyle::Plain;
};

/// Owns terminal setup for one output stream, independently of color and dashboard policies.
class EventTerminal
{
public:
    EventTerminal(
        ISlangWriter* writer,
        SlangWriterChannel channel,
        EventOutputOptions::Color color);
    ~EventTerminal();
    EventTerminal(const EventTerminal&) = delete;
    EventTerminal& operator=(const EventTerminal&) = delete;
    bool useColor() const { return m_color; }
    bool canRedraw() const { return m_redraw; }

private:
    bool m_color = false;
    bool m_redraw = false;
    void* m_console = nullptr;
    uint32_t m_originalMode = 0;
};

/// Shares source-path and lambda-name formatting between trace and statistics output.
class HGVMEventDisplay
{
public:
    explicit HGVMEventDisplay(bool fullNames = false);
    bool usesFullNames() const { return m_fullNames; }
    Slang::String getDisplayName(
        const Slang::HGVM::DebugMap* map,
        const Slang::String& name,
        const Slang::String& moduleName = {}) const;
    Slang::UnownedStringSlice getSourcePath(const Slang::String& path) const;
    /// Appends one shader role and semantic name, including its hit group when applicable.
    void appendPipelineShader(
        EventText& text,
        const Slang::HGVM::TrackedPipelineShader& shader,
        const Slang::HGVM::DebugMap* map) const;

private:
    Slang::String m_workingDirectory;
    bool m_fullNames;
};
} // namespace SlangITool
