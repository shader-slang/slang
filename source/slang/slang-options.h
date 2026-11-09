// slang-options.h
#ifndef SLANG_OPTIONS_H
#define SLANG_OPTIONS_H

#include "compiler-core/slang-source-loc.h"
#include "core/slang-basic.h"

namespace Slang
{

struct CommandOptions;
class DiagnosticSink;
class IArtifact;

UnownedStringSlice getCodeGenTargetName(SlangCompileTarget target);

/// The purpose for which a command line is parsed. Every purpose reads the same options, and the
/// purpose decides part of what the parser does with them, both while it reads the arguments and
/// after the last one:
///
/// - A compile (`CompleteCommandLine` or `CompileRequest`) loads positional input paths as it
///   reads them. After the last argument it resolves entry points and output paths, and gives a
///   target its full option set when it binds an output to that target. A session description
///   ignores positional input paths, skips that work, and gives each target the target options
///   that apply to it.
/// - A floating-point mode given with no `-target` is reported as ignored only for
///   `CompleteCommandLine`, because for the other purposes targets added after the parse
///   inherit it.
/// - A target option other than the profile and the floating-point mode, given before several
///   targets, is rejected only for `SessionDescription`.
///
/// The parser tests a purpose through `parsesForCompilation` and `canAddTargetsAfterParse`.
enum class CommandLineParsePurpose
{
    /// A complete slangc command line, parsed by a compile request in command-line compiler mode
    /// (`setCommandLineCompilerMode`). Nothing adds targets to the request after the parse.
    CompleteCommandLine,

    /// A command line given through the deprecated `ICompileRequest::processCommandLineArguments`
    /// to a request that is not in command-line compiler mode. The caller may add targets and
    /// entry points to the request after the parse.
    CompileRequest,

    /// The session description returned by `IGlobalSession::parseCommandLineArguments`. A session
    /// has targets and compiler options, but no source files, entry points or output paths.
    SessionDescription,
};

/// Parse the slangc command line `argv` into `compileRequestIn`, doing the work that `purpose`
/// calls for. Some options, such as `-load-core-module`, also act on the request's global
/// session. Diagnostics go to the request's sink, and the result is a failure if any error was
/// diagnosed, including by the checks made after the last argument.
SlangResult parseOptions(
    SlangCompileRequest* compileRequestIn,
    int argc,
    char const* const* argv,
    CommandLineParsePurpose purpose);

// Initialize command options. Holds the details how parsing works.
void initCommandOptions(CommandOptions& commandOptions);

enum class Stage : SlangUInt32;

SlangSourceLanguage findSourceLanguageFromPath(const String& path, Stage& outImpliedStage);

SlangResult createArtifactFromReferencedModule(
    String path,
    SourceLoc loc,
    DiagnosticSink* sink,
    IArtifact** outArtifact);

} // namespace Slang
#endif
