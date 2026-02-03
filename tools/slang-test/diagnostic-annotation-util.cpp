// diagnostic-annotation-util.cpp
#include "diagnostic-annotation-util.h"
#include "../../source/core/slang-string-util.h"

namespace Slang
{

SlangResult DiagnosticAnnotationUtil::parseAnnotations(
    const UnownedStringSlice& sourceText,
    const UnownedStringSlice& prefix,
    List<String>& outAnnotations)
{
    outAnnotations.clear();

    // Build the comment prefix we're looking for: "//" + prefix + " "
    StringBuilder markerBuilder;
    markerBuilder << "//" << prefix;
    String marker = markerBuilder.produceString();

    // Split source into lines
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(sourceText, lines);

    for (const auto& line : lines)
    {
        // Trim leading whitespace
        UnownedStringSlice trimmedLine = line.trim();

        // Check if line starts with our marker
        if (trimmedLine.startsWith(marker.getUnownedSlice()))
        {
            // Extract the expected substring after the marker
            // Skip past the marker
            UnownedStringSlice remaining =
                trimmedLine.tail(marker.getLength());

            // Trim any whitespace after the marker
            remaining = remaining.trim();

            // Add the expected substring
            if (remaining.getLength() > 0)
            {
                outAnnotations.add(String(remaining));
            }
        }
    }

    return SLANG_OK;
}

bool DiagnosticAnnotationUtil::checkAnnotations(
    const List<String>& annotations,
    const UnownedStringSlice& output,
    List<String>& outMissingAnnotations)
{
    outMissingAnnotations.clear();

    // Check each annotation appears in the output
    for (const auto& annotation : annotations)
    {
        if (output.indexOf(annotation.getUnownedSlice()) == -1)
        {
            outMissingAnnotations.add(annotation);
        }
    }

    return outMissingAnnotations.getCount() == 0;
}

} // namespace Slang
