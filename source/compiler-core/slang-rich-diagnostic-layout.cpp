// slang-rich-diagnostic-layout.cpp
#include "slang-rich-diagnostic-layout.h"

#include "slang-source-loc.h"

namespace Slang
{

String RichDiagnosticLayout::render(const RichDiagnostic& diagnostic)
{
    StringBuilder sb;

    // Collect information about all lines we need to display
    List<LineInfo> lineInfos;
    collectLineInfo(diagnostic, lineInfos);

    // Calculate gutter width based on maximum line number
    Index gutterWidth = calculateGutterWidth(lineInfos);

    // Render the header: error[E30019]: message
    renderHeader(sb, diagnostic);

    // Find the primary label for the main location pointer
    SourceLoc primaryLoc;
    for (const auto& label : diagnostic.labels)
    {
        if (label.isPrimary)
        {
            primaryLoc = label.loc;
            break;
        }
    }

    // Render primary location if we have one
    if (primaryLoc.isValid() && lineInfos.getCount() > 0)
    {
        // Find the primary line info
        for (const auto& lineInfo : lineInfos)
        {
            for (Index i = 0; i < lineInfo.labels.getCount(); i++)
            {
                if (lineInfo.labels[i].isPrimary)
                {
                    Index column =
                        lineInfo.labelColumns.getCount() > i ? lineInfo.labelColumns[i].first : 1;
                    renderPrimaryLocation(sb, lineInfo, column);
                    break;
                }
            }
        }
    }

    // Group lines by file for rendering
    String currentFile;
    for (Index lineIdx = 0; lineIdx < lineInfos.getCount(); lineIdx++)
    {
        const auto& lineInfo = lineInfos[lineIdx];

        // Add separator between different files or non-consecutive lines
        if (lineIdx > 0)
        {
            bool needsSeparator = (lineInfo.filePath != currentFile);
            if (!needsSeparator && lineIdx > 0)
            {
                // Check if lines are non-consecutive
                Index prevLine = lineInfos[lineIdx - 1].lineNumber;
                if (lineInfo.lineNumber > prevLine + 1)
                {
                    needsSeparator = true;
                }
            }

            if (needsSeparator)
            {
                // Render separator line
                sb.appendRepeatedChar(' ', gutterWidth);
                sb << " |\n";
            }
        }

        currentFile = lineInfo.filePath;

        // Render the source line
        renderSourceLine(sb, lineInfo, gutterWidth);

        // Render label rows for each label on this line
        for (Index labelIdx = 0; labelIdx < lineInfo.labels.getCount(); labelIdx++)
        {
            renderLabelRow(sb, lineInfo, gutterWidth, labelIdx);
        }
    }

    // Final separator before notes/helps
    if (diagnostic.notes.getCount() > 0 || diagnostic.helps.getCount() > 0)
    {
        sb.appendRepeatedChar(' ', gutterWidth);
        sb << " |\n";
    }

    // Render notes and helps
    renderNotesAndHelps(sb, diagnostic, gutterWidth);

    return sb.produceString();
}

void RichDiagnosticLayout::collectLineInfo(
    const RichDiagnostic& diagnostic,
    List<LineInfo>& outLineInfos)
{
    if (!m_sourceManager)
        return;

    // Helper lambda to find existing line info index
    auto findLineInfoIndex = [&outLineInfos](const String& filePath, Index lineNumber) -> Index {
        for (Index i = 0; i < outLineInfos.getCount(); i++)
        {
            if (outLineInfos[i].filePath == filePath && outLineInfos[i].lineNumber == lineNumber)
                return i;
        }
        return -1;
    };

    for (const auto& label : diagnostic.labels)
    {
        if (!label.loc.isValid())
            continue;

        SourceView* sourceView = m_sourceManager->findSourceViewRecursively(label.loc);
        if (!sourceView)
            continue;

        HumaneSourceLoc humaneLoc = sourceView->getHumaneLoc(label.loc);
        String filePath = humaneLoc.pathInfo.foundPath;
        Index lineNumber = humaneLoc.line;

        Index existingIdx = findLineInfoIndex(filePath, lineNumber);

        if (existingIdx >= 0)
        {
            // Add label to existing line info
            LineInfo& lineInfo = outLineInfos[existingIdx];
            lineInfo.labels.add(label);

            // Calculate column range for this label
            Index startCol = humaneLoc.column;
            Index endCol = startCol + 1; // Default to single character

            // Try to get token length from source
            SourceFile* sourceFile = sourceView->getSourceFile();
            if (sourceFile && sourceFile->hasContent())
            {
                UnownedStringSlice content = sourceFile->getContent();
                int offset = sourceView->getRange().getOffset(label.loc);
                if (offset >= 0 && offset < content.getLength())
                {
                    // Simple heuristic: find end of identifier/token
                    const char* start = content.begin() + offset;
                    const char* end = start;
                    const char* contentEnd = content.end();
                    while (end < contentEnd && *end != ' ' && *end != '\n' && *end != '\r' &&
                           *end != '(' && *end != ')' && *end != ',' && *end != ';')
                    {
                        end++;
                    }
                    endCol = startCol + (end - start);
                }
            }

            lineInfo.labelColumns.add(std::make_pair(startCol, endCol));
        }
        else
        {
            // Create new line info
            LineInfo lineInfo;
            lineInfo.filePath = filePath;
            lineInfo.lineNumber = lineNumber;
            lineInfo.labels.add(label);

            // Get line content
            SourceFile* sourceFile = sourceView->getSourceFile();
            if (sourceFile && sourceFile->hasContent())
            {
                UnownedStringSlice content = sourceFile->getContent();

                // Find the line content
                const char* lineStart = content.begin();
                Index currentLine = 1;
                while (currentLine < lineNumber && lineStart < content.end())
                {
                    if (*lineStart == '\n')
                        currentLine++;
                    lineStart++;
                }

                // Find line end
                const char* lineEnd = lineStart;
                while (lineEnd < content.end() && *lineEnd != '\n' && *lineEnd != '\r')
                {
                    lineEnd++;
                }

                lineInfo.content = expandTabs(UnownedStringSlice(lineStart, lineEnd));
            }

            // Calculate column range
            Index startCol = humaneLoc.column;
            Index endCol = startCol + 1;

            // Try to get better end column
            if (sourceFile && sourceFile->hasContent())
            {
                UnownedStringSlice content = sourceFile->getContent();
                int offset = sourceView->getRange().getOffset(label.loc);
                if (offset >= 0 && offset < content.getLength())
                {
                    const char* start = content.begin() + offset;
                    const char* end = start;
                    const char* contentEnd = content.end();
                    while (end < contentEnd && *end != ' ' && *end != '\n' && *end != '\r' &&
                           *end != '(' && *end != ')' && *end != ',' && *end != ';')
                    {
                        end++;
                    }
                    endCol = startCol + (end - start);
                }
            }

            lineInfo.labelColumns.add(std::make_pair(startCol, endCol));

            outLineInfos.add(lineInfo);
        }
    }

    // Sort by file path and line number
    outLineInfos.sort([](const LineInfo& a, const LineInfo& b) {
        if (a.filePath != b.filePath)
            return a.filePath < b.filePath;
        return a.lineNumber < b.lineNumber;
    });
}

void RichDiagnosticLayout::renderHeader(StringBuilder& sb, const RichDiagnostic& diagnostic)
{
    // Format: error[E30019]: message
    switch (diagnostic.severity)
    {
    case Severity::Error:
        sb << "error";
        break;
    case Severity::Warning:
        sb << "warning";
        break;
    case Severity::Note:
        sb << "note";
        break;
    case Severity::Fatal:
        sb << "fatal";
        break;
    default:
        sb << "diagnostic";
        break;
    }

    if (diagnostic.code.getLength() > 0)
    {
        sb << "[" << diagnostic.code << "]";
    }

    sb << ": " << diagnostic.message << "\n";
}

void RichDiagnosticLayout::renderPrimaryLocation(
    StringBuilder& sb,
    const LineInfo& lineInfo,
    Index column)
{
    // Format: --> file.slang:10:5
    sb << " --> " << lineInfo.filePath << ":" << lineInfo.lineNumber << ":" << column << "\n";
}

void RichDiagnosticLayout::renderSourceLine(
    StringBuilder& sb,
    const LineInfo& lineInfo,
    Index gutterWidth)
{
    // Format: 10 | source code here
    StringBuilder lineNumStr;
    lineNumStr << lineInfo.lineNumber;

    // Right-align line number
    Index padding = gutterWidth - lineNumStr.getLength();
    sb.appendRepeatedChar(' ', padding);
    sb << lineNumStr.getUnownedSlice();
    sb << " | ";
    sb << lineInfo.content;
    sb << "\n";
}

void RichDiagnosticLayout::renderLabelRow(
    StringBuilder& sb,
    const LineInfo& lineInfo,
    Index gutterWidth,
    Index labelIndex)
{
    if (labelIndex >= lineInfo.labels.getCount())
        return;

    const auto& label = lineInfo.labels[labelIndex];
    const auto& columns = lineInfo.labelColumns[labelIndex];

    // Format:    |         ^^^^ message here
    sb.appendRepeatedChar(' ', gutterWidth);
    sb << " | ";

    // Add spaces up to the start column
    Index startCol = columns.first;
    Index endCol = columns.second;

    // Ensure valid range
    if (startCol < 1)
        startCol = 1;
    if (endCol < startCol)
        endCol = startCol + 1;

    sb.appendRepeatedChar(' ', startCol - 1);

    // Add underline characters
    char underlineChar = label.isPrimary ? '^' : '-';
    Index underlineLen = endCol - startCol;
    if (underlineLen < 1)
        underlineLen = 1;
    sb.appendRepeatedChar(underlineChar, underlineLen);

    // Add the label message
    if (label.message.getLength() > 0)
    {
        sb << " " << label.message;
    }

    sb << "\n";
}

void RichDiagnosticLayout::renderNotesAndHelps(
    StringBuilder& sb,
    const RichDiagnostic& diagnostic,
    Index gutterWidth)
{
    // Render notes
    for (const auto& note : diagnostic.notes)
    {
        sb.appendRepeatedChar(' ', gutterWidth);
        sb << " = note: " << note << "\n";
    }

    // Render helps
    for (const auto& help : diagnostic.helps)
    {
        sb.appendRepeatedChar(' ', gutterWidth);
        sb << " = help: " << help << "\n";
    }
}

Index RichDiagnosticLayout::calculateGutterWidth(const List<LineInfo>& lineInfos)
{
    Index maxLineNum = 1;
    for (const auto& lineInfo : lineInfos)
    {
        if (lineInfo.lineNumber > maxLineNum)
            maxLineNum = lineInfo.lineNumber;
    }

    // Calculate digits needed
    Index digits = 1;
    Index num = maxLineNum;
    while (num >= 10)
    {
        num /= 10;
        digits++;
    }

    // Minimum width of 2
    return digits < 2 ? 2 : digits;
}

String RichDiagnosticLayout::expandTabs(const UnownedStringSlice& line)
{
    StringBuilder result;
    Index column = 0;

    for (const char* p = line.begin(); p < line.end(); p++)
    {
        if (*p == '\t')
        {
            // Add spaces to next tab stop
            Index nextTabStop = ((column / m_options.tabSize) + 1) * m_options.tabSize;
            Index spacesToAdd = nextTabStop - column;
            result.appendRepeatedChar(' ', spacesToAdd);
            column = nextTabStop;
        }
        else
        {
            result.appendChar(*p);
            column++;
        }
    }

    return result.produceString();
}

Index RichDiagnosticLayout::calculateDisplayColumn(const UnownedStringSlice& line, Index byteOffset)
{
    Index column = 1;

    for (Index i = 0; i < byteOffset && i < line.getLength(); i++)
    {
        if (line[i] == '\t')
        {
            // Move to next tab stop
            column = ((column / m_options.tabSize) + 1) * m_options.tabSize + 1;
        }
        else
        {
            column++;
        }
    }

    return column;
}

} // namespace Slang