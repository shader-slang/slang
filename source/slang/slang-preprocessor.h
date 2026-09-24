// Preprocessor.h
#ifndef SLANG_PREPROCESSOR_H_INCLUDED
#define SLANG_PREPROCESSOR_H_INCLUDED

#include "compiler-core/slang-include-system.h"
#include "compiler-core/slang-lexer.h"
#include "core/slang-basic.h"
#include "slang-profile.h"

namespace Slang
{

class DiagnosticSink;
class Linkage;
struct PreprocessorContentAssistInfo;

namespace preprocessor
{
struct Preprocessor;
}
using preprocessor::Preprocessor;

/// A handler for callbacks invoked by the preprocessor.
///
/// A client of the preprocessor can implement its own `PreprocessorHandler` subtype
/// in order to insert custom logic that implements higher-level policies
/// that the preprocessor shouldn't need to understand.
///
struct PreprocessorHandler
{
    virtual void handleEndOfTranslationUnit(Preprocessor* preprocessor);
    virtual void handleFileDependency(SourceFile* sourceFile);
};

/// Records where a `#pragma pack_matrix` state change takes effect in a preprocessed token stream.
/// The offset indexes the emitted token list.
/// The new mode applies to the token at that offset and all following tokens until another event.
struct MatrixLayoutPragmaEvent
{
    Index tokenOffset;
    SlangMatrixLayoutMode mode;
};

/// Stores the matrix-layout state before, during, and after one preprocessed token stream.
/// Events have strictly increasing offsets. Directives before the same output token coalesce.
/// A directive at EOF changes `finalMode` without adding an event. An Unknown initial or final mode
/// means that the target's matrix-layout default still applies. The timeline becomes immutable once
/// preprocessing completes, so parsers for deferred token ranges can share it.
struct MatrixLayoutPragmaInfo : RefObject
{
    SlangMatrixLayoutMode initialMode = SLANG_MATRIX_LAYOUT_MODE_UNKNOWN;
    List<MatrixLayoutPragmaEvent> events;
    SlangMatrixLayoutMode finalMode = SLANG_MATRIX_LAYOUT_MODE_UNKNOWN;

    /// Locations of valid `#pragma pack_matrix` directives in this token stream.
    List<SourceLoc> directiveLocations;
};

/// Description of a preprocessor options/dependencies
struct PreprocessorDesc
{
    /// Required: sink to use when emitting preprocessor diagnostic messages
    DiagnosticSink* sink = nullptr;

    /// Required: name pool to use when creating `Name`s from strings
    NamePool* namePool = nullptr;

    /// Required: file system to use when looking up files
    ISlangFileSystemExt* fileSystem = nullptr;

    /// Required: source manager to use when loading source files
    SourceManager* sourceManager = nullptr;

    /// Optional: include system to use when resolving `#include` directives
    IncludeSystem* includeSystem = nullptr;

    /// Optional: preprocessor `#define`s to assume are set on input
    Dictionary<String, String> const* defines = nullptr;

    /// Optional: handler for callbacks invoked during preprocessing
    PreprocessorHandler* handler = nullptr;

    /// Optional: additional information for code assist.
    PreprocessorContentAssistInfo* contentAssistInfo = nullptr;

    /// Matrix-layout state inherited from an earlier segment of the same source file.
    SlangMatrixLayoutMode initialMatrixLayoutMode = SLANG_MATRIX_LAYOUT_MODE_UNKNOWN;

    /// Optional: receives matrix-layout state changes produced during preprocessing.
    MatrixLayoutPragmaInfo* matrixLayoutPragmaInfo = nullptr;
};

/// The first source-language selection discovered while preprocessing one source segment.
///
/// The preprocessor diagnoses later conflicting directives and preserves this first selection so
/// the translation unit can choose one parser mode before any source file is parsed. A Slang
/// `#language` directive also records its version so the translation unit can enforce one version
/// for parsing and semantic checking of the complete module.
struct SourceLanguageDirective
{
    /// The selected language, or `Unknown` when the source contains no language directive.
    SourceLanguage language = SourceLanguage::Unknown;

    /// The location of the first directive that selected `language`.
    SourceLoc location;

    /// The version selected by a Slang `#language` directive, or `Unknown` for another language or
    /// when the source contains no directive.
    SlangLanguageVersion slangLanguageVersion = SLANG_LANGUAGE_VERSION_UNKNOWN;
};

/// Preprocess `file` and return its tokens and first source-language directive, if any.
///
/// Conflicting directives in the same source segment are diagnosed and do not replace the
/// first selection. `outLanguageVersion` changes only when a valid Slang `#language` directive is
/// present.
TokenList preprocessSource(
    SourceFile* file,
    PreprocessorDesc const& desc,
    SourceLanguageDirective& outSourceLanguageDirective,
    SlangLanguageVersion& outLanguageVersion);

/// Preprocess `file` using services and language-server state supplied by `linkage`.
///
/// The output and conflict behavior match the `PreprocessorDesc` overload.
TokenList preprocessSource(
    SourceFile* file,
    DiagnosticSink* sink,
    IncludeSystem* includeSystem,
    Dictionary<String, String> const& defines,
    Linkage* linkage,
    SourceLanguageDirective& outSourceLanguageDirective,
    SlangLanguageVersion& outLanguageVersion,
    PreprocessorHandler* handler = nullptr,
    SlangMatrixLayoutMode initialMatrixLayoutMode = SLANG_MATRIX_LAYOUT_MODE_UNKNOWN,
    MatrixLayoutPragmaInfo* matrixLayoutPragmaInfo = nullptr);

// The following functions are intended to be used inside of implementations
// of the `PreprocessorHandler` interface, in order to query the current
// state of the preprocessor.

/// Try to look up a macro with the given `macroName` and produce its value as a string
Result findMacroValue(
    Preprocessor* preprocessor,
    char const* macroName,
    String& outValue,
    SourceLoc& outLoc);

} // namespace Slang

#endif
