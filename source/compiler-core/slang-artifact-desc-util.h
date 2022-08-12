// slang-artifact-desc.h

#ifndef SLANG_ARTIFACT_DESC_UTIL_H
#define SLANG_ARTIFACT_DESC_UTIL_H

#include "slang-artifact.h"

namespace Slang
{

    /// Get the parent kind
ArtifactKind getParent(ArtifactKind kind);
    /// Returns true if kind is derived from base
bool isDerivedFrom(ArtifactKind kind, ArtifactKind base);
    /// Get the name for the kind
UnownedStringSlice getName(ArtifactKind kind);

    /// Get the parent payload
ArtifactPayload getParent(ArtifactPayload payload); 
    /// Returns true if payload is derived from base
bool isDerivedFrom(ArtifactPayload payload, ArtifactPayload base);
    /// Get the name for the payload
UnownedStringSlice getName(ArtifactPayload payload);

    /// Get the parent style
ArtifactStyle getParent(ArtifactStyle style);
    /// Returns true if style is derived from base
bool isDerivedFrom(ArtifactStyle style, ArtifactStyle base);
    /// Get the name for the style
UnownedStringSlice getName(ArtifactStyle style);

struct ArtifactDescUtil
{
    typedef ArtifactPayload Payload;
    typedef ArtifactKind Kind;
    typedef ArtifactStyle Style;
    typedef ArtifactDesc Desc;
    
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
    
    static String getBaseName(const ArtifactDesc& desc, IFileArtifactRepresentation* fileRep);

        /// Given a desc, and a basePath returns a suitable path for a entity of specified desc
    static SlangResult calcPathForDesc(const ArtifactDesc& desc, const UnownedStringSlice& basePath, StringBuilder& outPath);

    static SlangResult calcNameForDesc(const ArtifactDesc& desc, const UnownedStringSlice& basePath, StringBuilder& outPath);

        /// Given a target returns the ArtifactDesc
    static ArtifactDesc makeDescFromCompileTarget(SlangCompileTarget target);

        /// Make ArtifactDesc from target
    static bool isDescDerivedFrom(const ArtifactDesc& desc, const ArtifactDesc& from);
};

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
inline /* static */bool ArtifactDescUtil::isDescDerivedFrom(const ArtifactDesc& desc, const ArtifactDesc& from)
{
    // TODO(JS): Currently this ignores flags in desc. That may or may not be right 
    // long term.
    return isDerivedFrom(desc.kind, from.kind) &&
        isDerivedFrom(desc.payload, from.payload) &&
        isDerivedFrom(desc.style, from.style);
}

} // namespace Slang

#endif
