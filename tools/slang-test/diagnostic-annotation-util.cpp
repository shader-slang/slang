// diagnostic-annotation-util.cpp
#include "diagnostic-annotation-util.h"

#include "../../source/core/slang-string-util.h"

namespace Slang
{

// Internal structures
struct ParsedDiagnostic
{
    String errorCode; // e.g., "E20101"
    String severity;  // e.g., "warning", "error"
    String filename;
    int beginLine = 0;
    int beginCol = 0;
    int endLine = 0;
    int endCol = 0;
    String message;
};

struct Annotation
{
    enum class Type
    {
        SimpleSubstring, // Just check substring appears somewhere
        PositionBased    // Check specific line/column position and message
    };

    Type type = Type::SimpleSubstring;
    String expectedSubstring; // Message substring to match

    // For position-based annotations:
    int sourceLineNumber = 0; // Line number in source where diagnostic should appear
    int columnStart = 0;      // Starting column (1-based)
    int columnEnd = 0;        // Ending column (1-based)
};

// Internal functions
static SlangResult parseAnnotations(
    const UnownedStringSlice& sourceText,
    const UnownedStringSlice& prefix,
    List<Annotation>& outAnnotations,
    List<UnownedStringSlice>* outSourceLines);

static SlangResult parseMachineReadableDiagnostics(
    const UnownedStringSlice& output,
    List<ParsedDiagnostic>& outDiagnostics);

// Helper to generate suggested annotations for a group of diagnostics
static void generateSuggestedAnnotations(
    StringBuilder& sb,
    const List<const ParsedDiagnostic*>& diagnostics,
    int lineNumber,
    const UnownedStringSlice& prefix,
    const List<UnownedStringSlice>& sourceLines);

static bool checkAnnotations(
    const List<Annotation>& annotations,
    const List<ParsedDiagnostic>& diagnostics,
    const UnownedStringSlice& prefix,
    const List<UnownedStringSlice>& sourceLines,
    bool exhaustive,
    List<String>& outMissingAnnotations);

static SlangResult parseAnnotations(
    const UnownedStringSlice& sourceText,
    const UnownedStringSlice& prefix,
    List<Annotation>& outAnnotations,
    List<UnownedStringSlice>* outSourceLines)
{
    outAnnotations.clear();

    // Build the comment markers we're looking for
    StringBuilder lineMarkerBuilder;
    lineMarkerBuilder << "//" << prefix;
    String lineMarker = lineMarkerBuilder.produceString();

    StringBuilder blockStartBuilder;
    blockStartBuilder << "/*" << prefix;
    String blockStart = blockStartBuilder.produceString();

    // Split source into lines
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(sourceText, lines);

    // Store lines if requested
    if (outSourceLines)
    {
        *outSourceLines = lines;
    }

    int lastNonAnnotationLine = -1;
    bool inBlockComment = false;
    int blockCommentSourceLine = -1;

    for (Index i = 0; i < lines.getCount(); ++i)
    {
        const auto& line = lines[i];
        UnownedStringSlice trimmedLine = line.trim();

        // Check for block comment start
        if (!inBlockComment && trimmedLine.startsWith(blockStart.getUnownedSlice()))
        {
            inBlockComment = true;
            blockCommentSourceLine = lastNonAnnotationLine;
            continue;
        }

        // Check for block comment end
        if (inBlockComment && trimmedLine.startsWith(UnownedStringSlice::fromLiteral("*/")))
        {
            inBlockComment = false;
            continue;
        }

        // Inside block comment - parse as annotation
        if (inBlockComment)
        {
            // Skip empty lines
            if (trimmedLine.getLength() == 0)
                continue;

            Annotation annotation;

            // Check if this is a position-based annotation (contains ^)
            Index caretPos = line.indexOf('^');
            if (caretPos != -1)
            {
                // Position-based annotation
                annotation.type = Annotation::Type::PositionBased;

                // Count the carets to determine the span
                Index caretCount = 0;
                for (Index j = caretPos; j < line.getLength() && line[j] == '^'; ++j)
                {
                    caretCount++;
                }

                // Extract the message after the carets
                UnownedStringSlice remaining = line.tail(caretPos + caretCount).trim();

                annotation.expectedSubstring = String(remaining);
                annotation.sourceLineNumber = blockCommentSourceLine + 1; // 1-based line numbers
                // caretPos is the 0-based position of '^' in the line
                // Adding 1 converts it to a 1-based column number
                annotation.columnStart = int(caretPos + 1);
                annotation.columnEnd = int(caretPos + caretCount);

                if (annotation.expectedSubstring.getLength() > 0)
                {
                    outAnnotations.add(annotation);
                }
            }
            else
            {
                // Simple substring annotation
                annotation.type = Annotation::Type::SimpleSubstring;
                annotation.expectedSubstring = String(trimmedLine);

                if (annotation.expectedSubstring.getLength() > 0)
                {
                    outAnnotations.add(annotation);
                }
            }

            continue;
        }

        // Check for line comment annotation
        if (trimmedLine.startsWith(lineMarker.getUnownedSlice()))
        {
            Annotation annotation;

            // Skip past the marker
            UnownedStringSlice content = trimmedLine.tail(lineMarker.getLength());

            // Check if this is a position-based annotation (contains ^)
            Index caretPos = content.indexOf('^');
            if (caretPos != -1)
            {
                // Position-based annotation
                annotation.type = Annotation::Type::PositionBased;

                // Find where the carets start in the original line (not trimmed)
                Index lineCaretStart = line.indexOf('^');
                if (lineCaretStart == -1)
                {
                    // This shouldn't happen, but handle it
                    continue;
                }

                // Count the carets to determine the span
                Index caretCount = 0;
                for (Index j = caretPos; j < content.getLength() && content[j] == '^'; ++j)
                {
                    caretCount++;
                }

                // Extract the message after the carets
                Index messageStart = caretPos + caretCount;
                UnownedStringSlice message = content.tail(messageStart).trim();

                annotation.expectedSubstring = String(message);
                annotation.sourceLineNumber = lastNonAnnotationLine + 1; // 1-based line numbers
                // The caret position in the annotation line should directly correspond to the
                // column position in the source. Column numbering is 1-based.
                // lineCaretStart is the 0-based position of '^' in the annotation line.
                // Adding 1 converts it to a 1-based column number.
                annotation.columnStart = int(lineCaretStart + 1);
                annotation.columnEnd = int(lineCaretStart + caretCount);
            }
            else
            {
                // Simple substring annotation
                annotation.type = Annotation::Type::SimpleSubstring;
                annotation.expectedSubstring = String(content.trim());
            }

            if (annotation.expectedSubstring.getLength() > 0)
            {
                outAnnotations.add(annotation);
            }
        }
        else if (trimmedLine.getLength() > 0)
        {
            // Track the last non-empty, non-annotation line
            lastNonAnnotationLine = int(i);
        }
    }

    return SLANG_OK;
}

static SlangResult parseMachineReadableDiagnostics(
    const UnownedStringSlice& output,
    List<ParsedDiagnostic>& outDiagnostics)
{
    outDiagnostics.clear();

    // Split output into lines
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(output, lines);

    for (const auto& line : lines)
    {
        if (line.getLength() == 0)
            continue;

        // Machine-readable format:
        // E<code>\t<severity>\t<filename>\t<beginline>\t<begincol>\t<endline>\t<endcol>\t<message>
        List<UnownedStringSlice> parts;
        StringUtil::split(line, '\t', parts);

        if (parts.getCount() >= 8 && parts[0].startsWith(UnownedStringSlice::fromLiteral("E")))
        {
            ParsedDiagnostic diag;
            diag.errorCode = String(parts[0]);
            diag.severity = String(parts[1]);
            diag.filename = String(parts[2]);

            Int tempInt;
            if (SLANG_SUCCEEDED(StringUtil::parseInt(parts[3], tempInt)))
                diag.beginLine = int(tempInt);
            if (SLANG_SUCCEEDED(StringUtil::parseInt(parts[4], tempInt)))
                diag.beginCol = int(tempInt);
            if (SLANG_SUCCEEDED(StringUtil::parseInt(parts[5], tempInt)))
                diag.endLine = int(tempInt);
            if (SLANG_SUCCEEDED(StringUtil::parseInt(parts[6], tempInt)))
                diag.endCol = int(tempInt);

            // Message may contain tabs, so join remaining parts
            StringBuilder messageBuilder;
            for (Index i = 7; i < parts.getCount(); ++i)
            {
                if (i > 7)
                    messageBuilder << '\t';
                messageBuilder << parts[i];
            }
            diag.message = messageBuilder.produceString();

            outDiagnostics.add(diag);
        }
    }

    return SLANG_OK;
}

// Helper to generate suggested annotations for a group of diagnostics
static void generateSuggestedAnnotations(
    StringBuilder& sb,
    const List<const ParsedDiagnostic*>& diagnostics,
    int lineNumber,
    const UnownedStringSlice& prefix,
    const List<UnownedStringSlice>& sourceLines)
{
    sb << "  ⋮\n"; // Vertical ellipsis (indented)

    // Show source line (no indentation)
    if (lineNumber >= 1 && lineNumber <= sourceLines.getCount())
    {
        UnownedStringSlice sourceLine = sourceLines[lineNumber - 1];
        sb << sourceLine;
        if (!sourceLine.endsWith("\n"))
            sb << "\n";
    }

    // Calculate the prefix length: "//" + prefix
    int linePrefixLength = 2 + prefix.getLength();

    // Check if we should use block comment
    bool useBlockComment = false;
    for (const auto* diag : diagnostics)
    {
        if (diag->beginCol <= linePrefixLength)
        {
            useBlockComment = true;
            break;
        }
    }

    if (useBlockComment)
    {
        // Generate block comment format (no indentation)
        sb << "/*" << prefix << "\n";
        for (const auto* diag : diagnostics)
        {
            // Generate spacing to align caret with column
            // Caret should be at position (diag->beginCol - 1)
            StringBuilder spacingBuilder;
            int numSpaces = diag->beginCol - 1;
            for (int i = 0; i < numSpaces; ++i)
            {
                spacingBuilder << " ";
            }

            StringBuilder caretBuilder;
            int caretCount = diag->endCol - diag->beginCol + 1;
            for (int i = 0; i < caretCount; ++i)
            {
                caretBuilder << "^";
            }

            sb << spacingBuilder.getUnownedSlice() << caretBuilder.getUnownedSlice() << " "
               << diag->message << "\n";
        }
        sb << "*/\n";
    }
    else
    {
        // Generate line comment format (no indentation)
        for (const auto* diag : diagnostics)
        {
            // Generate spacing to align caret with column
            // Caret should be at position (diag->beginCol - 1)
            // Prefix takes up first linePrefixLength positions
            // So we need (diag->beginCol - 1) - linePrefixLength spaces
            StringBuilder spacingBuilder;
            int numSpaces = (diag->beginCol - 1) - linePrefixLength;
            for (int i = 0; i < numSpaces; ++i)
            {
                spacingBuilder << " ";
            }

            StringBuilder caretBuilder;
            int caretCount = diag->endCol - diag->beginCol + 1;
            for (int i = 0; i < caretCount; ++i)
            {
                caretBuilder << "^";
            }

            sb << "//" << prefix << spacingBuilder.getUnownedSlice()
               << caretBuilder.getUnownedSlice() << " " << diag->message << "\n";
        }
    }

    sb << "  ⋮\n"; // Trailing vertical ellipsis (indented)
}

static bool checkAnnotations(
    const List<Annotation>& annotations,
    const List<ParsedDiagnostic>& diagnostics,
    const UnownedStringSlice& prefix,
    const List<UnownedStringSlice>& sourceLines,
    bool exhaustive,
    List<String>& outMissingAnnotations)
{
    outMissingAnnotations.clear();

    List<bool> diagnosticMatched;
    diagnosticMatched.setCount(diagnostics.getCount());
    for (Index i = 0; i < diagnostics.getCount(); ++i)
    {
        diagnosticMatched[i] = false;
    }

    for (const auto& annotation : annotations)
    {
        bool found = false;

        if (annotation.type == Annotation::Type::SimpleSubstring)
        {
            // Simple substring matching - check if substring appears in any diagnostic
            // Can match against: message, severity, errorCode, or "severity errorCode"
            for (Index diagIdx = 0; diagIdx < diagnostics.getCount(); ++diagIdx)
            {
                if (diagnosticMatched[diagIdx])
                    continue;

                const auto& diag = diagnostics[diagIdx];
                UnownedStringSlice expected = annotation.expectedSubstring.getUnownedSlice();

                // Check message, severity, errorCode, or "severity errorCode"
                bool matches = false;
                if (diag.message.indexOf(expected) != -1)
                    matches = true;
                else if (diag.severity.indexOf(expected) != -1)
                    matches = true;
                else if (diag.errorCode.indexOf(expected) != -1)
                    matches = true;
                else
                {
                    StringBuilder combined;
                    combined << diag.severity << " " << diag.errorCode;
                    if (combined.produceString().indexOf(expected) != -1)
                        matches = true;
                }

                if (matches)
                {
                    found = true;
                    diagnosticMatched[diagIdx] = true;
                    break;
                }
            }

            if (!found)
            {
                StringBuilder sb;
                sb << "Simple substring match failed:\n";
                sb << "  Expected substring: \"" << annotation.expectedSubstring << "\"\n";

                if (diagnostics.getCount() == 0)
                {
                    sb << "  No diagnostics were produced\n";
                }
                else
                {
                    sb << "  Actual diagnostics:\n";
                    for (Index diagIdx = 0; diagIdx < diagnostics.getCount(); ++diagIdx)
                    {
                        const auto& diag = diagnostics[diagIdx];
                        sb << "    Line " << diag.beginLine << ", column ";
                        if (diag.beginCol == diag.endCol)
                            sb << diag.beginCol;
                        else
                            sb << diag.beginCol << "-" << diag.endCol;
                        sb << ": \"" << diag.message << "\"\n";
                    }

                    // Group diagnostics by line and generate suggestions
                    sb << "\n  Suggested position-based annotations you can copy:\n";
                    Dictionary<int, List<const ParsedDiagnostic*>> diagsByLine;
                    for (Index diagIdx = 0; diagIdx < diagnostics.getCount(); ++diagIdx)
                    {
                        const auto& diag = diagnostics[diagIdx];
                        if (!diagsByLine.containsKey(diag.beginLine))
                        {
                            diagsByLine.add(diag.beginLine, List<const ParsedDiagnostic*>());
                        }
                        diagsByLine[diag.beginLine].add(&diag);
                    }

                    for (const auto& [lineNum, diagList] : diagsByLine)
                    {
                        sb << "\n  After line " << lineNum << ":\n";
                        generateSuggestedAnnotations(sb, diagList, lineNum, prefix, sourceLines);
                    }
                }

                outMissingAnnotations.add(sb.produceString());
            }
        }
        else // Position-based
        {
            // Position-based matching - check line, column range, and message substring
            bool lineMatched = false;
            bool columnMatched = false;
            bool messageMatched = false;
            List<String> candidateDiagnostics;

            for (Index diagIdx = 0; diagIdx < diagnostics.getCount(); ++diagIdx)
            {
                const auto& diag = diagnostics[diagIdx];

                // Collect diagnostics on the same line for detailed reporting
                if (diag.beginLine == annotation.sourceLineNumber)
                {
                    lineMatched = true;
                    StringBuilder diagInfo;
                    diagInfo << "    Line " << diag.beginLine << ", column ";
                    if (diag.beginCol == diag.endCol)
                        diagInfo << diag.beginCol;
                    else
                        diagInfo << diag.beginCol << "-" << diag.endCol;
                    diagInfo << ": \"" << diag.message << "\"";
                    if (diagnosticMatched[diagIdx])
                        diagInfo << " (already matched)";
                    candidateDiagnostics.add(diagInfo.produceString());

                    if (diagnosticMatched[diagIdx])
                        continue;

                    // Check if this is a full match
                    if (diag.beginCol == annotation.columnStart &&
                        diag.endCol == annotation.columnEnd)
                    {
                        columnMatched = true;
                        UnownedStringSlice expected =
                            annotation.expectedSubstring.getUnownedSlice();

                        // Check message, severity, errorCode, or "severity errorCode"
                        bool matches = false;
                        if (diag.message.indexOf(expected) != -1)
                            matches = true;
                        else if (diag.severity.indexOf(expected) != -1)
                            matches = true;
                        else if (diag.errorCode.indexOf(expected) != -1)
                            matches = true;
                        else
                        {
                            StringBuilder combined;
                            combined << diag.severity << " " << diag.errorCode;
                            if (combined.produceString().indexOf(expected) != -1)
                                matches = true;
                        }

                        if (matches)
                        {
                            messageMatched = true;
                            found = true;
                            diagnosticMatched[diagIdx] = true;
                            break;
                        }
                    }
                }
            }

            if (!found)
            {
                StringBuilder sb;
                sb << "Position-based match failed:\n";
                sb << "  Expected: Line " << annotation.sourceLineNumber << ", column";
                if (annotation.columnEnd != annotation.columnStart)
                    sb << "s " << annotation.columnStart << "-" << annotation.columnEnd;
                else
                    sb << " " << annotation.columnStart;
                sb << ", message containing: \"" << annotation.expectedSubstring << "\"\n";

                if (!lineMatched)
                {
                    sb << "  No diagnostics found on line " << annotation.sourceLineNumber << "\n";

                    // Show nearby diagnostics for context
                    bool foundNearby = false;
                    for (const auto& diag : diagnostics)
                    {
                        int lineDistance = diag.beginLine - annotation.sourceLineNumber;
                        if (lineDistance >= -2 && lineDistance <= 2 && lineDistance != 0)
                        {
                            if (!foundNearby)
                            {
                                sb << "  Nearby diagnostics:\n";
                                foundNearby = true;
                            }
                            sb << "    Line " << diag.beginLine << ", column ";
                            if (diag.beginCol == diag.endCol)
                                sb << diag.beginCol;
                            else
                                sb << diag.beginCol << "-" << diag.endCol;
                            sb << ": \"" << diag.message << "\"\n";
                        }
                    }
                }
                else
                {
                    sb << "  Actual diagnostics on line " << annotation.sourceLineNumber << ":\n";
                    for (const auto& diagStr : candidateDiagnostics)
                    {
                        sb << diagStr << "\n";
                    }

                    if (columnMatched && !messageMatched)
                    {
                        sb << "  Note: Column position matched but message didn't contain expected "
                              "substring\n";
                    }
                    else if (!columnMatched)
                    {
                        sb << "  Note: Column position(s) don't match expected column";
                        if (annotation.columnEnd != annotation.columnStart)
                            sb << "s " << annotation.columnStart << "-" << annotation.columnEnd;
                        else
                            sb << " " << annotation.columnStart;
                        sb << "\n";
                    }

                    // Generate suggested annotations based on actual diagnostics
                    sb << "\n  Suggested annotations you can copy:\n";

                    // Collect diagnostics on this line
                    List<const ParsedDiagnostic*> diagsOnLine;
                    for (Index diagIdx = 0; diagIdx < diagnostics.getCount(); ++diagIdx)
                    {
                        const auto& diag = diagnostics[diagIdx];
                        if (diag.beginLine == annotation.sourceLineNumber)
                        {
                            diagsOnLine.add(&diag);
                        }
                    }

                    generateSuggestedAnnotations(
                        sb,
                        diagsOnLine,
                        annotation.sourceLineNumber,
                        prefix,
                        sourceLines);
                }

                outMissingAnnotations.add(sb.produceString());
            }
        }
    }

    // In exhaustive mode, check for diagnostics that weren't matched by any annotation
    if (exhaustive)
    {
        List<const ParsedDiagnostic*> unmatchedDiagnostics;
        for (Index i = 0; i < diagnostics.getCount(); ++i)
        {
            if (!diagnosticMatched[i])
            {
                unmatchedDiagnostics.add(&diagnostics[i]);
            }
        }

        if (unmatchedDiagnostics.getCount() > 0)
        {
            StringBuilder sb;
            sb << "Exhaustive check failed: Found " << unmatchedDiagnostics.getCount()
               << " diagnostic(s) without annotations:\n";

            // Group by line
            Dictionary<int, List<const ParsedDiagnostic*>> diagsByLine;
            for (const auto* diag : unmatchedDiagnostics)
            {
                if (!diagsByLine.containsKey(diag->beginLine))
                {
                    diagsByLine.add(diag->beginLine, List<const ParsedDiagnostic*>());
                }
                diagsByLine[diag->beginLine].add(diag);
            }

            // Show unannotated diagnostics grouped by line
            for (const auto& [lineNum, diagList] : diagsByLine)
            {
                sb << "\n  Line " << lineNum << ":\n";

                for (const auto* diag : diagList)
                {
                    sb << "    Column ";
                    if (diag->beginCol == diag->endCol)
                        sb << diag->beginCol;
                    else
                        sb << diag->beginCol << "-" << diag->endCol;
                    sb << ": \"" << diag->message << "\"\n";
                }

                // Generate suggestions for this line
                sb << "\n  Suggested annotations you can copy:\n";
                generateSuggestedAnnotations(sb, diagList, lineNum, prefix, sourceLines);
            }

            // Suggest using non-exhaustive mode as an alternative
            sb << "\n  Or add 'non-exhaustive' to skip checking unannotated diagnostics:\n";
            sb << "  //DIAGNOSTIC_TEST:SIMPLE(diag=" << prefix << ",non-exhaustive):\n";

            outMissingAnnotations.add(sb.produceString());
        }
    }

    return outMissingAnnotations.getCount() == 0;
}

// Public entry point
bool DiagnosticAnnotationUtil::checkDiagnosticAnnotations(
    const UnownedStringSlice& sourceText,
    const UnownedStringSlice& prefix,
    const UnownedStringSlice& machineReadableOutput,
    bool exhaustive,
    String& outErrorMessage)
{
    // Parse annotations from source and capture source lines
    List<Annotation> annotations;
    List<UnownedStringSlice> sourceLines;
    if (SLANG_FAILED(parseAnnotations(sourceText, prefix, annotations, &sourceLines)))
    {
        outErrorMessage = "Failed to parse diagnostic annotations";
        return false;
    }

    // Parse machine-readable diagnostics from output
    List<ParsedDiagnostic> diagnostics;
    if (SLANG_FAILED(parseMachineReadableDiagnostics(machineReadableOutput, diagnostics)))
    {
        outErrorMessage = "Failed to parse machine-readable diagnostic output";
        return false;
    }

    // Check if all annotations match diagnostics
    List<String> missingAnnotations;
    if (!checkAnnotations(
            annotations,
            diagnostics,
            prefix,
            sourceLines,
            exhaustive,
            missingAnnotations))
    {
        // Build error message
        StringBuilder sb;
        sb << "Diagnostic annotation check failed:\n";
        sb << "\n";
        for (Index i = 0; i < missingAnnotations.getCount(); ++i)
        {
            if (i > 0)
                sb << "\n";
            sb << missingAnnotations[i];
        }
        outErrorMessage = sb.produceString();
        return false;
    }

    return true;
}

} // namespace Slang
