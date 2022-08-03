// slang-artifact-info.h
#ifndef SLANG_ARTIFACT_INFO_H
#define SLANG_ARTIFACT_INFO_H

#include "slang-artifact.h"

namespace Slang
{

struct ArtifactInfoUtil
{
    typedef ArtifactPayload Payload;
    typedef ArtifactKind Kind;

        /// Returns true if the kind is binary linkable 
    static bool isKindBinaryLinkable(Kind kind);

        /// True if is a CPU target - either
    static bool isCpuLikeTarget(const ArtifactDesc& desc);
    
        /// True if is a CPU binary
    static bool isCpuBinary(const ArtifactDesc& desc);
        /// True if is a GPU usable (can be passed to a driver/API and be used 
    static bool isGpuUsable(const ArtifactDesc& desc);

        /// True if the desc holds textual information
    static bool isText(const ArtifactDesc& desc);

        /// Given an assembly type returns it's extension from the payload type
    static UnownedStringSlice getAssemblyExtensionForPayload(ArtifactPayload payload);

        /// True if artifact  appears to be linkable
    static bool isLinkable(const ArtifactDesc& desc);

        /// Try to determine the desc from just a file extension (passed without .)
    static ArtifactDesc getDescFromExtension(const UnownedStringSlice& slice);

        /// Try to determine the desc from a path
    static ArtifactDesc getDescFromPath(const UnownedStringSlice& slice);

        /// Gets the default file extension for the artifact type. Returns empty slice if not known
    static UnownedStringSlice getDefaultExtension(const ArtifactDesc& desc);

        /// Get the extension for CPU/Host for a kind
    static UnownedStringSlice getCpuExtensionForKind(Kind kind);

        /// Given a desc and a path returns the base name (stripped of prefix and extension)
    static String getBaseNameFromPath(const ArtifactDesc& desc, const UnownedStringSlice& path);

        /// Get the base name of this artifact.
        /// If there is a path set, will extract the name from that (stripping prefix, extension as necessary).
        /// Else if there is an explicit name set, this is returned.
        /// Else returns the empty string
    static String getBaseName(IArtifact* artifact);
    static String getBaseName(const ArtifactDesc& desc, IFileArtifactRepresentation* fileRep);

        /// Get the parent path (empty if there isn't one)
    static String getParentPath(IArtifact* artifact);
    static String getParentPath(IFileArtifactRepresentation* fileRep);

        /// Given a desc, and a basePath returns a suitable path for a entity of specified desc
    static SlangResult calcPathForDesc(const ArtifactDesc& desc, const UnownedStringSlice& basePath, StringBuilder& outPath);

};

} // namespace Slang

#endif
