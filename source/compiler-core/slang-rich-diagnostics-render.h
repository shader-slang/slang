#pragma once

#include "../core/slang-list.h"
#include "../core/slang-string.h"
#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-source-loc.h"

namespace Slang
{

struct SourceManager;

struct DiagnosticSpan
{
    // TODO: allow getting ranges instead of Locs here, currently we fall
    // back to using SourceLocationLexer
    Slang::SourceLoc loc;
    String message;
};

struct DiagnosticNote
{
    String message;
    DiagnosticSpan span;
};

//
// A struct capable of representing any diagnostic we want to display
//
struct GenericDiagnostic
{
    int code;
    Severity severity;
    String message;
    DiagnosticSpan primarySpan;
    List<DiagnosticSpan> secondarySpans;
    List<DiagnosticNote> notes;
};

String renderDiagnostic(
    DiagnosticSink::SourceLocationLexer sll,
    SourceManager* sm,
    const GenericDiagnostic& diag);

#ifdef SLANG_ENABLE_DIAGNOSTIC_RENDER_UNIT_TESTS
int slangRichDiagnosticsUnitTest(int argc, char* argv[]);
#endif

} // namespace Slang
