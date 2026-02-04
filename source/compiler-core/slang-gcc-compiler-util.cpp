// slang-gcc-compiler-util.cpp
#include "slang-gcc-compiler-util.h"

#include "../core/slang-char-util.h"
#include "../core/slang-common.h"
#include "../core/slang-io.h"
#include "../core/slang-shared-library.h"
#include "../core/slang-string-slice-pool.h"
#include "../core/slang-string-util.h"
#include "slang-artifact-desc-util.h"
#include "slang-artifact-diagnostic-util.h"
#include "slang-artifact-representation-impl.h"
#include "slang-artifact-util.h"
#include "slang-com-helper.h"

#include <mutex>

namespace Slang
{

static Index _findVersionEnd(const UnownedStringSlice& in)
{
    Index numDots = 0;
    const Index len = in.getLength();

    for (Index i = 0; i < len; ++i)
    {
        const char c = in[i];
        if (CharUtil::isDigit(c))
        {
            continue;
        }
        if (c == '.')
        {
            if (numDots >= 2)
            {
                return i;
            }
            numDots++;
            continue;
        }
        return i;
    }
    return len;
}

/* static */ SlangResult GCCDownstreamCompilerUtil::parseVersion(
    const UnownedStringSlice& text,
    const UnownedStringSlice& prefix,
    DownstreamCompilerDesc& outDesc)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(text, lines);

    for (auto line : lines)
    {
        Index prefixIndex = line.indexOf(prefix);
        if (prefixIndex < 0)
        {
            continue;
        }

        const UnownedStringSlice remainingSlice =
            UnownedStringSlice(line.begin() + prefixIndex + prefix.getLength(), line.end()).trim();

        const Index versionEndIndex = _findVersionEnd(remainingSlice);
        if (versionEndIndex < 0)
        {
            return SLANG_FAIL;
        }

        const UnownedStringSlice versionSlice(
            remainingSlice.begin(),
            remainingSlice.begin() + versionEndIndex);

        // Version is in format 0.0.0
        List<UnownedStringSlice> split;
        StringUtil::split(versionSlice, '.', split);
        List<Int> digits;

        for (auto v : split)
        {
            Int version;
            SLANG_RETURN_ON_FAIL(StringUtil::parseInt(v, version));
            digits.add(version);
        }

        if (digits.getCount() < 2)
        {
            return SLANG_FAIL;
        }

        outDesc.version.set(int(digits[0]), int(digits[1]));
        return SLANG_OK;
    }

    return SLANG_FAIL;
}

// Compiler version detection patterns
// Replaces parallel arrays with structured type for safety
struct CompilerVersionPattern
{
    const char* versionPrefix;
    SlangPassThrough compilerType;
};

static const CompilerVersionPattern s_compilerVersionPatterns[] = {
    {"clang version", SLANG_PASS_THROUGH_CLANG},
    {"gcc version", SLANG_PASS_THROUGH_GCC},
    {"Apple LLVM version", SLANG_PASS_THROUGH_CLANG},
    {"Apple metal version", SLANG_PASS_THROUGH_METAL},
};

SlangResult GCCDownstreamCompilerUtil::calcVersion(
    const ExecutableLocation& exe,
    DownstreamCompilerDesc& outDesc)
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(exe);
    cmdLine.addArg("-v");

    ExecuteResult exeRes;
    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, exeRes));

    // Note: Compiler output may have additional words before version string
    // e.g., "Ubuntu clang version 14.0.0" or "gcc version 13.1.0"
    // Try each known version prefix pattern
    for (const auto& pattern : s_compilerVersionPatterns)
    {
        outDesc.type = pattern.compilerType;
        UnownedStringSlice prefix(pattern.versionPrefix);

        if (SLANG_SUCCEEDED(parseVersion(exeRes.standardError.getUnownedSlice(), prefix, outDesc)))
        {
            return SLANG_OK;
        }
    }

    return SLANG_FAIL;
}

namespace
{ // anonymous

enum class LineParseResult
{
    Single,       ///< It's a single line
    Start,        ///< Line was the start of a message
    Continuation, ///< Not totally clear, add to previous line if nothing else hit
    Ignore,       ///< Ignore the line
};

// String constants - single source of truth for compiler/error pattern matching
struct GCCPatternStrings
{
    static constexpr const char* CLANG = "clang";
    static constexpr const char* GCC = "gcc";
    static constexpr const char* GPP = "g++";
    static constexpr const char* METAL = "metal";
    static constexpr const char* LD = "ld";
    static constexpr const char* UNDEFINED_REF = "undefined reference";
    static constexpr const char* TEXT_SECTION = "(.text";
    static constexpr const char* LD_RETURNED = "ld returned";
    static constexpr const char* LINKER_FAILED = "linker command failed";
    static constexpr const char* LINK_KEYWORD = "link";
};

// Pattern matcher interface for GCC-family compiler output
// Each pattern has a priority (lower = checked first) and a tryParse function
// that attempts to match and extract diagnostic information from a line
struct GCCPatternMatcher
{
    int priority;            // Lower values = higher priority (checked first)
    const char* patternName; // Human-readable name for debugging

    // Attempt to parse the line. Returns SLANG_OK if pattern matches.
    // On success, fills outDiagnostic and outLineParseResult.
    // On failure (pattern doesn't match), returns SLANG_FAIL.
    SlangResult (*tryParse)(
        SliceAllocator& allocator,
        const UnownedStringSlice& line,
        const List<UnownedStringSlice>& split,
        LineParseResult& outLineParseResult,
        ArtifactDiagnostic& outDiagnostic);
};

} // namespace

static SlangResult _parseSeverity(
    const UnownedStringSlice& in,
    ArtifactDiagnostic::Severity& outSeverity)
{
    typedef ArtifactDiagnostic::Severity Severity;

    if (in == "error" || in == "fatal error")
    {
        outSeverity = Severity::Error;
    }
    else if (in == "warning")
    {
        outSeverity = Severity::Warning;
    }
    else if (in == "info" || in == "note")
    {
        outSeverity = Severity::Info;
    }
    else
    {
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

// Pattern parsers - each handles a specific error format
// Ordered from simplest to most complex for easier understanding

// Fallback: Continuation line (always succeeds)
static SlangResult _parseContinuation(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    const List<UnownedStringSlice>& split,
    LineParseResult& outLineParseResult,
    ArtifactDiagnostic& outDiagnostic)
{
    SLANG_UNUSED(allocator);
    SLANG_UNUSED(line);
    SLANG_UNUSED(split);
    SLANG_UNUSED(outDiagnostic);

    outLineParseResult = LineParseResult::Continuation;
    return SLANG_OK;
}

// Pattern: severity:message (e.g., "error: message")
static SlangResult _parseSimpleSeverity(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    const List<UnownedStringSlice>& split,
    LineParseResult& outLineParseResult,
    ArtifactDiagnostic& outDiagnostic)
{
    SLANG_UNUSED(line);

    if (split.getCount() != 2)
        return SLANG_FAIL;

    const auto split0 = split[0].trim();

    if (SLANG_FAILED(_parseSeverity(split0, outDiagnostic.severity)))
        return SLANG_FAIL;

    outDiagnostic.stage = ArtifactDiagnostic::Stage::Compile;
    outDiagnostic.text = allocator.allocate(split[1].trim());
    outLineParseResult = LineParseResult::Single;

    return SLANG_OK;
}

// Pattern: ld:message (e.g., "ld: message")
static SlangResult _parseSimpleLdInfo(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    const List<UnownedStringSlice>& split,
    LineParseResult& outLineParseResult,
    ArtifactDiagnostic& outDiagnostic)
{
    SLANG_UNUSED(line);

    if (split.getCount() != 2)
        return SLANG_FAIL;

    const auto split0 = split[0].trim();
    if (split0 != UnownedStringSlice(GCCPatternStrings::LD))
        return SLANG_FAIL;

    outDiagnostic.stage = ArtifactDiagnostic::Stage::Link;
    outDiagnostic.severity = ArtifactDiagnostic::Severity::Info;
    outDiagnostic.text = allocator.allocate(split[1].trim());
    outLineParseResult = LineParseResult::Start;

    return SLANG_OK;
}

// Pattern: file:(.text+0x0):message (object file section errors)
// Handles: test-link.c:(.text+0xa):undefined reference to `thing'
static SlangResult _parseTextSectionError(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    const List<UnownedStringSlice>& split,
    LineParseResult& outLineParseResult,
    ArtifactDiagnostic& outDiagnostic)
{
    SLANG_UNUSED(line);

    if (split.getCount() != 3)
        return SLANG_FAIL;

    const auto split1 = split[1].trim();
    if (!split1.startsWith(UnownedStringSlice(GCCPatternStrings::TEXT_SECTION)))
        return SLANG_FAIL;

    const auto text = split[2].trim();

    // Check if this is a link error (contains "undefined reference")
    // This helps distinguish actual link errors from other (.text section messages
    bool isLinkError = text.indexOf(UnownedStringSlice(GCCPatternStrings::UNDEFINED_REF)) != -1;

    outDiagnostic.filePath = allocator.allocate(split[0]);
    outDiagnostic.location.line = 0;
    outDiagnostic.location.column = 0;
    outDiagnostic.severity = ArtifactDiagnostic::Severity::Error;
    outDiagnostic.stage =
        isLinkError ? ArtifactDiagnostic::Stage::Link : ArtifactDiagnostic::Stage::Compile;
    outDiagnostic.text = allocator.allocate(text);
    outLineParseResult = LineParseResult::Single;

    return SLANG_OK;
}

// Pattern: compiler:severity:message (e.g., "clang: error: message")
static SlangResult _parseCompilerCommandError(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    const List<UnownedStringSlice>& split,
    LineParseResult& outLineParseResult,
    ArtifactDiagnostic& outDiagnostic)
{
    SLANG_UNUSED(line);

    if (split.getCount() != 3)
        return SLANG_FAIL;

    const auto split0 = split[0].trim();
    const auto text = split[2].trim();

    // Check for compiler names (clang, metal, Clang, g++, gcc)
    if (!split0.startsWith(UnownedStringSlice(GCCPatternStrings::CLANG)) &&
        !split0.startsWith(UnownedStringSlice(GCCPatternStrings::METAL)) &&
        !split0.startsWith(UnownedStringSlice::fromLiteral("Clang")) &&
        split0 != UnownedStringSlice(GCCPatternStrings::GPP) &&
        split0 != UnownedStringSlice(GCCPatternStrings::GCC))
    {
        return SLANG_FAIL;
    }

    // Extract the severity
    SLANG_RETURN_ON_FAIL(_parseSeverity(split[1].trim(), outDiagnostic.severity));

    outDiagnostic.stage = ArtifactDiagnostic::Stage::Compile;

    // Check if this is a linker error
    if (text.startsWith(UnownedStringSlice(GCCPatternStrings::LINKER_FAILED)))
    {
        outDiagnostic.stage = ArtifactDiagnostic::Stage::Link;
    }
    else if (text.startsWith(UnownedStringSlice(GCCPatternStrings::LD_RETURNED)))
    {
        outDiagnostic.stage = ArtifactDiagnostic::Stage::Link;
    }

    outDiagnostic.text = allocator.allocate(text);
    outLineParseResult = LineParseResult::Start;

    return SLANG_OK;
}

// Pattern: filepath:line:section:undefined reference... (link error with line number)
static SlangResult _parseFileLinkError(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    const List<UnownedStringSlice>& split,
    LineParseResult& outLineParseResult,
    ArtifactDiagnostic& outDiagnostic)
{
    SLANG_UNUSED(line);

    if (split.getCount() != 4)
        return SLANG_FAIL;

    // Check if this is a link error based on message content
    if (split[3].indexOf(UnownedStringSlice(GCCPatternStrings::UNDEFINED_REF)) == -1)
        return SLANG_FAIL;

    // This is a link error with format: filepath:line:section:message
    Int lineNumber = 0;
    const auto split1 = split[1].trim();
    if (SLANG_FAILED(StringUtil::parseInt(split1, lineNumber)))
        return SLANG_FAIL;

    outDiagnostic.filePath = allocator.allocate(split[0]);
    outDiagnostic.location.line = lineNumber;
    outDiagnostic.location.column = 0;
    outDiagnostic.severity = ArtifactDiagnostic::Severity::Error;
    outDiagnostic.stage = ArtifactDiagnostic::Stage::Link;
    outDiagnostic.text = allocator.allocate(split[3]);
    outLineParseResult = LineParseResult::Single;

    return SLANG_OK;
}

// Pattern: objectfile:section:section:message (object file link error fallback)
// Handles patterns like: /tmp/file.o:something:section:message
static SlangResult _parseObjectFileLinkError(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    const List<UnownedStringSlice>& split,
    LineParseResult& outLineParseResult,
    ArtifactDiagnostic& outDiagnostic)
{
    SLANG_UNUSED(line);

    if (split.getCount() != 4)
        return SLANG_FAIL;

    // Check if first element has .o or .obj extension
    String ext = Path::getPathExt(split[0]);
    if (ext != "o" && ext != "obj")
        return SLANG_FAIL;

    outDiagnostic.filePath = allocator.allocate(split[1]);
    outDiagnostic.location.line = 0;
    outDiagnostic.location.column = 0;
    outDiagnostic.severity = ArtifactDiagnostic::Severity::Error;
    outDiagnostic.stage = ArtifactDiagnostic::Stage::Link;
    outDiagnostic.text = allocator.allocate(split[3]);
    outLineParseResult = LineParseResult::Start;

    return SLANG_OK;
}

// Pattern: gcc:filename:link error:message (GCC 13.x format)
static SlangResult _parseGCC13LinkError(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    const List<UnownedStringSlice>& split,
    LineParseResult& outLineParseResult,
    ArtifactDiagnostic& outDiagnostic)
{
    SLANG_UNUSED(line);

    if (split.getCount() != 4)
        return SLANG_FAIL;

    const auto split0 = split[0].trim();
    const auto split1 = split[1].trim();
    const auto split2 = split[2].trim();

    // Check for GCC 13.x format: "gcc 13.3: filename: link error : message"
    // or "gcc 13.3: : link error : message" (when filename is empty)
    // Verify it's actually a link error by checking split[2] contains "link"
    if (!split0.startsWith(UnownedStringSlice(GCCPatternStrings::GCC)) &&
        !split0.startsWith(UnownedStringSlice(GCCPatternStrings::GPP)))
    {
        return SLANG_FAIL;
    }

    if (split2.indexOf(UnownedStringSlice(GCCPatternStrings::LINK_KEYWORD)) == -1)
        return SLANG_FAIL;

    // "link error" is not a standard severity keyword
    outDiagnostic.severity = ArtifactDiagnostic::Severity::Error;
    outDiagnostic.stage = ArtifactDiagnostic::Stage::Link;

    // Use split1 as filename if not empty
    if (split1.getLength() > 0)
    {
        outDiagnostic.filePath = allocator.allocate(split1);
    }

    outDiagnostic.text = allocator.allocate(split[3].trim());
    outLineParseResult = LineParseResult::Single;

    return SLANG_OK;
}

// Pattern: ld:file:line:section:undefined reference... (ld-prefixed linker error)
static SlangResult _parseLdLinkerError(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    const List<UnownedStringSlice>& split,
    LineParseResult& outLineParseResult,
    ArtifactDiagnostic& outDiagnostic)
{
    SLANG_UNUSED(line);

    if (split.getCount() < 5)
        return SLANG_FAIL;

    const auto split0 = split[0].trim();
    if (!split0.startsWith(UnownedStringSlice(GCCPatternStrings::LD)))
        return SLANG_FAIL;

    // Check for link error indicators
    Index undefinedIdx = split[4].indexOf(UnownedStringSlice(GCCPatternStrings::UNDEFINED_REF));
    bool startsWithParen = split[3].trim().startsWith(UnownedStringSlice::fromLiteral("("));

    if (undefinedIdx == -1 && !startsWithParen)
        return SLANG_FAIL;

    // Link error format: /usr/bin/ld:file:line:section:message
    outDiagnostic.filePath = allocator.allocate(split[1]);
    StringUtil::parseInt(split[2], outDiagnostic.location.line);
    outDiagnostic.location.column = 0;
    outDiagnostic.severity = ArtifactDiagnostic::Severity::Error;
    outDiagnostic.stage = ArtifactDiagnostic::Stage::Link;
    outDiagnostic.text = allocator.allocate(split[4].begin(), split.getLast().end());
    outLineParseResult = LineParseResult::Single;

    return SLANG_OK;
}

// Pattern: file:line:column:severity:message (standard compile error)
static SlangResult _parseStandardCompileError(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    const List<UnownedStringSlice>& split,
    LineParseResult& outLineParseResult,
    ArtifactDiagnostic& outDiagnostic)
{
    SLANG_UNUSED(line);

    // Requires exactly 5+ splits: file:line:column:severity:message
    if (split.getCount() < 5)
        return SLANG_FAIL;

    // Must be valid severity
    if (SLANG_FAILED(_parseSeverity(split[3].trim(), outDiagnostic.severity)))
        return SLANG_FAIL;

    // Parse line number
    SLANG_RETURN_ON_FAIL(StringUtil::parseInt(split[1], outDiagnostic.location.line));

    // Parse column number (FIXES BUG: was missing in original)
    SLANG_RETURN_ON_FAIL(StringUtil::parseInt(split[2], outDiagnostic.location.column));

    outDiagnostic.stage = ArtifactDiagnostic::Stage::Compile;
    outDiagnostic.filePath = allocator.allocate(split[0]);
    outDiagnostic.text = allocator.allocate(split[4].begin(), split.getLast().end());
    outLineParseResult = LineParseResult::Start;

    return SLANG_OK;
}

namespace
{ // anonymous

// Pattern registry - ORDERED by priority (low to high number)
// Priority ranges:
//   0-99:   Link errors (check first due to ambiguity with standard errors)
//   100-199: Compiler-prefixed errors (clang:, gcc:)
//   200-299: Standard compile errors (file:line:col:severity:msg)
//   300-399: Simple errors (severity:msg)
//   400+:    Fallback/continuation
static const GCCPatternMatcher s_gccPatterns[] = {
    // Link errors (priority 0-99)
    {10, "ld-linker-error", _parseLdLinkerError},
    {20, "gcc13-link-error", _parseGCC13LinkError},
    {25, "object-file-link-error", _parseObjectFileLinkError},
    {30, "file-link-error", _parseFileLinkError},
    {40, "text-section-error", _parseTextSectionError},

    // Compiler-prefixed (priority 100-199)
    {100, "compiler-command-error", _parseCompilerCommandError},
    {110, "simple-ld-info", _parseSimpleLdInfo},

    // Standard errors (priority 200-299)
    {200, "standard-compile-error", _parseStandardCompileError},

    // Simple patterns (priority 300-399)
    {300, "simple-severity", _parseSimpleSeverity},

    // Fallback (priority 400+)
    {400, "continuation", _parseContinuation},
};

} // anonymous namespace

static SlangResult _parseGCCFamilyLine(
    SliceAllocator& allocator,
    const UnownedStringSlice& line,
    LineParseResult& outLineParseResult,
    ArtifactDiagnostic& outDiagnostic)
{
    // Set to default case
    outLineParseResult = LineParseResult::Ignore;

    /* Example error output patterns handled by pattern matchers:

    Standard compile error (file:line:column:severity:message):
        tests/cpp-compiler/c-compile-error.c:8:13: error: 'b' was not declared in this scope
        int a = b + c;
        ^

    Link errors with ld prefix:
        /tmp/ccS0JCWe.o:c-compile-link-error.c:(.rdata$.refptr.thing[.refptr.thing]+0x0): undefined
        reference to `thing' collect2: error: ld returned 1 exit status

    Clang/GCC command errors:
        clang: warning: treating 'c' input as 'c++' when in C++ mode, this behavior is deprecated
        [-Wdeprecated] Undefined symbols for architecture x86_64:
        "_thing", referenced from:
        _main in c-compile-link-error-a83ace.o
        ld: symbol(s) not found for architecture x86_64
        clang: error: linker command failed with exit code 1 (use -v to see invocation)

    Link errors with file:line:section:message format:
        /tmp/c-compile-link-error-ccf151.o: In function `main':
        c-compile-link-error.c:(.text+0x19): undefined reference to `thing'
        clang: error: linker command failed with exit code 1 (use -v to see invocation)

    GCC 13.x link errors:
        gcc 13.3: filename: link error : undefined reference to `thing'

    Fatal errors (missing headers):
        /path/slang-cpp-prelude.h:4:10: fatal error: ../slang.h: No such file or directory
        #include "slang.h"
        ^~~~~~~~~~~~
        compilation terminated.

    Command-line errors:
        g++: error: unrecognized command line option '-std=c++14'

    See pattern matcher functions above for detailed parsing logic.
    */

    /* /tmp/c-compile-link-error-301c8c.o: In function `main':
       /home/travis/build/shader-slang/slang/tests/cpp-compiler/c-compile-link-error.c:10: undefined
       reference to `thing' clang-7: error: linker command failed with exit code 1 (use -v to see
       invocation)*/

    outDiagnostic.stage = ArtifactDiagnostic::Stage::Compile;

    // Split by colons to extract diagnostic components
    List<UnownedStringSlice> split;
    StringUtil::split(line, ':', split);

    // Handle Windows drive letters (C:, D:, etc.)
    // Combine drive letter with path: ["C", "path", ...] -> ["C:path", ...]
    if (split.getCount() > 1 && split[0].getLength() == 1)
    {
        const char c = split[0][0];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        {
            UnownedStringSlice path(split[0].begin(), split[1].end());
            split.removeAt(0);
            split[0] = path;
        }
    }

    // Try each pattern matcher in priority order until one succeeds
    for (const auto& pattern : s_gccPatterns)
    {
        if (SLANG_SUCCEEDED(
                pattern.tryParse(allocator, line, split, outLineParseResult, outDiagnostic)))
        {
            return SLANG_OK;
        }
    }

    // No pattern matched - should not happen as _parseContinuation always succeeds
    outLineParseResult = LineParseResult::Continuation;
    return SLANG_OK;
}

/* static */ SlangResult GCCDownstreamCompilerUtil::parseOutput(
    const ExecuteResult& exeRes,
    IArtifactDiagnostics* diagnostics)
{
    LineParseResult prevLineResult = LineParseResult::Ignore;

    SliceAllocator allocator;

    diagnostics->reset();
    diagnostics->setRaw(SliceUtil::asCharSlice(exeRes.standardError));

    // Debug output for downstream compiler diagnostics
    // Set environment variable: SLANG_GCC_PARSER_DEBUG=1
    // This will print:
    //   1. Raw compiler stderr output
    //   2. Each parsed diagnostic with file:line:column and message
    // Useful for debugging compiler integration and error parsing issues
    // Thread-safe initialization using std::call_once
    static std::once_flag s_debugInitFlag;
    static bool s_debugEnabled = false;

    std::call_once(
        s_debugInitFlag,
        []()
        {
            StringBuilder envValue;
            if (SLANG_SUCCEEDED(PlatformUtil::getEnvironmentVariable(
                    UnownedStringSlice("SLANG_GCC_PARSER_DEBUG"),
                    envValue)))
            {
                UnownedStringSlice value = envValue.getUnownedSlice();
                if (value == "1" || value == "true" || value == "TRUE")
                {
                    s_debugEnabled = true;
                }
            }
        });

    if (s_debugEnabled)
    {
        // Debug output: show raw compiler output
        fprintf(stderr, "\n=== GCC/Clang Compiler Output (stderr) ===\n");
        fprintf(
            stderr,
            "%.*s",
            int(exeRes.standardError.getLength()),
            exeRes.standardError.begin());
        fprintf(stderr, "=== End Compiler Output ===\n\n");
        fprintf(stderr, "=== Parsing Diagnostics ===\n");
    }

    // We hold in workDiagnostics so as it is more convenient to append to the last with a
    // continuation also means we don't hold the allocations of building up continuations, just the
    // results when finally allocated at the end
    List<ArtifactDiagnostic> workDiagnostics;

    for (auto line : LineParser(exeRes.standardError.getUnownedSlice()))
    {
        ArtifactDiagnostic diagnostic;

        LineParseResult lineRes;

        SLANG_RETURN_ON_FAIL(_parseGCCFamilyLine(allocator, line, lineRes, diagnostic));

        switch (lineRes)
        {
        case LineParseResult::Start:
            {
                // It's start of a new message
                workDiagnostics.add(diagnostic);
                prevLineResult = LineParseResult::Start;
                break;
            }
        case LineParseResult::Single:
            {
                // It's a single message, without anything following
                workDiagnostics.add(diagnostic);
                prevLineResult = LineParseResult::Ignore;
                break;
            }
        case LineParseResult::Continuation:
            {
                if (prevLineResult == LineParseResult::Start ||
                    prevLineResult == LineParseResult::Continuation)
                {
                    if (workDiagnostics.getCount() > 0)
                    {
                        auto& last = workDiagnostics.getLast();

                        // TODO(JS): Note that this is somewhat wasteful as every time we append we
                        // just allocate more memory to hold the result. If we had an allocator
                        // dedicated to 'text' we could perhaps just append to the end of the last
                        // allocation
                        //
                        // We are now in a continuation, add to the last
                        StringBuilder buf;
                        buf.append(asStringSlice(last.text));
                        buf.append("\n");
                        buf.append(line);

                        last.text = allocator.allocate(buf);
                    }
                    prevLineResult = LineParseResult::Continuation;
                }
                break;
            }
        case LineParseResult::Ignore:
            {
                prevLineResult = lineRes;
                break;
            }
        default:
            return SLANG_FAIL;
        }
    }

    for (const auto& diagnostic : workDiagnostics)
    {
        diagnostics->add(diagnostic);
    }

    if (s_debugEnabled)
    {
        // Debug output: show parsed diagnostics
        fprintf(stderr, "=== Parsed Diagnostics Summary ===\n");
        fprintf(stderr, "Total diagnostics: %d\n", int(workDiagnostics.getCount()));
        for (Index i = 0; i < workDiagnostics.getCount(); ++i)
        {
            const auto& diag = workDiagnostics[i];
            const char* severityStr = "Unknown";
            switch (diag.severity)
            {
            case ArtifactDiagnostic::Severity::Error:
                severityStr = "Error";
                break;
            case ArtifactDiagnostic::Severity::Warning:
                severityStr = "Warning";
                break;
            case ArtifactDiagnostic::Severity::Info:
                severityStr = "Info";
                break;
            }
            const char* stageStr =
                diag.stage == ArtifactDiagnostic::Stage::Compile ? "Compile" : "Link";

            fprintf(stderr, "[%d] %s %s: ", int(i), stageStr, severityStr);

            if (asStringSlice(diag.filePath).getLength() > 0)
            {
                fprintf(
                    stderr,
                    "%.*s",
                    int(asStringSlice(diag.filePath).getLength()),
                    asStringSlice(diag.filePath).begin());
            }

            if (diag.location.line > 0)
            {
                fprintf(stderr, ":%d", int(diag.location.line));
                if (diag.location.column > 0)
                {
                    fprintf(stderr, ":%d", int(diag.location.column));
                }
            }

            fprintf(
                stderr,
                ": %.*s\n",
                int(asStringSlice(diag.text).getLength()),
                asStringSlice(diag.text).begin());
        }
        fprintf(stderr, "=== End Parsed Diagnostics ===\n\n");
    }

    if (diagnostics->hasOfAtLeastSeverity(ArtifactDiagnostic::Severity::Error) ||
        exeRes.resultCode != 0)
    {
        diagnostics->setResult(SLANG_FAIL);
    }

    return SLANG_OK;
}

/* static */ SlangResult GCCDownstreamCompilerUtil::calcCompileProducts(
    const CompileOptions& options,
    ProductFlags flags,
    IOSFileArtifactRepresentation* lockFile,
    List<ComPtr<IArtifact>>& outArtifacts)
{
    SLANG_ASSERT(options.modulePath.count);

    outArtifacts.clear();

    if (flags & ProductFlag::Execution)
    {
        StringBuilder builder;
        const auto desc = ArtifactDescUtil::makeDescForCompileTarget(options.targetType);
        SLANG_RETURN_ON_FAIL(
            ArtifactDescUtil::calcPathForDesc(desc, asStringSlice(options.modulePath), builder));

        auto fileRep = OSFileArtifactRepresentation::create(
            IOSFileArtifactRepresentation::Kind::Owned,
            builder.getUnownedSlice(),
            lockFile);
        auto artifact = ArtifactUtil::createArtifact(desc);
        artifact->addRepresentation(fileRep);

        outArtifacts.add(artifact);
    }

    return SLANG_OK;
}

/* static */ SlangResult GCCDownstreamCompilerUtil::calcArgs(
    const CompileOptions& options,
    CommandLine& cmdLine)
{
    SLANG_ASSERT(options.modulePath.count);

    PlatformKind platformKind = (options.platform == PlatformKind::Unknown)
                                    ? PlatformUtil::getPlatformKind()
                                    : options.platform;

    const auto targetDesc = ArtifactDescUtil::makeDescForCompileTarget(options.targetType);

    if (options.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP)
    {
        cmdLine.addArg("-fvisibility=hidden");

        // C++17 since we share headers with slang itself (which uses c++17)
        cmdLine.addArg("-std=c++17");
    }

    if (targetDesc.payload == ArtifactDesc::Payload::MetalAIR)
    {
        cmdLine.addArg("-std=metal3.1");
    }

    // Our generated code very often casts between dissimilar types with the
    // knowledge that they have the same representation. This is strictly
    // speaking UB, and GCC 10+ is happy to take advantage of this, stop it.
    cmdLine.addArg("-fno-strict-aliasing");

    // TODO(JS): Here we always set -m32 on x86. It could be argued it is only necessary when
    // creating a shared library but if we create an object file, we don't know what to choose
    // because we don't know what final usage is. It could also be argued that the platformKind
    // could define the actual desired target - but as it stands we only have a target of 'Linux'
    // (as opposed to Win32/64). Really it implies we need an arch enumeration too.
    //
    // For now we just make X86 binaries try and produce x86 compatible binaries as fixes the
    // immediate problems.
#if SLANG_PROCESSOR_X86
    /* Used to specify the processor more broadly. For a x86 binary we need to make sure we build
    x86 builds even when on an x64 system. -m32 -m64*/
    cmdLine.addArg("-m32");
#endif

    switch (options.optimizationLevel)
    {
    case OptimizationLevel::None:
        {
            // No optimization
            cmdLine.addArg("-O0");
            break;
        }
    case OptimizationLevel::Default:
        {
            cmdLine.addArg("-Os");
            break;
        }
    case OptimizationLevel::High:
        {
            cmdLine.addArg("-O2");
            break;
        }
    case OptimizationLevel::Maximal:
        {
            cmdLine.addArg("-O3");
            break;
        }
    default:
        break;
    }

    switch (options.debugInfoType)
    {
    case DebugInfoType::None:
        // gcc accepts -g0, but it is effectively the same as not passing -g at all.
        //  No debug info is generated.
        cmdLine.addArg("-g0");
        break;

    case DebugInfoType::Minimal:
        // Minimal: Line numbers only for stack traces
        // Use -g1 for minimal debug info or -gline-tables-only if available
        cmdLine.addArg("-g1");
        break;

    case DebugInfoType::Standard:
        // Standard: Full debug information (GCC default level)
        cmdLine.addArg("-g2");
        break;

    case DebugInfoType::Maximal:
        // Maximal: Maximum debug information including macro definitions
        cmdLine.addArg("-g3");
        break;
    }

    if (options.flags & CompileOptions::Flag::Verbose)
    {
        cmdLine.addArg("-v");
    }

    switch (options.floatingPointMode)
    {
    case FloatingPointMode::Default:
        break;
    case FloatingPointMode::Precise:
        {
            // cmdLine.addArg("-fno-unsafe-math-optimizations");
            break;
        }
    case FloatingPointMode::Fast:
        {
            // We could enable SSE with -mfpmath=sse
            // But that would only make sense on a x64/x86 type processor and only if that feature
            // is present (it is on all x64)
            cmdLine.addArg("-ffast-math");
            break;
        }
    }

    StringBuilder moduleFilePath;
    SLANG_RETURN_ON_FAIL(ArtifactDescUtil::calcPathForDesc(
        targetDesc,
        asStringSlice(options.modulePath),
        moduleFilePath));

    cmdLine.addArg("-o");
    cmdLine.addArg(moduleFilePath);

    switch (options.targetType)
    {
    case SLANG_SHADER_SHARED_LIBRARY:
    case SLANG_HOST_SHARED_LIBRARY:
        {
            // Shared library
            cmdLine.addArg("-shared");

            if (PlatformUtil::isFamily(PlatformFamily::Unix, platformKind))
            {
                // Position independent
                cmdLine.addArg("-fPIC");
            }
            break;
        }
    case SLANG_HOST_EXECUTABLE:
        {
            cmdLine.addArg("-rdynamic");
            break;
        }
    case SLANG_OBJECT_CODE:
        {
            // Don't link, just produce object file
            cmdLine.addArg("-c");
            break;
        }
    default:
        break;
    }

    // Add defines
    for (const auto& define : options.defines)
    {
        StringBuilder builder;

        builder << "-D";
        builder << define.nameWithSig;
        if (define.value.count)
        {
            builder << "=" << asStringSlice(define.value);
        }

        cmdLine.addArg(builder);
    }

    // Add includes
    for (const auto& include : options.includePaths)
    {
        cmdLine.addArg("-I");
        cmdLine.addArg(asString(include));
    }

    // Link options
    if (0) // && options.targetType != TargetType::Object)
    {
        // linkOptions << "-Wl,";
        // cmdLine.addArg(linkOptions);
    }

    if (options.targetType == SLANG_SHADER_SHARED_LIBRARY)
    {
        if (!PlatformUtil::isFamily(PlatformFamily::Apple, platformKind))
        {
            // On MacOS, this linker option is not supported. That's ok though in
            // so far as on MacOS it does report any unfound symbols without the option.

            // Linker flag to report any undefined symbols as a link error
            cmdLine.addArg("-Wl,--no-undefined");
        }
    }

    // Files to compile, need to be on the file system.
    for (IArtifact* sourceArtifact : options.sourceArtifacts)
    {
        ComPtr<IOSFileArtifactRepresentation> fileRep;

        // TODO(JS):
        // Do we want to keep the file on the file system? It's probably reasonable to do so.
        SLANG_RETURN_ON_FAIL(sourceArtifact->requireFile(ArtifactKeep::Yes, fileRep.writeRef()));
        cmdLine.addArg(fileRep->getPath());
    }

    // Add the library paths

    if (options.libraryPaths.count && (options.targetType == SLANG_HOST_EXECUTABLE))
    {
        if (PlatformUtil::isFamily(PlatformFamily::Apple, platformKind))
            cmdLine.addArg("-Wl,-rpath,@loader_path,-rpath,@loader_path/../lib");
        else
            cmdLine.addArg("-Wl,-rpath,$ORIGIN,-rpath,$ORIGIN/../lib");
    }

    StringSlicePool libPathPool(StringSlicePool::Style::Default);

    for (const auto& libPath : options.libraryPaths)
    {
        libPathPool.add(libPath);
    }

    // Artifacts might add library paths
    for (IArtifact* artifact : options.libraries)
    {
        const auto artifactDesc = artifact->getDesc();
        // If it's a library for CPU types, try and use it
        if (ArtifactDescUtil::isCpuBinary(artifactDesc) &&
            artifactDesc.kind == ArtifactKind::Library)
        {
            ComPtr<IOSFileArtifactRepresentation> fileRep;

            // Get the name and path (can be empty) to the library
            SLANG_RETURN_ON_FAIL(artifact->requireFile(ArtifactKeep::Yes, fileRep.writeRef()));

            const UnownedStringSlice path(fileRep->getPath());
            libPathPool.add(Path::getParentDirectory(path));

            cmdLine.addPrefixPathArg(
                "-l",
                ArtifactDescUtil::getBaseNameFromPath(artifact->getDesc(), path));
        }
    }

    if (options.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP &&
        !PlatformUtil::isFamily(PlatformFamily::Windows, platformKind))
    {
        // Make STD libs available
        cmdLine.addArg("-lstdc++");
        // Make maths lib available
        cmdLine.addArg("-lm");
    }

    for (const auto& libPath : libPathPool.getAdded())
    {
        // Note that any escaping of the path is handled in the ProcessUtil::
        cmdLine.addArg("-L");
        cmdLine.addArg(libPath);
        cmdLine.addArg("-F");
        cmdLine.addArg(libPath);
    }

    // Add compiler specific options from user.
    for (auto compilerSpecificArg : options.compilerSpecificArguments)
    {
        const char* const arg = compilerSpecificArg;
        cmdLine.addArg(arg);
    }

    return SLANG_OK;
}

/* static */ SlangResult GCCDownstreamCompilerUtil::createCompiler(
    const ExecutableLocation& exe,
    ComPtr<IDownstreamCompiler>& outCompiler)
{
    DownstreamCompilerDesc desc;
    SLANG_RETURN_ON_FAIL(GCCDownstreamCompilerUtil::calcVersion(exe, desc));

    auto compiler = new GCCDownstreamCompiler(desc);
    ComPtr<IDownstreamCompiler> compilerIntf(compiler);
    compiler->m_cmdLine.setExecutableLocation(exe);

    outCompiler.swap(compilerIntf);
    return SLANG_OK;
}

/* static */ SlangResult GCCDownstreamCompilerUtil::locateGCCCompilers(
    const String& path,
    ISlangSharedLibraryLoader* loader,
    DownstreamCompilerSet* set)
{
    SLANG_UNUSED(loader);

    ComPtr<IDownstreamCompiler> compiler;
    if (SLANG_SUCCEEDED(createCompiler(ExecutableLocation(path, "g++"), compiler)))
    {
        // A downstream compiler for Slang must currently support C++17 - such that
        // the prelude and generated code works.
        //
        // The first version of gcc that supports stable `-std=c++17` is 9.0
        // https://gcc.gnu.org/projects/cxx-status.html

        auto desc = compiler->getDesc();
        if (desc.version.m_major < 9)
        {
            // If the version isn't 9 or higher, we don't add this version of the compiler.
            return SLANG_OK;
        }

        set->addCompiler(compiler);
    }
    return SLANG_OK;
}

/* static */ SlangResult GCCDownstreamCompilerUtil::locateClangCompilers(
    const String& path,
    ISlangSharedLibraryLoader* loader,
    DownstreamCompilerSet* set)
{
    SLANG_UNUSED(loader);

    ComPtr<IDownstreamCompiler> compiler;
    if (SLANG_SUCCEEDED(createCompiler(ExecutableLocation(path, "clang++"), compiler)))
    {
        set->addCompiler(compiler);
    }
    return SLANG_OK;
}

} // namespace Slang
