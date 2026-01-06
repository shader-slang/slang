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

namespace Slang
{

//
// Tests to use while developing
//

#ifdef SLANG_ENABLE_DIAGNOSTIC_RENDER_UNIT_TESTS

namespace
{

//
// Some structures for describing a test case
//

struct SourceLoc
{
    String fileName;
    Int64 line;
    Int64 column;

    SourceLoc()
        : line(0), column(0)
    {
    }
    SourceLoc(const String& file, Int64 ln, Int64 col)
        : fileName(file), line(ln), column(col)
    {
    }
};

struct TestDiagnosticSpan
{
    SourceLoc location;
    String message;
    Int64 length;

    TestDiagnosticSpan()
        : length(0)
    {
    }
    TestDiagnosticSpan(const SourceLoc& loc, const String& msg, Int64 len = 0)
        : location(loc), message(msg), length(len)
    {
    }
};

struct TestDiagnosticNote
{
    String message;
    TestDiagnosticSpan span;
};

struct TestGenericDiagnostic
{
    Int64 code;
    Severity severity;
    String message;
    TestDiagnosticSpan primarySpan;
    List<TestDiagnosticSpan> secondarySpans;
    List<TestDiagnosticNote> notes;
};

struct TestData
{
    const char* name;
    const char* sourceFileName;
    const char* sourceContent;
    const char* expectedOutput;
    TestGenericDiagnostic diagnostic;
    SourceFile* sourceFile = nullptr;
};

//
//
//

TestData testCases[] = {
    {.name = "undeclared_identifier",
     .sourceFileName = "example.slang",
     .sourceContent = R"(struct VertexInput {
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct PixelShader {
    float4 main(VertexInput input) : SV_Target {
        float4 color = tex2D(someSampler, input.texCoord); // Undefined sampler
        return color;
    }
}
)",
     .expectedOutput = R"(
error[E1001]: use of undeclared identifier 'someSampler'
 --> example.slang:8:30
  |
8 | float4 color = tex2D(someSampler, input.texCoord); // Undefined sampler
  |                      ^^^^^^^^^^^ not found in this scope
)",
     .diagnostic =
         {.code = 1001,
          .severity = Severity::Error,
          .message = "use of undeclared identifier 'someSampler'",
          .primarySpan = {SourceLoc("example.slang", 8, 30), "not found in this scope", 11},
          .secondarySpans = List<TestDiagnosticSpan>(),
          .notes = List<TestDiagnosticNote>()}},

    {.name = "type_mismatch_with_secondary",
     .sourceFileName = "example.slang",
     .sourceContent = R"(struct VertexInput {
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR;
    Int64 invalid_field; // This field has issues
};

struct PixelShader {
    float4 main(VertexInput input) : SV_Target {
        return color * 2.0 + input.invalid_field; // Type mismatch
    }
}
)",
     .expectedOutput = R"(error[E1002]: cannot add `float4` and `Int64`
  --> example.slang:10:28
   |
 5 | Int64 invalid_field; // This field has issues
   | ----------------- field declared here
...
10 |     return color * 2.0 + input.invalid_field; // Type mismatch
   |            ----------- ^ ------------------- Int64
   |            |           |
   |            |           no implementation for `float4 + Int64`
   |            |
   |            float4
)",
     .diagnostic =
         {.code = 1002,
          .severity = Severity::Error,
          .message = "cannot add `float4` and `Int64`",
          .primarySpan =
              {SourceLoc("example.slang", 10, 28), "no implementation for `float4 + Int64`", 1},
          .secondarySpans =
              []()
          {
              List<TestDiagnosticSpan> spans;
              spans.add({SourceLoc("example.slang", 5, 5), "field declared here", 17});
              spans.add({SourceLoc("example.slang", 10, 16), "float4", 11});
              spans.add({SourceLoc("example.slang", 10, 30), "Int64", 19});
              return spans;
          }(),
          .notes = List<TestDiagnosticNote>()}},

    {.name = "undeclared_variable",
     .sourceFileName = "example.slang",
     .sourceContent = R"(struct VertexInput {
    float4 position : POSITION;
};

struct PixelShader {
    float4 main(VertexInput input) : SV_Target {
        Int64 x = undefinedVariable; // Undefined variable
        return float4(x, 0, 0, 1);
    }
}
)",
     .expectedOutput = R"(
error[E1003]: use of undeclared identifier 'undefinedVariable'
 --> example.slang:7:17
  |
7 | Int64 x = undefinedVariable; // Undefined variable
  |         ^^^^^^^^^^^^^^^^^ not found in this scope
)",
     .diagnostic =
         {.code = 1003,
          .severity = Severity::Error,
          .message = "use of undeclared identifier 'undefinedVariable'",
          .primarySpan = {SourceLoc("example.slang", 7, 17), "not found in this scope", 17},
          .secondarySpans = List<TestDiagnosticSpan>(),
          .notes = List<TestDiagnosticNote>()}},

    {.name = "wrong_type_assignment",
     .sourceFileName = "example.slang",
     .sourceContent = R"(struct VertexInput {
    float4 position : POSITION;
};

struct PixelShader {
    float4 main(VertexInput input) : SV_Target {
        float y = "string"; // Wrong type assignment
        return float4(y, 0, 0, 1);
    }
}
)",
     .expectedOutput = R"(
error[E1004]: mismatched types
 --> example.slang:7:19
  |
7 | float y = "string"; // Wrong type assignment
  | -----     ^^^^^^^^ expected `float`, found `&str`
  | |
  | expected due to this type
)",
     .diagnostic =
         {.code = 1004,
          .severity = Severity::Error,
          .message = "mismatched types",
          .primarySpan = {SourceLoc("example.slang", 7, 19), "expected `float`, found `&str`", 8},
          .secondarySpans =
              []()
          {
              List<TestDiagnosticSpan> spans;
              spans.add({SourceLoc("example.slang", 7, 9), "expected due to this type", 5});
              return spans;
          }(),
          .notes = List<TestDiagnosticNote>()}},

    {.name = "division_by_zero_warning",
     .sourceFileName = "math.slang",
     .sourceContent = R"(float3 normalize(float3 v) {
    float len = sqrt(dot(v, v));
    return v / len; // Potential division by zero
}

struct VertexShader {
    float4 main() : SV_Position {
        return float4(0, 0, 0, 1);
    }
}
)",
     .expectedOutput = R"(
warning[E2001]: potential division by zero
 --> math.slang:3:14
  |
2 | float len = sqrt(dot(v, v));
  |             --------------- length computed here
3 | return v / len; // Potential division by zero
  |          ^ division by zero if `len` is 0.0

note: consider using 'normalize' builtin function instead
 --- math.slang:1:8
  |
1 | float3 normalize(float3 v) {
  |        ---------
)",
     .diagnostic = {
         .code = 2001,
         .severity = Severity::Warning,
         .message = "potential division by zero",
         .primarySpan = {SourceLoc("math.slang", 3, 14), "division by zero if `len` is 0.0", 1},
         .secondarySpans =
             []()
         {
             List<TestDiagnosticSpan> spans;
             spans.add({SourceLoc("math.slang", 2, 17), "length computed here", 15});
             return spans;
         }(),
         .notes =
             []()
         {
             List<TestDiagnosticNote> notes;
             notes.add(
                 {.message = "consider using 'normalize' builtin function instead",
                  .span = {SourceLoc("math.slang", 1, 8), "", 9}});
             return notes;
         }()}},

    {.name = "mismatched_types_complex",
     .sourceFileName = "example.slang",
     .sourceContent = R"(struct Data {
    string name = "User";
};

struct PixelShader {
    float4 main() : SV_Target {
        Data data;
        float result = 5.0 + data.name;
        return float4(result, 0, 0, 1);
    }
}
)",
     .expectedOutput = R"(
error[E0308]: mismatched types
 --> example.slang:8:28
  |
2 | string name = "User";
  | ------ defined as `string` here
...
8 |     float result = 5.0 + data.name;
  |                    --- ^ --------- expected `float`
  |                    |   |
  |                    |   `+` cannot be applied to these types
  |                    |
  |                    float
)",
     .diagnostic = {
         .code = 308,
         .severity = Severity::Error,
         .message = "mismatched types",
         .primarySpan =
             {SourceLoc("example.slang", 8, 28), "`+` cannot be applied to these types", 1},
         .secondarySpans =
             []()
         {
             List<TestDiagnosticSpan> spans;
             spans.add({SourceLoc("example.slang", 2, 5), "defined as `string` here", 6});
             spans.add({SourceLoc("example.slang", 8, 24), "float", 3});
             spans.add({SourceLoc("example.slang", 8, 30), "expected `float`", 9});
             return spans;
         }(),
         .notes = List<TestDiagnosticNote>()}}};

const size_t NUM_TESTS = sizeof(testCases) / sizeof(testCases[0]);

//
//
//

String trimNewlines(const String& str)
{
    Index start = 0;
    while (start < str.getLength() && (str[start] == '\n' || str[start] == '\r'))
        ++start;
    Index end = str.getLength();
    while (end > start && (str[end - 1] == '\n' || str[end - 1] == '\r'))
        --end;
    return str.subString(start, end - start);
}

Int64 calculateFallbackLength(const UnownedStringSlice& line, Int64 col)
{
    if (col < 1 || col > Int64{line.getLength()})
        return 1;
    Int64 start = col - 1;
    Int64 len = 0;
    for (Index i = start; i < line.getLength(); ++i)
    {
        char ch = line[i];
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')
            ++len;
        else
            break;
    }
    return len > 0 ? len : 1;
}

//
// TestData -> RenderDiagnostic
//

Slang::SourceLoc calcSourceLoc(SourceView* view, Int64 lineIndex, Int64 columnOneBased)
{
    SLANG_ASSERT(view);
    SourceFile* file = view->getSourceFile();
    SourceFile::OffsetRange lineRange = file->getOffsetRangeAtLineIndex(Index(lineIndex));
    uint32_t lineStart = lineRange.isValid() ? lineRange.start : 0;
    uint32_t columnOffset = columnOneBased > 0 ? uint32_t(columnOneBased - 1) : 0;
    if (lineRange.isValid())
    {
        uint32_t lineLength = lineRange.getCount();
        if (columnOffset > lineLength)
            columnOffset = lineLength;
    }
    Slang::SourceLoc base = view->getRange().begin;
    return base + Int64(lineStart + columnOffset);
}

DiagnosticSpan createRenderSpan(SourceView* view, const TestDiagnosticSpan& inputSpan)
{
    DiagnosticSpan outSpan;
    outSpan.message = inputSpan.message;

    if (!view)
        return outSpan;

    SourceFile* file = view->getSourceFile();

    // Convert 1-based line to 0-based index
    Int64 lineIndex = std::max(Int64{0}, inputSpan.location.line - 1);

    // Resolve length if not provided (legacy fallback behavior)
    Int64 length = inputSpan.length;
    if (length <= 0)
    {
        // Fetch line content to calculate fallback length
        UnownedStringSlice lineContent = file->getLineAtIndex(lineIndex);
        length = calculateFallbackLength(lineContent, inputSpan.location.column);
    }

    Slang::SourceLoc begin = calcSourceLoc(view, lineIndex, inputSpan.location.column);
    Slang::SourceLoc end =
        calcSourceLoc(view, lineIndex, inputSpan.location.column + inputSpan.length);

    outSpan.range = Slang::SourceRange(begin, end);
    return outSpan;
}

GenericDiagnostic createRenderDiagnostic(SourceManager& sm, TestData& test)
{
    GenericDiagnostic outDiag;
    outDiag.code = test.diagnostic.code;
    outDiag.severity = test.diagnostic.severity;
    outDiag.message = test.diagnostic.message;

    // Create the source infrastructure if not already present
    if (!test.sourceFile)
    {
        PathInfo pathInfo = PathInfo::makePath(String(test.sourceFileName));
        test.sourceFile = sm.createSourceFileWithString(pathInfo, test.sourceContent);
    }

    // Create view for the file
    SourceView* view = sm.createSourceView(test.sourceFile, nullptr, Slang::SourceLoc());

    // Map spans
    outDiag.primarySpan = createRenderSpan(view, test.diagnostic.primarySpan);

    for (const auto& sec : test.diagnostic.secondarySpans)
    {
        outDiag.secondarySpans.add(createRenderSpan(view, sec));
    }

    for (const auto& note : test.diagnostic.notes)
    {
        DiagnosticNote outNote;
        outNote.message = note.message;
        outNote.span = createRenderSpan(view, note.span);
        outDiag.notes.add(outNote);
    }

    return outDiag;
}

//
//
//

void writeTempFile(const std::string& path, const std::string& content)
{
    std::ofstream file(path);
    file << content << '\n';
}

Int64 runDiff(const std::string& expected, const std::string& actual)
{
    writeTempFile("expected.tmp", expected);
    writeTempFile("actual.tmp", actual);
    Int64 result = std::system("diff -u expected.tmp actual.tmp");
    std::remove("expected.tmp");
    std::remove("actual.tmp");
    return result;
}

} // namespace

int slangRichDiagnosticsUnitTest(int argc, char* argv[])
{
    Int64 maxTests = -1;
    bool useColor = false;
    bool useUnicode = false;
    bool forceOutput = false;

    for (Int64 i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--until") == 0 && i + 1 < argc)
        {
            maxTests = std::atoi(argv[i + 1]);
            ++i;
        }
        else if (std::strcmp(argv[i], "--color") == 0)
        {
            useColor = true;
        }
        else if (std::strcmp(argv[i], "--unicode") == 0)
        {
            useUnicode = true;
        }
        else if (std::strcmp(argv[i], "--output") == 0)
        {
            forceOutput = true;
        }
    }

    if (maxTests == 0)
    {
        std::cout << "Test harness initialized with " << NUM_TESTS << " test cases.\n";
        return 0;
    }

    Int64 testLimit = (maxTests == -1) ? Int64{NUM_TESTS} : std::min(maxTests, Int64{NUM_TESTS});

    std::cout << "Running " << testLimit << " test(s)...\n";

    Int64 passed = 0;
    Int64 failed = 0;

    SourceManager sm;
    sm.initialize(nullptr, nullptr);
    DiagnosticSink sink;

    DiagnosticRenderOptions options;
    options.enableTerminalColors = useColor;
    options.enableUnicode = useUnicode;

    for (Int64 i = 0; i < testLimit; ++i)
    {
        TestData& test = testCases[i];

        std::cout << "\nTest " << (i + 1) << ": " << test.name << '\n';

        // 1. Convert Input Data -> System Types
        GenericDiagnostic renderDiag = createRenderDiagnostic(sm, test);

        // 2. Render using the provided options
        String actualOutput =
            renderDiagnostic(sink.getSourceLocationLexer(), &sm, options, renderDiag);

        if (forceOutput)
        {
            std::cout << "--- Render Output ---\n";
            std::cout << actualOutput.getBuffer();
            std::cout << "---------------------\n";
        }

        // If color or unicode is enabled, we don't compare against expected text because
        // the expected text doesn't contain ANSI escape sequences.
        if (useColor || useUnicode)
        {
            std::cout << "Visual Check (Color/Unicode Enabled)\n";
            ++passed;
            continue;
        }

        String actualTrimmed = trimNewlines(actualOutput);
        String expectedTrimmed = trimNewlines(test.expectedOutput);

        if (actualTrimmed == expectedTrimmed)
        {
            std::cout << "PASS\n";
            ++passed;
        }
        else
        {
            std::cout << "FAIL - Output mismatch\nRunning diff...\n";
            runDiff(
                std::string(expectedTrimmed.getBuffer()),
                std::string(actualTrimmed.getBuffer()));
            ++failed;
        }
    }

    std::cout << "\nResults: " << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}

#endif // SLANG_ENABLE_DIAGNOSTIC_RENDER_UNIT_TESTS

} // namespace Slang
