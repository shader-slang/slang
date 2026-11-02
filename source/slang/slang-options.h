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

/// The use a command line is parsed for. The parser reads the same options for every purpose,
/// but the work after the last argument depends on what the result is used for.
enum class CommandLineParsePurpose
{
    /// A complete slangc command line: nothing is added to the request after parsing.
    CompleteCommandLine,

    /// A compile request that the caller configures incrementally through the API, and may add
    /// targets and entry points to after parsing.
    CompileRequest,

    /// The session description returned by `IGlobalSession::parseCommandLineArguments`. A session
    /// has targets and compiler options, but no source files, entry points or output paths.
    SessionDescription,
};

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
