// slang-artifact-info.cpp
#include "slang-artifact-info.h"

#include "../core/slang-type-text-util.h"
#include "../core/slang-io.h"

namespace Slang {

static ArtifactPayloadInfo::Lookup _makePayloadInfoLookup()
{
    ArtifactPayloadInfo::Lookup values;
    memset(&values, 0, sizeof(values));
    
    
    typedef ArtifactPayload Payload;
    typedef ArtifactPayloadInfo::Flag Flag;
    typedef ArtifactPayloadInfo::Flags Flags;
    typedef ArtifactPayloadInfo::Flavor Flavor;

    struct Info
    {
        Payload payload;
        Flavor flavor;
        Flags flags;
    };

    const Info infos[] =
    {
        {Payload::None,             Flavor::None,         0},
        {Payload::Unknown,          Flavor::Unknown,      0},

        // It seems as if DXBC is potentially linkable from
        // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-appendix-keywords#export

        // We can't *actually* link PTX or SPIR-V currently but it is in principal possible
        // so let's say we accept for now

        {Payload::DXIL,             Flavor::Binary,       Flag::IsGpuNative | Flag::IsLinkable},
        {Payload::DXBC,             Flavor::Binary,       Flag::IsGpuNative | Flag::IsLinkable}, 
        {Payload::SPIRV,            Flavor::Binary,       Flag::IsGpuNative | Flag::IsLinkable }, 
        {Payload::PTX,              Flavor::Binary,       Flag::IsGpuNative | Flag::IsLinkable },

        {Payload::DXILAssembly,     Flavor::Assembly,     0},
        {Payload::DXBCAssembly,     Flavor::Assembly,     0},
        {Payload::SPIRVAssembly,    Flavor::Assembly,     0},
        {Payload::PTXAssembly,      Flavor::Assembly,     0},

        {Payload::HostCPU,          Flavor::Binary,       Flag::IsCpuNative | Flag::IsLinkable},

        // Do we want some other Flavor for these?
        {Payload::SlangIR,          Flavor::Binary,       Flag::IsLinkable},
        {Payload::LLVMIR,           Flavor::Binary,       0},
        {Payload::SlangAST,         Flavor::Binary,       0},

        {Payload::X86,              Flavor::Binary,       Flag::IsCpuNative | Flag::IsLinkable},
        {Payload::X86_64,           Flavor::Binary,       Flag::IsCpuNative | Flag::IsLinkable},
        {Payload::AARCH,            Flavor::Binary,       Flag::IsCpuNative | Flag::IsLinkable},
        {Payload::AARCH64,          Flavor::Binary,       Flag::IsCpuNative | Flag::IsLinkable},

        {Payload::HLSL,             Flavor::Source,       0},
        {Payload::GLSL,             Flavor::Source,       0},
        {Payload::CPP,              Flavor::Source,       0},
        {Payload::C,                Flavor::Source,       0},
        {Payload::CUDA,             Flavor::Source,       0},
        {Payload::Slang,            Flavor::Source,       0},

        {Payload::DebugInfo,        Flavor::Unknown,      0},

        {Payload::Diagnostics,      Flavor::Unknown,      0},

        {Payload::Zip,              Flavor::Container,    0},
    };

    for (auto info : infos)
    {
        auto& v = values.values[Index(info.payload)];
        v.flavor = info.flavor;
        v.flags = info.flags;
    }

    return values;
}

/* static */const ArtifactPayloadInfo::Lookup ArtifactPayloadInfo::Lookup::g_values = _makePayloadInfoLookup();

/* !!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactInfoUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!! */


namespace { // anonymous
struct KindExtension
{
    ArtifactKind kind;
    UnownedStringSlice ext;
};
} // anonymous

#define SLANG_KIND_EXTENSION(kind, ext) \
    { ArtifactKind::kind, UnownedStringSlice::fromLiteral(ext) },

static const KindExtension g_cpuKindExts[] =
{
#if SLANG_WINDOWS_FAMILY
    SLANG_KIND_EXTENSION(Library, "lib")
    SLANG_KIND_EXTENSION(ObjectCode, "obj")
    SLANG_KIND_EXTENSION(Executable, "exe")
    SLANG_KIND_EXTENSION(SharedLibrary, "dll")
#else 
    SLANG_KIND_EXTENSION(Library, "a")
    SLANG_KIND_EXTENSION(ObjectCode, "o")
    SLANG_KIND_EXTENSION(Executable, "")

#if __CYGWIN__
    SLANG_KIND_EXTENSION(SharedLibrary, "dll")
#elif SLANG_APPLE_FAMILY
    SLANG_KIND_EXTENSION(SharedLibrary, "dylib")
#else
    SLANG_KIND_EXTENSION(SharedLibrary, "so")
#endif

#endif
};

/* static */bool ArtifactInfoUtil::isKindBinaryLinkable(Kind kind)
{
    switch (kind)
    {
        case Kind::Library:
        case Kind::ObjectCode:
        {
            return true;
        }
        default: break;
    }
    return false;
}

/* static */bool ArtifactInfoUtil::isBinaryLinkable(const ArtifactDesc& desc)
{
    return isKindBinaryLinkable(desc.kind) &&
        getInfo(desc.payload).isSet(ArtifactPayloadInfo::Flag::IsLinkable);
}

/* static */bool ArtifactInfoUtil::isPayloadCpuBinary(Payload payload)
{
    auto info = getInfo(payload);
    return info.isSet(ArtifactPayloadInfo::Flag::IsCpuNative) && info.flavor == ArtifactPayloadInfo::Flavor::Binary;
}

/* static */bool ArtifactInfoUtil::isPayloadGpuBinary(Payload payload)
{
    auto info = getInfo(payload);
    return info.isSet(ArtifactPayloadInfo::Flag::IsGpuNative) && info.flavor == ArtifactPayloadInfo::Flavor::Binary;
}

/* static */bool ArtifactInfoUtil::isPayloadCpuTarget(Payload payload)
{
    return isPayloadCpuBinary(payload) ||
        (payload == Payload::C || payload == Payload::CPP);
}

/* static */UnownedStringSlice ArtifactInfoUtil::getDefaultExtensionForPayload(Payload payload)
{
    switch (payload)
    {
    case Payload::None:         return UnownedStringSlice();
    case Payload::Unknown:      return UnownedStringSlice::fromLiteral("unknown");

    case Payload::DXIL:         return UnownedStringSlice::fromLiteral("dxil");
    case Payload::DXBC:         return UnownedStringSlice::fromLiteral("dxbc");
    case Payload::SPIRV:        return UnownedStringSlice::fromLiteral("spirv");

    case Payload::PTX:          return UnownedStringSlice::fromLiteral("ptx");

    case Payload::X86:
    case Payload::X86_64:
    case Payload::AARCH:
    case Payload::AARCH64:
    case Payload::HostCPU:
    {
        return UnownedStringSlice();
    }

    case Payload::SlangIR:      return UnownedStringSlice::fromLiteral("slang-ir");
    case Payload::LLVMIR:       return UnownedStringSlice::fromLiteral("llvm-ir");

    case Payload::HLSL:         return UnownedStringSlice::fromLiteral("hlsl");
    case Payload::GLSL:         return UnownedStringSlice::fromLiteral("glsl");

    case Payload::CPP:          return UnownedStringSlice::fromLiteral("cpp");
    case Payload::C:            return UnownedStringSlice::fromLiteral("c");

    case Payload::CUDA:         return UnownedStringSlice::fromLiteral("cu");

    case Payload::Slang:        return UnownedStringSlice::fromLiteral("slang");

    case Payload::Zip:          return UnownedStringSlice::fromLiteral("zip");

    default: break;
    }

    SLANG_UNEXPECTED("Unknown content type");
}

/* static */ArtifactDesc ArtifactInfoUtil::getDescFromExtension(const UnownedStringSlice& slice)
{
    if (slice == "slang-module" ||
        slice == "slang-lib")
    {
        return ArtifactDesc::make(ArtifactKind::Library, ArtifactPayload::SlangIR);
    }

    for (const auto& kindExt : g_cpuKindExts)
    {
        if (slice == kindExt.ext)
        {
            // We'll assume it's for the host CPU for now..
            return ArtifactDesc::make(kindExt.kind, Payload::HostCPU);
        }
    }

    const auto target = TypeTextUtil::findCompileTargetFromExtension(slice);

    return ArtifactDesc::makeFromCompileTarget(target);
}

/* static */ArtifactDesc ArtifactInfoUtil::getDescFromPath(const UnownedStringSlice& slice)
{
    auto extension = Path::getPathExt(slice);
    return getDescFromExtension(extension);
}

/* static*/ UnownedStringSlice ArtifactInfoUtil::getCpuExtensionForKind(Kind kind)
{
    for (const auto& kindExt : g_cpuKindExts)
    {
        if (kind == kindExt.kind)
        {
            return kindExt.ext;
        }
    }
    return UnownedStringSlice();
}

UnownedStringSlice ArtifactInfoUtil::getDefaultExtension(const ArtifactDesc& desc)
{
    if (ArtifactInfoUtil::isPayloadCpuBinary(desc.payload))
    {
        return getCpuExtensionForKind(desc.kind);
    }
    else
    {
        return getDefaultExtensionForPayload(desc.payload);
    }
}

/* static */String ArtifactInfoUtil::getBaseNameFromPath(const ArtifactDesc& desc, const UnownedStringSlice& path)
{
    String name = Path::getFileName(path);

    const bool isSharedLibraryPrefixPlatform = SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY;
    if (isSharedLibraryPrefixPlatform)
    {
        // Strip lib prefix
        if (ArtifactInfoUtil::isCpuBinary(desc) &&
            (desc.kind == ArtifactKind::Library ||
                desc.kind == ArtifactKind::SharedLibrary))
        {
            // If it starts with lib strip it
            if (name.startsWith("lib"))
            {
                const String stripLib = name.getUnownedSlice().tail(3);
                name = stripLib;
            }
        }
    }

    // Strip any extension 
    {
        auto descExt = ArtifactInfoUtil::getDefaultExtension(desc);
        // Strip the extension if it's a match
        if (descExt.getLength() &&
            Path::getPathExt(name) == descExt)
        {
            name = Path::getFileNameWithoutExt(name);
        }
    }

    return name;
}

/* static */String ArtifactInfoUtil::getBaseName(IArtifact* artifact)
{
    const auto pathType = artifact->getPathType();

    // If we have a path, get the base name from that
    if (pathType != ArtifactPathType::None)
    {
        UnownedStringSlice path(artifact->getPath());
        const auto desc = artifact->getDesc();

        return getBaseNameFromPath(desc, path);
    }

    // Else use the name
    return artifact->getName();
}

/* static */String ArtifactInfoUtil::getParentPath(IArtifact* artifact)
{
    const auto pathType = artifact->getPathType();
    const auto path = artifact->getPath();
    
    if (pathType != ArtifactPathType::None && *path != 0)
    {
        return Path::getParentDirectory(path);
    }
    return String();
}

} // namespace Slang
