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

List<std::string> getSourceLines(const std::string& content)
{
    // Parse the embedded shader snippet into logical lines once so downstream layout
    // code can work with indexed access; the intermediate stringstream keeps the
    // implementation simple while guaranteeing consistent handling of CRLF vs LF.
    List<std::string> lines;
    std::stringstream ss(content);
    std::string line;
    while (std::getline(ss, line))
        lines.add(line);
    return lines;
}

std::string trimNewlines(const std::string& s)
{
    // Harness output comparisons assume deterministic framing, so we aggressively
    // trim leading/trailing newline noise that often shows up in raw heredoc data.
    size_t start = 0;
    while (start < s.length() && (s[start] == '\n' || s[start] == '\r'))
        ++start;
    size_t end = s.length();
    while (end > start && (s[end - 1] == '\n' || s[end - 1] == '\r'))
        --end;
    return s.substr(start, end - start);
}

std::string repeat(char c, int n)
{
    return n <= 0 ? std::string() : std::string(static_cast<size_t>(n), c);
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

std::string stripIndent(const std::string& text, size_t indent)
{
    if (indent == 0 || text.empty())
        return text;
    size_t usable = std::min(indent, text.size());
    return text.substr(usable);
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
        std::string severity;
        int code = 0;
        std::string message;
    } header;

    struct Location
    {
        std::string fileName;
        int line = 0;
        int col = 0;
        int gutterIndent = 0;
    } primaryLoc;

    SectionLayout primarySection;

    struct NoteEntry
    {
        std::string message;
        Location loc;
        SectionLayout section;
    };
    List<NoteEntry> notes;
};

int resolveSpanLength(const DiagnosticSpan& span, const List<std::string>& sourceLines)
{
    if (span.length > 0)
        return span.length;
    int lineIndex = span.location.line;
    if (lineIndex >= 0 && lineIndex < static_cast<int>(sourceLines.getCount()))
        return calculateFallbackLength(
            sourceLines[static_cast<size_t>(lineIndex)],
            span.location.column);
    return 1;
}

LayoutSpan makeLayoutSpan(
    const DiagnosticSpan& span,
    bool isPrimary,
    const List<std::string>& sourceLines)
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
    const List<std::string>& sourceLines)
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

    std::map<int, HighlightLine> grouped;
    for (const auto& span : spans)
    {
        HighlightLine& line = grouped[span.line];
        line.number = span.line;
        if (line.content.empty() && span.line >= 0 &&
            span.line < static_cast<int>(sourceLines.getCount()))
            line.content = sourceLines[static_cast<Index>(span.line)];
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

List<std::string> buildAnnotationRows(const HighlightLine& line, size_t indentShift)
{
    // Convert intra-line highlights into the familiar caret/label ladder: first
    // lay down the underline with different glyphs for primary vs secondary spans,
    // attach the closest label inline when possible, then fall back to vertical
    // connectors so multiple labels can coexist without clobbering each other.
    List<std::string> rows;
    if (line.spans.getCount() == 0)
        return rows;

    List<LabelInfo> labels;
    std::string underline;
    int cursor = 1;

    for (const auto& span : line.spans)
    {
        int effectiveColumn = std::max(1, span.column - static_cast<int>(indentShift));
        int length = std::max(1, span.length);
        int spaces = std::max(0, effectiveColumn - cursor);
        underline += repeat(' ', spaces);
        underline += repeat(span.isPrimary ? '^' : '-', length);
        cursor = effectiveColumn + length;

        if (!span.label.empty())
            labels.add(LabelInfo{effectiveColumn, span.label});
    }

    if (labels.getCount() > 0)
    {
        labels.sort(
            [](const LabelInfo& a, const LabelInfo& b) { return a.column > b.column; });
        underline += " " + labels.getFirst().text;
        labels.removeAt(0);
    }

    rows.add(underline);
    if (labels.getCount() == 0)
        return rows;

    auto sortedLabels = labels;
    sortedLabels.sort(
        [](const LabelInfo& a, const LabelInfo& b) { return a.column < b.column; });

    std::string connector;
    int pos = 1;
    for (const auto& info : sortedLabels)
    {
        int spaces = std::max(0, info.column - pos);
        connector += repeat(' ', spaces) + "|";
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

        std::string labelRow;
        int current = 1;
        for (const auto& info : active)
        {
            int spaces = std::max(0, info.column - current);
            labelRow += repeat(' ', spaces);
            if (info.column == target.column)
            {
                labelRow += info.text;
                current = info.column + static_cast<int>(info.text.length());
            }
            else
            {
                labelRow += "|";
                current = info.column + 1;
            }
        }
        rows.add(labelRow);
    }

    return rows;
}

void printAnnotationRow(std::ostream& ss, int gutterWidth, const std::string& content)
{
    if (content.empty())
        return;
    ss << repeat(' ', gutterWidth + 1) << "| " << content << '\n';
}

void renderSectionBody(std::ostream& ss, const SectionLayout& section)
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
            const std::string label = line.number >= 0 ? std::to_string(line.number) : "?";
            int padding = std::max(0, section.maxGutterWidth - static_cast<int>(label.length()));
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

    layout.header.severity = diag.severity;
    layout.header.code = diag.code;
    layout.header.message = diag.message;

    layout.primaryLoc.fileName = diag.primarySpan.location.fileName;
    layout.primaryLoc.line = diag.primarySpan.location.line;
    layout.primaryLoc.col = diag.primarySpan.location.column;

    List<std::string> sourceLines = getSourceLines(data.sourceContent);

    List<LayoutSpan> allSpans;
    allSpans.add(makeLayoutSpan(diag.primarySpan, true, sourceLines));
    for (const auto& s : diag.secondarySpans)
        allSpans.add(makeLayoutSpan(s, false, sourceLines));

    layout.primarySection = buildSectionLayout(allSpans, sourceLines);
    layout.primaryLoc.gutterIndent = layout.primarySection.maxGutterWidth;

    for (const auto& note : diag.notes)
    {
        DiagnosticLayout::NoteEntry noteEntry;
        noteEntry.message = note.message;
        noteEntry.loc.fileName = note.span.location.fileName;
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
    std::stringstream ss;

    ss << layout.header.severity << "[E" << std::setfill('0') << std::setw(4) << layout.header.code
       << std::setfill(' ') << "]: " << layout.header.message << '\n';

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

    return ss.str();
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

        std::string actualOutput = trimNewlines(renderDiagnostic(test));
        std::string expectedOutput = trimNewlines(test.expectedOutput);

        if (actualOutput == expectedOutput)
        {
            std::cout << "PASS\n";
            ++passed;
        }
        else
        {
            std::cout << "FAIL - Output mismatch\nRunning diff...\n";
            runDiff(expectedOutput, actualOutput);
            ++failed;
        }
    }

    std::cout << "\nResults: " << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}
