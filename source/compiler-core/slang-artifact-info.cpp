// slang-artifact-info.cpp
#include "slang-artifact-info.h"

#include "../core/slang-type-text-util.h"
#include "../core/slang-io.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactInfoUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

namespace { // anonymous
struct KindExtension
{
    ArtifactKind kind;
    UnownedStringSlice ext;
};
} // anonymous

#define SLANG_KIND_EXTENSION(kind, ext) \
    { ArtifactKind::kind, toSlice(ext) },

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

/* static */ bool ArtifactInfoUtil::isCpuBinary(const ArtifactDesc& desc) 
{ 
    return isDerivedFrom(desc.kind, ArtifactKind::BinaryLike) && isDerivedFrom(desc.payload, ArtifactPayload::CPULike); 
}

/* static */bool ArtifactInfoUtil::isText(const ArtifactDesc& desc)
{
    // If it's derived from text...
    if (isDerivedFrom(desc.kind, ArtifactKind::Text))
    {
        return true;
    }

    // Special case PTX...
    if (isDerivedFrom(desc.kind, ArtifactKind::BinaryLike))
    {
        return desc.payload == ArtifactPayload::PTX;
    }

    // Not text
    return false;
}

/* static */bool ArtifactInfoUtil::isGpuUsable(const ArtifactDesc& desc) 
{ 
    if (isDerivedFrom(desc.kind, ArtifactKind::BinaryLike))
    {
        return isDerivedFrom(desc.payload, ArtifactPayload::KernelLike);
    }

    // PTX is a kind of special case, it's an 'assembly' (low level text represention) that can be passed 
    // to CUDA runtime
    return desc.kind == ArtifactKind::Assembly && desc.payload == ArtifactPayload::PTX;
}

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
    if (isDerivedFrom(desc.kind, ArtifactKind::BinaryLike))
    {
        if (isDerivedFrom(desc.payload, ArtifactPayload::KernelLike))
        {
            // It seems as if DXBC is potentially linkable from
            // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-appendix-keywords#export

            // We can't *actually* link PTX or SPIR-V currently but it is in principal possible
            // so let's say we accept for now

            return true;
        }
        else if (isDerivedFrom(desc.payload, ArtifactPayload::CPULike))
        {
            // If kind is exe or shared library, linking will arguably not work
            if (desc.kind == ArtifactKind::SharedLibrary ||
                desc.kind == ArtifactKind::Executable)
            {
                return false;
            }

            return true;
        }
        else if (isDerivedFrom(desc.payload, ArtifactPayload::GeneralIR))
        {
            // We'll *assume* IR is linkable
            return true;
        }
    }
    return false;
}

/* static */bool ArtifactInfoUtil::isCpuTarget(const ArtifactDesc& desc)
{
    if (isDerivedFrom(desc.kind, ArtifactKind::BinaryLike))
    {
        return isDerivedFrom(desc.payload, ArtifactPayload::CPULike);
    }
    else if (isDerivedFrom(desc.kind, ArtifactKind::Source))
    {
        // We'll assume C/C++ are targetting CPU, although that is perhaps somewhat arguable.
        return desc.payload == Payload::C || desc.payload == Payload::Cpp;
    }

    return false;
}


/* static */ArtifactDesc ArtifactInfoUtil::getDescFromExtension(const UnownedStringSlice& slice)
{
    if (slice == "slang-module" ||
        slice == "slang-lib")
    {
        return ArtifactDesc::make(ArtifactKind::Library, ArtifactPayload::SlangIR);
    }

    // Metal
    // https://developer.apple.com/documentation/metal/shader_libraries/building_a_library_with_metal_s_command-line_tools
    if (slice == toSlice("air"))
    {
        return ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::MetalAIR);
    }
    else if (slice == toSlice("metallib") || slice == toSlice("metalar"))
    {
        return ArtifactDesc::make(ArtifactKind::Library, ArtifactPayload::MetalAIR);
    }

    if (slice == toSlice("zip"))
    {
        return ArtifactDesc::make(ArtifactKind::Zip, ArtifactPayload::Unknown);
    }
    else if (slice == toSlice("riff"))
    {
        return ArtifactDesc::make(ArtifactKind::Riff, ArtifactPayload::Unknown);
    }

    if (slice == toSlice("asm"))
    {
        // We'll assume asm means current CPU assembler..
        return ArtifactDesc::make(ArtifactKind::Assembly, ArtifactPayload::HostCPU);
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

UnownedStringSlice ArtifactInfoUtil::getAssemblyExtensionForPayload(ArtifactPayload payload)
{
    switch (payload)
    {
        case ArtifactPayload::DXIL:     return toSlice("dxil-asm");
        case ArtifactPayload::DXBC:     return toSlice("dxbc-asm");
        case ArtifactPayload::SPIRV:    return toSlice("spv-asm");
        case ArtifactPayload::PTX:     return toSlice("ptx");

        // TODO(JS):
        // Not sure what to do for metal - does it have an assembly name?

        default: break;
    }

    // We'll just use asm for all CPU assembly type
    if (isDerivedFrom(payload, ArtifactPayload::CPULike))
    {
        return toSlice("asm");
    }

    if (isDerivedFrom(payload, ArtifactPayload::GeneralIR))
    {
        switch (payload)
        {
            case ArtifactPayload::SlangIR:     return toSlice("slang-ir-asm");
            case ArtifactPayload::LLVMIR:     return toSlice("llvm-ir-asm");
            break;
        }
    }

    return UnownedStringSlice();
}

UnownedStringSlice ArtifactInfoUtil::getDefaultExtension(const ArtifactDesc& desc)
{
    switch (desc.kind)
    {
        case ArtifactKind::Zip:          return toSlice("zip");
        case ArtifactKind::Riff:         return toSlice("riff");
        case ArtifactKind::Assembly:     
        {
            return getAssemblyExtensionForPayload(desc.payload);
        }
        case ArtifactKind::Source:
        {
            switch (desc.payload)
            {
                case Payload::HLSL:         return toSlice("hlsl");
                case Payload::GLSL:         return toSlice("glsl");

                case Payload::Cpp:          return toSlice("cpp");
                case Payload::C:            return toSlice("c");

                case Payload::Metal:        return toSlice("metal");

                case Payload::CUDA:         return toSlice("cu");

                case Payload::Slang:        return toSlice("slang");
                default: break;
            }
        }
        default: break;
    }

    if (ArtifactInfoUtil::isCpuTarget(desc))
    {
        return getCpuExtensionForKind(desc.kind);
    }

    if (isDerivedFrom(desc.kind, ArtifactKind::BinaryLike))
    {
        switch (desc.payload)
        {
            case Payload::None:         return UnownedStringSlice();
            case Payload::Unknown:      return toSlice("unknown");

            case Payload::DXIL:         return toSlice("dxil");
            case Payload::DXBC:         return toSlice("dxbc");
            case Payload::SPIRV:        return toSlice("spv");

            case Payload::PTX:          return toSlice("ptx");

            case Payload::LLVMIR:       return toSlice("llvm-ir");

            case Payload::SlangIR:      
            {
                return (desc.kind == ArtifactKind::Library) ? toSlice("slang-module") : toSlice("slang-ir");
            }
            case Payload::MetalAIR:      
            {
                // https://developer.apple.com/documentation/metal/shader_libraries/building_a_library_with_metal_s_command-line_tools
                return (desc.kind == ArtifactKind::Library) ? toSlice("metallib") : toSlice("air");
            }
            default: break;
        }
    }

    return UnownedStringSlice();
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
