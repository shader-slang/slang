#include "../core/slang-dictionary.h"
#include "../core/slang-list.h"
#include "../core/slang-string-util.h"
#include "../core/slang-string.h"
#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-source-loc.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <utility>

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

namespace Slang
{
namespace
{

// ----------------------------------------------------------------------------
// TEST DATA DEFINITIONS (INPUT)
// These structures match the user-provided test cases exactly.
// ----------------------------------------------------------------------------

struct SourceLoc
{
    String fileName;
    int line;
    int column;

    SourceLoc()
        : line(0), column(0)
    {
    }
    SourceLoc(const String& file, int ln, int col)
        : fileName(file), line(ln), column(col)
    {
    }
};

struct DiagnosticSpan
{
    SourceLoc location;
    String message;
    int length;

    DiagnosticSpan()
        : length(0)
    {
    }
    DiagnosticSpan(const SourceLoc& loc, const String& msg, int len = 0)
        : location(loc), message(msg), length(len)
    {
    }
};

struct DiagnosticNote
{
    String message;
    DiagnosticSpan span;
};

struct GenericDiagnostic
{
    int code;
    String severity;
    String message;
    DiagnosticSpan primarySpan;
    List<DiagnosticSpan> secondarySpans;
    List<DiagnosticNote> notes;
};

struct TestData
{
    const char* name;
    const char* sourceFileName;
    const char* sourceContent;
    const char* expectedOutput;
    GenericDiagnostic diagnostic;
    SourceFile* sourceFile = nullptr;
};

// ----------------------------------------------------------------------------
// RENDERER DATA DEFINITIONS (NATIVE)
// These structures are what the renderer consumes, using real SourceLoc/Range.
// ----------------------------------------------------------------------------

struct RenderSpan
{
    Slang::SourceRange range;
    String message;
};

struct RenderNote
{
    String message;
    RenderSpan span;
};

struct RenderDiagnostic
{
    int code;
    String severity;
    String message;
    RenderSpan primarySpan;
    List<RenderSpan> secondarySpans;
    List<RenderNote> notes;
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
          .severity = "error",
          .message = "use of undeclared identifier 'someSampler'",
          .primarySpan = {SourceLoc("example.slang", 8, 30), "not found in this scope", 11},
          .secondarySpans = List<DiagnosticSpan>(),
          .notes = List<DiagnosticNote>()}},

    {.name = "type_mismatch_with_secondary",
     .sourceFileName = "example.slang",
     .sourceContent = R"(struct VertexInput {
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR;
    int invalid_field; // This field has issues
};

struct PixelShader {
    float4 main(VertexInput input) : SV_Target {
        return color * 2.0 + input.invalid_field; // Type mismatch
    }
}
)",
     .expectedOutput = R"(error[E1002]: cannot add `float4` and `int`
  --> example.slang:10:28
   |
 5 | int invalid_field; // This field has issues
   | ----------------- field declared here
...
10 |     return color * 2.0 + input.invalid_field; // Type mismatch
   |            ----------- ^ ------------------- int
   |            |           |
   |            |           no implementation for `float4 + int`
   |            float4
)",
     .diagnostic =
         {.code = 1002,
          .severity = "error",
          .message = "cannot add `float4` and `int`",
          .primarySpan =
              {SourceLoc("example.slang", 10, 28), "no implementation for `float4 + int`", 1},
          .secondarySpans =
              []()
          {
              List<DiagnosticSpan> spans;
              spans.add({SourceLoc("example.slang", 5, 5), "field declared here", 17});
              spans.add({SourceLoc("example.slang", 10, 16), "float4", 11});
              spans.add({SourceLoc("example.slang", 10, 30), "int", 19});
              return spans;
          }(),
          .notes = List<DiagnosticNote>()}},

    {.name = "undeclared_variable",
     .sourceFileName = "example.slang",
     .sourceContent = R"(struct VertexInput {
    float4 position : POSITION;
};

struct PixelShader {
    float4 main(VertexInput input) : SV_Target {
        int x = undefinedVariable; // Undefined variable
        return float4(x, 0, 0, 1);
    }
}
)",
     .expectedOutput = R"(
error[E1003]: use of undeclared identifier 'undefinedVariable'
 --> example.slang:7:17
  |
7 | int x = undefinedVariable; // Undefined variable
  |         ^^^^^^^^^^^^^^^^^ not found in this scope
)",
     .diagnostic =
         {.code = 1003,
          .severity = "error",
          .message = "use of undeclared identifier 'undefinedVariable'",
          .primarySpan = {SourceLoc("example.slang", 7, 17), "not found in this scope", 17},
          .secondarySpans = List<DiagnosticSpan>(),
          .notes = List<DiagnosticNote>()}},

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
          .severity = "error",
          .message = "mismatched types",
          .primarySpan = {SourceLoc("example.slang", 7, 19), "expected `float`, found `&str`", 8},
          .secondarySpans =
              []()
          {
              List<DiagnosticSpan> spans;
              spans.add({SourceLoc("example.slang", 7, 9), "expected due to this type", 5});
              return spans;
          }(),
          .notes = List<DiagnosticNote>()}},

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
         .severity = "warning",
         .message = "potential division by zero",
         .primarySpan = {SourceLoc("math.slang", 3, 14), "division by zero if `len` is 0.0", 1},
         .secondarySpans =
             []()
         {
             List<DiagnosticSpan> spans;
             spans.add({SourceLoc("math.slang", 2, 17), "length computed here", 15});
             return spans;
         }(),
         .notes =
             []()
         {
             List<DiagnosticNote> notes;
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
         .severity = "error",
         .message = "mismatched types",
         .primarySpan =
             {SourceLoc("example.slang", 8, 28), "`+` cannot be applied to these types", 1},
         .secondarySpans =
             []()
         {
             List<DiagnosticSpan> spans;
             spans.add({SourceLoc("example.slang", 2, 5), "defined as `string` here", 5});
             spans.add({SourceLoc("example.slang", 8, 24), "float", 3});
             spans.add({SourceLoc("example.slang", 8, 30), "expected `float`", 9});
             return spans;
         }(),
         .notes = List<DiagnosticNote>()}}};

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

String repeat(char c, int n)
{
    if (n <= 0)
        return String();
    StringBuilder sb;
    for (int i = 0; i < n; i++)
        sb << c;
    return sb;
}

int calculateFallbackLength(const UnownedStringSlice& line, int col)
{
    if (col < 1 || col > static_cast<int>(line.getLength()))
        return 1;
    int start = col - 1;
    int len = 0;
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

String stripIndent(const String& text, size_t indent)
{
    if (indent == 0 || text.getLength() == 0)
        return text;
    String str = text;
    Index usable = std::min(static_cast<Index>(indent), str.getLength());
    return str.subString(usable, str.getLength() - usable);
}

// ============================================================================
// CONVERSION: TestData -> RenderDiagnostic
// ============================================================================

Slang::SourceLoc calcSourceLoc(SourceView* view, int lineIndex, int columnOneBased)
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
    return base + Int(lineStart + columnOffset);
}

RenderSpan createRenderSpan(SourceView* view, const DiagnosticSpan& inputSpan)
{
    RenderSpan outSpan;
    outSpan.message = inputSpan.message;

    if (!view)
        return outSpan;

    SourceFile* file = view->getSourceFile();

    // Convert 1-based line to 0-based index
    int lineIndex = std::max(0, inputSpan.location.line - 1);

    // Resolve length if not provided (legacy fallback behavior)
    int length = inputSpan.length;
    if (length <= 0)
    {
        // Fetch line content to calculate fallback length
        UnownedStringSlice lineContent = file->getLineAtIndex(lineIndex);
        length = calculateFallbackLength(lineContent, inputSpan.location.column);
    }

    Slang::SourceLoc begin = calcSourceLoc(view, lineIndex, inputSpan.location.column);
    Slang::SourceLoc end = begin;
    if (length > 1)
        end = begin + Int(length - 1);

    outSpan.range = Slang::SourceRange(begin, end);
    return outSpan;
}

RenderDiagnostic createRenderDiagnostic(SourceManager& sm, TestData& test)
{
    RenderDiagnostic outDiag;
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
        RenderNote outNote;
        outNote.message = note.message;
        outNote.span = createRenderSpan(view, note.span);
        outDiag.notes.add(outNote);
    }

    return outDiag;
}

// ============================================================================
// DIAGNOSTIC LAYOUT & RENDERER (Uses SourceManager API only)
// ============================================================================

struct LayoutSpan
{
    int line;
    int col;
    int length;
    String label;
    bool isPrimary;
    Slang::SourceLoc startLoc; // Keep for finding source file later
};

struct LineHighlight
{
    int column = 0;
    int length = 1;
    String label;
    bool isPrimary = false;
};

struct HighlightedLine
{
    int number = 0;
    String content;
    List<LineHighlight> spans;
};

struct LayoutBlock
{
    bool showGap = false;
    List<HighlightedLine> lines;
};

struct SectionLayout
{
    int maxGutterWidth = 0;
    size_t commonIndent = 0;
    List<LayoutBlock> blocks;
};

struct DiagnosticLayout
{
    struct Header
    {
        String severity;
        int code = 0;
        String message;
    } header;

    struct Location
    {
        String fileName;
        int line = 0;
        int col = 0;
        int gutterIndent = 0;
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

LayoutSpan makeLayoutSpan(SourceManager& sm, const RenderSpan& span, bool isPrimary)
{
    LayoutSpan layoutSpan;

    // Use SourceManager to get humane (display) location
    HumaneSourceLoc humane = sm.getHumaneLoc(span.range.begin);

    layoutSpan.line = static_cast<int>(humane.line);
    layoutSpan.col = static_cast<int>(humane.column);
    layoutSpan.length = static_cast<int>(span.range.getSize() + 1); // +1 because inclusive range
    layoutSpan.label = span.message;
    layoutSpan.isPrimary = isPrimary;
    layoutSpan.startLoc = span.range.begin;

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

SectionLayout buildSectionLayout(SourceManager& sm, const List<LayoutSpan>& spans)
{
    SectionLayout section;
    if (spans.getCount() == 0)
        return section;

    int maxLineNum = 0;
    for (const auto& span : spans)
        maxLineNum = std::max(maxLineNum, span.line);
    section.maxGutterWidth = static_cast<int>(std::to_string(std::max(1, maxLineNum)).length());

    Dictionary<int, HighlightedLine> grouped;
    for (const auto& span : spans)
    {
        HighlightedLine& line = grouped[span.line];
        line.number = span.line;

        // Retrieve content if not already set
        if (line.content.getLength() == 0)
        {
            SourceView* view = sm.findSourceView(span.startLoc);
            if (view)
            {
                SourceFile* file = view->getSourceFile();
                // HumaneLoc line is 1-based, getLineAtIndex is 0-based
                line.content = StringUtil::trimEndOfLine(file->getLineAtIndex(span.line - 1));
            }
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

    List<int> lineNumbers;
    for (const auto& [number, _] : grouped)
        lineNumbers.add(number);
    lineNumbers.sort();

    LayoutBlock currentBlock;
    int prevLine = std::numeric_limits<int>::min();
    for (int number : lineNumbers)
    {
        bool hasGap = prevLine != std::numeric_limits<int>::min() && number > prevLine + 1;
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
    int column = 0;
    String text;
};

List<String> buildAnnotationRows(const HighlightedLine& line, size_t indentShift)
{
    List<String> rows;
    if (line.spans.getCount() == 0)
        return rows;

    List<LabelInfo> labels;
    String underline;
    int cursor = 1;

    for (const auto& span : line.spans)
    {
        int effectiveColumn = std::max(1, span.column - static_cast<int>(indentShift));
        int length = std::max(1, span.length);
        int spaces = std::max(0, effectiveColumn - cursor);
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
    int pos = 1;
    for (const auto& info : sortedLabels)
    {
        int spaces = std::max(0, info.column - pos);
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
        int current = 1;
        for (const auto& info : active)
        {
            int spaces = std::max(0, info.column - current);
            labelRow = labelRow + repeat(' ', spaces);
            if (info.column == target.column)
            {
                labelRow = labelRow + String(info.text);
                current = info.column + static_cast<int>(info.text.getLength());
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

void printAnnotationRow(StringBuilder& ss, int gutterWidth, const String& content)
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
            int padding = std::max(0, section.maxGutterWidth - static_cast<int>(label.getLength()));
            ss << repeat(' ', padding) << label << " | "
               << stripIndent(line.content, section.commonIndent) << '\n';

            for (const auto& row : buildAnnotationRows(line, section.commonIndent))
                printAnnotationRow(ss, section.maxGutterWidth, row);
        }
    }
}

DiagnosticLayout createLayout(SourceManager& sm, const RenderDiagnostic& diag)
{
    DiagnosticLayout layout;

    layout.header.severity = diag.severity;
    layout.header.code = diag.code;
    layout.header.message = diag.message;

    // Use SourceManager to get humane info for the primary location
    HumaneSourceLoc humaneLoc = sm.getHumaneLoc(diag.primarySpan.range.begin);
    layout.primaryLoc.fileName = humaneLoc.pathInfo.foundPath;
    layout.primaryLoc.line = static_cast<int>(humaneLoc.line);
    layout.primaryLoc.col = static_cast<int>(humaneLoc.column);

    List<LayoutSpan> allSpans;
    allSpans.add(makeLayoutSpan(sm, diag.primarySpan, true));
    for (const auto& s : diag.secondarySpans)
        allSpans.add(makeLayoutSpan(sm, s, false));

    layout.primarySection = buildSectionLayout(sm, allSpans);
    layout.primaryLoc.gutterIndent = layout.primarySection.maxGutterWidth;

    for (const auto& note : diag.notes)
    {
        DiagnosticLayout::NoteEntry noteEntry;
        noteEntry.message = note.message;

        HumaneSourceLoc noteHumane = sm.getHumaneLoc(note.span.range.begin);
        noteEntry.loc.fileName = noteHumane.pathInfo.foundPath;
        noteEntry.loc.line = static_cast<int>(noteHumane.line);
        noteEntry.loc.col = static_cast<int>(noteHumane.column);

        List<LayoutSpan> noteSpans;
        noteSpans.add(makeLayoutSpan(sm, note.span, false));
        noteEntry.section = buildSectionLayout(sm, noteSpans);
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

    return ss;
}

String renderDiagnostic(SourceManager& sm, const RenderDiagnostic& diag)
{
    DiagnosticLayout layout = createLayout(sm, diag);
    return renderFromLayout(layout);
}

// ============================================================================
// MAIN HARNESS
// ============================================================================

void writeTempFile(const std::string& path, const std::string& content)
{
    std::ofstream file(path);
    file << content << '\n';
}

int runDiff(const std::string& expected, const std::string& actual)
{
    writeTempFile("expected.tmp", expected);
    writeTempFile("actual.tmp", actual);
    int result = std::system("diff -u expected.tmp actual.tmp");
    std::remove("expected.tmp");
    std::remove("actual.tmp");
    return result;
}
} // namespace

int slangRichDiagnosticsUnitTest(int argc, char* argv[])
{
    int maxTests = -1;
    for (int i = 1; i < argc; ++i)
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

    int testLimit = (maxTests == -1) ? static_cast<int>(NUM_TESTS)
                                     : std::min(maxTests, static_cast<int>(NUM_TESTS));

    std::cout << "Running " << testLimit << " test(s)...\n";

    int passed = 0;
    int failed = 0;

    SourceManager sm;
    sm.initialize(nullptr, nullptr);
    DiagnosticSink sink;

    for (int i = 0; i < testLimit; ++i)
    {
        TestData& test = testCases[i];

        // Reset source manager for each test to keep IDs deterministic if needed,
        // or just to simulate fresh environment. For this harness, we reuse one sm
        // but it doesn't matter much as long as SourceLocs are valid.

        std::cout << "\nTest " << (i + 1) << ": " << test.name << '\n';

        // 1. Convert Input Data -> System Types
        // This simulates how the compiler would already have these structures ready.
        RenderDiagnostic renderDiag = createRenderDiagnostic(sm, test);

        // 2. Render using ONLY the system types and SourceManager
        // The renderer has no access to 'test.sourceContent' or 'test.diagnostic' structs.
        String actualOutput = trimNewlines(renderDiagnostic(sm, renderDiag));
        String expectedOutput = trimNewlines(test.expectedOutput);

        if (actualOutput == expectedOutput)
        {
            std::cout << "PASS\n";
            ++passed;
        }
        else
        {
            std::cout << "FAIL - Output mismatch\nRunning diff...\n";
            runDiff(std::string(expectedOutput.getBuffer()), std::string(actualOutput.getBuffer()));
            ++failed;
        }
    }

    std::cout << "\nResults: " << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}
} // namespace Slang
