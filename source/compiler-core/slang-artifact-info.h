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
    static bool isCpuTarget(const ArtifactDesc& desc);
    
        /// True if is a CPU binary
    static bool isCpuBinary(const ArtifactDesc& desc) { return isDerivedFrom(desc.kind, ArtifactKind::Binary) && isDerivedFrom(desc.payload, ArtifactPayload::CPU); }
        /// True if is a GPU binary
    static bool isGpuBinary(const ArtifactDesc& desc) { return isDerivedFrom(desc.kind, ArtifactKind::Binary) && isDerivedFrom(desc.payload, ArtifactPayload::Kernel); }

        /// Given an assembly type returns it's extension from the payload type
    static UnownedStringSlice getAssemblyExtensionForPayload(ArtifactPayload payload);

        /// True if artifact  appears to be binary linkable
    static bool isBinaryLinkable(const ArtifactDesc& desc);

        /// Get the default extension type for a payload type
    //static UnownedStringSlice getDefaultExtensionForPayload(Payload payload);

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

        /// Get the parent path (empty if there isn't one)
    static String getParentPath(IArtifact* artifact);
};

} // namespace Slang

#endif
