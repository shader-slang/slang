#pragma once

#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-source-loc.h"
#include "core/slang-list.h"
#include "core/slang-string.h"

namespace Slang
{

struct SourceManager;

struct DiagnosticSpan
{
    Slang::SourceRange range;
    String message;
};

struct DiagnosticNote
{
    String message;
    DiagnosticSpan span;
    List<DiagnosticSpan> secondarySpans;
    /// The originating diagnostic definition, for use by renderers that need the diagnostic id.
    /// Points into static storage; may be null for synthetic notes.
    const DiagnosticInfo* diagnosticInfo = nullptr;
};

//
// A struct capable of representing any diagnostic we want to display
//
struct GenericDiagnostic
{
    Int64 code;
    Severity severity;
    String message;
    DiagnosticSpan primarySpan;
    List<DiagnosticSpan> secondarySpans;
    List<DiagnosticNote> notes;
};

struct DiagnosticRenderOptions
{
    bool enableTerminalColors = false;
    bool enableUnicode = false;
};

String renderDiagnostic(
    DiagnosticSink::SourceLocationLexer sll,
    SourceManager* sm,
    DiagnosticRenderOptions opts,
    const GenericDiagnostic& diag);

String renderDiagnosticMachineReadable(
    DiagnosticSink::SourceLocationLexer sll,
    SourceManager* sm,
    const GenericDiagnostic& diag);

} // namespace Slang
