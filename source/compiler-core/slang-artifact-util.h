// slang-artifact-util.h
#ifndef SLANG_ARTIFACT_UTIL_H
#define SLANG_ARTIFACT_UTIL_H

#include "slang-artifact.h"
#include "slang-artifact-representation.h"

namespace Slang
{

struct ArtifactUtil
{
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

        /// Find the path/name associated with the artifact.
        /// The path is *not* necessarily the path on the file system. The order of search is 
        /// * If the artifact has a name return that
        /// * If the artifact has a IPathFileArtifactRepresentation (that isn't temporary) return it's path 
        /// * If not found return an empty slice
    static UnownedStringSlice findPath(IArtifact* artifact);
        /// Find a name
    static UnownedStringSlice findName(IArtifact* artifact);

        /// Sometimes we have artifacts that don't specify a payload type - perhaps because they can be interpretted in different ways
        /// This function uses the associated name and file representations to infer a extension. If none is found returns an empty slice.
    static UnownedStringSlice inferExtension(IArtifact* artifact);

        /// Given a desc and a basePath returns a suitable path for a entity of specified desc
    static SlangResult calcPath(IArtifact* artifact, const UnownedStringSlice& basePath, StringBuilder& outPath);

        /// Given a desc and a baseName works out the the output file name
    static SlangResult calcName(IArtifact* artifact, const UnownedStringSlice& baseName, StringBuilder& outName);
};

} // namespace Slang

#endif
