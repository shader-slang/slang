#include <algorithm>
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

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

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
    std::vector<DiagnosticSpan> secondarySpans;
    std::vector<DiagnosticNote> notes;
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
          .secondarySpans = {}}},

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
          .secondarySpans =
              {{SourceLoc("example.slang", 5, 5), "field declared here", 17},
               {SourceLoc("example.slang", 10, 16), "float4", 11},
               {SourceLoc("example.slang", 10, 30), "int", 19}}}},

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
          .secondarySpans = {}}},

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
          .secondarySpans = {{SourceLoc("example.slang", 7, 9), "expected due to this type", 5}}}},

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
          .secondarySpans = {{SourceLoc("math.slang", 2, 17), "length computed here", 15}},
          .notes =
              {{.message = "consider using 'normalize' builtin function instead",
                .span = {SourceLoc("math.slang", 1, 8), "", 9}}}}},

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
         .secondarySpans = {
             {SourceLoc("example.slang", 2, 5), "defined as `string` here", 5},
             {SourceLoc("example.slang", 8, 24), "float", 3},
             {SourceLoc("example.slang", 8, 30), "expected `float`", 9}}}}};

const size_t NUM_TESTS = sizeof(testCases) / sizeof(testCases[0]);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

std::vector<std::string> getSourceLines(const std::string& content)
{
    std::vector<std::string> lines;
    std::stringstream ss(content);
    std::string line;
    while (std::getline(ss, line))
        lines.push_back(line);
    return lines;
}

std::string trimNewlines(const std::string& s)
{
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
    std::vector<LineHighlight> spans;
};

struct LayoutBlock
{
    bool showGap = false;
    std::vector<HighlightLine> lines;
};

struct SectionLayout
{
    int maxGutterWidth = 0;
    size_t commonIndent = 0;
    std::vector<LayoutBlock> blocks;
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
    std::vector<NoteEntry> notes;
};

size_t findCommonIndent(
    const std::vector<std::string>& allLines,
    const std::vector<LayoutSpan>& spans)
{
    size_t minIndent = std::numeric_limits<size_t>::max();
    for (const auto& span : spans)
    {
        size_t idx = static_cast<size_t>(span.line);
        if (idx >= allLines.size())
            continue;
        const std::string& line = allLines[idx];
        if (line.empty())
            continue;
        size_t indent = 0;
        while (indent < line.size() && (line[indent] == ' ' || line[indent] == '\t'))
            ++indent;
        minIndent = std::min(minIndent, indent);
    }
    return (minIndent == std::numeric_limits<size_t>::max()) ? 0 : minIndent;
}

int resolveSpanLength(const DiagnosticSpan& span, const std::vector<std::string>& sourceLines)
{
    if (span.length > 0)
        return span.length;
    int lineIndex = span.location.line;
    if (lineIndex >= 0 && lineIndex < static_cast<int>(sourceLines.size()))
        return calculateFallbackLength(
            sourceLines[static_cast<size_t>(lineIndex)],
            span.location.column);
    return 1;
}

LayoutSpan makeLayoutSpan(
    const DiagnosticSpan& span,
    bool isPrimary,
    const std::vector<std::string>& sourceLines)
{
    LayoutSpan layoutSpan;
    layoutSpan.line = span.location.line;
    layoutSpan.col = span.location.column;
    layoutSpan.length = resolveSpanLength(span, sourceLines);
    layoutSpan.label = span.message;
    layoutSpan.isPrimary = isPrimary;
    return layoutSpan;
}

SectionLayout buildSectionLayout(
    const std::vector<LayoutSpan>& spans,
    const std::vector<std::string>& sourceLines)
{
    SectionLayout section;
    if (spans.empty())
        return section;

    section.commonIndent = findCommonIndent(sourceLines, spans);

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
            span.line < static_cast<int>(sourceLines.size()))
            line.content = sourceLines[static_cast<size_t>(span.line)];
        line.spans.push_back(LineHighlight{span.col, span.length, span.label, span.isPrimary});
    }

    for (auto& [_, line] : grouped)
    {
        std::sort(
            line.spans.begin(),
            line.spans.end(),
            [](const LineHighlight& a, const LineHighlight& b)
            {
                if (a.column != b.column)
                    return a.column < b.column;
                return a.length < b.length;
            });
    }

    std::vector<int> lineNumbers;
    lineNumbers.reserve(grouped.size());
    for (const auto& [number, _] : grouped)
        lineNumbers.push_back(number);
    std::sort(lineNumbers.begin(), lineNumbers.end());

    LayoutBlock currentBlock;
    int prevLine = std::numeric_limits<int>::min();
    for (int number : lineNumbers)
    {
        bool hasGap = prevLine != std::numeric_limits<int>::min() && number > prevLine + 1;
        if (hasGap && !currentBlock.lines.empty())
        {
            section.blocks.push_back(currentBlock);
            currentBlock = LayoutBlock{};
            currentBlock.showGap = true;
        }
        else if (currentBlock.lines.empty())
        {
            currentBlock.showGap = false;
        }

        currentBlock.lines.push_back(grouped[number]);
        prevLine = number;
    }

    if (!currentBlock.lines.empty())
        section.blocks.push_back(currentBlock);

    return section;
}

// ============================================================================
// RENDERING UTILITIES
// ============================================================================

std::string buildMarkerRow(const HighlightLine& line, size_t indentShift)
{
    if (line.spans.empty())
        return {};
    std::string row;
    int cursor = 1;
    for (const auto& span : line.spans)
    {
        int effectiveColumn = std::max(1, span.column - static_cast<int>(indentShift));
        int spaces = std::max(0, effectiveColumn - cursor);
        row += repeat(' ', spaces);
        int length = std::max(1, span.length);
        row += repeat(span.isPrimary ? '^' : '-', length);
        cursor = effectiveColumn + length;
    }
    return row;
}

std::vector<std::string> buildLabelRows(const HighlightLine& line, size_t indentShift)
{
    std::vector<std::string> rows;
    for (const auto& span : line.spans)
    {
        if (span.label.empty())
            continue;
        int effectiveColumn = std::max(1, span.column - static_cast<int>(indentShift));
        std::string padding = repeat(' ', effectiveColumn - 1);
        rows.push_back(padding + "|");
        rows.push_back(padding + span.label);
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

            const std::string markerRow = buildMarkerRow(line, section.commonIndent);
            printAnnotationRow(ss, section.maxGutterWidth, markerRow);

            for (const std::string& labelRow : buildLabelRows(line, section.commonIndent))
                printAnnotationRow(ss, section.maxGutterWidth, labelRow);
        }
    }
}

DiagnosticLayout createLayout(const TestData& data)
{
    DiagnosticLayout layout;
    const auto& diag = data.diagnostic;

    layout.header.severity = diag.severity;
    layout.header.code = diag.code;
    layout.header.message = diag.message;

    layout.primaryLoc.fileName = diag.primarySpan.location.fileName;
    layout.primaryLoc.line = diag.primarySpan.location.line;
    layout.primaryLoc.col = diag.primarySpan.location.column;

    std::vector<std::string> sourceLines = getSourceLines(data.sourceContent);

    std::vector<LayoutSpan> allSpans;
    allSpans.push_back(makeLayoutSpan(diag.primarySpan, true, sourceLines));
    for (const auto& s : diag.secondarySpans)
        allSpans.push_back(makeLayoutSpan(s, false, sourceLines));

    layout.primarySection = buildSectionLayout(allSpans, sourceLines);
    layout.primaryLoc.gutterIndent = layout.primarySection.maxGutterWidth;

    for (const auto& note : diag.notes)
    {
        DiagnosticLayout::NoteEntry noteEntry;
        noteEntry.message = note.message;
        noteEntry.loc.fileName = note.span.location.fileName;
        noteEntry.loc.line = note.span.location.line;
        noteEntry.loc.col = note.span.location.column;

        std::vector<LayoutSpan> noteSpans = {makeLayoutSpan(note.span, false, sourceLines)};
        noteEntry.section = buildSectionLayout(noteSpans, sourceLines);
        noteEntry.loc.gutterIndent = noteEntry.section.maxGutterWidth;

        layout.notes.push_back(std::move(noteEntry));
    }

    return layout;
}

std::string renderFromLayout(const DiagnosticLayout& layout)
{
    std::stringstream ss;

    ss << layout.header.severity << "[E" << std::setfill('0') << std::setw(4) << layout.header.code
       << std::setfill(' ') << "]: " << layout.header.message << '\n';

    ss << repeat(' ', layout.primaryLoc.gutterIndent) << "--> " << layout.primaryLoc.fileName << ":"
       << layout.primaryLoc.line << ":" << layout.primaryLoc.col << '\n';

    if (!layout.primarySection.blocks.empty())
    {
        ss << repeat(' ', layout.primarySection.maxGutterWidth + 1) << "|\n";
        renderSectionBody(ss, layout.primarySection);
    }

    for (const auto& note : layout.notes)
    {
        ss << "\nnote: " << note.message << '\n';
        ss << repeat(' ', note.loc.gutterIndent) << "--- " << note.loc.fileName << ":"
           << note.loc.line << ":" << note.loc.col << '\n';
        if (!note.section.blocks.empty())
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

int main(int argc, char* argv[])
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
