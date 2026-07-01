#include "slang-rich-diagnostics-render.h"

#include "../core/slang-char-encode.h"
#include "../core/slang-dictionary.h"
#include "../core/slang-list.h"
#include "../core/slang-string-util.h"
#include "../core/slang-string.h"
#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-source-loc.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <ranges>

#ifdef SLANG_ENABLE_DIAGNOSTIC_RENDER_UNIT_TESTS

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#endif

namespace Slang
{
namespace
{

//
// The general structure of the below is that we take a 'GenericDiagnostic' (which contains the
// diagnostic without regard for how it's presented to the user) and convert that into a
// 'DiagnosticLayout' (which concerns itself with the logical layout of a diagnostic, if not the
// specifics of pasting characters and that) using 'createLayout'. From there we call
// 'renderFromLayout' which actually puts the lines and characters together
//

// Return the number of terminal columns occupied by a single Unicode code
// point. This is a coarse approximation of POSIX `wcwidth`: zero-width
// combining marks count as 0, the common East-Asian "wide" and "fullwidth"
// ranges count as 2, and everything else counts as 1. It exists so caret/
// underline rows line up under multi-byte and wide characters in source
// snippets (e.g. CJK text), which a naive byte- or code-point count gets wrong.
Int64 codePointDisplayWidth(Char32 c)
{
    // A tab advances by an indeterminate amount, but since the renderer emits
    // the source line verbatim (tabs are not expanded), counting it as a single
    // column keeps the caret line consistent with the rest of this code, which
    // also treats one tab as one column (e.g. indent detection). This matches
    // the pre-existing behavior before display-width handling was introduced.
    if (c == '\t')
        return 1;

    // Other control characters (NUL, the C0 range, and the C1 range) have no
    // sensible width; treat as zero so they don't shift the underline.
    if (c == 0 || c < 0x20 || (c >= 0x7f && c < 0xa0))
        return 0;

    // Zero-width combining marks and joiners.
    if ((c >= 0x0300 && c <= 0x036f) || // combining diacritical marks
        (c >= 0x1ab0 && c <= 0x1aff) || // combining diacritical marks extended
        (c >= 0x1dc0 && c <= 0x1dff) || // combining diacritical marks supplement
        (c >= 0x20d0 && c <= 0x20ff) || // combining diacritical marks for symbols
        (c >= 0xfe20 && c <= 0xfe2f) || // combining half marks
        c == 0x200b ||                  // zero-width space
        c == 0x200d)                    // zero-width joiner
        return 0;

    // East-Asian wide and fullwidth ranges (subset covering the common cases).
    if ((c >= 0x1100 && c <= 0x115f) || // Hangul Jamo
        (c >= 0x2e80 && c <= 0x303e) || // CJK radicals, Kangxi, symbols
        (c >= 0x3041 && c <= 0x33ff) || // Hiragana, Katakana, CJK symbols
        (c >= 0x3400 && c <= 0x4dbf) || // CJK Unified Ideographs Extension A
        (c >= 0x4e00 && c <= 0x9fff) || // CJK Unified Ideographs
        (c >= 0xa000 && c <= 0xa4cf) || // Yi
        (c >= 0xac00 && c <= 0xd7a3) || // Hangul Syllables
        (c >= 0xf900 && c <= 0xfaff) || // CJK Compatibility Ideographs
        (c >= 0xfe30 && c <= 0xfe4f) || // CJK Compatibility Forms
        (c >= 0xff00 && c <= 0xff60) || // Fullwidth Forms
        (c >= 0xffe0 && c <= 0xffe6) || // Fullwidth signs
        (c >= 0x20000 && c <= 0x3fffd)) // CJK Unified Ideographs Extension B+
        return 2;

    return 1;
}

// Decode a single UTF-8 code point from `[cur, end)`, advancing `cur`. The
// reader handed to `getUnicodePointFromUTF8` returns 0 once `cur` reaches `end`,
// so a truncated multi-byte sequence at the end of the buffer terminates the
// decode rather than reading past `end` (the trailing continuation bytes a
// well-formed sequence would supply are simply treated as zero).
Char32 decodeCodePoint(const char*& cur, const char* const end)
{
    // Read through `unsigned char` so bytes >= 0x80 aren't sign-extended into
    // the decoder, which would corrupt multi-byte sequences on platforms where
    // `char` is signed.
    return getUnicodePointFromUTF8(
        [&]() -> Byte { return cur < end ? Byte(static_cast<unsigned char>(*cur++)) : 0; });
}

// Return the number of terminal columns occupied by a UTF-8 slice, summing the
// display width of each code point.
Int64 displayWidth(const UnownedStringSlice& text)
{
    Int64 width = 0;
    const char* cur = text.begin();
    const char* const end = text.end();
    while (cur < end)
        width += codePointDisplayWidth(decodeCodePoint(cur, end));
    return width;
}

// Given a line of UTF-8 source and a 1-based code-point column (as produced by
// the source manager), return the corresponding 0-based *byte* offset into the
// line. A non-positive column is treated as the start of the line, and columns
// past the end clamp to the end of the line.
Int64 byteOffsetForCodePointColumn(const UnownedStringSlice& line, Int64 codePointColumn)
{
    const char* cur = line.begin();
    const char* const end = line.end();
    Int64 column = 1;
    while (cur < end && column < codePointColumn)
    {
        decodeCodePoint(cur, end);
        ++column;
    }
    return Int64(cur - line.begin());
}

struct DiagnosticRenderer
{
public:
    DiagnosticRenderer(
        SourceManager* sm,
        DiagnosticSink::SourceLocationLexer sll,
        DiagnosticRenderOptions opts)
        : m_sourceManager(sm)
        , m_lexer(sll)
        , m_options(opts)
        , m_glyphs(opts.enableUnicode ? s_unicodeGlyphs : s_asciiGlyphs)
    {
    }

    String render(const GenericDiagnostic& diag)
    {
        DiagnosticLayout layout = createLayout(diag);
        return renderFromLayout(layout);
    }

private:
    SourceManager* m_sourceManager;
    DiagnosticSink::SourceLocationLexer m_lexer;
    DiagnosticRenderOptions m_options;

    // Colors safe on both dark and light terminal color schemes
    // See https://blog.xoria.org/terminal-colors/
    enum class TerminalColor
    {
        Regular,
        BoldRegular,
        Red,
        Green,
        Yellow,
        Magenta,
        Cyan,
        BrightRed,
        BrightMagenta,
        BoldRed,
        BoldBrightRed,
        BoldMagenta,
        BoldBrightMagenta,
    };

    struct Glyphs
    {
        // the underlines are characters such as
        // ┬──────────, where the first character there is the 'Join' one, it
        // joins the underline to the column going to the message
        const char* primaryUnderline;
        const char* primaryUnderlineJoin;
        const char* secondaryUnderline;
        const char* secondaryUnderlineJoin;

        // A vertical pipe
        const char* vertical;
        // A north-east corner, from the bottom of the pipe to introduce text on the right
        const char* corner;

        // to mark filenames at the beginning of a diagnostic
        const char* arrow;
        const char* noteDash;
        // A south-east corner to close the gutter at the end (curves from down to left)
        const char* gutterCorner;
    };
    // Unicode box-drawing glyphs as raw UTF-8 bytes to avoid source encoding assumptions:
    //   ━ U+2501  ┯ U+252F  ─ U+2500  ┬ U+252C  │ U+2502
    //   ╰ U+2570  ╭ U+256D  ╼ U+257C  ╯ U+256F
    constexpr static Glyphs s_unicodeGlyphs = {
        "\xe2\x94\x81",
        "\xe2\x94\xaf",
        "\xe2\x94\x80",
        "\xe2\x94\xac",
        "\xe2\x94\x82",
        "\xe2\x95\xb0 ",
        " \xe2\x95\xad\xe2\x95\xbc",
        " \xe2\x95\xad\xe2\x95\xbc",
        "\xe2\x95\xaf",
    };
    constexpr static Glyphs s_asciiGlyphs = {"^", "^", "-", "-", "|", "`", "-->", "---", "'"};
    const Glyphs& m_glyphs;

    // A single highlight on a line, with an optional label to be connected
    struct LineHighlight
    {
        Int64 column;
        Int64 length;
        String label;   // May be empty
        bool isPrimary; // Does this one deserve special attention
    };

    // A line of source code, along with some sections which are highlighted
    // and labeled
    struct HighlightedLine
    {
        Int64 number = 0;
        UnownedStringSlice content;
        List<LineHighlight> spans;
        bool sourceAvailable = false;
    };

    // A collection of nearby HighlightedLines
    struct LayoutBlock
    {
        bool showGap = false;
        List<HighlightedLine> lines;
    };

    // A collection of blocks
    struct SectionLayout
    {
        Int64 maxGutterWidth;
        size_t commonIndent;
        List<LayoutBlock> blocks;
    };

    // A full diagnostic, sorted and ready for rendering
    struct DiagnosticLayout
    {
        struct Header
        {
            String severity;
            Severity severityValue;
            Int64 code = 0;
            String message;
        } header;

        struct Location
        {
            String fileName;
            Int64 line = 0;
            Int64 col = 0;
            Int64 gutterIndent = 0;
            PathInfo::Type pathType = PathInfo::Type::Unknown;
        } primaryLoc;

        SectionLayout primarySection;

        struct NoteEntry
        {
            String message;
            Location loc;
            SectionLayout section;
        };
        List<NoteEntry> notes;
    };

    // Introduce and reset a terminal color
    String color(TerminalColor c, const String& text) const
    {
        if (!m_options.enableTerminalColors)
            return text;
        const char* code = "";
        switch (c)
        {
        case TerminalColor::Regular:
            code = "\x1B[0m";
            break;
        case TerminalColor::BoldRegular:
            code = "\x1B[1m";
            break;
        case TerminalColor::Red:
            code = "\x1B[31m";
            break;
        case TerminalColor::Green:
            code = "\x1B[32m";
            break;
        case TerminalColor::Yellow:
            code = "\x1B[33m";
            break;
        case TerminalColor::Magenta:
            code = "\x1B[35m";
            break;
        case TerminalColor::Cyan:
            code = "\x1B[36m";
            break;
        case TerminalColor::BrightRed:
            code = "\x1B[91m";
            break;
        case TerminalColor::BrightMagenta:
            code = "\x1B[95m";
            break;
        case TerminalColor::BoldRed:
            code = "\x1B[1;31m";
            break;
        case TerminalColor::BoldBrightRed:
            code = "\x1B[1;91m";
            break;
        case TerminalColor::BoldMagenta:
            code = "\x1B[1;35m";
            break;
        case TerminalColor::BoldBrightMagenta:
            code = "\x1B[1;95m";
            break;
        }
        return String(code) + text + "\x1B[0m";
    }

    String repeat(char c, Int64 n) const
    {
        String ret;
        if (n > 0)
            ret.appendRepeatedChar(c, n);
        return ret;
    }

    // s should be a single width character
    String repeat(const char* s, Int64 n) const
    {
        String ret;
        for (Int64 i = 0; i < n; ++i)
            ret.append(s);
        return ret;
    }

    //
    // Organizes diagnostic spans into a structured layout for terminal rendering.
    //
    // The function processes source code highlights through several stages:
    //
    // 1. Gutter Calculation:
    //    Determines the maximum line number to establish a consistent width for the
    //    left-hand gutter (line numbers).
    //
    // 2. Groups individual spans by their line number.
    //
    // 3. Block Segmentation
    //    Iterates through sorted line numbers to group contiguous lines into
    //    'LayoutBlocks'. If a jump in line numbers is detected (e.g., line 10
    //    followed by line 20), it closes the current block and starts a new one
    //    marked with a 'gap', allowing the renderer to insert ellipsis (...)
    //    between disconnected code snippets.
    //
    // 5. Indent Optimization:
    //    Calculates a common indentation across all blocks to shift the code
    //    horizontally, maximizing visible space by removing unnecessary leading whitespace.
    //

    //
    // Layout span what's given to each section in buildSectionLayout, these
    // are sorted and merged into HighlightedLines
    //
    struct LayoutSpan
    {
        Int64 line;
        Int64 col;
        Int64 length;
        String label;
        bool isPrimary;
        Slang::SourceLoc startLoc;
    };

    SectionLayout buildSectionLayout(List<LayoutSpan>& spans)
    {
        SectionLayout section;
        if (spans.getCount() == 0)
            return section;

        Int64 maxLineNum = 1;
        for (const auto& span : spans)
            maxLineNum = std::max(maxLineNum, span.line);
        section.maxGutterWidth = static_cast<Int64>(std::log10(maxLineNum)) + 1;

        Dictionary<Int64, HighlightedLine> grouped;
        for (auto& span : spans)
        {
            HighlightedLine& line = grouped[span.line];
            line.number = span.line;
            if (line.content.getLength() == 0 && !line.sourceAvailable)
            {
                SourceView* view =
                    m_sourceManager ? m_sourceManager->findSourceView(span.startLoc) : nullptr;
                if (view)
                {
                    line.sourceAvailable = true;
                    // Use the *actual* (non-remapped) line so that a #line
                    // directive doesn't cause us to display the wrong source.
                    auto actualLine = view->getHumaneLoc(span.startLoc, SourceLocType::Actual).line;
                    // Get the line content and trim end-of-line characters and trailing whitespace
                    UnownedStringSlice rawLine = StringUtil::trimEndOfLine(
                        view->getSourceFile()->getLineAtIndex(actualLine - 1));
                    // Trim trailing whitespace but preserve leading whitespace (indentation)
                    line.content = UnownedStringSlice(rawLine.begin(), rawLine.trim().end());
                }
            }
            // `span.col` is a 1-based *code-point* column and `span.length` is a
            // *byte* length, but the renderer lays out carets/underlines in
            // terminal *display* columns. Translate both into display-column
            // space here, using the actual source bytes, so that multi-byte and
            // wide (e.g. CJK) characters in the source line don't shift or
            // shrink the underline. See issue #5219.
            //
            // When the source line is unavailable (e.g. built-in modules) we
            // keep the raw column/length; only the label is rendered in that
            // case (see renderOrphanedLabels).
            Int64 column = span.col;
            Int64 length = span.length;
            if (line.content.getLength() > 0 && span.col > 0)
            {
                Int64 byteOffset = byteOffsetForCodePointColumn(line.content, span.col);
                if (m_lexer && length <= 0 && byteOffset < line.content.getLength())
                    length = m_lexer(line.content.tail(byteOffset)).getLength();

                Int64 underlineEnd = std::min(
                    (Int64)line.content.getLength(),
                    byteOffset + std::max(Int64{0}, length));
                column = 1 + displayWidth(line.content.head(byteOffset));
                length =
                    displayWidth(line.content.subString(byteOffset, underlineEnd - byteOffset));
            }
            line.spans.add({column, length, span.label, span.isPrimary});
        }

        List<Int64> lineNumbers;
        for (auto& [num, line] : grouped)
        {
            line.spans.sort([](const auto& a, const auto& b) { return a.column < b.column; });
            lineNumbers.add(num);
        }
        lineNumbers.sort();

        LayoutBlock currentBlock;
        Int64 prevLine = -1;
        for (Int64 number : lineNumbers)
        {
            if (prevLine != -1 && number > prevLine + 1)
            {
                section.blocks.add(currentBlock);
                currentBlock = {true, {}};
            }
            currentBlock.lines.add(grouped[number]);
            prevLine = number;
        }
        section.blocks.add(currentBlock);
        section.commonIndent = findCommonIndent(section.blocks);
        return section;
    }

    // Returns true if any line in the section has source text available.
    // When source is unavailable (e.g. built-in modules), we skip rendering
    // the section body to avoid showing empty source lines with underlines.
    bool sectionHasSourceAvailable(const SectionLayout& section) const
    {
        for (const auto& block : section.blocks)
            for (const auto& line : block.lines)
                if (line.sourceAvailable)
                    return true;
        return false;
    }

    Int64 findCommonIndent(const List<LayoutBlock>& blocks)
    {
        Index minIndent = std::numeric_limits<Index>::max();
        for (const auto& b : blocks)
            for (const auto& l : b.lines)
            {
                if (l.content.getLength() == 0)
                    continue;
                Index i = 0;
                while (i < l.content.getLength() && (l.content[i] == ' ' || l.content[i] == '\t'))
                    ++i;
                minIndent = std::min(minIndent, i);
            }
        return minIndent == std::numeric_limits<Index>::max() ? 0 : minIndent;
    }

    //
    // Render a line of source code, and if there are colors enabled render it
    // with colors matching the highlights
    //
    void renderSourceLine(StringBuilder& ss, const HighlightedLine& line, Int64 indent)
    {
        UnownedStringSlice content =
            line.content.tail(std::min((Int64)line.content.getLength(), indent));
        if (!m_options.enableTerminalColors || line.spans.getCount() == 0)
        {
            ss << content;
            return;
        }

        // Spans are positioned in display columns (see buildSectionLayout), but
        // `content` is a byte slice, so map each display column to a byte offset
        // before slicing to avoid splitting multi-byte UTF-8 sequences.
        auto byteOffsetForDisplayColumn = [&](Int64 displayColumn) -> Int64
        {
            const char* cur = content.begin();
            const char* const end = content.end();
            Int64 col = 1;
            while (cur < end && col < displayColumn)
                col += codePointDisplayWidth(decodeCodePoint(cur, end));
            return Int64(cur - content.begin());
        };

        Int64 cursorByte = 0;
        for (const auto& span : line.spans)
        {
            Int64 start = span.column - indent;
            Int64 startByte = byteOffsetForDisplayColumn(start);
            Int64 endByte = byteOffsetForDisplayColumn(start + span.length);

            // Emit any plain (uncolored) text between the previous span and this
            // one, then the colored region. Clamp to `cursorByte` so overlapping
            // spans don't re-emit (and duplicate) bytes already written.
            if (startByte > cursorByte)
                ss << content.subString(cursorByte, startByte - cursorByte);
            startByte = std::max(startByte, cursorByte);

            TerminalColor c = span.isPrimary ? TerminalColor::Red : TerminalColor::Cyan;
            if (endByte > startByte)
                ss << color(c, String(content.subString(startByte, endByte - startByte)));
            cursorByte = std::max(cursorByte, endByte);
        }
        if (cursorByte < content.getLength())
            ss << content.tail(cursorByte);
    }

    //
    // Generates the multi-line underlines and pipes for a line of code
    //
    // The process follows a specific directional logic to handle overlapping or multi-line labels:
    //
    // 1. Forward Pass (Spans):
    //    Iterates through spans to build the primary underline row (using '^' or '-').
    //    It calculates spacing based on column offsets and collects label metadata.
    //
    // 2. Reverse Iteration (Left-to-Right):
    //    When building the "connector" row ('|') and subsequent label rows, the code
    //    iterates through the sorted labels in reverse (left-to-right). This ensures
    //    that vertical bars are drawn in the correct horizontal sequence.
    //
    // 3. Nested Iteration (Vertical Stacking):
    //    The final loop iterates through labels from right-to-left (the outer loop) to
    //    determine which label text to print on the current row. For each row, it
    //    traverses left-to-right (the inner reverse view) to draw the necessary
    //    vertical connectors for labels that haven't been printed yet.
    //
    List<String> buildAnnotationRows(const HighlightedLine& line, Int64 indentShift)
    {
        List<String> rows;
        if (line.spans.getCount() == 0)
            return rows;
        struct Label
        {
            Int64 col;
            String text;
            bool isPrimary;
        };
        List<Label> labels;
        StringBuilder sub;
        Int64 cursor = 1;

        for (Int64 i = 0; i < line.spans.getCount(); ++i)
        {
            const auto& span = line.spans[i];
            const bool isLast = i == line.spans.getCount() - 1;
            Int64 col = std::max(Int64{1}, span.column - indentShift);
            const char* glyph =
                span.isPrimary ? m_glyphs.primaryUnderline : m_glyphs.secondaryUnderline;
            const char* joinGlyph = isLast || (!span.label.getLength()) ? glyph
                                    : span.isPrimary ? m_glyphs.primaryUnderlineJoin
                                                     : m_glyphs.secondaryUnderlineJoin;
            sub << repeat(' ', std::max(Int64{0}, col - cursor))
                << color(
                       span.isPrimary ? TerminalColor::Red : TerminalColor::Cyan,
                       joinGlyph + repeat(glyph, std::max(Int64{0}, span.length - 1)));
            cursor = col + span.length;
            if (span.label.getLength() > 0)
                labels.add({col, span.label, span.isPrimary});
        }
        labels.sort([](const auto& a, const auto& b) { return a.col > b.col; });
        if (labels.getCount() > 0)
        {
            sub << " " << labels[0].text;
            labels.removeAt(0);
        }
        rows.add(sub.produceString());

        while (labels.getCount() > 0)
        {
            StringBuilder conn;
            Int64 p = 1;
            for (const auto& l : labels | std::views::reverse)
            {
                conn << repeat(' ', l.col - p)
                     << color(
                            l.isPrimary ? TerminalColor::Red : TerminalColor::Cyan,
                            m_glyphs.vertical);
                p = l.col + 1;
            }
            rows.add(conn.produceString());

            StringBuilder lab;
            Int64 c = 1;
            Label target = labels[0];
            for (const auto& l : labels | std::views::reverse)
            {
                if (l.col > target.col)
                    break;
                lab << repeat(' ', l.col - c);
                if (l.col == target.col)
                {
                    // Bend the bottom of the pipe towards the text
                    lab << color(
                               l.isPrimary ? TerminalColor::Red : TerminalColor::Cyan,
                               m_glyphs.corner)
                        << l.text;
                    c = l.col + l.text.getLength();
                }
                else
                {
                    lab << color(
                        l.isPrimary ? TerminalColor::Red : TerminalColor::Cyan,
                        m_glyphs.vertical);
                    c = l.col + 1;
                }
            }
            rows.add(lab.produceString());
            // O(n^2), but it's not like we're going to be rendering
            // diagnostics with hundreds of labels
            labels.removeAt(0);
        }
        return rows;
    }

    // When source text is unavailable (e.g. built-in modules), render span
    // labels with a compact gutter structure (pipe + closing corner), using a
    // minimal gutter width since there are no line numbers to display.
    void renderOrphanedLabels(StringBuilder& ss, const SectionLayout& section, Int64 gutterWidth)
    {
        String gutter =
            repeat(' ', gutterWidth + 1) + color(TerminalColor::Cyan, m_glyphs.vertical);
        for (const auto& block : section.blocks)
            for (const auto& line : block.lines)
                for (const auto& span : line.spans)
                    if (span.label.getLength() > 0)
                        ss << gutter << " " << color(TerminalColor::BoldRed, span.label) << "\n";
        ss << color(
                  TerminalColor::Cyan,
                  repeat(m_glyphs.secondaryUnderline, gutterWidth + 1) + m_glyphs.gutterCorner)
           << "\n";
    }

    void renderSectionBody(StringBuilder& ss, const SectionLayout& section)
    {
        for (const auto& block : section.blocks)
        {
            if (block.showGap)
                ss << "...\n";
            for (const auto& line : block.lines)
            {
                String label = String(line.number);
                ss << repeat(' ', section.maxGutterWidth - label.getLength())
                   << color(TerminalColor::BoldRegular, label) << " "
                   << color(TerminalColor::Cyan, m_glyphs.vertical) << " ";
                renderSourceLine(ss, line, section.commonIndent);
                ss << "\n";

                auto rows = buildAnnotationRows(line, section.commonIndent);
                for (const auto& row : rows)
                    ss << repeat(' ', section.maxGutterWidth + 1)
                       << color(TerminalColor::Cyan, m_glyphs.vertical) << " " << row << "\n";
            }
        }
    }

    DiagnosticLayout createLayout(const GenericDiagnostic& diag)
    {
        DiagnosticLayout layout;
        layout.header.severity = getSeverityName(diag.severity);
        layout.header.severityValue = diag.severity;
        layout.header.code = diag.code;
        layout.header.message = diag.message;

        HumaneSourceLoc humaneLoc;
        if (m_sourceManager)
        {
            humaneLoc = m_sourceManager->getHumaneLoc(diag.primarySpan.range.begin);
        }
        layout.primaryLoc.fileName = humaneLoc.pathInfo.foundPath;
        layout.primaryLoc.line = humaneLoc.line;
        layout.primaryLoc.col = humaneLoc.column;
        layout.primaryLoc.pathType = humaneLoc.pathInfo.type;

        List<LayoutSpan> allSpans;
        allSpans.add(makeLayoutSpan(diag.primarySpan, true));
        for (const auto& s : diag.secondarySpans)
            allSpans.add(makeLayoutSpan(s, false));

        layout.primarySection = buildSectionLayout(allSpans);
        layout.primaryLoc.gutterIndent = layout.primarySection.maxGutterWidth;

        for (const auto& note : diag.notes)
        {
            DiagnosticLayout::NoteEntry noteEntry;
            noteEntry.message = note.message;
            HumaneSourceLoc noteHumane;
            if (m_sourceManager)
            {
                noteHumane = m_sourceManager->getHumaneLoc(note.span.range.begin);
            }
            noteEntry.loc.fileName = noteHumane.pathInfo.foundPath;
            noteEntry.loc.line = noteHumane.line;
            noteEntry.loc.col = noteHumane.column;
            noteEntry.loc.pathType = noteHumane.pathInfo.type;

            List<LayoutSpan> noteSpans;
            noteSpans.add(makeLayoutSpan(note.span, false));
            // Add additional spans attached to the note
            for (const auto& additionalSpan : note.secondarySpans)
                noteSpans.add(makeLayoutSpan(additionalSpan, false));
            noteEntry.section = buildSectionLayout(noteSpans);
            noteEntry.loc.gutterIndent = noteEntry.section.maxGutterWidth;
            layout.notes.add(std::move(noteEntry));
        }
        return layout;
    }

    LayoutSpan makeLayoutSpan(const DiagnosticSpan& span, bool isPrimary)
    {
        HumaneSourceLoc humane;
        if (m_sourceManager)
        {
            humane = m_sourceManager->getHumaneLoc(span.range.begin);
        }
        return {
            humane.line,
            humane.column,
            span.range.begin == span.range.end ? -1 : span.range.getOffset(span.range.end),
            span.message,
            isPrimary,
            span.range.begin};
    }

    void renderLocation(StringBuilder& ss, const DiagnosticLayout::Location& loc) const
    {
        ss << repeat(' ', loc.gutterIndent) << color(TerminalColor::Cyan, m_glyphs.arrow) << " ";
        if (loc.pathType == PathInfo::Type::CommandLine)
        {
            // For command line sources, don't show line:col
            ss << loc.fileName << "\n";
        }
        else
        {
            ss << loc.fileName << ":" << loc.line << ":" << loc.col << "\n";
        }
    }

    void renderNoteLocation(StringBuilder& ss, const DiagnosticLayout::Location& loc) const
    {
        ss << repeat(' ', loc.gutterIndent) << color(TerminalColor::Cyan, m_glyphs.noteDash) << " ";
        if (loc.pathType == PathInfo::Type::CommandLine)
        {
            // For command line sources, don't show line:col
            ss << loc.fileName << "\n";
        }
        else
        {
            ss << loc.fileName << ":" << loc.line << ":" << loc.col << "\n";
        }
    }

    String renderFromLayout(const DiagnosticLayout& layout)
    {
        StringBuilder ss;
        TerminalColor sevColor = (layout.header.severityValue >= Severity::Error)
                                     ? TerminalColor::Red
                                     : TerminalColor::Yellow;
        ss << color(sevColor, layout.header.severity);
        if (layout.header.code >= 0)
        {
            String codeStr = String(layout.header.code);
            ss << "[E" << repeat('0', 5 - codeStr.getLength()) << codeStr << "]";
        }
        ss << ": " << color(TerminalColor::BoldRegular, layout.header.message) << "\n";

        // Skip location and source snippet for diagnostics without meaningful locations
        // (line 0 indicates SourceLoc() was used, meaning no source location)
        bool hasValidLocation =
            layout.primaryLoc.line > 0 || layout.primaryLoc.fileName.getLength() > 0;
        if (hasValidLocation)
        {
            bool primaryHasSource = sectionHasSourceAvailable(layout.primarySection);
            if (primaryHasSource)
            {
                renderLocation(ss, layout.primaryLoc);
                if (layout.primarySection.blocks.getCount() > 0)
                {
                    ss << repeat(' ', layout.primarySection.maxGutterWidth + 1)
                       << color(TerminalColor::Cyan, m_glyphs.vertical) << "\n";
                    renderSectionBody(ss, layout.primarySection);
                    ss << color(
                              TerminalColor::Cyan,
                              repeat(
                                  m_glyphs.secondaryUnderline,
                                  layout.primarySection.maxGutterWidth + 1) +
                                  m_glyphs.gutterCorner)
                       << "\n";
                }
            }
            else
            {
                constexpr Int64 kOrphanGutterWidth = 0;
                DiagnosticLayout::Location loc = layout.primaryLoc;
                loc.gutterIndent = kOrphanGutterWidth;
                renderLocation(ss, loc);
                if (layout.primarySection.blocks.getCount() > 0)
                    renderOrphanedLabels(ss, layout.primarySection, kOrphanGutterWidth);
            }
        }
        for (const auto& note : layout.notes)
        {
            ss << "\n" << color(TerminalColor::Cyan, "note") << ": " << note.message << "\n";
            bool noteHasSource = sectionHasSourceAvailable(note.section);
            if (noteHasSource)
            {
                renderNoteLocation(ss, note.loc);
                if (note.section.blocks.getCount() > 0)
                {
                    ss << repeat(' ', note.section.maxGutterWidth + 1)
                       << color(TerminalColor::Cyan, m_glyphs.vertical) << "\n";
                    renderSectionBody(ss, note.section);
                    ss << color(
                              TerminalColor::Cyan,
                              repeat(m_glyphs.secondaryUnderline, note.section.maxGutterWidth + 1) +
                                  m_glyphs.gutterCorner)
                       << "\n";
                }
            }
            else
            {
                constexpr Int64 kOrphanGutterWidth = 0;
                DiagnosticLayout::Location noteLoc = note.loc;
                noteLoc.gutterIndent = kOrphanGutterWidth;
                renderNoteLocation(ss, noteLoc);
                if (note.section.blocks.getCount() > 0)
                    renderOrphanedLabels(ss, note.section, kOrphanGutterWidth);
            }
        }
        return ss.produceString();
    }
};

} // namespace

String renderDiagnostic(
    DiagnosticSink::SourceLocationLexer sll,
    SourceManager* sm,
    DiagnosticRenderOptions options,
    const GenericDiagnostic& diag)
{
    DiagnosticRenderer renderer(sm, sll, options);
    return renderer.render(diag);
}

String renderDiagnosticMachineReadable(
    DiagnosticSink::SourceLocationLexer sll,
    SourceManager* sm,
    const GenericDiagnostic& diag)
{
    StringBuilder sb;

    // Format:
    // E<code>\t<severity>\t<filename>\t<beginline>\t<begincol>\t<endline>\t<endcol>\t<message>

    // Format the error code as E##### (e.g., E00001, E12345)
    // Use empty string for negative codes (-1 is used as placeholder for no code)
    String codeStr;
    if (diag.code >= 0)
    {
        String numStr = String(diag.code);
        String padding;
        padding.appendRepeatedChar('0', std::max<Int64>(0, 5 - numStr.getLength()));
        codeStr = padding + numStr;
    }

    // Helper lambda to output a span in the machine-readable format
    // Returns false if the span was skipped (0,0 location with no message)
    auto outputSpan = [&](const DiagnosticSpan& span, const char* severity, const String& message)
    {
        HumaneSourceLoc beginLoc = sm->getHumaneLoc(span.range.begin);
        HumaneSourceLoc endLoc = sm->getHumaneLoc(span.range.end);

        // When span has zero length (begin == end), use the lexer to find token boundaries,
        // mirroring the logic in buildSectionLayout for rich diagnostics
        if (sll && span.range.begin == span.range.end && beginLoc.line > 0)
        {
            SourceView* view = sm->findSourceView(span.range.begin);
            if (view)
            {
                auto actualLine = view->getHumaneLoc(span.range.begin, SourceLocType::Actual).line;
                UnownedStringSlice rawLine = StringUtil::trimEndOfLine(
                    view->getSourceFile()->getLineAtIndex(actualLine - 1));
                UnownedStringSlice lineContent =
                    UnownedStringSlice(rawLine.begin(), rawLine.trim().end());
                if (lineContent.getLength() > 0 && beginLoc.column > 0 &&
                    beginLoc.column - 1 < lineContent.getLength())
                {
                    Int64 tokenLen = sll(lineContent.tail(beginLoc.column - 1)).getLength();
                    if (tokenLen > 0)
                    {
                        endLoc.line = beginLoc.line;
                        endLoc.column = beginLoc.column + tokenLen;
                    }
                }
            }
        }

        // Check for locationless span (0,0)
        bool isLocationless = (beginLoc.line == 0 && beginLoc.column == 0);
        if (isLocationless)
        {
            // Skip outputting 0,0 spans with no message - they provide no value
            if ((strcmp(severity, "span") == 0 || strcmp(severity, "note-span") == 0) &&
                message.getLength() == 0)
            {
                return false;
            }
        }

        sb << "E" << codeStr << "\t";
        sb << severity << "\t";
        sb << beginLoc.pathInfo.foundPath << "\t";
        sb << beginLoc.line << "\t";
        sb << beginLoc.column << "\t";
        sb << endLoc.line << "\t";
        sb << endLoc.column << "\t";
        sb << message << "\n";
        return true;
    };

    // Output primary diagnostic
    outputSpan(diag.primarySpan, getSeverityName(diag.severity), diag.message);

    // Output primary span message (if it has a valid location or message)
    outputSpan(diag.primarySpan, "span", diag.primarySpan.message);

    // Output secondary spans
    for (const auto& secondarySpan : diag.secondarySpans)
    {
        outputSpan(secondarySpan, "span", secondarySpan.message);
    }

    // Output notes
    for (const auto& note : diag.notes)
    {
        // Output the note's primary span
        outputSpan(note.span, "note", note.message);

        // Output any secondary spans attached to the note
        for (const auto& secondarySpan : note.secondarySpans)
        {
            outputSpan(secondarySpan, "note-span", secondarySpan.message);
        }
    }

    return sb.produceString();
}

} // namespace Slang
