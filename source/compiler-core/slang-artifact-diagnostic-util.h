// slang-artifact-diagnostic-util.h
#ifndef SLANG_ARTIFACT_DIAGNOSTIC_UTIL_H
#define SLANG_ARTIFACT_DIAGNOSTIC_UTIL_H

#include "slang-artifact.h"

#include "slang-artifact-associated.h"

#include "../core/slang-memory-arena.h"

namespace Slang
{

/* 
The reason to wrap in a struct rathan than have as free functions is doing so will lead to compile time 
errors with incorrect usage around temporarires.
*/
struct CharSliceCaster
{
        /// The slice will only be in scope whilst the string is
    static TerminatedCharSlice asTerminatedCharSlice(const String& in) { auto unowned = in.getUnownedSlice(); return TerminatedCharSlice(unowned.begin(), unowned.getLength()); }

    static CharSlice asCharSlice(const String& in) { auto unowned = in.getUnownedSlice(); return CharSlice(unowned.begin(), unowned.getLength()); }

private:
        // We don't want temporaries to be 'asSliced' so disable
    static TerminatedCharSlice asTerminatedCharSlice(const String&& in) = delete;
    static CharSlice asCharSlice(const String&& in) = delete;
};

SLANG_FORCE_INLINE UnownedStringSlice asStringSlice(const CharSlice& slice)
{
    return UnownedStringSlice(slice.begin(), slice.end());
}

SLANG_FORCE_INLINE CharSlice asCharSlice(const UnownedStringSlice& slice)
{
    return CharSlice(slice.begin(), slice.getLength());
}

struct CharSliceAllocator
{
    TerminatedCharSlice allocate(const Slice<char>& slice);
    TerminatedCharSlice allocate(const UnownedStringSlice& slice);
    TerminatedCharSlice allocate(const String& in) { return allocate(in.getUnownedSlice()); }
    TerminatedCharSlice allocate(const char* in);
    TerminatedCharSlice allocate(const char* start, const char* end) { return allocate(UnownedStringSlice(start, end)); }

    void deallocateAll() { m_arena.deallocateAll(); }

    CharSliceAllocator():
        m_arena(1024)
    {
    }
protected:
    
    MemoryArena m_arena;
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactDiagnosticUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

struct ArtifactDiagnosticUtil
{
    typedef ArtifactDiagnostic::Severity Severity;

    /// Given severity return as text
    static UnownedStringSlice getSeverityText(Severity severity);

    /// Given a path, that holds line number and potentially column number in () after path, writes result into outDiagnostic
    static SlangResult splitPathLocation(CharSliceAllocator& allocator, const UnownedStringSlice& pathLocation, ArtifactDiagnostic& outDiagnostic);

    /// Split the line (separated by :), where a path is at pathIndex 
    static SlangResult splitColonDelimitedLine(const UnownedStringSlice& line, Int pathIndex, List<UnownedStringSlice>& outSlices);

    typedef SlangResult(*LineParser)(CharSliceAllocator& allocator, const UnownedStringSlice& line, List<UnownedStringSlice>& lineSlices, ArtifactDiagnostic& outDiagnostic);

    /// Given diagnostics in inText that are colon delimited, use lineParser to do per line parsing.
    static SlangResult parseColonDelimitedDiagnostics(CharSliceAllocator& allocator, const UnownedStringSlice& inText, Int pathIndex, LineParser lineParser, IArtifactDiagnostics* diagnostics);

    /// Maybe add a note
    static void maybeAddNote(const UnownedStringSlice& in, IArtifactDiagnostics* diagnostics);
};


} // namespace Slang

#endif
