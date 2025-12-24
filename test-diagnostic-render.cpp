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
    int length; // Added explicit length field

    DiagnosticSpan()
        : length(0)
    {
    }

    // Constructor with length
    DiagnosticSpan(const SourceLoc& loc, const std::string& msg, int len = 0)
        : location(loc), message(msg), length(len)
    {
    }
};

struct GenericDiagnostic
{
    int code;
    std::string severity;
    std::string message;
    DiagnosticSpan primarySpan;
    std::vector<DiagnosticSpan> secondarySpans;
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

/*
   Updates made to Test 1:
   1. Corrected `column` to 30. In the source line `        float4...`, 'someSampler' starts at
   index 29 (col 30).
   2. Added `length` of 11 for 'someSampler'.
   3. Updated `expectedOutput` to match the source indentation (8 spaces) and the new column
   position.
*/


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
8 |         float4 color = tex2D(someSampler, input.texCoord); // Undefined sampler
  |                              ^^^^^^^^^^^ not found in this scope
)",
     .diagnostic =
         {.code = 1001,
          .severity = "error",
          .message = "use of undeclared identifier 'someSampler'",
          // Corrected span: "someSampler" starts at col 30, length 11
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
05 |     int invalid_field; // This field has issues
   |     ----------------- field declared here
...
10 |         return color * 2.0 + input.invalid_field; // Type mismatch
   |                ----------- ^ ------------------- int
   |                |           |
   |                |           no implementation for `float4 + int`
   |                float4
)",
     .diagnostic =
         {.code = 1002,
          .severity = "error",
          .message = "cannot add `float4` and `int`",
          .primarySpan =
              {SourceLoc("example.slang", 10, 28), "no implementation for `float4 + int`", 1},
          .secondarySpans =
              {// Line 5: "int invalid_field"
               // Indent 4 + "int " (4) + "invalid_field" (13) = 17 chars.
               // "int" starts at 5. Length 17 covers "int invalid_field".
               {SourceLoc("example.slang", 5, 5), "field declared here", 17},

               // Line 10: Left operand "color * 2.0"
               // Indent 4 + "return " (7) = 11. "color" starts at 12.
               // "color * 2.0" length 11.
               {SourceLoc("example.slang", 10, 16), "float4", 11},

               // Line 10: Right operand "input.invalid_field"
               // 24 is '+', 25 is space, 26 is start.
               // "input.invalid_field" length 19.
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
7 |     int x = undefinedVariable; // Undefined variable
  |             ^^^^^^^^^^^^^^^^^ not found in this scope
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
  --> example.slang:15:15
   |
15 |     float y = "string"; // Wrong type assignment
   |     -----     ^^^^^^^^ expected `float`, found `&str`
   |     |
   |     expected due to this type
)",
     .diagnostic =
         {.code = 1004,
          .severity = "error",
          .message = "mismatched types",
          .primarySpan = {SourceLoc("example.slang", 15, 15), "expected `float`, found `&str`", 8},
          .secondarySpans = {{SourceLoc("example.slang", 15, 5), "expected due to this type", 5}}}},

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
 --> math.slang:7:14
  |
6 |     float len = sqrt(dot(v, v));
  |                 --------------- length computed here
7 |     return v / len; // Potential division by zero
  |              ^ division by zero if `len` is 0.0

note: consider using 'normalize' builtin function instead
 --> math.slang:5:8
  |
5 | float3 normalize(float3 v) {
  |        ^^^^^^^^^
)",
     .diagnostic =
         {.code = 2001,
          .severity = "warning",
          .message = "potential division by zero",
          .primarySpan = {SourceLoc("math.slang", 7, 14), "division by zero if `len` is 0.0", 1},
          .secondarySpans = {{SourceLoc("math.slang", 6, 17), "length computed here", 15}}}},

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
  --> example.slang:12:18
   |
4  |     string name = "User";
   |            ----   ------ secondary span: defined as `string` here
...
12 |     float result = 5.0 + name;
   |                    --- ^ ---- primary span: expected `float`
   |                    |   |
   |                    |   `+` cannot be applied to these types
   |                    float
)",
     .diagnostic = {
         .code = 308,
         .severity = "error",
         .message = "mismatched types",
         .primarySpan =
             {SourceLoc("example.slang", 12, 18), "`+` cannot be applied to these types", 1},
         .secondarySpans = {{SourceLoc("example.slang", 4, 12), "defined as `string` here", 4}}}}};

const size_t NUM_TESTS = sizeof(testCases) / sizeof(testCases[0]);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Extract lines from source content
std::vector<std::string> getSourceLines(const std::string& content)
{
    std::vector<std::string> lines;
    std::stringstream ss(content);
    std::string line;
    while (std::getline(ss, line))
    {
        lines.push_back(line);
    }
    return lines;
}

// Find minimum common whitespace indentation across non-empty lines
size_t findCommonIndent(const std::vector<std::string>& lines)
{
    size_t minIndent = SIZE_MAX;
    bool foundNonEmpty = false;

    for (const auto& line : lines)
    {
        if (line.empty())
            continue;

        // Skip lines that look like top-level declarations if they start at 0
        // This is a heuristic.
        size_t indent = 0;
        for (char c : line)
        {
            if (c == ' ' || c == '\t')
                indent++;
            else
                break;
        }

        if (!foundNonEmpty || indent < minIndent)
        {
            minIndent = indent;
            foundNonEmpty = true;
        }
    }
    return foundNonEmpty ? minIndent : 0;
}

// Helper to remove newlines from start/end of a string
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

// Helper: Repeat a character N times
std::string repeat(char c, int n)
{
    if (n <= 0)
        return "";
    return std::string(n, c);
}

// Helper: Calculate length of a token if length is 0 (fallback)
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
// DIAGNOSTIC RENDERER
// ============================================================================

/*
   LAYOUT ALGORITHM:
   The renderer uses an intermediate `DiagnosticLayout` struct.
   1. It extracts the relevant line from the source.
   2. It calculates the "gutter width" based on the line number (e.g., "9" -> 1 char, "10" -> 2
   chars).
   3. It constructs the underline string by calculating padding spaces based on the column index.
      - Uses explicit span.length if available.
      - Falls back to identifier scanning if length is 0.
   4. The `renderFromLayout` function then simply prints these pre-calculated strings, ensuring
      alignment of the vertical bar `|`.
*/

// Represents a single line of code and its associated annotation lines
struct LayoutSnippet
{
    int lineNum;
    std::string content; // The raw source line
    std::vector<std::string>
        annotations; // Fully formatted annotation lines (e.g. "   |  ^^ label")
                     // These strings are printed relative to the gutter separator
};

// Represents a contiguous block of code (e.g. lines 4-5)
struct LayoutBlock
{
    bool showGap; // Render "..." before this block?
    std::vector<LayoutSnippet> snippets;
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
        int line;
        int col;
        int gutterIndent;
    } primaryLoc;

    int maxGutterWidth; // Width of the line number column
    std::vector<LayoutBlock> blocks;
};


struct LayoutSpan
{
    int line;
    int col;
    int length;
    std::string label;
    bool isPrimary;
};

DiagnosticLayout createLayout(const TestData& data)
{
    DiagnosticLayout layout;
    const auto& diag = data.diagnostic;

    // 1. Header & Primary Loc
    layout.header.severity = diag.severity;
    layout.header.code = diag.code;
    layout.header.message = diag.message;
    layout.primaryLoc.fileName = diag.primarySpan.location.fileName;
    layout.primaryLoc.line = diag.primarySpan.location.line;
    layout.primaryLoc.col = diag.primarySpan.location.column;

    // 2. Flatten Spans
    std::vector<LayoutSpan> allSpans;
    allSpans.push_back(
        {diag.primarySpan.location.line,
         diag.primarySpan.location.column,
         diag.primarySpan.length,
         diag.primarySpan.message,
         true});
    for (const auto& s : diag.secondarySpans)
    {
        allSpans.push_back({s.location.line, s.location.column, s.length, s.message, false});
    }

    // Sort by Line (Ascending), then Column (Ascending)
    std::sort(
        allSpans.begin(),
        allSpans.end(),
        [](const LayoutSpan& a, const LayoutSpan& b)
        {
            if (a.line != b.line)
                return a.line < b.line;
            return a.col < b.col;
        });

    // Calc max gutter
    int maxLine = 0;
    for (const auto& s : allSpans)
        maxLine = std::max(maxLine, s.line);
    layout.maxGutterWidth = std::to_string(maxLine).length();
    layout.primaryLoc.gutterIndent = layout.maxGutterWidth;

    // 3. Build Blocks
    std::vector<std::string> sourceLines = getSourceLines(data.sourceContent);
    int prevLine = -1;

    for (size_t i = 0; i < allSpans.size();)
    {
        int currentLine = allSpans[i].line;

        // New Block Logic
        LayoutBlock block;
        // Show gap if we skipped more than 1 line (e.g. 5 -> 10)
        block.showGap = (prevLine != -1 && currentLine > prevLine + 1);

        // Collect spans for this line
        std::vector<LayoutSpan> lineSpans;
        while (i < allSpans.size() && allSpans[i].line == currentLine)
        {
            lineSpans.push_back(allSpans[i]);
            i++;
        }

        LayoutSnippet snippet;
        snippet.lineNum = currentLine;
        if (currentLine > 0 && currentLine <= (int)sourceLines.size())
        {
            snippet.content = sourceLines[currentLine];
        }

        // --- Generate Annotations (Waterfall Logic) ---

        // 1. Base Underline Row (Left-to-Right)
        std::string underlineRow = "";
        int currentPos = 1;

        // Sort Left-to-Right for drawing the underline itself
        // (Already sorted by collection logic, but ensuring safety)
        std::vector<LayoutSpan> sortedByCol = lineSpans;

        for (const auto& span : sortedByCol)
        {
            int spaces = std::max(0, span.col - currentPos);
            underlineRow += repeat(' ', spaces);
            char marker = span.isPrimary ? '^' : '-';
            int len = (span.length > 0) ? span.length : 1;
            underlineRow += repeat(marker, len);
            currentPos = span.col + len;
        }

        // 2. Rightmost Label Inline Optimization
        // If the rightmost span has a label, place it on the same line
        std::vector<LayoutSpan> pendingLabels;
        for (const auto& s : sortedByCol)
        {
            if (!s.label.empty())
                pendingLabels.push_back(s);
        }

        // Sort pending labels Right-to-Left (Descending Column)
        std::sort(
            pendingLabels.begin(),
            pendingLabels.end(),
            [](const LayoutSpan& a, const LayoutSpan& b) { return a.col > b.col; });

        if (!pendingLabels.empty())
        {
            // Check the rightmost label (first in pendingLabels)
            const auto& rightmost = pendingLabels[0];
            // Since we sorted Left-to-Right for the row generation, the end of string is the end of
            // rightmost span We can just append the label. Note: This assumes no visual overlap
            // with previous text, which is true if we just appended.
            underlineRow += " " + rightmost.label;

            // Remove from pending
            pendingLabels.erase(pendingLabels.begin());
        }

        snippet.annotations.push_back(underlineRow);

        // 3. Waterfall: Remaining labels drop down
        // pendingLabels is already sorted Right-to-Left.

        // If we have remaining labels, output a "Connector Row" first
        // This connects the underlines down to the labels.
        if (!pendingLabels.empty())
        {
            std::string connectorRow = "";
            int curPos = 1;
            // Draw pipes for all pending labels
            // We need to iterate Left-to-Right to build the string
            std::vector<LayoutSpan> sortedPending = pendingLabels;
            std::sort(
                sortedPending.begin(),
                sortedPending.end(),
                [](auto a, auto b) { return a.col < b.col; });

            for (const auto& span : sortedPending)
            {
                int spaces = std::max(0, span.col - curPos);
                connectorRow += repeat(' ', spaces);
                connectorRow += "|";
                curPos = span.col + 1;
            }
            snippet.annotations.push_back(connectorRow);
        }

        // Now print the labels one by one (Right-to-Left order from pendingLabels)
        for (size_t k = 0; k < pendingLabels.size(); ++k)
        {
            LayoutSpan target = pendingLabels[k];
            std::string labelRow = "";
            int curPos = 1;

            // We need to draw pipes for all spans that are to the LEFT of target (and target
            // itself) Filter: spans in pendingLabels that have col <= target.col
            std::vector<LayoutSpan> activeSpans;
            for (const auto& s : pendingLabels)
            {
                if (s.col <= target.col)
                    activeSpans.push_back(s);
            }
            // Sort Left-to-Right for string building
            std::sort(
                activeSpans.begin(),
                activeSpans.end(),
                [](auto a, auto b) { return a.col < b.col; });

            for (const auto& span : activeSpans)
            {
                int spaces = std::max(0, span.col - curPos);
                labelRow += repeat(' ', spaces);

                if (span.col == target.col)
                {
                    // This is the target, print label
                    labelRow += span.label;
                    // Note: This consumes the rest of the line, assumes no labels to the right
                    // (true by R-to-L loop)
                    curPos = span.col + (int)span.label.length();
                }
                else
                {
                    // Span to the left, print pipe
                    labelRow += "|";
                    curPos = span.col + 1;
                }
            }
            snippet.annotations.push_back(labelRow);
        }

        block.snippets.push_back(snippet);
        layout.blocks.push_back(block);
        prevLine = currentLine;
    }

    return layout;
}

// ============================================================================
// RENDERER
// ============================================================================

std::string renderFromLayout(const DiagnosticLayout& layout)
{
    std::stringstream ss;

    // Header
    ss << layout.header.severity << "[E" << layout.header.code << "]: " << layout.header.message
       << "\n";

    // Primary Loc
    ss << repeat(' ', layout.primaryLoc.gutterIndent) << "--> " << layout.primaryLoc.fileName << ":"
       << layout.primaryLoc.line << ":" << layout.primaryLoc.col << "\n";

    // Top Separator
    ss << repeat(' ', layout.maxGutterWidth + 1) << "|\n";

    for (const auto& block : layout.blocks)
    {
        if (block.showGap)
        {
            ss << "...\n";
        }

        for (const auto& snippet : block.snippets)
        {
            // Code Line
            std::string lineStr = std::to_string(snippet.lineNum);
            int padding = layout.maxGutterWidth - (int)lineStr.length();
            // Pad with '0' if user wants leading zeros, but typically space padding is used in
            // compilers. Looking at expected output: "05 |" implies zero padding for smaller
            // numbers? Or just " 5 |"? Test 2 output says "05". Let's implement zero padding if
            // length < max width.
            if (layout.maxGutterWidth > 1 && lineStr.length() < (size_t)layout.maxGutterWidth)
            {
                ss << repeat('0', padding) << lineStr << " | " << snippet.content << "\n";
            }
            else
            {
                ss << repeat(' ', padding) << lineStr << " | " << snippet.content << "\n";
            }

            // Annotation Lines
            for (const auto& ann : snippet.annotations)
            {
                ss << repeat(' ', layout.maxGutterWidth + 1) << "| " << ann << "\n";
            }
        }
    }

    // Note: The main harness `trimNewlines` will handle trailing newlines.
    return ss.str();
}

std::string renderDiagnostic(const TestData& testData)
{
    // Currently only handling the primary span for the first test case
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
    expectedFile.close();

    std::ofstream actualFile("actual.tmp");
    actualFile << actual;
    actualFile.close();

    int result = std::system("diff -u expected.tmp actual.tmp");

    std::remove("expected.tmp");
    std::remove("actual.tmp");

    return result;
}

int main(int argc, char* argv[])
{
    int maxTests = -1;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--until") == 0 && i + 1 < argc)
        {
            maxTests = std::atoi(argv[i + 1]);
            i++;
        }
    }

    if (maxTests == 0)
    {
        std::cout << "Test harness initialized with " << NUM_TESTS << " test cases." << std::endl;
        return 0;
    }

    int testCount = (maxTests == -1) ? NUM_TESTS : std::min(maxTests, (int)NUM_TESTS);

    std::cout << "Running " << testCount << " test(s)..." << std::endl;

    int passed = 0;
    int failed = 0;

    for (int i = 0; i < testCount; i++)
    {
        const TestData& test = testCases[i];
        std::cout << "\nTest " << (i + 1) << ": " << test.name << std::endl;

        std::string actualOutput = renderDiagnostic(test);
        std::string expectedOutput = trimNewlines(test.expectedOutput);
        actualOutput = trimNewlines(actualOutput); // Trim actual too for fair comparison

        if (actualOutput == expectedOutput)
        {
            std::cout << "PASS" << std::endl;
            passed++;
        }
        else
        {
            std::cout << "FAIL - Output mismatch" << std::endl;
            std::cout << "Running diff..." << std::endl;
            runDiff(expectedOutput, actualOutput);
            failed++;
        }
    }

    std::cout << "\nResults: " << passed << " passed, " << failed << " failed" << std::endl;
    return failed > 0 ? 1 : 0;
}
