// slang-artifact-diagnostic-util.h
#ifndef SLANG_ARTIFACT_DIAGNOSTIC_UTIL_H
#define SLANG_ARTIFACT_DIAGNOSTIC_UTIL_H

#include "slang-artifact-associated.h"
#include "slang-artifact.h"
#include "slang-diagnostic-sink.h"
#include "slang-slice-allocator.h"

namespace Slang
{

class DiagnosticSink;

struct ArtifactDiagnosticUtil
{
    typedef ArtifactDiagnostic::Severity Severity;

    /// Map an artifact diagnostic severity onto the DiagnosticSink scale:
    /// Info becomes a note, Warning passes through unchanged, anything
    /// unrecognized is reported as an error. One source of truth for this
    /// mapping — downstream-compiler reporting (slang-pass-through.cpp)
    /// delegates here.
    static Slang::Severity getSinkSeverity(ArtifactDiagnostic::Severity severity);

    /// Report the diagnostics attached to `artifact` to `sink` when `result`
    /// indicates a failure. Every attached diagnostic is reported verbatim,
    /// whatever its severity — no filtering happens (current producers only
    /// attach error-severity diagnostics, but a future producer attaching a
    /// warning would have it echoed). Nothing happens when `result` is a
    /// success or the artifact carries no `IArtifactDiagnostics`
    /// representation. Downstream code generators (for example the LLVM
    /// builder) package their failures as artifacts rather than writing to the
    /// sink themselves, so without this step such a failure would return
    /// without any visible message.
    static void reportDiagnosticsOnFailure(
        SlangResult result,
        IArtifact* artifact,
        DiagnosticSink* sink);

    /// Report every diagnostic attached to `artifact` to `sink`, whatever its
    /// severity. Use when the failure is signaled by the error artifact
    /// itself rather than by a `SlangResult`.
    static void reportDiagnostics(IArtifact* artifact, DiagnosticSink* sink);

    /// Given severity return as text
    static UnownedStringSlice getSeverityText(Severity severity);

    /// Given a path, that holds line number and potentially column number in () after path, writes
    /// result into outDiagnostic
    static SlangResult splitPathLocation(
        SliceAllocator& allocator,
        const UnownedStringSlice& pathLocation,
        ArtifactDiagnostic& outDiagnostic);

    /// Split the line (separated by :), where a path is at pathIndex
    static SlangResult splitColonDelimitedLine(
        const UnownedStringSlice& line,
        Int pathIndex,
        List<UnownedStringSlice>& outSlices);

    typedef SlangResult (*LineParser)(
        SliceAllocator& allocator,
        const UnownedStringSlice& line,
        List<UnownedStringSlice>& lineSlices,
        ArtifactDiagnostic& outDiagnostic);

    /// Given diagnostics in inText that are colon delimited, use lineParser to do per line parsing.
    static SlangResult parseColonDelimitedDiagnostics(
        SliceAllocator& allocator,
        const UnownedStringSlice& inText,
        Int pathIndex,
        LineParser lineParser,
        IArtifactDiagnostics* diagnostics);

    /// Maybe add a note
    static void maybeAddNote(const UnownedStringSlice& in, IArtifactDiagnostics* diagnostics);
};

} // namespace Slang

#endif
