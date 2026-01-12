#include "slang-rich-diagnostics-render.h"

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

    enum class TerminalColor
    {
        Red,
        Yellow,
        Cyan,
        Blue,
        Bold,
        Reset
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
    };
    constexpr static Glyphs s_unicodeGlyphs = {"━", "┯", "─", "┬", "│", "╰ ", "-->", "---"};
    constexpr static Glyphs s_asciiGlyphs = {"^", "^", "-", "-", "|", "`", "-->", "---"};
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
        case TerminalColor::Red:
            code = "\x1B[31;1m";
            break;
        case TerminalColor::Yellow:
            code = "\x1B[33;1m";
            break;
        case TerminalColor::Cyan:
            code = "\x1B[36;1m";
            break;
        case TerminalColor::Blue:
            code = "\x1B[34;1m";
            break;
        case TerminalColor::Bold:
            code = "\x1B[1m";
            break;
        case TerminalColor::Reset:
            code = "\x1B[0m";
            break;
        default:
            return text;
        }
        return String(code) + text + "\x1B[0m";
    }

    String repeat(char c, Int64 n) const
    {
        String ret;
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
            if (line.content.getLength() == 0)
            {
                SourceView* view = m_sourceManager->findSourceView(span.startLoc);
                if (view)
                    line.content = StringUtil::trimEndOfLine(
                        view->getSourceFile()->getLineAtIndex(span.line - 1));
            }
            if (m_lexer && span.length <= 0)
                span.length = m_lexer(line.content.tail(span.col - 1)).getLength();
            line.spans.add({span.col, span.length, span.label, span.isPrimary});
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

        Int64 cursor = 1;
        for (const auto& span : line.spans)
        {
            Int64 start = span.column - indent;
            if (start > cursor)
                ss << content.subString(cursor - 1, start - cursor);

            TerminalColor c = span.isPrimary ? TerminalColor::Red : TerminalColor::Cyan;
            ss << color(c, String(content.subString(std::max(Int64{0}, start - 1), span.length)));
            cursor = start + span.length;
        }
        if (cursor - 1 < content.getLength())
            ss << content.tail(cursor - 1);
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
                   << color(TerminalColor::Bold, label) << " "
                   << color(TerminalColor::Blue, m_glyphs.vertical) << " ";
                renderSourceLine(ss, line, section.commonIndent);
                ss << "\n";

                auto rows = buildAnnotationRows(line, section.commonIndent);
                for (const auto& row : rows)
                    ss << repeat(' ', section.maxGutterWidth + 1)
                       << color(TerminalColor::Blue, m_glyphs.vertical) << " " << row << "\n";
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

        HumaneSourceLoc humaneLoc = m_sourceManager->getHumaneLoc(diag.primarySpan.range.begin);
        layout.primaryLoc.fileName = humaneLoc.pathInfo.foundPath;
        layout.primaryLoc.line = humaneLoc.line;
        layout.primaryLoc.col = humaneLoc.column;

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
            HumaneSourceLoc noteHumane = m_sourceManager->getHumaneLoc(note.span.range.begin);
            noteEntry.loc.fileName = noteHumane.pathInfo.foundPath;
            noteEntry.loc.line = noteHumane.line;
            noteEntry.loc.col = noteHumane.column;

            List<LayoutSpan> noteSpans;
            noteSpans.add(makeLayoutSpan(note.span, false));
            noteEntry.section = buildSectionLayout(noteSpans);
            noteEntry.loc.gutterIndent = noteEntry.section.maxGutterWidth;
            layout.notes.add(std::move(noteEntry));
        }
        return layout;
    }

    LayoutSpan makeLayoutSpan(const DiagnosticSpan& span, bool isPrimary)
    {
        HumaneSourceLoc humane = m_sourceManager->getHumaneLoc(span.range.begin);
        return {
            humane.line,
            humane.column,
            span.range.begin == span.range.end ? -1 : span.range.getOffset(span.range.end),
            span.message,
            isPrimary,
            span.range.begin};
    }

    String renderFromLayout(const DiagnosticLayout& layout)
    {
        StringBuilder ss;
        TerminalColor sevColor = (layout.header.severityValue >= Severity::Error)
                                     ? TerminalColor::Red
                                     : TerminalColor::Yellow;
        ss << color(sevColor, layout.header.severity);
        String codeStr = String(layout.header.code);
        while (codeStr.getLength() < 4)
            codeStr = "0" + codeStr;
        ss << "[E" << codeStr << "]"
           << ": " << color(TerminalColor::Bold, layout.header.message) << "\n";
        ss << repeat(' ', layout.primaryLoc.gutterIndent)
           << color(TerminalColor::Blue, m_glyphs.arrow) << " " << layout.primaryLoc.fileName << ":"
           << layout.primaryLoc.line << ":" << layout.primaryLoc.col << "\n";

        if (layout.primarySection.blocks.getCount() > 0)
        {
            ss << repeat(' ', layout.primarySection.maxGutterWidth + 1)
               << color(TerminalColor::Blue, m_glyphs.vertical) << "\n";
            renderSectionBody(ss, layout.primarySection);
        }
        for (const auto& note : layout.notes)
        {
            ss << "\n" << color(TerminalColor::Cyan, "note") << ": " << note.message << "\n";
            ss << repeat(' ', note.loc.gutterIndent)
               << color(TerminalColor::Blue, m_glyphs.noteDash) << " " << note.loc.fileName << ":"
               << note.loc.line << ":" << note.loc.col << "\n";
            if (note.section.blocks.getCount() > 0)
            {
                ss << repeat(' ', note.section.maxGutterWidth + 1)
                   << color(TerminalColor::Blue, m_glyphs.vertical) << "\n";
                renderSectionBody(ss, note.section);
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

} // namespace Slang
