// slang-artifact-util.h
#ifndef SLANG_ARTIFACT_UTIL_H
#define SLANG_ARTIFACT_UTIL_H

#include "slang-artifact.h"
#include "slang-artifact-representation.h"

namespace Slang
{

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

        /// Returns the rep of the artifact that contains a temporary (identified by having a lock) on 
        /// the OS file system
    static IFileArtifactRepresentation* findFileSystemTemporaryFile(IArtifact* artifact);
        /// Returns true if the artifact contains a file on the OS file system
    static IFileArtifactRepresentation* findFileSystemFile(IArtifact* artifact);
        /// Returns the rep of a file system file which *isn't* temporary
    static IFileArtifactRepresentation* findFileSystemPrimaryFile(IArtifact* artifact);

        /// Find the path/name associated with the artifact.
        /// The path is *not* necessarily the path on the file system. The order of search is 
        /// * If the artifact has a name return that
        /// * If the artifact has a IFileArtifactRepresentation (that isn't temporary) return it's path 
        /// * If not found return an empty slice
    static UnownedStringSlice findPath(IArtifact* artifact);

};

} // namespace Slang

#endif
