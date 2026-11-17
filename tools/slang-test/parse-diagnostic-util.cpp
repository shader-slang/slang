// parse-diagnostic-util.cpp

#include "parse-diagnostic-util.h"

#include "compiler-core/slang-artifact-associated-impl.h"
#include "compiler-core/slang-artifact-diagnostic-util.h"
#include "compiler-core/slang-downstream-compiler.h"
#include "core/slang-byte-encode-util.h"
#include "core/slang-char-util.h"
#include "core/slang-hex-dump-util.h"
#include "core/slang-string-util.h"
#include "core/slang-type-text-util.h"
#include "slang-com-helper.h"

using namespace Slang;

/* static */ SlangResult ParseDiagnosticUtil::parseGenericLine(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    List<UnownedStringSlice>& lineSlices,
    ArtifactDiagnostic& outDiagnostic)
{
    /* e:\git\somewhere\tests\diagnostics\syntax-error-intrinsic.slang(13): error C2018:  unknown
     * character '0x40' */
    if (lineSlices.getCount() < 3)
    {
        return SLANG_FAIL;
    }

    {
        const UnownedStringSlice severityAndCodeSlice = lineSlices[1].trim();
        // Get the code
        outDiagnostic.code =
            allocator.allocate(StringUtil::getAtInSplit(severityAndCodeSlice, ' ', 1).trim());

        const UnownedStringSlice severitySlice =
            StringUtil::getAtInSplit(severityAndCodeSlice, ' ', 0);

        outDiagnostic.severity = ArtifactDiagnostic::Severity::Error;
        if (severitySlice == UnownedStringSlice::fromLiteral("warning"))
        {
            outDiagnostic.severity = ArtifactDiagnostic::Severity::Warning;
        }
        else if (severitySlice == UnownedStringSlice::fromLiteral("info"))
        {
            outDiagnostic.severity = ArtifactDiagnostic::Severity::Info;
        }
    }

    // Get the location info
    SLANG_RETURN_ON_FAIL(
        ArtifactDiagnosticUtil::splitPathLocation(allocator, lineSlices[0], outDiagnostic));

    outDiagnostic.text = allocator.allocate(lineSlices[2].begin(), line.end());
    return SLANG_OK;
}

// Matches a severity word at the start of inText and sets outPrefix to the matched word and
// outSeverity to its class. Shared by both Slang diagnostic renderings: the rich-header
// matcher passes a full header line (`error[E30015]: ...`), while the classic callers pass
// only the severity-and-code slice between the path and the message (`error 31101` in
// `path(11): error 31101: ...`). Returns SlangResult rather than bool so the classic path
// can chain it with SLANG_RETURN_ON_FAIL.
static SlangResult _matchSlangSeverityPrefix(
    const UnownedStringSlice& inText,
    ArtifactDiagnostic::Severity& outSeverity,
    UnownedStringSlice& outPrefix)
{
    UnownedStringSlice text(inText.trim());

    struct SeverityPrefix
    {
        UnownedStringSlice prefix;
        ArtifactDiagnostic::Severity severity;
    };
    static const SeverityPrefix prefixes[] = {
        {UnownedStringSlice::fromLiteral("note"), ArtifactDiagnostic::Severity::Info},
        {UnownedStringSlice::fromLiteral("warning"), ArtifactDiagnostic::Severity::Warning},
        {UnownedStringSlice::fromLiteral("error"), ArtifactDiagnostic::Severity::Error},
        {UnownedStringSlice::fromLiteral("fatal error"), ArtifactDiagnostic::Severity::Error},
        {UnownedStringSlice::fromLiteral("internal error"), ArtifactDiagnostic::Severity::Error},
        {UnownedStringSlice::fromLiteral("unknown error"), ArtifactDiagnostic::Severity::Error}};

    for (const auto& entry : prefixes)
    {
        if (text.startsWith(entry.prefix))
        {
            outPrefix = entry.prefix;
            outSeverity = entry.severity;
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
}

// Parses the classic severity-and-code slice `error 31101`. The severity word is matched by
// _matchSlangSeverityPrefix; outCode stays 0 when nothing follows the word, and any
// non-numeric tail (e.g. `error E30015:`) fails the call — the mechanism that keeps
// bracketed and E-prefixed spellings from claiming classic Slang identity. Success here is
// what identifies a line as a classic Slang diagnostic (_isSlangDiagnostic).
static SlangResult _getSlangDiagnosticSeverity(
    const UnownedStringSlice& inText,
    ArtifactDiagnostic::Severity& outSeverity,
    Int& outCode)
{
    UnownedStringSlice prefix;
    SLANG_RETURN_ON_FAIL(_matchSlangSeverityPrefix(inText, outSeverity, prefix));

    outCode = 0;

    UnownedStringSlice tail = inText.trim().tail(prefix.getLength()).trim();
    if (tail.getLength() > 0)
    {
        SLANG_RETURN_ON_FAIL(StringUtil::parseInt(tail, outCode));
    }

    return SLANG_OK;
}

static bool _isSlangDiagnostic(const UnownedStringSlice& line)
{
    /*
    tests/diagnostics/accessors.slang(11): error 31101: accessors other than 'set' must not have
    parameters
    */

    UnownedStringSlice initial = StringUtil::getAtInSplit(line, ':', 0);

    // Handle if path has :
    const Index typeIndex = (initial.getLength() == 1 && CharUtil::isAlpha(initial[0])) ? 2 : 1;
    // Extract the type/code slice
    UnownedStringSlice typeSlice = StringUtil::getAtInSplit(line, ':', typeIndex);

    ArtifactDiagnostic::Severity type;
    Int code;
    return SLANG_SUCCEEDED(_getSlangDiagnosticSeverity(typeSlice, type, code));
}

/* static */ SlangResult ParseDiagnosticUtil::parseSlangLine(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    List<UnownedStringSlice>& lineSlices,
    ArtifactDiagnostic& outDiagnostic)
{
    /*
    tests/diagnostics/accessors.slang(11): error 31101: accessors other than 'set' must not have
    parameters
    */

    // Can be larger than 3, because might be : in the actual error text
    if (lineSlices.getCount() < 3)
    {
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(
        ArtifactDiagnosticUtil::splitPathLocation(allocator, lineSlices[0], outDiagnostic));
    Int code;
    SLANG_RETURN_ON_FAIL(_getSlangDiagnosticSeverity(lineSlices[1], outDiagnostic.severity, code));

    if (code != 0)
    {
        StringBuilder buf;
        buf << code;
        outDiagnostic.code = allocator.allocate(buf);
    }

    outDiagnostic.text = allocator.allocate(lineSlices[2].begin(), line.end());
    return SLANG_OK;
}

/*
A rich Slang diagnostic — the rust-style layout rendered by
slang-rich-diagnostics-render.cpp — renders as a block:

    error[E30015]: undefined identifier
     --> tests/foo.slang:4:15
      |
    4 | void main() { undefined_thing; }
      |               ^^^^^^^^^^^^^^^ undefined identifier 'undefined_thing'

The header line carries the severity, an optional code in brackets, and the message; the
location line that follows carries the path and position. A note repeats the same shape
with `note:` and a `---` location marker, except that a note header carries no code
bracket. This is slangc's default rendering for its diagnostics.
*/
// Matches the header line of a rich record: a severity word, an optional `[E<digits>]` code,
// then `: message`. The out-parameters are meaningful only on a match. When requireCode is
// set, a header without a code bracket matches only if its severity is note: every slang
// error/warning diagnostic carries a code, while the codeless standalone notes (the
// checkpoint-reporting family, code -1) can legitimately lead a stream — so a downstream
// tool's bare `error:` or `warning:` line still cannot claim Slang identity, and a bare
// downstream `note:` line that did would parse as an Info record, the same content the
// generic parser keeps as a note.
static bool _matchSlangRichDiagnosticHeader(
    const UnownedStringSlice& inLine,
    ArtifactDiagnostic::Severity& outSeverity,
    UnownedStringSlice& outCode,
    UnownedStringSlice& outMessage,
    bool requireCode)
{
    UnownedStringSlice text(inLine.trim());

    UnownedStringSlice severityPrefix;
    if (SLANG_FAILED(_matchSlangSeverityPrefix(text, outSeverity, severityPrefix)))
    {
        return false;
    }
    text = text.tail(severityPrefix.getLength());

    outCode = UnownedStringSlice();
    if (text.startsWith(UnownedStringSlice::fromLiteral("[")))
    {
        // The renderer decorates the code as `[E#####]`; the digits after the `E` are the
        // code. Anything in the brackets other than `E<digits>` makes the whole line a
        // non-header (in the record parser that flushes a pending record and demotes the
        // line to a note).
        const Index closeIndex = text.indexOf(']');
        if (closeIndex <= 0)
        {
            return false;
        }

        UnownedStringSlice code = text.subString(1, closeIndex - 1);
        if (!code.startsWith(UnownedStringSlice::fromLiteral("E")))
        {
            return false;
        }
        code = code.tail(1);

        if (code.getLength() == 0)
        {
            return false;
        }
        for (const char c : code)
        {
            if (!CharUtil::isDigit(c))
            {
                return false;
            }
        }

        outCode = code;
        text = text.tail(closeIndex + 1);
    }
    else if (requireCode && outSeverity != ArtifactDiagnostic::Severity::Info)
    {
        return false;
    }

    if (!text.startsWith(UnownedStringSlice::fromLiteral(":")))
    {
        return false;
    }

    outMessage = text.tail(1).trim();
    return true;
}

// The location line of a rich record; sets outLocation to its `path:line:col` text, which
// is meaningful only on a match. The markers mirror the renderer's glyph tables
// (s_asciiGlyphs / s_unicodeGlyphs in source/compiler-core/slang-rich-diagnostics-render.cpp):
// ascii `-->` for a primary location and `---` for a note location; the unicode set uses the
// `╭╼` glyph for both (reachable only from hand-crafted input under this harness — slang-test
// pipes stderr, so unicode glyphs never auto-enable; kept so hand-written expected files in
// the unicode spelling parse too). Two other renderer lines also begin with dashes after
// trimming — the snippet gutter close (`---'`) and a three-dash secondary underline — but
// they are harmless: a location line always immediately follows its header, because the
// renderer only draws a snippet under a location line (a positionless record renders no
// snippet), so no pending header can attach to them; they degrade to notes.
static bool _matchSlangRichLocationLine(
    const UnownedStringSlice& inLine,
    UnownedStringSlice& outLocation)
{
    static const UnownedStringSlice markers[] = {
        UnownedStringSlice::fromLiteral("-->"),
        UnownedStringSlice::fromLiteral("---"),
        UnownedStringSlice::fromLiteral(
            "\xe2\x95\xad\xe2\x95\xbc")}; // UTF-8 for ╭╼ (U+256D U+257C)

    UnownedStringSlice text(inLine.trim());
    for (const auto& marker : markers)
    {
        if (text.startsWith(marker))
        {
            outLocation = text.tail(marker.getLength()).trim();
            return true;
        }
    }
    return false;
}

// Sets the diagnostic's location from the `path:line:col` text of a rich location line.
// The renderer emits two shapes — `path:line:col`, or a bare path for a command-line
// source — and the split takes the last two colon-separated parts only when both parse as
// integers (StringUtil::parseInt also accepts a sign or hex prefix, which is harmless
// here: the renderer writes plain decimal positions). A colon-bearing path therefore
// cannot be mis-split: a located `e:\x\f.slang:4:15` splits on its last two parts, and a
// bare drive-letter path fails the column parse so the whole text stays the path. Any
// other tail (including a `path:line` without a column, which the renderer never emits)
// likewise keeps the whole text as the filePath with the position left unset, matching
// the harness convention that an unset position is line and column 0.
static void _setSlangRichLocation(
    SliceAllocator& allocator,
    const UnownedStringSlice& inLocation,
    ArtifactDiagnostic& outDiagnostic)
{
    UnownedStringSlice path(inLocation);

    const Index colIndex = path.lastIndexOf(':');
    if (colIndex > 0)
    {
        const UnownedStringSlice colSlice = path.tail(colIndex + 1);
        const Index lineIndex = path.head(colIndex).lastIndexOf(':');
        if (lineIndex >= 0)
        {
            const UnownedStringSlice lineSlice =
                path.subString(lineIndex + 1, colIndex - lineIndex - 1);

            Int lineNumber, colNumber;
            if (SLANG_SUCCEEDED(StringUtil::parseInt(lineSlice, lineNumber)) &&
                SLANG_SUCCEEDED(StringUtil::parseInt(colSlice, colNumber)))
            {
                outDiagnostic.location.line = lineNumber;
                outDiagnostic.location.column = colNumber;
                path = path.head(lineIndex);
            }
        }
    }

    outDiagnostic.filePath = allocator.allocate(path);
}

// True if any line of inText is a rich header. Used to decide, after identifyCompiler has
// named Slang, whether the text is rich-rendered; requires the coded header form (see
// _matchSlangRichDiagnosticHeader for why that is the identifying shape).
static bool _isSlangRichDiagnostics(const UnownedStringSlice& inText)
{
    UnownedStringSlice text(inText), line;
    while (StringUtil::extractLine(text, line))
    {
        ArtifactDiagnostic::Severity severity;
        UnownedStringSlice code, message;
        if (_matchSlangRichDiagnosticHeader(line, severity, code, message, true))
        {
            return true;
        }
    }
    return false;
}

/* static */ SlangResult ParseDiagnosticUtil::parseSlangRichDiagnostics(
    const UnownedStringSlice& inText,
    IArtifactDiagnostics* diagnostics)
{
    SliceAllocator allocator;

    // The header line starts a record and holds its severity, code and message; the
    // location line completes it. The renderer writes the header message verbatim and the
    // catalog has newline-bearing formats (the internal-error family), so a record's
    // message can span lines: continuation lines are buffered into the pending record's
    // text until its location line, the next header, or the end of the text shows the
    // record is complete. A header that never receives a location line is a record
    // without a source position, emitted with the accumulated text.
    ArtifactDiagnostic pending;
    bool hasPending = false;

    UnownedStringSlice text(inText), line;
    while (StringUtil::extractLine(text, line))
    {
        ArtifactDiagnostic::Severity severity;
        UnownedStringSlice code, message;
        if (_matchSlangRichDiagnosticHeader(line, severity, code, message, false))
        {
            if (hasPending)
            {
                diagnostics->add(pending);
            }

            pending = ArtifactDiagnostic();
            pending.severity = severity;
            pending.stage = ArtifactDiagnostic::Stage::Compile;
            if (code.getLength() > 0)
            {
                // The matcher validated the code slice is non-empty and all digits, so the
                // parse cannot fail except on absurd overflow; assert rather than silently
                // dropping the code. Store the result normalized through Int, as the
                // classic parse does, so both renderings produce the same code string (the
                // renderer zero-pads the bracket).
                Int codeNumber = 0;
                SLANG_ASSERT(SLANG_SUCCEEDED(StringUtil::parseInt(code, codeNumber)));
                StringBuilder buf;
                buf << codeNumber;
                pending.code = allocator.allocate(buf);
            }
            pending.text = allocator.allocate(message);
            hasPending = true;
            continue;
        }

        UnownedStringSlice location;
        if (_matchSlangRichLocationLine(line, location))
        {
            if (hasPending)
            {
                _setSlangRichLocation(allocator, location, pending);
                diagnostics->add(pending);
                hasPending = false;
            }
            else
            {
                // A location line with no pending header carries no diagnostic of its own
                // (renderer output always sends a header first; this branch only sees
                // malformed or hand-crafted text). Keep the raw line as a note rather than
                // dropping it.
                ArtifactDiagnosticUtil::maybeAddNote(line, diagnostics);
            }
            continue;
        }

        if (hasPending)
        {
            // A continuation of the pending record's (possibly multi-line) message. Blank
            // lines carry no text — records are separated by an empty line before notes —
            // and snippet lines cannot collide: the renderer only draws them under a
            // location line, which completes the record first.
            if (line.trim().getLength() > 0)
            {
                StringBuilder buf;
                buf.append(asStringSlice(pending.text));
                buf.append("\n");
                buf.append(line);
                pending.text = allocator.allocate(buf);
            }
            continue;
        }

        // Gutter, snippet and underline lines of a rendered block are kept as notes.
        ArtifactDiagnosticUtil::maybeAddNote(line, diagnostics);
    }

    if (hasPending)
    {
        diagnostics->add(pending);
    }

    return SLANG_OK;
}

/* static */ SlangResult ParseDiagnosticUtil::splitDiagnosticLine(
    const CompilerIdentity& compilerIdentity,
    const UnownedStringSlice& line,
    const UnownedStringSlice& linePrefix,
    List<UnownedStringSlice>& outSlices)
{
    StringUtil::split(line, ':', outSlices);

    // If we have a prefix (typically identifying the compiler), remove so same code can be used for
    // output with prefixes and without
    if (linePrefix.getLength())
    {
        SLANG_ASSERT(outSlices[0].startsWith(linePrefix));
        outSlices.removeAt(0);
    }

    /*
    glslang: ERROR: tests/diagnostics/syntax-error-intrinsic.slang:13: '@' : unexpected token
    dxc: tests/diagnostics/syntax-error-intrinsic.slang:14:2: error: expected expression
    fxc: tests/diagnostics/syntax-error-intrinsic.slang(14,2): error X3000: syntax error: unexpected
    token '@' Visual Studio 14.0:
    e:\git\somewhere\tests\diagnostics\syntax-error-intrinsic.slang(13): error C2018:  unknown
    character '0x40' NVRTC 11.0: tests/diagnostics/syntax-error-intrinsic.slang(13): error :
    unrecognized token tests/diagnostics/accessors.slang(11): error 31101: accessors other than
    'set' must not have parameters
    */

    // The index where the path starts
    const Int pathIndex = 0;

    // Now we want to fix up a path as might have drive letter, and therefore :
    // If this is the situation then we need to have a slice after the one at the index
    if (outSlices.getCount() > pathIndex + 1)
    {
        const UnownedStringSlice pathStart = outSlices[pathIndex].trim();
        if (pathStart.getLength() == 1 && CharUtil::isAlpha(pathStart[0]))
        {
            // Splice back together
            outSlices[pathIndex] =
                UnownedStringSlice(outSlices[pathIndex].begin(), outSlices[pathIndex + 1].end());
            outSlices.removeAt(pathIndex + 1);
        }
    }

    return SLANG_OK;
}

static SlangResult _findDownstreamCompiler(
    const UnownedStringSlice& slice,
    SlangPassThrough& outDownstreamCompiler)
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

/* static */ SlangResult ParseDiagnosticUtil::identifyCompiler(
    const UnownedStringSlice& inText,
    CompilerIdentity& outIdentity)
{
    outIdentity = CompilerIdentity();

    // This might be overkill - we should be able to identify the compiler from the first line, of
    // the diagnostics. Here, we go through each line trying to identify the compiler. For
    // downstream compilers, the only way to identify unambiguously is via the compiler name prefix.
    // For Slang we *assume* if there isn't such a prefix, and it 'looks like' a Slang diagnostic
    // that it is

    UnownedStringSlice text(inText), line;
    while (StringUtil::extractLine(text, line))
    {
        // A rich header identifies Slang: the classic check below cannot match one (the
        // severity word and code bracket are glued together inside the first
        // colon-separated slice, so no slice carries the bare `error <code>` shape that
        // check needs), and no downstream-compiler name prefixes it, so without this
        // check pure rich output would not identify at all. Which codeless headers still
        // identify — notes, but not bare error/warning lines — and why that is safe is the
        // contract of _matchSlangRichDiagnosticHeader's requireCode.
        ArtifactDiagnostic::Severity richSeverity;
        UnownedStringSlice richCode, richMessage;
        if (_matchSlangRichDiagnosticHeader(line, richSeverity, richCode, richMessage, true))
        {
            outIdentity = CompilerIdentity::makeSlang();
            return SLANG_OK;
        }

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

/* static */ ParseDiagnosticUtil::LineParser ParseDiagnosticUtil::getLineParser(
    const CompilerIdentity& compilerIdentity)
{
    switch (compilerIdentity.m_type)
    {
    case CompilerIdentity::Slang:
        return &parseSlangLine;
    case CompilerIdentity::DownstreamCompiler:
        return &parseGenericLine;
    default:
        return nullptr;
    }
}

static bool _isWhitespace(const UnownedStringSlice& slice)
{
    for (const char c : slice)
    {
        if (!CharUtil::isWhitespace(c))
        {
            return false;
        }
    }
    return true;
}

/* static */ SlangResult ParseDiagnosticUtil::parseDiagnostics(
    const UnownedStringSlice& inText,
    IArtifactDiagnostics* diagnostics)
{
    if (_isWhitespace(inText))
    {
        // If it's empty, then there are no diagnostics to add.
        return SLANG_OK;
    }

    CompilerIdentity compilerIdentity;
    SLANG_RETURN_ON_FAIL(ParseDiagnosticUtil::identifyCompiler(inText, compilerIdentity));

    // Frontend diagnostics render as rich blocks; parse them as records so a block's
    // location line reaches its diagnostic. identifyCompiler only decides which compiler
    // produced the output — which of Slang's two renderings is in use is decided here, by
    // scanning again for a rich header. A stream that mixes rich blocks with
    // downstream-prefixed lines identifies by line order: whichever rendering's line
    // matches first wins, and the other rendering's lines degrade in the winning parser —
    // to notes, or to positionless records for codeless headers in the rich parser.
    // Hybrid parsing is out of scope.
    if (compilerIdentity.m_type == CompilerIdentity::Slang && _isSlangRichDiagnostics(inText))
    {
        return parseSlangRichDiagnostics(inText, diagnostics);
    }

    UnownedStringSlice linePrefix;
    if (compilerIdentity.m_type == CompilerIdentity::Type::DownstreamCompiler)
    {
        linePrefix = TypeTextUtil::getPassThroughAsHumanText(compilerIdentity.m_downstreamCompiler);
    }
    else
    {
        // For Slang there isn't *currently* a prefix ever used, but that might change in the future
        // For now we assume no prefix.
    }

    return parseDiagnostics(inText, compilerIdentity, linePrefix, diagnostics);
}

/* static */ SlangResult ParseDiagnosticUtil::parseDiagnostics(
    const UnownedStringSlice& inText,
    const CompilerIdentity& compilerIdentity,
    const UnownedStringSlice& linePrefix,
    IArtifactDiagnostics* diagnostics)
{
    auto lineParser = getLineParser(compilerIdentity);
    if (!lineParser)
    {
        return SLANG_FAIL;
    }

    List<UnownedStringSlice> splitLine;

    SliceAllocator allocator;

    UnownedStringSlice text(inText), line;
    while (StringUtil::extractLine(text, line))
    {
        bool isValidSplit = false;
        // And the first entry must contain the prefix, else assume it's a note
        if (linePrefix.getLength() > 0 && line.startsWith(linePrefix))
        {
            // Try with the line prefix
            isValidSplit =
                SLANG_SUCCEEDED(splitDiagnosticLine(compilerIdentity, line, linePrefix, splitLine));
        }

        if (!isValidSplit)
        {
            // Try without the prefix, as some output output's only some lines with the prefix (GLSL
            // for example)
            isValidSplit = SLANG_SUCCEEDED(
                splitDiagnosticLine(compilerIdentity, line, UnownedStringSlice(), splitLine));
        }

        // If we don't have a valid split then just assume it's a note
        if (!isValidSplit)
        {
            diagnostics->maybeAddNote(asCharSlice(line));
            continue;
        }

        ArtifactDiagnostic diagnostic;
        diagnostic.severity = ArtifactDiagnostic::Severity::Error;
        diagnostic.stage = ArtifactDiagnostic::Stage::Compile;
        diagnostic.location.line = 0;

        if (SLANG_SUCCEEDED(lineParser(allocator, line, splitLine, diagnostic)))
        {
            diagnostics->add(diagnostic);
        }
        else
        {
            // If couldn't parse, just add as a note
            ArtifactDiagnosticUtil::maybeAddNote(line, diagnostics);
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

static bool _isAtEnd(const UnownedStringSlice& text, const UnownedStringSlice& line)
{
    if (line != "}")
    {
        return false;
    }
    // We need to get the *next* line. If it is "}" then this isn't the final closing
    UnownedStringSlice remaining(text);
    UnownedStringSlice nextLine;
    StringUtil::extractLine(remaining, nextLine);

    return (nextLine != toSlice("}"));
}

/* static */ SlangResult ParseDiagnosticUtil::parseOutputInfo(
    const UnownedStringSlice& inText,
    OutputInfo& out)
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
                    const UnownedStringSlice valueSlice =
                        _getEquals(line.tail(resultCodePrefix.getLength()));
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

                        UnownedStringSlice valueSlice =
                            _getEquals(line.tail(startsWith->getLength()));
                        if (!valueSlice.isChar('{'))
                        {
                            return SLANG_FAIL;
                        }
                        // Okay we now inside std out or std error, so update the state
                        state =
                            (startsWith == &stdErrorPrefix) ? State::InStdError : State::InStdOut;
                    }
                }
                break;
            }
        case State::InStdError:
        case State::InStdOut:
            {
                if (_isAtEnd(text, line))
                {
                    String& dst = (state == State::InStdError) ? out.stdError : out.stdOut;
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


/* static */ bool ParseDiagnosticUtil::areEqual(
    const UnownedStringSlice& a,
    const UnownedStringSlice& b,
    EqualityFlags flags)
{
    auto diagsA = ArtifactDiagnostics::create();
    auto diagsB = ArtifactDiagnostics::create();

    SlangResult resA = ParseDiagnosticUtil::parseDiagnostics(a, diagsA);
    SlangResult resB = ParseDiagnosticUtil::parseDiagnostics(b, diagsB);

    /*
        TODO(JS): In the past we needed special handling of the core module, when
        in some builds the path contains the core module.

        For now we don't seem to need this, this is for future reference, if there
        is an issue with needing to specially handle this.

       static const UnownedStringSlice coreModuleNames[] =
        {
            UnownedStringSlice::fromLiteral("core.meta.slang"),
            UnownedStringSlice::fromLiteral("hlsl.meta.slang"),
            UnownedStringSlice::fromLiteral("slang-core-module.cpp"),
        };
        */

    // Must have both succeeded, and have the same amount of lines
    if (SLANG_SUCCEEDED(resA) && SLANG_SUCCEEDED(resB) && diagsA->getCount() == diagsB->getCount())
    {
        const auto count = diagsA->getCount();
        for (Index i = 0; i < count; ++i)
        {
            ArtifactDiagnostic diagA = *diagsA->getAt(i);
            ArtifactDiagnostic diagB = *diagsB->getAt(i);

            // Check if we need to ignore line numbers
            if (flags & EqualityFlag::IgnoreLineNos)
            {
                const ArtifactDiagnostic::Location loc;

                diagA.location = loc;
                diagB.location = loc;
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
