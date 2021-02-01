// parse-diagnostic-util.cpp

#include "parse-diagnostic-util.h"

#include "../../source/core/slang-hex-dump-util.h"
#include "../../source/core/slang-type-text-util.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-byte-encode-util.h"
#include "../../source/core/slang-char-util.h"

#include "../../source/core/slang-downstream-compiler.h"

using namespace Slang;

/* static */SlangResult ParseDiagnosticUtil::splitPathLocation(const UnownedStringSlice& pathLocation, DownstreamDiagnostic& outDiagnostic)
{
    const Index lineStartIndex = pathLocation.lastIndexOf('(');
    if (lineStartIndex >= 0)
    {
        outDiagnostic.filePath = UnownedStringSlice(pathLocation.head(lineStartIndex).trim());

        const UnownedStringSlice tail = pathLocation.tail(lineStartIndex + 1);
        const Index lineEndIndex = tail.indexOf(')');

        if (lineEndIndex >= 0)
        {
            // Extract the location info
            UnownedStringSlice locationSlice(tail.begin(), tail.begin() + lineEndIndex);
            List<UnownedStringSlice> locationSlices;
            StringUtil::split(locationSlice, ',', locationSlices);

            Int locationLine = 0;
            Int locationCol = 0;

            SLANG_RETURN_ON_FAIL(StringUtil::parseInt(locationSlices[0], locationLine));
            if (locationSlices.getCount() > 1)
            {
                SLANG_RETURN_ON_FAIL(StringUtil::parseInt(locationSlices[1], locationCol));
            }

            outDiagnostic.fileLine = locationLine;
        }
    }
    else
    {
        outDiagnostic.filePath = pathLocation;
    }
    return SLANG_OK;
}

/* static */SlangResult ParseDiagnosticUtil::parseFXCLine(const UnownedStringSlice& line,  List<UnownedStringSlice>& lineSlices, DownstreamDiagnostic& outDiagnostic)
{
    SLANG_RETURN_ON_FAIL(splitPathLocation(lineSlices[1], outDiagnostic));

    if (lineSlices.getCount() > 2)
    {
        UnownedStringSlice errorSlice = lineSlices[2].trim();
        Index spaceIndex = errorSlice.indexOf(' ');
        if (spaceIndex >= 0)
        {
            UnownedStringSlice diagnosticType = errorSlice.head(spaceIndex);
            UnownedStringSlice code = errorSlice.tail(spaceIndex + 1).trim();

            if (diagnosticType == "warning")
            {
                outDiagnostic.type = DownstreamDiagnostic::Type::Warning;
            }

            outDiagnostic.code = code;
        }
        else
        {
            outDiagnostic.code = errorSlice;
        }

        if (lineSlices.getCount() > 3)
        {
            outDiagnostic.text = UnownedStringSlice(lineSlices[3].begin(), line.end());
        }
    }

    return SLANG_OK;
}

/* static */SlangResult ParseDiagnosticUtil::parseDXCLine(const UnownedStringSlice& line,  List<UnownedStringSlice>& lineSlices, DownstreamDiagnostic& outDiagnostic)
{
    /*     dxc: tests/diagnostics/syntax-error-intrinsic.slang:14:2: error: expected expression */

    outDiagnostic.filePath = lineSlices[1];

    SLANG_RETURN_ON_FAIL(StringUtil::parseInt(lineSlices[2], outDiagnostic.fileLine));

    Int lineCol;
    SLANG_RETURN_ON_FAIL(StringUtil::parseInt(lineSlices[3], lineCol));

    UnownedStringSlice typeSlice = lineSlices[4].trim();

    if (typeSlice == UnownedStringSlice::fromLiteral("warning"))
    {
        outDiagnostic.type = DownstreamDiagnostic::Type::Warning;
    }

    // The rest of the line
    outDiagnostic.text = UnownedStringSlice(lineSlices[5].begin(), line.end());
    return SLANG_OK;
}

/* static */ SlangResult ParseDiagnosticUtil::parseGlslangLine(const UnownedStringSlice& line, List<UnownedStringSlice>& lineSlices, DownstreamDiagnostic& outDiagnostic)
{
    if (lineSlices.getCount() < 5)
    {
        return SLANG_FAIL;
    }

    /*  glslang: ERROR: tests/diagnostics/syntax-error-intrinsic.slang:13: '@' : unexpected token
    */

    UnownedStringSlice typeSlice = lineSlices[1].trim();
    if (typeSlice.caseInsensitiveEquals(UnownedStringSlice::fromLiteral("warning")))
    {
        outDiagnostic.type = DownstreamDiagnostic::Type::Warning;
    }

    outDiagnostic.filePath = lineSlices[2];

    SLANG_RETURN_ON_FAIL(StringUtil::parseInt(lineSlices[3], outDiagnostic.fileLine));
    outDiagnostic.text = UnownedStringSlice(lineSlices[4].begin(), line.end());
    return SLANG_OK;
}

/* static */SlangResult ParseDiagnosticUtil::parseGenericLine(const UnownedStringSlice& line, List<UnownedStringSlice>& lineSlices, DownstreamDiagnostic& outDiagnostic)
{
    /* Visual Studio 14.0: e:\git\somewhere\tests\diagnostics\syntax-error-intrinsic.slang(13): error C2018:  unknown character '0x40' */

    UnownedStringSlice errorSlice = lineSlices[2].trim();
    UnownedStringSlice typeSlice = StringUtil::getAtInSplit(errorSlice, ' ', 0);
    if (typeSlice == UnownedStringSlice::fromLiteral("warning"))
    {
        outDiagnostic.type = DownstreamDiagnostic::Type::Warning;
    }
    else if (typeSlice == UnownedStringSlice::fromLiteral("info"))
    {
        outDiagnostic.type = DownstreamDiagnostic::Type::Info;
    }

    // Get the code
    outDiagnostic.code = StringUtil::getAtInSplit(errorSlice, ' ', 1);

    // Get the location info
    SLANG_RETURN_ON_FAIL(splitPathLocation(lineSlices[1], outDiagnostic));

    outDiagnostic.text = UnownedStringSlice(lineSlices[3].begin(), line.end());

    return SLANG_OK;
}

/* static */ SlangResult ParseDiagnosticUtil::splitCompilerDiagnosticLine(SlangPassThrough downstreamCompiler, const UnownedStringSlice& line, UnownedStringSlice& linePrefix, List<UnownedStringSlice>& outSlices)
{
    /*
    glslang: ERROR: tests/diagnostics/syntax-error-intrinsic.slang:13: '@' : unexpected token
    dxc: tests/diagnostics/syntax-error-intrinsic.slang:14:2: error: expected expression
    fxc: tests/diagnostics/syntax-error-intrinsic.slang(14,2): error X3000: syntax error: unexpected token '@'
    Visual Studio 14.0: e:\git\somewhere\tests\diagnostics\syntax-error-intrinsic.slang(13): error C2018:  unknown character '0x40'
    NVRTC 11.0: tests/diagnostics/syntax-error-intrinsic.slang(13): error : unrecognized token
    */

    Int pathIndex = 1;

    StringUtil::split(line, ':', outSlices);

    if (downstreamCompiler == SLANG_PASS_THROUGH_GLSLANG)
    {
        // If we don't have the prefix then add it
        if (!outSlices[0].trim().startsWith(linePrefix))
        {
            outSlices.insert(0, linePrefix);
        }
        pathIndex = 2;
    }

    // Make sure this seems plausible
    if (outSlices.getCount() > pathIndex + 1)
    {
        UnownedStringSlice pathStart = outSlices[pathIndex].trim();

        if (pathStart.getLength() == 1 && CharUtil::isAlpha(pathStart[0]))
        {
            // Splice back together
            outSlices[pathIndex] = UnownedStringSlice(outSlices[pathIndex].begin(), outSlices[pathIndex + 1].end());
            outSlices.removeAt(pathIndex + 1);
        }
    }

    return SLANG_OK;
}

static void _addDiagnosticNote(const UnownedStringSlice& in, List<DownstreamDiagnostic>& outDiagnostics)
{
    // Don't bother adding an empty line
    if (in.trim().getLength() == 0)
    {
        return;
    }

    // Make it a note on the output
    DownstreamDiagnostic diagnostic;
    diagnostic.reset();
    diagnostic.type = DownstreamDiagnostic::Type::Info;
    diagnostic.text = in;
    outDiagnostics.add(diagnostic);
}

static SlangResult _findDownstreamCompiler(const UnownedStringSlice& slice, SlangPassThrough& outDownstreamCompiler)
{
    for (Index i = SLANG_PASS_THROUGH_NONE + 1; i < SLANG_PASS_THROUGH_COUNT_OF; ++i)
    {
        const SlangPassThrough downstreamCompiler = SlangPassThrough(i);
        UnownedStringSlice name = TypeTextUtil::getPassThroughAsHumanText(downstreamCompiler);

        if (slice.startsWith(name))
        {
            outDownstreamCompiler = downstreamCompiler;
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
}

/* static */SlangResult ParseDiagnosticUtil::parseDiagnostics(const UnownedStringSlice& inText, List<DownstreamDiagnostic>& outDiagnostics)
{
    // TODO(JS):
    // As it stands output of downstream compilers isn't standardized. This should be improved upon, and perhaps
    // we should have a function that will parse the standardized output
    // Currently dxc/fxc/glslang, use a different downstream path

    // We look for the first l

    SlangPassThrough downstreamCompiler = SLANG_PASS_THROUGH_NONE;
    UnownedStringSlice linePrefix;

    List<UnownedStringSlice> splitLine;

    UnownedStringSlice text(inText), line;
    while (StringUtil::extractLine(text, line))
    {
        UnownedStringSlice initial = StringUtil::getAtInSplit(line, ':', 0);
       
        if (downstreamCompiler == SLANG_PASS_THROUGH_NONE)
        {
            // First entry that begins with a numeral indicates the version number
            if (SLANG_FAILED(_findDownstreamCompiler(initial, downstreamCompiler)))
            {
                continue;
            }
         
            linePrefix = TypeTextUtil::getPassThroughAsHumanText(downstreamCompiler);
        }

        if (line.indexOf(':') < 0 )
        {
            _addDiagnosticNote(line, outDiagnostics);
            continue;
        }

        if (SLANG_FAILED(splitCompilerDiagnosticLine(downstreamCompiler, line, linePrefix, splitLine)))
        {
            _addDiagnosticNote(line, outDiagnostics);
            continue;
        }

        // If doesn't have prefix, just add as note
        if (!splitLine[0].trim().startsWith(linePrefix))
        {
            _addDiagnosticNote(line, outDiagnostics);
            continue;
        }

        DownstreamDiagnostic diagnostic;
        diagnostic.type = DownstreamDiagnostic::Type::Error;
        diagnostic.stage = DownstreamDiagnostic::Stage::Compile;
        diagnostic.fileLine = 0;
        
        SlangResult parseRes;

        switch (downstreamCompiler)
        {
            case SLANG_PASS_THROUGH_FXC:
            {
                parseRes = parseFXCLine(line, splitLine, diagnostic);
                break;
            }
            case SLANG_PASS_THROUGH_DXC:
            {
                parseRes = parseDXCLine(line, splitLine, diagnostic);
                break;
            }
            case SLANG_PASS_THROUGH_GLSLANG:
            {
                parseRes = parseGlslangLine(line, splitLine, diagnostic);
                break;
            }
            default:
            {
                parseRes = parseGenericLine(line, splitLine, diagnostic);
                break;
            }
        }

        // If couldn't parse, just add as a note
        if (SLANG_FAILED(parseRes))
        {
            _addDiagnosticNote(line, outDiagnostics);
        }
        else
        {
            outDiagnostics.add(diagnostic);
        }
    }

    return SLANG_OK;
}
