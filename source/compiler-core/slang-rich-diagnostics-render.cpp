#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../core/slang-list.h"
#include "../core/slang-string.h"
#include "../core/slang-dictionary.h"
#include "../core/slang-string-util.h"

using namespace Slang;

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

namespace
{
struct SourceLoc
{
    std::string fileName;
    int line;
    int column;

    SourceLoc()
        : line(0), column(0)
    {
    }
    SourceLoc(const std::string& file, int ln, int col)
        : fileName(file), line(ln), column(col)
    {
    }
};

struct DiagnosticSpan
{
    SourceLoc location;
    std::string message;
    int length;

    DiagnosticSpan()
        : length(0)
    {
    }
    DiagnosticSpan(const SourceLoc& loc, const std::string& msg, int len = 0)
        : location(loc), message(msg), length(len)
    {
    }
};

struct DiagnosticNote
{
    std::string message;
    DiagnosticSpan span;
};

struct GenericDiagnostic
{
    int code;
    std::string severity;
    std::string message;
    DiagnosticSpan primarySpan;
    List<DiagnosticSpan> secondarySpans;
    List<DiagnosticNote> notes;
};

struct TestData
{
    const char* name;
    const char* sourceFile;
    const char* sourceContent;
    const char* expectedOutput;
    GenericDiagnostic diagnostic;
};

// ============================================================================
// TEST DATA
// ============================================================================

TestData testCases[] = {
    {.name = "undeclared_identifier",
     .sourceFile = "example.slang",
     .sourceContent = R"(
struct VertexInput {
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
     .sourceFile = "example.slang",
     .sourceContent = R"(
struct VertexInput {
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
     .expectedOutput = R"(
error[E1002]: cannot add `float4` and `int`
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
          .secondarySpans = []() {
              List<DiagnosticSpan> spans;
              spans.add({SourceLoc("example.slang", 5, 5), "field declared here", 17});
              spans.add({SourceLoc("example.slang", 10, 16), "float4", 11});
              spans.add({SourceLoc("example.slang", 10, 30), "int", 19});
              return spans;
          }(),
          .notes = List<DiagnosticNote>()}},

    {.name = "undeclared_variable",
     .sourceFile = "example.slang",
     .sourceContent = R"(
struct VertexInput {
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
     .sourceFile = "example.slang",
     .sourceContent = R"(
struct VertexInput {
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
          .secondarySpans = []() {
              List<DiagnosticSpan> spans;
              spans.add({SourceLoc("example.slang", 7, 9), "expected due to this type", 5});
              return spans;
          }(),
          .notes = List<DiagnosticNote>()}},

    {.name = "division_by_zero_warning",
     .sourceFile = "math.slang",
     .sourceContent = R"(
float3 normalize(float3 v) {
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
     .diagnostic =
         {.code = 2001,
          .severity = "warning",
          .message = "potential division by zero",
          .primarySpan = {SourceLoc("math.slang", 3, 14), "division by zero if `len` is 0.0", 1},
          .secondarySpans = []() {
              List<DiagnosticSpan> spans;
              spans.add({SourceLoc("math.slang", 2, 17), "length computed here", 15});
              return spans;
          }(),
          .notes = []() {
              List<DiagnosticNote> notes;
              notes.add({.message = "consider using 'normalize' builtin function instead",
                        .span = {SourceLoc("math.slang", 1, 8), "", 9}});
              return notes;
          }()}},

    {.name = "mismatched_types_complex",
     .sourceFile = "example.slang",
     .sourceContent = R"(
struct Data {
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
         .secondarySpans = []() {
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

List<String> getSourceLines(const std::string& content)
{
    // Parse the embedded shader snippet into logical lines once so downstream layout
    // code can work with indexed access; using StringUtil::split handles both CRLF and LF.
    List<String> lines;
    UnownedStringSlice contentSlice(content.c_str(), content.length());
    List<UnownedStringSlice> slices;
    StringUtil::split(contentSlice, '\n', slices);
    
    // Convert slices to strings, handling potential CRLF endings
    for (const auto& slice : slices)
    {
        String line(slice);
        // Remove trailing \r if present (for CRLF line endings)
        if (line.getLength() > 0 && line[line.getLength() - 1] == '\r')
            line = line.subString(0, line.getLength() - 1);
        lines.add(line);
    }
    return lines;
}

String trimNewlines(const std::string& s)
{
    // Harness output comparisons assume deterministic framing, so we aggressively
    // trim leading/trailing newline noise that often shows up in raw heredoc data.
    String str(s.c_str());
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
    if (n <= 0) return String();
    StringBuilder sb;
    for (int i = 0; i < n; i++)
        sb << c;
    return sb;
}

int calculateFallbackLength(const std::string& line, int col)
{
    // When the diagnostic lacks an explicit length we infer one by walking the
    // identifier starting at the reported column; this keeps highlighting useful
    // without requiring every test case to spell out exact spans.
    if (col < 1 || col > static_cast<int>(line.length()))
        return 1;
    int start = col - 1;
    int len = 0;
    for (size_t i = static_cast<size_t>(start); i < line.length(); ++i)
    {
        char ch = line[i];
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')
            ++len;
        else
            break;
    }
    return len > 0 ? len : 1;
}

String stripIndent(const std::string& text, size_t indent)
{
    if (indent == 0 || text.empty())
        return String(text.c_str());
    String str(text.c_str());
    Index usable = std::min(static_cast<Index>(indent), str.getLength());
    return str.subString(usable, str.getLength() - usable);
}

// ============================================================================
// DIAGNOSTIC LAYOUT
// ============================================================================

struct LayoutSpan
{
    int line;
    int col;
    int length;
    std::string label;
    bool isPrimary;
};

struct LineHighlight
{
    int column = 0;
    int length = 1;
    std::string label;
    bool isPrimary = false;
};

struct HighlightLine
{
    int number = 0;
    std::string content;
    List<LineHighlight> spans;
};

struct LayoutBlock
{
    bool showGap = false;
    List<HighlightLine> lines;
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

int resolveSpanLength(const DiagnosticSpan& span, const List<String>& sourceLines)
{
    if (span.length > 0)
        return span.length;
    int lineIndex = span.location.line;
    if (lineIndex >= 0 && lineIndex < static_cast<int>(sourceLines.getCount()))
        return calculateFallbackLength(
            std::string(sourceLines[static_cast<Index>(lineIndex)].getBuffer()),
            span.location.column);
    return 1;
}

LayoutSpan makeLayoutSpan(
    const DiagnosticSpan& span,
    bool isPrimary,
    const List<String>& sourceLines)
{
    LayoutSpan layoutSpan;
    layoutSpan.line = span.location.line;
    layoutSpan.col = span.location.column;
    layoutSpan.length = resolveSpanLength(span, sourceLines);
    layoutSpan.label = span.message;
    layoutSpan.isPrimary = isPrimary;
    return layoutSpan;
}

size_t findCommonIndent(const List<LayoutBlock>& blocks)
{
    size_t minIndent = std::numeric_limits<size_t>::max();
    for (const auto& block : blocks)
    {
        for (const auto& line : block.lines)
        {
            if (line.content.empty())
                continue;
            size_t indent = 0;
            while (indent < line.content.size() &&
                   (line.content[indent] == ' ' || line.content[indent] == '\t'))
                ++indent;
            if (indent < minIndent)
                minIndent = indent;
        }
    }
    return (minIndent == std::numeric_limits<size_t>::max()) ? 0 : minIndent;
}

SectionLayout buildSectionLayout(
    const List<LayoutSpan>& spans,
    const List<String>& sourceLines)
{
    // Transform resolved spans into grouped, display-ready blocks: we bucket spans
    // per line, preserve source text for each line, keep gaps explicit so rendering
    // can insert ellipses, and measure shared indentation so highlights align even
    // when shader code is heavily indented in the fixture.
    SectionLayout section;
    if (spans.getCount() == 0)
        return section;

    int maxLineNum = 0;
    for (const auto& span : spans)
        maxLineNum = std::max(maxLineNum, span.line);
    section.maxGutterWidth = static_cast<int>(std::to_string(std::max(1, maxLineNum)).length());

    Dictionary<int, HighlightLine> grouped;
    for (const auto& span : spans)
    {
        HighlightLine& line = grouped[span.line];
        line.number = span.line;
        if (line.content.empty() && span.line >= 0 &&
            span.line < static_cast<int>(sourceLines.getCount()))
            line.content = std::string(sourceLines[static_cast<Index>(span.line)].getBuffer());
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
// RENDERING UTILITIES
// ============================================================================

struct LabelInfo
{
    int column = 0;
    std::string text;
};

List<String> buildAnnotationRows(const HighlightLine& line, size_t indentShift)
{
    // Convert intra-line highlights into the familiar caret/label ladder: first
    // lay down the underline with different glyphs for primary vs secondary spans,
    // attach the closest label inline when possible, then fall back to vertical
    // connectors so multiple labels can coexist without clobbering each other.
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

        if (!span.label.empty())
            labels.add(LabelInfo{effectiveColumn, span.label});
    }

    if (labels.getCount() > 0)
    {
        labels.sort(
            [](const LabelInfo& a, const LabelInfo& b) { return a.column > b.column; });
        underline = underline + " " + String(labels.getFirst().text.c_str());
        labels.removeAt(0);
    }

    rows.add(underline);
    if (labels.getCount() == 0)
        return rows;

    auto sortedLabels = labels;
    sortedLabels.sort(
        [](const LabelInfo& a, const LabelInfo& b) { return a.column < b.column; });

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

        active.sort(
            [](const LabelInfo& a, const LabelInfo& b) { return a.column < b.column; });

        String labelRow;
        int current = 1;
        for (const auto& info : active)
        {
            int spaces = std::max(0, info.column - current);
            labelRow = labelRow + repeat(' ', spaces);
            if (info.column == target.column)
            {
                labelRow = labelRow + String(info.text.c_str());
                current = info.column + static_cast<int>(info.text.length());
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
    // Rendering mirrors the layout structure: we print each block with the computed
    // gutter width, insert ellipses whenever the block reported a gap, then stream
    // the highlight ladder rows so the final text matches the snapshot stored in
    // the tests byte-for-byte.
    for (const auto& block : section.blocks)
    {
        if (block.showGap)
            ss << "...\n";

        for (const auto& line : block.lines)
        {
            const String label = line.number >= 0 ? String(std::to_string(line.number).c_str()) : "?";
            int padding = std::max(0, section.maxGutterWidth - static_cast<int>(label.getLength()));
            ss << repeat(' ', padding) << label << " | "
               << stripIndent(line.content, section.commonIndent) << '\n';

            for (const auto& row : buildAnnotationRows(line, section.commonIndent))
                printAnnotationRow(ss, section.maxGutterWidth, row);
        }
    }
}

DiagnosticLayout createLayout(const TestData& data)
{
    // createLayout is the bridge between raw fixture data and the renderer: it
    // copies headline metadata, resolves all spans (primary, secondary, notes),
    // builds the shared section layout once, and stores everything in a single
    // structure so the rendering phase can be a pure formatting pass.
    DiagnosticLayout layout;
    const auto& diag = data.diagnostic;

    layout.header.severity = String(diag.severity.c_str());
    layout.header.code = diag.code;
    layout.header.message = String(diag.message.c_str());

    layout.primaryLoc.fileName = String(diag.primarySpan.location.fileName.c_str());
    layout.primaryLoc.line = diag.primarySpan.location.line;
    layout.primaryLoc.col = diag.primarySpan.location.column;

    List<String> sourceLines = getSourceLines(data.sourceContent);

    List<LayoutSpan> allSpans;
    allSpans.add(makeLayoutSpan(diag.primarySpan, true, sourceLines));
    for (const auto& s : diag.secondarySpans)
        allSpans.add(makeLayoutSpan(s, false, sourceLines));

    layout.primarySection = buildSectionLayout(allSpans, sourceLines);
    layout.primaryLoc.gutterIndent = layout.primarySection.maxGutterWidth;

    for (const auto& note : diag.notes)
    {
        DiagnosticLayout::NoteEntry noteEntry;
        noteEntry.message = String(note.message.c_str());
        noteEntry.loc.fileName = String(note.span.location.fileName.c_str());
        noteEntry.loc.line = note.span.location.line;
        noteEntry.loc.col = note.span.location.column;

        List<LayoutSpan> noteSpans;
        noteSpans.add(makeLayoutSpan(note.span, false, sourceLines));
        noteEntry.section = buildSectionLayout(noteSpans, sourceLines);
        noteEntry.loc.gutterIndent = noteEntry.section.maxGutterWidth;

        layout.notes.add(std::move(noteEntry));
    }

    return layout;
}

std::string renderFromLayout(const DiagnosticLayout& layout)
{
    // This function owns the string assembly for everything the harness compares:
    // severity header, primary location, annotated source, and optional notes.
    // Keeping it side-effect free makes it trivial to diff the produced output.
    StringBuilder ss;

    ss << layout.header.severity << "[E";
    // Format error code with leading zeros to 4 digits
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

    return std::string(ss.getBuffer());
}

std::string renderDiagnostic(const TestData& testData)
{
    DiagnosticLayout layout = createLayout(testData);
    return renderFromLayout(layout);
}

// ============================================================================
// MAIN HARNESS
// ============================================================================

void writeTempFile(const std::string& path, const std::string& content)
{
    // Helper for diffing: we materialize both the expected and actual buffers as
    // plain files so we can lean on the system `diff` without dragging in a lib.
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
    // The harness accepts an optional `--until N` switch so developers can run a
    // prefix of the fixture set; we parse it once, fall back to all tests, then
    // drive a simple pass/fail loop that renders diagnostics, compares them to the
    // captured golden output, and shells out to `diff` when anything diverges.
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

    for (int i = 0; i < testLimit; ++i)
    {
        const TestData& test = testCases[i];
        std::cout << "\nTest " << (i + 1) << ": " << test.name << '\n';

        String actualOutput = trimNewlines(renderDiagnostic(test));
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
