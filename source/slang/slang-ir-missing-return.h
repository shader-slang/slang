// slang-ir-missing-return.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;
enum class CodeGenTarget;

/// @brief Checks for missing returns
///
/// @param[in]  module           IR module
/// @param[out] sink             Diagnostics sink
/// @param[in]  languageVersion  Source language version. If the language is not Slang or not known,
///                              set SlangLanguageVersion::SLANG_LANGUAGE_VERSION_UNKNOWN
/// @param[in]  target           Compilation target
/// @param[in]  diagnoseWarning  Whether warnings should be diagnose
///
/// This IR check pass is performed twice:
/// - once during IR lowering with source language version set and code gen
///   target CodeGenTarget::None; and
/// - once during linking with source language version
///   SlangLanguageVersion::SLANG_LANGUAGE_VERSION_UNKNOWN and code gen target
///   specified.
///
/// Slang language version 202C and above makes missing returns an unconditional
/// error. (GitHub issue #12264)
///
/// Some code gen targets allow missing returns while some do not. When the
/// source language allows missing returns, the first pass, where no target is
/// specified, emits warnings on missing returns, and the second pass may
/// additionally emit errors when compiling for targets that do not support
/// missing returns.
///
/// On the second pass, `diagnoseWarning` is set to false to suppress warnings, ensuring that only
/// errors are emitted. This prevents duplicate warnings from appearing in both passes.
///
void checkForMissingReturns(
    IRModule* module,
    DiagnosticSink* sink,
    SlangLanguageVersion languageVersion,
    CodeGenTarget target,
    bool diagnoseWarning);

} // namespace Slang
