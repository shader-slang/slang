// slang-artifact-info.h
#ifndef SLANG_ARTIFACT_INFO_H
#define SLANG_ARTIFACT_INFO_H

#include "slang-artifact.h"

namespace Slang
{

/* We want to centralize information about artifact descs and related types, such that when new types are added they only need
to be added/altered in one place */
struct ArtifactPayloadInfo
{
    typedef ArtifactPayloadInfo This;
    enum class Flavor : uint8_t
    {
        Unknown,
        None,
        Assembly,
        Source,
        Container,
        Binary, 
        CountOf,
    };

    typedef uint8_t Flags;
    struct Flag
    {
        enum Enum : Flags
        {
            IsCpuNative = 0x01,     ///< True if is a CPU native type
            IsGpuNative = 0x02,     ///< True if is a GPU native type
            IsLinkable  = 0x04,     ///< True if in principal is linkable
        };
    };

    bool isSet(Flag::Enum flag) const { return (flags & Flags(flag)) != 0; }
    bool isReset(Flag::Enum flag) const { return (flags & Flags(flag)) == 0; }

    struct Lookup;

    Flags flags;
    Flavor flavor;
};

struct ArtifactPayloadInfo::Lookup
{
    void setFlag(ArtifactPayload payload, Flag::Enum flag) { values[Index(payload)].flags |= Flags(flag); }
    void setFlags(ArtifactPayload payload, Flags flags) { values[Index(payload)].flags |= flags; }

    This values[Index(ArtifactPayload::CountOf)];
    static const Lookup g_values;
};

SLANG_FORCE_INLINE ArtifactPayloadInfo getInfo(ArtifactPayload payload) { return ArtifactPayloadInfo::Lookup::g_values.values[Index(payload)]; }

struct ArtifactInfoUtil
{
    typedef ArtifactPayload Payload;
    typedef ArtifactKind Kind;

        /// Returns true if the kind is binary linkable 
    static bool isKindBinaryLinkable(Kind kind);

        /// Returns true if the payload type is CPU
    static bool isPayloadCpuBinary(Payload payload);
        /// Returns true if the payload type is applicable to the GPU
    static bool isPayloadGpuBinary(Payload payload);

        /// True if is a CPU target
    static bool isPayloadCpuTarget(Payload payload);

        /// True if is a CPU target - either
    static bool isCpuTarget(const ArtifactDesc& desc) { return isPayloadCpuTarget(desc.payload); }

        /// True if is a CPU binary
    static bool isCpuBinary(const ArtifactDesc& desc) { return isPayloadCpuBinary(desc.payload); }
        /// True if is a GPU binary
    static bool isGpuBinary(const ArtifactDesc& desc) { return isPayloadGpuBinary(desc.payload); }

        /// True if artifact  appears to be binary linkable
    static bool isBinaryLinkable(const ArtifactDesc& desc);

        /// Get the default extension type for a payload type
    static UnownedStringSlice getDefaultExtensionForPayload(Payload payload);

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
