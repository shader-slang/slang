#ifndef SLANG_DIAGNOSTICS_H
#define SLANG_DIAGNOSTICS_H

#include "../compiler-core/slang-diagnostic-sink.h"
#include "../compiler-core/slang-source-loc.h"
#include "../compiler-core/slang-token.h"
#include "../core/slang-basic.h"
#include "../core/slang-writer.h"
#include "slang.h"

namespace Slang
{
DiagnosticInfo const* findDiagnosticByName(UnownedStringSlice const& name);
const DiagnosticsLookup* getDiagnosticsLookup();
SlangResult overrideDiagnostic(
    DiagnosticSink* sink,
    DiagnosticSink* outDiagnostic,
    const UnownedStringSlice& identifier,
    Severity originalSeverity,
    Severity overrideSeverity);
SlangResult overrideDiagnostics(
    DiagnosticSink* sink,
    DiagnosticSink* outDiagnostic,
    const UnownedStringSlice& identifierList,
    Severity originalSeverity,
    Severity overrideSeverity);

namespace Diagnostics
{
#define DIAGNOSTIC(id, severity, name, messageFormat) extern const DiagnosticInfo name;
#include "slang-diagnostic-defs.h"
} // namespace Diagnostics
} // namespace Slang

// NOTE: The SLANG_INTERNAL_ERROR, SLANG_UNIMPLEMENTED, and SLANG_DIAGNOSE_UNEXPECTED macros
// require the full struct definitions from slang-rich-diagnostics.h.
// Users of these macros must include slang-rich-diagnostics.h in their source files.
// These macros use diagnoseWithPos which extracts SourceLoc from various position types.
// The struct initialization uses positional args: Unimplemented{feature}, Unexpected{message}

#ifdef _DEBUG
#define SLANG_INTERNAL_ERROR(sink, pos)                                                     \
    (sink)->diagnoseWithPos(                                                                \
        Slang::SourceLoc(__LINE__, 0, 0, __FILE__),                                         \
        Slang::Diagnostics::InternalCompilerError{})
#define SLANG_UNIMPLEMENTED(sink, pos, what)                                                \
    (sink)->diagnoseWithPos(                                                                \
        Slang::SourceLoc(__LINE__, 0, 0, __FILE__),                                         \
        Slang::Diagnostics::Unimplemented{(what)})

#else
#define SLANG_INTERNAL_ERROR(sink, pos)                                                     \
    (sink)->diagnoseWithPos((pos), Slang::Diagnostics::InternalCompilerError{})
#define SLANG_UNIMPLEMENTED(sink, pos, what)                                                \
    (sink)->diagnoseWithPos((pos), Slang::Diagnostics::Unimplemented{(what)})

#endif

#define SLANG_DIAGNOSE_UNEXPECTED(sink, pos, message)                                       \
    (sink)->diagnoseWithPos((pos), Slang::Diagnostics::Unexpected{(message)})

#endif
