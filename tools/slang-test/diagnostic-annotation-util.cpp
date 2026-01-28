// diagnostic-annotation-util.cpp
#include "diagnostic-annotation-util.h"
#include "../../source/core/slang-string-util.h"

namespace Slang
{

// Internal structures
struct Diagnostic
{
    String errorCode;     // e.g., "E20101"
    String severity;      // e.g., "warning", "error"
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
        SimpleSubstring,  // Just check substring appears somewhere
        PositionBased    // Check specific line/column position and message
    };

    Type type = Type::SimpleSubstring;
    String expectedSubstring;  // Message substring to match

    // For position-based annotations:
    int sourceLineNumber = 0;   // Line number in source where diagnostic should appear
    int columnStart = 0;        // Starting column (1-based)
    int columnEnd = 0;          // Ending column (1-based)
};

// Internal functions
static SlangResult parseAnnotations(
    const UnownedStringSlice& sourceText,
    const UnownedStringSlice& prefix,
    List<Annotation>& outAnnotations);

static SlangResult parseMachineReadableDiagnostics(
    const UnownedStringSlice& output,
    List<Diagnostic>& outDiagnostics);

static bool checkAnnotations(
    const List<Annotation>& annotations,
    const List<Diagnostic>& diagnostics,
    List<String>& outMissingAnnotations);

static SlangResult parseAnnotations(
    const UnownedStringSlice& sourceText,
    const UnownedStringSlice& prefix,
    List<Annotation>& outAnnotations)
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
    List<Diagnostic>& outDiagnostics)
{
    outDiagnostics.clear();

    // Split output into lines
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(output, lines);

    for (const auto& line : lines)
    {
        if (line.getLength() == 0)
            continue;

        // Machine-readable format: E<code>\t<severity>\t<filename>\t<beginline>\t<begincol>\t<endline>\t<endcol>\t<message>
        List<UnownedStringSlice> parts;
        StringUtil::split(line, '\t', parts);

        if (parts.getCount() >= 8 && parts[0].startsWith(UnownedStringSlice::fromLiteral("E")))
        {
            Diagnostic diag;
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

static bool checkAnnotations(
    const List<Annotation>& annotations,
    const List<Diagnostic>& diagnostics,
    List<String>& outMissingAnnotations)
{
    outMissingAnnotations.clear();

    for (const auto& annotation : annotations)
    {
        bool found = false;

        if (annotation.type == Annotation::Type::SimpleSubstring)
        {
            // Simple substring matching - check if substring appears in any diagnostic
            for (const auto& diag : diagnostics)
            {
                if (diag.message.indexOf(annotation.expectedSubstring.getUnownedSlice()) != -1)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                StringBuilder sb;
                sb << "Simple substring match failed: '" << annotation.expectedSubstring << "'";
                outMissingAnnotations.add(sb.produceString());
            }
        }
        else // Position-based
        {
            // Position-based matching - check line, column range, and message substring
            for (const auto& diag : diagnostics)
            {
                // Check if line number matches
                if (diag.beginLine != annotation.sourceLineNumber)
                    continue;

                // Check if column range matches (both are 1-based)
                if (diag.beginCol != annotation.columnStart)
                    continue;

                if (diag.endCol != annotation.columnEnd)
                    continue;

                // Check if message contains expected substring
                if (diag.message.indexOf(annotation.expectedSubstring.getUnownedSlice()) != -1)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                StringBuilder sb;
                sb << "Position-based match failed at line " << annotation.sourceLineNumber
                   << ", columns " << annotation.columnStart;
                if (annotation.columnEnd != annotation.columnStart)
                    sb << "-" << annotation.columnEnd;
                sb << ": '" << annotation.expectedSubstring << "'";
                outMissingAnnotations.add(sb.produceString());
            }
        }
    }

    return outMissingAnnotations.getCount() == 0;
}

// Public entry point
bool DiagnosticAnnotationUtil::checkDiagnosticAnnotations(
    const UnownedStringSlice& sourceText,
    const UnownedStringSlice& prefix,
    const UnownedStringSlice& machineReadableOutput,
    String& outErrorMessage)
{
    // Parse annotations from source
    List<Annotation> annotations;
    if (SLANG_FAILED(parseAnnotations(sourceText, prefix, annotations)))
    {
        outErrorMessage = "Failed to parse diagnostic annotations";
        return false;
    }

    // Parse machine-readable diagnostics from output
    List<Diagnostic> diagnostics;
    if (SLANG_FAILED(parseMachineReadableDiagnostics(machineReadableOutput, diagnostics)))
    {
        outErrorMessage = "Failed to parse machine-readable diagnostic output";
        return false;
    }

    // Check if all annotations match diagnostics
    List<String> missingAnnotations;
    if (!checkAnnotations(annotations, diagnostics, missingAnnotations))
    {
        // Build error message
        StringBuilder sb;
        sb << "Diagnostic annotation check failed:\n";
        for (const auto& missing : missingAnnotations)
        {
            sb << "  " << missing << "\n";
        }
        outErrorMessage = sb.produceString();
        return false;
    }

    return true;
}

} // namespace Slang
