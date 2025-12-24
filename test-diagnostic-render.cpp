#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
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
    int length; // explicit length field

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
    // Test 1: Undeclared identifier
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

    // Test 2: Type mismatch with secondary span
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
    // Test 3: Undeclared variable
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

    // Test 4: Wrong type assignment
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
    // Test 5: Division by zero warning
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
    // Test 6: Complex mismatched types
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
             {SourceLoc("example.slang", 8, 30), "expected `float`", 9},
         }}}};

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
        start++;
    size_t end = s.length();
    while (end > start && (s[end - 1] == '\n' || s[end - 1] == '\r'))
        end--;
    return s.substr(start, end - start);
}

std::string repeat(char c, int n)
{
    return n <= 0 ? std::string() : std::string(n, c);
}

int calculateFallbackLength(const std::string& line, int col)
{
    if (col < 1 || col > (int)line.length())
        return 1;
    int start = col - 1;
    int len = 0;
    for (size_t i = start; i < line.length(); ++i)
    {
        char c = line[i];
        if (isalnum(c) || c == '_')
            len++;
        else
            break;
    }
    return len > 0 ? len : 1;
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

struct LayoutSnippet
{
    int lineNum;
    std::string content;
    std::vector<std::string> annotations;
};

struct LayoutBlock
{
    bool showGap;
    std::vector<LayoutSnippet> snippets;
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
        int code;
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
    const std::vector<LayoutSpan>& allSpans)
{
    size_t minIndent = SIZE_MAX;
    for (const auto& s : allSpans)
    {
        size_t idx = static_cast<size_t>(s.line);
        if (idx >= allLines.size())
            continue;
        const std::string& line = allLines[idx];
        if (line.empty())
            continue;
        size_t indent = 0;
        while (indent < line.size() && (line[indent] == ' ' || line[indent] == '\t'))
            ++indent;
        if (indent < minIndent)
            minIndent = indent;
    }
    return (minIndent == SIZE_MAX) ? 0 : minIndent;
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

    std::vector<LayoutSpan> sortedSpans = spans;
    std::sort(
        sortedSpans.begin(),
        sortedSpans.end(),
        [](const LayoutSpan& a, const LayoutSpan& b)
        {
            if (a.line != b.line)
                return a.line < b.line;
            return a.col < b.col;
        });

    int maxLine = 0;
    for (const auto& s : sortedSpans)
        maxLine = std::max(maxLine, s.line);
    section.maxGutterWidth = static_cast<int>(std::to_string(std::max(1, maxLine)).length());
    section.commonIndent = findCommonIndent(sourceLines, sortedSpans);

    int prevLine = -1;
    size_t i = 0;
    while (i < sortedSpans.size())
    {
        int currentLine = sortedSpans[i].line;
        LayoutBlock block;
        block.showGap = (prevLine != -1 && currentLine > prevLine + 1);

        std::vector<LayoutSpan> lineSpans;
        while (i < sortedSpans.size() && sortedSpans[i].line == currentLine)
            lineSpans.push_back(sortedSpans[i++]);

        LayoutSnippet snippet;
        snippet.lineNum = currentLine;
        if (currentLine >= 0 && currentLine < static_cast<int>(sourceLines.size()))
            snippet.content = sourceLines[static_cast<size_t>(currentLine)];

        std::string underlineRow;
        int currentPos = 1;
        for (const auto& span : lineSpans)
        {
            int spaces = std::max(0, span.col - currentPos);
            underlineRow += repeat(' ', spaces);
            char marker = span.isPrimary ? '^' : '-';
            int len = (span.length > 0) ? span.length : 1;
            underlineRow += repeat(marker, len);
            currentPos = span.col + len;
        }

        std::vector<LayoutSpan> pendingLabels;
        for (const auto& s : lineSpans)
            if (!s.label.empty())
                pendingLabels.push_back(s);
        std::sort(
            pendingLabels.begin(),
            pendingLabels.end(),
            [](const LayoutSpan& a, const LayoutSpan& b) { return a.col > b.col; });
        if (!pendingLabels.empty())
        {
            underlineRow += " " + pendingLabels.front().label;
            pendingLabels.erase(pendingLabels.begin());
        }
        snippet.annotations.push_back(underlineRow);

        if (!pendingLabels.empty())
        {
            std::string connectorRow;
            int curPos = 1;
            auto sortedPending = pendingLabels;
            std::sort(
                sortedPending.begin(),
                sortedPending.end(),
                [](const LayoutSpan& a, const LayoutSpan& b) { return a.col < b.col; });
            for (const auto& span : sortedPending)
            {
                int spaces = std::max(0, span.col - curPos);
                connectorRow += repeat(' ', spaces) + "|";
                curPos = span.col + 1;
            }
            snippet.annotations.push_back(connectorRow);
        }
        for (size_t k = 0; k < pendingLabels.size(); ++k)
        {
            LayoutSpan target = pendingLabels[k];
            std::string labelRow;
            int curPos = 1;
            std::vector<LayoutSpan> activeSpans;
            for (const auto& s : pendingLabels)
                if (s.col <= target.col)
                    activeSpans.push_back(s);
            std::sort(
                activeSpans.begin(),
                activeSpans.end(),
                [](const LayoutSpan& a, const LayoutSpan& b) { return a.col < b.col; });
            for (const auto& span : activeSpans)
            {
                int spaces = std::max(0, span.col - curPos);
                labelRow += repeat(' ', spaces);
                if (span.col == target.col)
                {
                    labelRow += span.label;
                    curPos = span.col + static_cast<int>(span.label.length());
                }
                else
                {
                    labelRow += "|";
                    curPos = span.col + 1;
                }
            }
            snippet.annotations.push_back(labelRow);
        }

        block.snippets.push_back(snippet);
        section.blocks.push_back(block);
        prevLine = currentLine;
    }

    return section;
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

// ============================================================================
// RENDERER
// ============================================================================

std::string renderFromLayout(const DiagnosticLayout& layout)
{
    std::stringstream ss;

    ss << layout.header.severity << "[E" << std::setfill('0') << std::setw(4) << layout.header.code
       << "]: " << layout.header.message << '\n';

    ss << repeat(' ', layout.primaryLoc.gutterIndent) << "--> " << layout.primaryLoc.fileName << ":"
       << layout.primaryLoc.line << ":" << layout.primaryLoc.col << "\n";

    ss << repeat(' ', layout.primarySection.maxGutterWidth + 1) << "|\n";

    auto renderBlocks = [&ss](const SectionLayout& section)
    {
        for (const auto& block : section.blocks)
        {
            if (block.showGap)
                ss << "...\n";

            for (const auto& snippet : block.snippets)
            {
                std::string lineStr = std::to_string(snippet.lineNum);
                int padding = section.maxGutterWidth - static_cast<int>(lineStr.length());
                if (padding < 0)
                    padding = 0;
                ss << repeat(' ', padding) << lineStr << " | ";

                std::string dedented = snippet.content;
                if (section.commonIndent > 0 && dedented.size() >= section.commonIndent)
                    dedented.erase(0, section.commonIndent);
                ss << dedented << "\n";

                for (const auto& ann : snippet.annotations)
                {
                    ss << repeat(' ', section.maxGutterWidth + 1) << "| ";
                    std::string dedentedAnn = ann;
                    if (section.commonIndent > 0 && dedentedAnn.size() >= section.commonIndent)
                        dedentedAnn.erase(0, section.commonIndent);
                    ss << dedentedAnn << "\n";
                }
            }
        }
    };

    renderBlocks(layout.primarySection);

    for (const auto& note : layout.notes)
    {
        ss << "\nnote: " << note.message << "\n";
        ss << repeat(' ', note.loc.gutterIndent) << "--- " << note.loc.fileName << ":"
           << note.loc.line << ":" << note.loc.col << "\n";
        ss << repeat(' ', note.section.maxGutterWidth + 1) << "|\n";
        renderBlocks(note.section);
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

int runDiff(const std::string& expected, const std::string& actual)
{
    std::ofstream expectedFile("expected.tmp");
    expectedFile << expected;
    expectedFile << "\n";
    expectedFile.close();

    std::ofstream actualFile("actual.tmp");
    actualFile << actual;
    actualFile << "\n";
    actualFile.close();

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

    int testCount = (maxTests == -1) ? NUM_TESTS : std::min(maxTests, static_cast<int>(NUM_TESTS));
    std::cout << "Running " << testCount << " test(s)...\n";

    int passed = 0, failed = 0;
    for (int i = 0; i < testCount; ++i)
    {
        const TestData& test = testCases[i];
        std::cout << "\nTest " << (i + 1) << ": " << test.name << "\n";

        std::string actualOutput = renderDiagnostic(test);
        std::string expectedOutput = trimNewlines(test.expectedOutput);
        actualOutput = trimNewlines(actualOutput);

        if (actualOutput == expectedOutput)
        {
            std::cout << "PASS\n";
            passed++;
        }
        else
        {
            std::cout << "FAIL - Output mismatch\nRunning diff...\n";
            runDiff(expectedOutput, actualOutput);
            failed++;
        }
    }
    std::cout << "\nResults: " << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}
