// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "hgvm-event-display.h"

#include "core/slang-char-encode.h"
#include "core/slang-char-util.h"
#include "core/slang-io.h"

#include <cstdlib>

#if SLANG_WINDOWS_FAMILY
#include <windows.h>
#endif

namespace SlangITool
{
using namespace Slang;
using namespace Slang::HGVM;

/// Reuses Slang's stage vocabulary, with the user-facing fragment spelling for pixel shaders.
static const char* getPipelineShaderStageName(SlangStage stage)
{
    if (stage == SLANG_STAGE_FRAGMENT)
        return "fragment";
    switch (stage)
    {
#define PROFILE_STAGE(TAG, NAME, VAL) \
    case VAL:                         \
        return #NAME;
#include "slang/slang-profile-defs.h"
    default:
        SLANG_UNEXPECTED("pipeline shader must have a concrete stage");
    }
}

void HGVMEventDisplay::appendPipelineShader(
    EventText& text,
    const TrackedPipelineShader& shader,
    const DebugMap* map) const
{
    text << EventTextStyle::Plain;
    if (shader.hitGroupIndex != UINT32_MAX)
        text << "hitgroup[" << shader.hitGroupIndex << "] ";
    text << getPipelineShaderStageName(shader.stage) << ' ' << EventTextStyle::Type;
    auto name = map ? map->findEntryPointName(shader.entryPoint.recordWordOffset) : nullptr;
    text << (name ? getDisplayName(map, *name) : String("<unknown>"));
}

EventTerminal::EventTerminal(
    ISlangWriter* writer,
    SlangWriterChannel channel,
    EventOutputOptions::Color color)
{
    bool terminal = writer->isConsole();
    const char* term = std::getenv("TERM");
    bool dumb = term && strcmp(term, "dumb") == 0;
    bool capable = !dumb;
#if SLANG_WINDOWS_FAMILY
    if (terminal && (capable || color == EventOutputOptions::Color::Always))
    {
        HANDLE console = GetStdHandle(
            channel == SLANG_WRITER_CHANNEL_STD_ERROR ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        capable = GetConsoleMode(console, &mode) &&
                  SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        if (capable)
        {
            m_console = console;
            m_originalMode = mode;
        }
    }
#else
    SLANG_UNUSED(channel);
    capable = capable && term && *term;
#endif
    m_redraw = terminal && capable && !dumb;
    const char* noColor = std::getenv("NO_COLOR");
    m_color = color == EventOutputOptions::Color::Always ||
              (color == EventOutputOptions::Color::Auto && m_redraw && !(noColor && *noColor));
}

EventTerminal::~EventTerminal()
{
#if SLANG_WINDOWS_FAMILY
    if (m_console)
        SetConsoleMode(m_console, m_originalMode);
#endif
}

void EventText::addSpan(Index start)
{
    Index end = m_text.getLength();
    if (start == end)
        return;
    if (m_spans.getCount() && m_spans.getLast().style == m_style)
        m_spans.getLast().end = end;
    else
        m_spans.add({start, end, m_style});
}

static const char* getStyleEscape(EventTextStyle style)
{
    switch (style)
    {
    case EventTextStyle::Plain:
        return "\x1b[0m";
    case EventTextStyle::Label:
        return "\x1b[1m";
    case EventTextStyle::Type:
        return "\x1b[36m";
    case EventTextStyle::String:
        return "\x1b[32m";
    case EventTextStyle::Duration:
        return "\x1b[33m";
    case EventTextStyle::Number:
        return "\x1b[35m";
    case EventTextStyle::Dim:
        return "\x1b[2m";
    }
    SLANG_UNREACHABLE("Unknown event text style");
}

/// Decodes one bounded character so clipping cannot split UTF-8 or read beyond the line.
static Char32 readDisplayCharacter(UnownedStringSlice text, Index& offset, Index end)
{
    return getUnicodePointFromUTF8(
        [&]() -> Byte { return offset < end ? Byte(text[offset++]) : 0; });
}

String EventText::render(bool color, Index start, Index end, int maxColumns) const
{
    auto text = m_text.getUnownedSlice();
    SLANG_ASSERT(start >= 0 && start <= end && end <= text.getLength());
    Index clippedEnd = end;
    bool ellipsis = false;
    if (maxColumns >= 0)
    {
        int columns = 0;
        Index fitWithEllipsis = start;
        for (Index i = start; i < end;)
        {
            Index characterStart = i;
            Char32 c = readDisplayCharacter(text, i, end);
            // Control characters are rendered as spaces, including tabs and embedded escapes.
            columns +=
                (c < 32 || (c >= 127 && c < 160)) ? 1 : int(UTF8Util::getCodePointDisplayWidth(c));
            if (columns <= maxColumns - 3)
                fitWithEllipsis = i;
            if (columns > maxColumns)
            {
                ellipsis = maxColumns >= 3;
                clippedEnd = ellipsis ? fitWithEllipsis : characterStart;
                break;
            }
        }
    }

    StringBuilder result;
    EventTextStyle active = EventTextStyle::Plain;
    Index spanIndex = 0;
    for (Index i = start; i < clippedEnd;)
    {
        while (spanIndex < m_spans.getCount() && m_spans[spanIndex].end <= i)
            ++spanIndex;
        SLANG_ASSERT(spanIndex < m_spans.getCount());
        Index characterStart = i;
        Char32 c = readDisplayCharacter(text, i, clippedEnd);
        auto style = c == '\n' ? EventTextStyle::Plain : m_spans[spanIndex].style;
        if (color && style != active)
        {
            if (active != EventTextStyle::Plain)
                result << getStyleEscape(EventTextStyle::Plain);
            if (style != EventTextStyle::Plain)
                result << getStyleEscape(style);
            active = style;
        }
        if (c == '\n' && maxColumns < 0)
            result << '\n';
        else if (c < 32 || (c >= 127 && c < 160))
            result << ' ';
        else
            result << text.subString(characterStart, i - characterStart);
    }
    if (color && active != EventTextStyle::Plain)
        result << getStyleEscape(EventTextStyle::Plain);
    if (ellipsis)
        result << "...";
    return result.produceString();
}

HGVMEventDisplay::HGVMEventDisplay(bool fullNames)
    : m_workingDirectory(Path::getCurrentPath()), m_fullNames(fullNames)
{
}

/// Strips the working-directory prefix at a path boundary without accessing the source file.
/// For example, a working directory ending in `src` must not shorten a path under `src-other`.
UnownedStringSlice HGVMEventDisplay::getSourcePath(const String& path) const
{
    auto result = path.getUnownedSlice();
    Index prefixLength = m_workingDirectory.getLength();
    if (!prefixLength || result.getLength() <= prefixLength)
        return result;
    for (Index i = 0; i < prefixLength; ++i)
    {
        char a = m_workingDirectory[i], b = result[i];
        if (Path::isDelimiter(a) && Path::isDelimiter(b))
            continue;
#if SLANG_WINDOWS_FAMILY
        a = CharUtil::toLower(a);
        b = CharUtil::toLower(b);
#endif
        if (a != b)
            return result;
    }
    if (!Path::isDelimiter(m_workingDirectory[prefixLength - 1]))
    {
        if (!Path::isDelimiter(result[prefixLength]))
            return result;
        ++prefixLength;
    }
    return result.tail(prefixLength);
}

/// Treats qualification as part of an identifier, so a suffix never matches another declaration.
static bool isNameCharacter(char c)
{
    return CharUtil::isAlphaOrDigit(c) || c == '_' || c == '.' || c == ':' || c == '$' ||
           c == '/' || c == '\\' || c == '-';
}

/// Formats canonical names for display without changing their lookup identities.
/// Consider `kernels::run<Buffer<models::Scene.Node>, _slang_Lambda_0>`: serialized module
/// names identify the removable prefixes, while `Scene.Node` remains a nested declaration.
/// Lambda lookup uses the original name and module before replacing it with a source location.
String HGVMEventDisplay::getDisplayName(
    const DebugMap* map,
    const String& name,
    const String& moduleName) const
{
    StringBuilder result;
    auto text = name.getUnownedSlice();
    for (Index i = 0; i < text.getLength();)
    {
        const LambdaLocation* match = nullptr;
        Index matchLength = 0;
        if (map && (!i || !isNameCharacter(text[i - 1])))
        {
            String qualifier;
            for (const auto& module : map->getModuleNames())
            {
                Index length = module.getLength();
                if (length > qualifier.getLength() && i + length + 2 <= text.getLength() &&
                    text.tail(i).startsWith(module.getUnownedSlice()) &&
                    text.subString(i + length, 2) == "::")
                    qualifier = module;
            }
            if (qualifier.getLength())
            {
                if (m_fullNames)
                    result << qualifier << "::";
                i += qualifier.getLength() + 2;
            }
            for (const auto& entry : map->getLambdaLocations())
            {
                auto candidate = entry.first.getUnownedSlice();
                Index end = i + candidate.getLength();
                if (candidate.getLength() <= matchLength || end > text.getLength() ||
                    (end < text.getLength() && isNameCharacter(text[end])) ||
                    !text.tail(i).startsWith(candidate))
                    continue;
                // A container's module does not identify the modules of its type arguments.
                auto location = map->findLambdaLocation(
                    entry.first,
                    qualifier.getLength()               ? qualifier
                    : i == 0 && end == text.getLength() ? moduleName
                                                        : String());
                if (location)
                {
                    match = location;
                    matchLength = candidate.getLength();
                }
            }
        }
        if (match)
        {
            result << "lambda@file(" << getSourcePath(match->path) << ':' << match->line << ':'
                   << match->column << ')';
            i += matchLength;
        }
        else if (i < text.getLength())
            result << text[i++];
    }
    return result.produceString();
}

} // namespace SlangITool
