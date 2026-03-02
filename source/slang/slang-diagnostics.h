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

// All diagnostics are now defined in slang-diagnostics.lua and generated
// via slang-rich-diagnostics.h. The old slang-diagnostic-defs.h has been removed.
} // namespace Slang

// NOTE: The SLANG_INTERNAL_ERROR, SLANG_UNIMPLEMENTED, and SLANG_DIAGNOSE_UNEXPECTED macros
// require the full struct definitions from slang-rich-diagnostics.h.
// Users of these macros must include slang-rich-diagnostics.h in their source files.

// Helper macros for stringification
#define SLANG_DIAG_STRINGIFY_(x) #x
#define SLANG_DIAG_STRINGIFY(x) SLANG_DIAG_STRINGIFY_(x)

#ifdef _DEBUG
// In debug builds, emit both the user's source location and the compiler location
// Note is emitted first since the main diagnostic may abort compilation
#define SLANG_INTERNAL_ERROR(sink, pos)                                                            \
    do                                                                                             \
    {                                                                                              \
        (sink)->diagnoseRaw(                                                                       \
            Slang::Severity::Note,                                                                 \
            "note: internal error triggered at " __FILE__                                          \
            ":" SLANG_DIAG_STRINGIFY(__LINE__) "\n");                                              \
        (sink)->diagnose(Slang::Diagnostics::InternalCompilerError{Slang::getDiagnosticPos(pos)}); \
    } while (0)
#define SLANG_UNIMPLEMENTED(sink, pos, what)                                                       \
    do                                                                                             \
    {                                                                                              \
        (sink)->diagnoseRaw(                                                                       \
            Slang::Severity::Note,                                                                 \
            "note: unimplemented triggered at " __FILE__ ":" SLANG_DIAG_STRINGIFY(__LINE__) "\n"); \
        (sink)->diagnose(Slang::Diagnostics::Unimplemented{(what), Slang::getDiagnosticPos(pos)}); \
    } while (0)
#else
#define SLANG_INTERNAL_ERROR(sink, pos) \
    (sink)->diagnose(Slang::Diagnostics::InternalCompilerError{Slang::getDiagnosticPos(pos)})
#define SLANG_UNIMPLEMENTED(sink, pos, what) \
    (sink)->diagnose(Slang::Diagnostics::Unimplemented{(what), Slang::getDiagnosticPos(pos)})
#endif

#define SLANG_DIAGNOSE_UNEXPECTED(sink, pos, message) \
    (sink)->diagnose(Slang::Diagnostics::Unexpected{(message), Slang::getDiagnosticPos(pos)})

#endif
