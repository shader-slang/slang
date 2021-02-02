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

static SlangResult _getSlangDiagnosticType(const UnownedStringSlice& type, DownstreamDiagnostic::Type& outType, Int& outCode)
{
    static const UnownedStringSlice prefixes[] =
    {
        UnownedStringSlice::fromLiteral("note"),
        UnownedStringSlice::fromLiteral("warning"),
        UnownedStringSlice::fromLiteral("error"),
        UnownedStringSlice::fromLiteral("fatal error"),
        UnownedStringSlice::fromLiteral("internal error"),
        UnownedStringSlice::fromLiteral("unknown error")
    };

    Int index = -1;

    for (Index i = 0; i < SLANG_COUNT_OF(prefixes); ++i)
    {
        const auto& prefix = prefixes[i];
        if (type.startsWith(prefix))
        {
            index = i;
            break;
        }
    }

    
    switch (index)
    {
        case -1:    return SLANG_FAIL;
        case 0:     outType = DownstreamDiagnostic::Type::Info; break;
        case 1:     outType = DownstreamDiagnostic::Type::Warning; break;
        default:    outType = DownstreamDiagnostic::Type::Error; break;
    }

    outCode = 0;

    UnownedStringSlice tail = type.tail(prefixes[index].getLength()).trim();
    if (tail.getLength() > 0)
    {
        SLANG_RETURN_ON_FAIL(StringUtil::parseInt(tail, outCode));
    }

    return SLANG_OK;
}

static bool _isSlangDiagnostic(const UnownedStringSlice& line)
{
    /*
    tests/diagnostics/accessors.slang(11): error 31101: accessors other than 'set' must not have parameters
    */

    UnownedStringSlice initial = StringUtil::getAtInSplit(line, ':', 0);

    // Handle if path has : 
    const Index typeIndex = (initial.getLength() == 1 && CharUtil::isAlpha(initial[0])) ? 2 : 1;
    // Extract the type/code slice
    UnownedStringSlice typeSlice = StringUtil::getAtInSplit(line, ':', typeIndex).trim();

    DownstreamDiagnostic::Type type;
    Int code;
    return SLANG_SUCCEEDED(_getSlangDiagnosticType(typeSlice, type, code));
}

/* static */SlangResult ParseDiagnosticUtil::parseSlangLine(const UnownedStringSlice& line, List<UnownedStringSlice>& lineSlices, DownstreamDiagnostic& outDiagnostic)
{
    /*
    tests/diagnostics/accessors.slang(11): error 31101: accessors other than 'set' must not have parameters
    */

    // Can be larger than 3, because might be : in the actual error text
    if (lineSlices.getCount() < 3)
    {
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(splitPathLocation(lineSlices[0], outDiagnostic));
    Int code;
    SLANG_RETURN_ON_FAIL(_getSlangDiagnosticType(lineSlices[1], outDiagnostic.type, code));

    if (code != 0)
    {
        StringBuilder buf;
        buf << code;
        outDiagnostic.code = buf.ProduceString();
    }

    outDiagnostic.text = UnownedStringSlice(lineSlices[2].begin(), line.end());
    return SLANG_OK;
}

/* static */ SlangResult _splitSlangDiagnosticLine(const UnownedStringSlice& line, List<UnownedStringSlice>& outSlices)
{
    StringUtil::split(line, ':', outSlices);

    const Int pathIndex = 0;
    UnownedStringSlice pathStart = outSlices[pathIndex].trim();
    if (pathStart.getLength() == 1 && CharUtil::isAlpha(pathStart[0]))
    {
        // Splice back together
        outSlices[pathIndex] = UnownedStringSlice(outSlices[pathIndex].begin(), outSlices[pathIndex + 1].end());
        outSlices.removeAt(pathIndex + 1);
    }
    return SLANG_OK;
}

/* Given a downstream comoiler, handle special cases when splitting the diagnostic line (such as handling : in path) */
/* static */ SlangResult _splitCompilerDiagnosticLine(SlangPassThrough downstreamCompiler, const UnownedStringSlice& line, UnownedStringSlice& linePrefix, List<UnownedStringSlice>& outSlices)
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

    bool isSlang = false;
    SlangPassThrough downstreamCompiler = SLANG_PASS_THROUGH_NONE;
    UnownedStringSlice linePrefix;

    List<UnownedStringSlice> splitLine;

    UnownedStringSlice text(inText), line;
    while (StringUtil::extractLine(text, line))
    {
        UnownedStringSlice initial = StringUtil::getAtInSplit(line, ':', 0);

        if (isSlang == false && downstreamCompiler == SLANG_PASS_THROUGH_NONE)
        {
            if (_isSlangDiagnostic(line))
            {
                isSlang = true;
            }
            else
            {
                // First entry that begins with a numeral indicates the version number
                if (SLANG_FAILED(_findDownstreamCompiler(initial, downstreamCompiler)))
                {
                    continue;
                }
  
                linePrefix = TypeTextUtil::getPassThroughAsHumanText(downstreamCompiler);
            }
        }

        if (!isSlang)
        {
            // If it's not slang then, we must have a defined downstream compiler
            SLANG_ASSERT(downstreamCompiler != SLANG_PASS_THROUGH_NONE);

            if (line.indexOf(':') < 0 )
            {
                _addDiagnosticNote(line, outDiagnostics);
                continue;
            }

            if (SLANG_FAILED(_splitCompilerDiagnosticLine(downstreamCompiler, line, linePrefix, splitLine)))
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
        }
        else
        {
           if (SLANG_FAILED(_splitSlangDiagnosticLine(line, splitLine)))
           {
               _addDiagnosticNote(line, outDiagnostics);
               continue;
           }
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
                if (isSlang)
                {
                    parseRes = parseSlangLine(line, splitLine, diagnostic);
                }
                else
                {
                    parseRes = parseGenericLine(line, splitLine, diagnostic);
                }
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

static UnownedStringSlice _getEquals(const UnownedStringSlice& in)
{
    Index equalsIndex = in.indexOf('=');
    if (equalsIndex < 0)
    {
        return UnownedStringSlice();
    }
    return in.tail(equalsIndex + 1).trim();
}

/* static */SlangResult ParseDiagnosticUtil::parseOutputInfo(const UnownedStringSlice& inText, OutputInfo& out)
{
    enum State
    {
        Normal,
        InStdError,
        InStdOut,
    };

    UnownedStringSlice resultCodePrefix = UnownedStringSlice::fromLiteral("result code");
    UnownedStringSlice stdErrorPrefix = UnownedStringSlice::fromLiteral("standard error");
    UnownedStringSlice stdOutputPrefix = UnownedStringSlice::fromLiteral("standard output");

    
    List<UnownedStringSlice> lines;

    State state = State::Normal;

    UnownedStringSlice text(inText), line;
    while (StringUtil::extractLine(text, line))
    {
        switch (state)
        {
            case State::Normal:
            {
                if (line.startsWith(resultCodePrefix))
                {
                    // Split past the equal
                    const UnownedStringSlice valueSlice = _getEquals(line.tail(resultCodePrefix.getLength()));
                    Int value;
                    SLANG_RETURN_ON_FAIL(StringUtil::parseInt(valueSlice, value));
                    out.resultCode = int(value);
                }
                else
                {
                    UnownedStringSlice* startsWith = nullptr;
                    if (line.startsWith(stdErrorPrefix))
                    {
                        startsWith = &stdErrorPrefix;
                    }
                    else if (line.startsWith(stdOutputPrefix))
                    {
                        startsWith = &stdOutputPrefix;
                    }

                    if (startsWith)
                    {
                        // Clear the lines buffer
                        lines.clear();

                        UnownedStringSlice valueSlice = _getEquals(line.tail(startsWith->getLength()));
                        if (!valueSlice.isChar('{'))
                        {
                            return SLANG_FAIL;
                        }
                        // Okay we now inside std out or std error, so update the state
                        state = (startsWith == &stdErrorPrefix) ? State::InStdError : State::InStdOut;
                    }
                }
                break;
            }
            case State::InStdError:
            case State::InStdOut:
            {
                if (line == "}")
                {
                    String& dst = state == State::InStdError ? out.stdError : out.stdOut;
                    if (lines.getCount() > 0)
                    {
                        dst = UnownedStringSlice(lines[0].begin(), lines.getLast().end());
                    }
                    state = State::Normal;
                }
                else
                {
                    lines.add(line);
                }
            }
        }
    }

    return (state == State::Normal) ? SLANG_OK : SLANG_FAIL;
}

