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

            UnownedStringSlice slices[2];
            const Index numSlices = StringUtil::split(locationSlice, ',', 2, slices);
            Int locationIndex[2] = { 0, 0 };

            for (Index i = 0; i < numSlices; ++i)
            {
                SLANG_RETURN_ON_FAIL(StringUtil::parseInt(slices[i], locationIndex[i]));
            }

            // Store the line
            outDiagnostic.fileLine = locationIndex[0];
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

static SlangResult _getSlangDiagnosticType(const UnownedStringSlice& inText, DownstreamDiagnostic::Type& outType, Int& outCode)
{
    UnownedStringSlice text(inText.trim());

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
        if (text.startsWith(prefix))
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

    UnownedStringSlice tail = text.tail(prefixes[index].getLength()).trim();
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
    UnownedStringSlice typeSlice = StringUtil::getAtInSplit(line, ':', typeIndex);

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

/* static */ SlangResult ParseDiagnosticUtil::splitDiagnosticLine(const CompilerIdentity& compilerIdentity, const UnownedStringSlice& line, UnownedStringSlice& linePrefix, List<UnownedStringSlice>& outSlices)
{
    /*
    glslang: ERROR: tests/diagnostics/syntax-error-intrinsic.slang:13: '@' : unexpected token
    dxc: tests/diagnostics/syntax-error-intrinsic.slang:14:2: error: expected expression
    fxc: tests/diagnostics/syntax-error-intrinsic.slang(14,2): error X3000: syntax error: unexpected token '@'
    Visual Studio 14.0: e:\git\somewhere\tests\diagnostics\syntax-error-intrinsic.slang(13): error C2018:  unknown character '0x40'
    NVRTC 11.0: tests/diagnostics/syntax-error-intrinsic.slang(13): error : unrecognized token
    tests/diagnostics/accessors.slang(11): error 31101: accessors other than 'set' must not have parameters
    */


    // Need to determine where the path is located, and that depends on the compiler
    Int pathIndex = 1;

    StringUtil::split(line, ':', outSlices);
    if (compilerIdentity.m_type == CompilerIdentity::Slang)
    {
        pathIndex = 0;
    }
    else if (compilerIdentity.m_type == CompilerIdentity::DownstreamCompiler)
    {
        if (compilerIdentity.m_downstreamCompiler == SLANG_PASS_THROUGH_GLSLANG)
        {
            // We need to special case GLSL output for lines without the prefix, we add it
            if (!outSlices[0].trim().startsWith(linePrefix))
            {
                outSlices.insert(0, linePrefix);
            }
            pathIndex = 2;
        }
    }

    // Now we want to fix up a path as might have drive letter, and therefore :
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

    // If there's nothing previous, we'll ignore too, as note should be in addition to
    // a pre-existing error/warning
    if (outDiagnostics.getCount() == 0)
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

/* static */SlangResult ParseDiagnosticUtil::identifyCompiler(const UnownedStringSlice& inText, CompilerIdentity& outIdentity)
{
    outIdentity = CompilerIdentity();

    UnownedStringSlice text(inText), line;
    while (StringUtil::extractLine(text, line))
    {
        UnownedStringSlice initial = StringUtil::getAtInSplit(line, ':', 0);

        if (_isSlangDiagnostic(line))
        {
            outIdentity = CompilerIdentity::makeSlang();
            return SLANG_OK;
        }
        else
        {
            SlangPassThrough downstreamCompiler;
            // First entry that begins with a numeral indicates the version number
            if (SLANG_SUCCEEDED(_findDownstreamCompiler(initial, downstreamCompiler)))
            {
                outIdentity = CompilerIdentity::make(downstreamCompiler);
                return SLANG_OK;
            }
        }
    }

    return SLANG_FAIL;
}

/* static */ParseDiagnosticUtil::LineParser ParseDiagnosticUtil::getLineParser(const CompilerIdentity& compilerIdentity)
{
    if (compilerIdentity.m_type == CompilerIdentity::Slang)
    {
        return &parseSlangLine;
    }
    else if (compilerIdentity.m_type == CompilerIdentity::DownstreamCompiler)
    {
        switch (compilerIdentity.m_downstreamCompiler)
        {
            case SLANG_PASS_THROUGH_FXC:        return &parseFXCLine;
            case SLANG_PASS_THROUGH_DXC:        return &parseDXCLine;
            case SLANG_PASS_THROUGH_GLSLANG:    return &parseGlslangLine;
            default:                            return &parseGenericLine;
        }
    }
    return nullptr;
}

/* static */SlangResult ParseDiagnosticUtil::parseDiagnostics(const UnownedStringSlice& inText, List<DownstreamDiagnostic>& outDiagnostics)
{
    // TODO(JS):
    // As it stands output of downstream compilers isn't standardized. This can be improved upon - and if so
    // we should have a function that will parse the standardized output
    // Currently dxc/fxc/glslang, use a different downstream path

    CompilerIdentity compilerIdentity;
    SLANG_RETURN_ON_FAIL(ParseDiagnosticUtil::identifyCompiler(inText, compilerIdentity));

    auto lineParser = getLineParser(compilerIdentity);
    if (!lineParser)
    {
        return SLANG_FAIL;
    }

    UnownedStringSlice linePrefix = TypeTextUtil::getPassThroughAsHumanText(compilerIdentity.m_downstreamCompiler);

    List<UnownedStringSlice> splitLine;

    UnownedStringSlice text(inText), line;
    while (StringUtil::extractLine(text, line))
    {
        // Downstream compilers have prefix on lines, if they start a diagnostic
        if (compilerIdentity.m_type == CompilerIdentity::Type::DownstreamCompiler)
        {
            // If it's not slang then, we must have a defined downstream compiler
            SLANG_ASSERT(compilerIdentity.m_downstreamCompiler != SLANG_PASS_THROUGH_NONE);
            
            // And the first entry must contain the prefix, else assume it's a note
            if (!line.startsWith(linePrefix))
            {
                _addDiagnosticNote(line, outDiagnostics);
                continue;
            }
        }

        // Try splitting, if we can't assume it's a note from a previous error
        if (SLANG_FAILED(splitDiagnosticLine(compilerIdentity, line, linePrefix, splitLine)))
        {
            _addDiagnosticNote(line, outDiagnostics);
            continue;
        }

        DownstreamDiagnostic diagnostic;
        diagnostic.type = DownstreamDiagnostic::Type::Error;
        diagnostic.stage = DownstreamDiagnostic::Stage::Compile;
        diagnostic.fileLine = 0;
        
        if (SLANG_SUCCEEDED(lineParser(line, splitLine, diagnostic)))
        {
            outDiagnostics.add(diagnostic);   
        }
        else
        {
            // If couldn't parse, just add as a note
            _addDiagnosticNote(line, outDiagnostics);
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


/* static */bool ParseDiagnosticUtil::areEqual(const UnownedStringSlice& a, const UnownedStringSlice& b, EqualityFlags flags)
{
    List<DownstreamDiagnostic> diagsA, diagsB;
    SlangResult resA = ParseDiagnosticUtil::parseDiagnostics(a, diagsA);
    SlangResult resB = ParseDiagnosticUtil::parseDiagnostics(b, diagsB);

    /*
        TODO(JS): In the past we needed special handling of the stdlib, when
        in some builds the path contains the stdlib.

        For now we don't seem to need this, this is for future reference, if there
        is an issue with needing to specially handle this.

       static const UnownedStringSlice stdLibNames[] =
        {
            UnownedStringSlice::fromLiteral("core.meta.slang"),
            UnownedStringSlice::fromLiteral("hlsl.meta.slang"),
            UnownedStringSlice::fromLiteral("slang-stdlib.cpp"),
        };
        */

    // Must have both succeeded, and have the same amount of lines
    if (SLANG_SUCCEEDED(resA) && SLANG_SUCCEEDED(resB) &&
        diagsA.getCount() == diagsB.getCount())
    {
        for (Index i = 0; i < diagsA.getCount(); ++i)
        {
            DownstreamDiagnostic diagA = diagsA[i];
            DownstreamDiagnostic diagB = diagsB[i];

            // Check if we need to ignore line numbers
            if (flags & EqualityFlag::IgnoreLineNos)
            {
                diagA.fileLine = 0;
                diagB.fileLine = 0;
            }

            if (diagA != diagB)
            {
                return false;
            }
        }

        return true;
    }
    
    return false;
}
