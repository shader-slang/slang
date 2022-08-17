// slang-artifact-util.h
#ifndef SLANG_ARTIFACT_UTIL_H
#define SLANG_ARTIFACT_UTIL_H

#include "slang-artifact.h"
#include "slang-artifact-representation.h"

#include "../core/slang-memory-arena.h"

namespace Slang
{

struct ArtifactSliceUtil
{
        /// The slice will only be in scope whilst the string is
    static ZeroTerminatedCharSlice asSlice(const String& in) { auto unowned = in.getUnownedSlice(); return ZeroTerminatedCharSlice(unowned.begin(), unowned.getLength()); }

private:
        // We don't want temporaries to be 'asSliced' so disable
    static ZeroTerminatedCharSlice asSlice(const String&& in) = delete;
};

struct ArtifactSliceAllocator
{
    ZeroTerminatedCharSlice allocate(const Slice<char>& slice);
    ZeroTerminatedCharSlice allocate(const UnownedStringSlice& slice);
    ZeroTerminatedCharSlice allocate(const String& in) { return allocate(in.getUnownedSlice()); }
    ZeroTerminatedCharSlice allocate(const char* in);
    ZeroTerminatedCharSlice allocate(const char* start, const char* end) { return allocate(UnownedStringSlice(start, end); }

    void deallocateAll() { m_arena.deallocateAll(); }

    ArtifactSliceAllocator():
        m_arena(1024)
    {
    }
protected:
    
    MemoryArena m_arena;
};

struct ArtifactUtil
{

        /// Get the base name of this artifact.
        /// If there is a path set, will extract the name from that (stripping prefix, extension as necessary).
        /// Else if there is an explicit name set, this is returned.
        /// Else returns the empty string
    static String getBaseName(IArtifact* artifact);

        /// Get the parent path (empty if there isn't one)
    static String getParentPath(IArtifact* artifact);
    static String getParentPath(IFileArtifactRepresentation* fileRep);

        /// Create an empty container which is compatible with the desc
    static ComPtr<IArtifactContainer> createContainer(const ArtifactDesc& desc);

        /// Create a generic container
    static ComPtr<IArtifactContainer> createResultsContainer();

        /// Creates an empty artifact for a type
    static ComPtr<IArtifact> createArtifactForCompileTarget(SlangCompileTarget target);

        /// Create an artifact 
    static ComPtr<IArtifact> createArtifact(const ArtifactDesc& desc, const char* name);
    static ComPtr<IArtifact> createArtifact(const ArtifactDesc& desc);

        /// Returns true if an artifact is 'significant'
    static bool isSignificant(IArtifact* artifact, void* data = nullptr);
        /// Find a significant artifact
    static IArtifact* findSignificant(IArtifact* artifact);
};

} // namespace Slang

#endif
