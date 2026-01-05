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

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

String repeat(char c, Int64 n)
{
    if (n <= 0)
        return String();
    StringBuilder sb;
    for (Int64 i = 0; i < n; i++)
        sb << c;
    return sb;
}

String stripIndent(const String& text, Int64 indent)
{
    if (indent == 0 || text.getLength() == 0)
        return text;
    String str = text;
    Int64 usable = std::min(indent, Int64{str.getLength()});
    return str.subString(Index(usable), str.getLength() - usable);
}

// ============================================================================
// DIAGNOSTIC LAYOUT & RENDERER (Uses SourceManager API only)
// ============================================================================

struct LayoutSpan
{
    Int64 line;
    Int64 col;
    Int64 length;
    String label;
    bool isPrimary;
    Slang::SourceLoc startLoc;
};

struct LineHighlight
{
    Int64 column = 0;
    Int64 length = 1;
    String label;
    bool isPrimary = false;
};

struct HighlightedLine
{
    Int64 number = 0;
    UnownedStringSlice content;
    List<LineHighlight> spans;
};

struct LayoutBlock
{
    bool showGap = false;
    List<HighlightedLine> lines;
};

struct SectionLayout
{
    Int64 maxGutterWidth = 0;
    size_t commonIndent = 0;
    List<LayoutBlock> blocks;
};

struct DiagnosticLayout
{
    struct Header
    {
        String severity;
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

Count getLexedLength(
    DiagnosticSink::SourceLocationLexer sll,
    const UnownedStringSlice& line,
    Int64 col)
{
    if (sll)
    {
        return sll(line.tail(col - 1)).getLength();
    }
    return -1;
}

LayoutSpan makeLayoutSpan(SourceManager* sm, const DiagnosticSpan& span, bool isPrimary)
{
    LayoutSpan layoutSpan;

    // Use SourceManager to get humane (display) location
    HumaneSourceLoc humane = sm->getHumaneLoc(span.loc);

    layoutSpan.line = humane.line;
    layoutSpan.col = humane.column;
    layoutSpan.length =
        -1; // Currently we have to use the SourceLocationLexer later to get the size after the fact
    layoutSpan.label = span.message;
    layoutSpan.isPrimary = isPrimary;
    layoutSpan.startLoc = span.loc;

    return layoutSpan;
}

size_t findCommonIndent(const List<LayoutBlock>& blocks)
{
    Index minIndent = std::numeric_limits<Index>::max();
    for (const auto& block : blocks)
    {
        for (const auto& line : block.lines)
        {
            if (line.content.getLength() == 0)
                continue;
            Index indent = 0;
            while (indent < line.content.getLength() &&
                   (line.content[indent] == ' ' || line.content[indent] == '\t'))
                ++indent;
            if (indent < minIndent)
                minIndent = indent;
        }
    }
    return (minIndent == std::numeric_limits<Index>::max()) ? 0 : minIndent;
}

SectionLayout buildSectionLayout(
    DiagnosticSink::SourceLocationLexer sll,
    SourceManager* sm,
    List<LayoutSpan>& spans)
{
    SectionLayout section;
    if (spans.getCount() == 0)
        return section;

    Int64 maxLineNum = 0;
    for (const auto& span : spans)
        maxLineNum = std::max(maxLineNum, span.line);
    section.maxGutterWidth = Int64(std::to_string(std::max(Int64{1}, maxLineNum)).length());

    Dictionary<Int64, HighlightedLine> grouped;
    for (auto& span : spans)
    {
        HighlightedLine& line = grouped[span.line];
        line.number = span.line;

        // Retrieve content if not already set
        if (line.content.getLength() == 0)
        {
            SourceView* view = sm->findSourceView(span.startLoc);
            if (view)
            {
                SourceFile* file = view->getSourceFile();
                // HumaneLoc line is 1-based, getLineAtIndex is 0-based
                line.content = StringUtil::trimEndOfLine(file->getLineAtIndex(span.line - 1));
            }
        }

        if (sll && span.length == -1)
        {
            span.length = getLexedLength(sll, line.content, span.col);
        }

        line.spans.add(LineHighlight{span.col, span.length, span.label, span.isPrimary});
    }

    for (auto& [_, line] : grouped)
    {
        line.spans.sort(
            [](const LineHighlight& a, const LineHighlight& b)
            {
                if (a.column != b.column)
                    return a.column < b.column;
                return a.length < b.length;
            });
    }

    List<Int64> lineNumbers;
    for (const auto& [number, _] : grouped)
        lineNumbers.add(number);
    lineNumbers.sort();

    LayoutBlock currentBlock;
    Int64 prevLine = std::numeric_limits<Int64>::min();
    for (Int64 number : lineNumbers)
    {
        bool hasGap = prevLine != std::numeric_limits<Int64>::min() && number > prevLine + 1;
        if (hasGap && currentBlock.lines.getCount() > 0)
        {
            section.blocks.add(currentBlock);
            currentBlock = LayoutBlock{};
            currentBlock.showGap = true;
        }
        else if (currentBlock.lines.getCount() == 0)
        {
            currentBlock.showGap = false;
        }

        currentBlock.lines.add(grouped[number]);
        prevLine = number;
    }

    if (currentBlock.lines.getCount() > 0)
        section.blocks.add(currentBlock);

    section.commonIndent = findCommonIndent(section.blocks);
    return section;
}

// ============================================================================
// RENDERING STRINGS
// ============================================================================

struct LabelInfo
{
    Int64 column = 0;
    String text;
};

List<String> buildAnnotationRows(const HighlightedLine& line, Int64 indentShift)
{
    List<String> rows;
    if (line.spans.getCount() == 0)
        return rows;

    List<LabelInfo> labels;
    String underline;
    Int64 cursor = 1;

    for (const auto& span : line.spans)
    {
        Int64 effectiveColumn = std::max(Int64{1}, span.column - indentShift);
        Int64 length = std::max(Int64{1}, span.length);
        Int64 spaces = std::max(Int64{0}, effectiveColumn - cursor);
        underline = underline + repeat(' ', spaces);
        underline = underline + repeat(span.isPrimary ? '^' : '-', length);
        cursor = effectiveColumn + length;

        if (span.label.getLength() > 0)
            labels.add(LabelInfo{effectiveColumn, span.label});
    }

    if (labels.getCount() > 0)
    {
        labels.sort([](const LabelInfo& a, const LabelInfo& b) { return a.column > b.column; });
        underline = underline + " " + String(labels.getFirst().text);
        labels.removeAt(0);
    }

    rows.add(underline);
    if (labels.getCount() == 0)
        return rows;

    auto sortedLabels = labels;
    sortedLabels.sort([](const LabelInfo& a, const LabelInfo& b) { return a.column < b.column; });

    String connector;
    Int64 pos = 1;
    for (const auto& info : sortedLabels)
    {
        Int64 spaces = std::max(Int64{0}, info.column - pos);
        connector = connector + repeat(' ', spaces) + "|";
        pos = info.column + 1;
    }
    rows.add(connector);

    for (const auto& target : labels)
    {
        List<LabelInfo> active;
        for (const auto& candidate : labels)
            if (candidate.column <= target.column)
                active.add(candidate);

        active.sort([](const LabelInfo& a, const LabelInfo& b) { return a.column < b.column; });

        String labelRow;
        Int64 current = 1;
        for (const auto& info : active)
        {
            Int64 spaces = std::max(Int64{0}, info.column - current);
            labelRow = labelRow + repeat(' ', spaces);
            if (info.column == target.column)
            {
                labelRow = labelRow + String(info.text);
                current = info.column + info.text.getLength();
            }
            else
            {
                labelRow = labelRow + "|";
                current = info.column + 1;
            }
        }
        rows.add(labelRow);
    }

    return rows;
}

void printAnnotationRow(StringBuilder& ss, Int64 gutterWidth, const String& content)
{
    if (content.getLength() == 0)
        return;
    ss << repeat(' ', gutterWidth + 1) << "| " << content << '\n';
}

void renderSectionBody(StringBuilder& ss, const SectionLayout& section)
{
    for (const auto& block : section.blocks)
    {
        if (block.showGap)
            ss << "...\n";

        for (const auto& line : block.lines)
        {
            const String label =
                line.number >= 0 ? String(std::to_string(line.number).c_str()) : "?";
            Int64 padding = std::max(Int64{0}, section.maxGutterWidth - label.getLength());
            ss << repeat(' ', padding) << label << " | "
               << stripIndent(line.content, section.commonIndent) << '\n';

            for (const auto& row : buildAnnotationRows(line, section.commonIndent))
                printAnnotationRow(ss, section.maxGutterWidth, row);
        }
    }
}

DiagnosticLayout createLayout(
    DiagnosticSink::SourceLocationLexer sll,
    SourceManager* sm,
    const GenericDiagnostic& diag)
{
    DiagnosticLayout layout;

    layout.header.severity = getSeverityName(diag.severity);
    layout.header.code = diag.code;
    layout.header.message = diag.message;

    // Use SourceManager to get humane info for the primary location
    HumaneSourceLoc humaneLoc = sm->getHumaneLoc(diag.primarySpan.loc);
    layout.primaryLoc.fileName = humaneLoc.pathInfo.foundPath;
    layout.primaryLoc.line = humaneLoc.line;
    layout.primaryLoc.col = humaneLoc.column;

    List<LayoutSpan> allSpans;
    allSpans.add(makeLayoutSpan(sm, diag.primarySpan, true));
    for (const auto& s : diag.secondarySpans)
        allSpans.add(makeLayoutSpan(sm, s, false));

    layout.primarySection = buildSectionLayout(sll, sm, allSpans);
    layout.primaryLoc.gutterIndent = layout.primarySection.maxGutterWidth;

    for (const auto& note : diag.notes)
    {
        DiagnosticLayout::NoteEntry noteEntry;
        noteEntry.message = note.message;

        HumaneSourceLoc noteHumane = sm->getHumaneLoc(note.span.loc);
        noteEntry.loc.fileName = noteHumane.pathInfo.foundPath;
        noteEntry.loc.line = noteHumane.line;
        noteEntry.loc.col = noteHumane.column;

        List<LayoutSpan> noteSpans;
        noteSpans.add(makeLayoutSpan(sm, note.span, false));
        noteEntry.section = buildSectionLayout(sll, sm, noteSpans);
        noteEntry.loc.gutterIndent = noteEntry.section.maxGutterWidth;

        layout.notes.add(std::move(noteEntry));
    }

    return layout;
}

String renderFromLayout(const DiagnosticLayout& layout)
{
    StringBuilder ss;

    ss << layout.header.severity << "[E";
    String codeStr = String(layout.header.code);
    while (codeStr.getLength() < 4)
        codeStr = "0" + codeStr;
    ss << codeStr << "]: " << layout.header.message << '\n';

    ss << repeat(' ', layout.primaryLoc.gutterIndent) << "--> " << layout.primaryLoc.fileName << ":"
       << layout.primaryLoc.line << ":" << layout.primaryLoc.col << '\n';

    if (layout.primarySection.blocks.getCount() > 0)
    {
        ss << repeat(' ', layout.primarySection.maxGutterWidth + 1) << "|\n";
        renderSectionBody(ss, layout.primarySection);
    }

    for (const auto& note : layout.notes)
    {
        ss << "\nnote: " << note.message << '\n';
        ss << repeat(' ', note.loc.gutterIndent) << "--- " << note.loc.fileName << ":"
           << note.loc.line << ":" << note.loc.col << '\n';
        if (note.section.blocks.getCount() > 0)
        {
            ss << repeat(' ', note.section.maxGutterWidth + 1) << "|\n";
            renderSectionBody(ss, note.section);
        }
    }

    ss << '\n';

    return ss;
}

} // namespace

// ============================================================================
// PUBLIC API IMPLEMENTATION
// ============================================================================

String renderDiagnostic(
    DiagnosticSink::SourceLocationLexer sll,
    SourceManager* sm,
    const GenericDiagnostic& diag)
{
    DiagnosticLayout layout = createLayout(sll, sm, diag);
    return renderFromLayout(layout);
}

// ============================================================================
// UNIT TEST FUNCTIONALITY
// ============================================================================

#ifdef SLANG_ENABLE_DIAGNOSTIC_RENDER_UNIT_TESTS

namespace
{

// ----------------------------------------------------------------------------
// TEST DATA DEFINITIONS (INPUT)
// These structures match the user-provided test cases exactly.
// ----------------------------------------------------------------------------

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

// ============================================================================
// TEST DATA
// ============================================================================

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
  | ----- defined as `string` here
...
8 |     float result = 5.0 + data.name;
  |                    --- ^ --------- expected `float`
  |                    |   |
  |                    |   `+` cannot be applied to these types
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
             spans.add({SourceLoc("example.slang", 2, 5), "defined as `string` here", 5});
             spans.add({SourceLoc("example.slang", 8, 24), "float", 3});
             spans.add({SourceLoc("example.slang", 8, 30), "expected `float`", 9});
             return spans;
         }(),
         .notes = List<TestDiagnosticNote>()}}};

const size_t NUM_TESTS = sizeof(testCases) / sizeof(testCases[0]);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

String trimNewlines(const String& str)
{
    // Harness output comparisons assume deterministic framing, so we aggressively
    // trim leading/trailing newline noise that often shows up in raw heredoc data.
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
    if (col < 1 || col > Int64{line.getLength()))
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

// ============================================================================
// CONVERSION: TestData -> RenderDiagnostic
// ============================================================================

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
        Int64 lineIndex = std::max(0, inputSpan.location.line - 1);

        // Resolve length if not provided (legacy fallback behavior)
        Int64 length = inputSpan.length;
        if (length <= 0)
        {
            // Fetch line content to calculate fallback length
            UnownedStringSlice lineContent = file->getLineAtIndex(lineIndex);
            length = calculateFallbackLength(lineContent, inputSpan.location.column);
        }

        Slang::SourceLoc begin = calcSourceLoc(view, lineIndex, inputSpan.location.column);
        Slang::SourceLoc end = begin;
        if (length > 1)
            end = begin + Int64(length - 1);

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
        // Note: In a real compiler, SourceViews are managed; here we create one for the test case
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

// ============================================================================
// MAIN HARNESS
// ============================================================================

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

Int64 slangRichDiagnosticsUnitTest(Int64 argc, char* argv[])
{
    Int64 maxTests = -1;
    for (Int64 i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--until") == 0 && i + 1 < argc)
        {
            maxTests = std::atoi(argv[i + 1]);
            ++i;
        }
    }

    if (maxTests == 0)
    {
        std::cout << "Test harness initialized with " << NUM_TESTS << " test cases.\n";
        return 0;
    }

    Int64 testLimit = (maxTests == -1) ? Int64
    {NUM_TESTS)
                                       : std::min(maxTests, Int64{NUM_TESTS));

            std::cout << "Running " << testLimit << " test(s)...\n";

            Int64 passed = 0;
            Int64 failed = 0;

            SourceManager sm;
            sm.initialize(nullptr, nullptr);
            DiagnosticSink sink;

            for (Int64 i = 0; i < testLimit; ++i)
            {
                TestData& test = testCases[i];

                // Reset source manager for each test to keep IDs deterministic if needed,
                // or just to simulate fresh environment. For this harness, we reuse one sm
                // but it doesn't matter much as long as SourceLocs are valid.

                std::cout << "\nTest " << (i + 1) << ": " << test.name << '\n';

                // 1. Convert Input Data -> System Types
                // This simulates how the compiler would already have these structures ready.
                GenericDiagnostic renderDiag = createRenderDiagnostic(sm, test);

                // 2. Render using ONLY the system types and SourceManager
                // The renderer has no access to 'test.sourceContent' or 'test.diagnostic' structs.
                String actualOutput = trimNewlines(renderDiagnostic(&sm, renderDiag));
                String expectedOutput = trimNewlines(test.expectedOutput);

                if (actualOutput == expectedOutput)
                {
                    std::cout << "PASS\n";
                    ++passed;
                }
                else
                {
                    std::cout << "FAIL - Output mismatch\nRunning diff...\n";
                    runDiff(
                        std::string(expectedOutput.getBuffer()),
                        std::string(actualOutput.getBuffer()));
                    ++failed;
                }
            }

            std::cout << "\nResults: " << passed << " passed, " << failed << " failed\n";
            return failed > 0 ? 1 : 0;
}

#endif // SLANG_ENABLE_DIAGNOSTIC_RENDER_UNIT_TESTS

    } // namespace Slang
