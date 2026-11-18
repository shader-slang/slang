// slang-artifact-diagnostic-util.h
#ifndef SLANG_ARTIFACT_DIAGNOSTIC_UTIL_H
#define SLANG_ARTIFACT_DIAGNOSTIC_UTIL_H

#include "slang-artifact-associated.h"
#include "slang-artifact.h"
#include "slang-slice-allocator.h"

namespace Slang
{

class DiagnosticSink;

struct ArtifactDiagnosticUtil
{
    typedef ArtifactDiagnostic::Severity Severity;

    /// Report the diagnostics attached to an error `artifact` to `sink` when
    /// `result` indicates a failure. Downstream code generators (for example
    /// the LLVM builder) package their failures as artifacts rather than
    /// writing to the sink themselves, so without this step such a failure
    /// would return without any visible message.
    static void reportDiagnosticsOnFailure(
        SlangResult result,
        IArtifact* artifact,
        DiagnosticSink* sink);

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
