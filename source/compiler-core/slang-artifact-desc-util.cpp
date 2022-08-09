// slang-artifact-desc-util.cpp
#include "slang-artifact-desc-util.h"

#include "slang-artifact-representation.h"

#include "../core/slang-type-text-util.h"
#include "../core/slang-io.h"

namespace Slang {

namespace { // anonymous

struct HierarchicalEnumEntry
{
    Index value;
    Index parent;
    const char* name;
};

static bool _isHierarchicalEnumOk(ConstArrayView<HierarchicalEnumEntry> entries, Count countOf)
{
    // All values should be set
    if (entries.getCount() != countOf)
    {
        return false;
    }

    List<uint8_t> isUsed;
    isUsed.setCount(countOf);
    ::memset(isUsed.getBuffer(), 0, countOf);

    for (const auto& entry : entries)
    {
        const auto value = entry.value;
        // Must be in range
        if (value < 0 || value >= countOf)
        {
            return false;
        }

        if (isUsed[value] != 0)
        {
            return false;
        }
        // Mark as used
        isUsed[value]++;
    }

    // There can't be any gaps
    for (auto v : isUsed)
    {
        if (v == 0)
        {
            return false;
        }
    }

    // Okay, looks reasonable..
    return true;
}

template <typename T>
struct HierarchicalEnumTable
{
    HierarchicalEnumTable(ConstArrayView<HierarchicalEnumEntry> entries)
    {
        // Remove warnings around this not being used.
        {
            const auto unused = _isHierarchicalEnumOk;
            SLANG_UNUSED(unused);
        }

        SLANG_COMPILE_TIME_ASSERT(Index(T::Invalid) < Index(T::Base));
        SLANG_ASSERT(entries.getCount() == Count(T::CountOf));

        SLANG_ASSERT(_isHierarchicalEnumOk(entries, Count(T::CountOf)));

        ::memset(&m_parents, 0, sizeof(m_parents));

        for (const auto& entry : entries)
        {
            const auto value = entry.value;
            m_parents[value] = T(entry.parent);
            m_names[value] = UnownedStringSlice(entry.name);
        }

        // TODO(JS): NOTE! If we wanted to use parent to indicate if a value was *invalid* 
        // we would want the Parent of Base to be Base.
        //
        // Base parent should be invalid
        SLANG_ASSERT(getParent(T::Base) == T::Invalid);
        // Invalids parent should be invalid
        SLANG_ASSERT(getParent(T::Invalid) == T::Invalid);
    }

    T getParent(T kind) const
    {
        return (kind >= T::CountOf) ?
            T::Invalid :
            m_parents[Index(kind)];
    }
    UnownedStringSlice getName(T kind) const
    {
        return (kind >= T::CountOf) ?
            UnownedStringSlice() :
            m_names[Index(kind)];
    }

    bool isDerivedFrom(T type, T base) const
    {
        if (Index(type) >= Index(T::CountOf))
        {
            return false;
        }

        do
        {
            if (type == base)
            {
                return true;
            }
            type = m_parents[Index(type)];
        } while (Index(type) > Index(T::Base));

        return false;
    }

protected:
    T m_parents[Count(T::CountOf)];
    UnownedStringSlice m_names[Count(T::CountOf)];
};

} // anonymous

// Macro utils to create "enum hierarchy" tables

#define SLANG_HIERARCHICAL_ENUM_GET_VALUES(ENUM_TYPE, ENUM_TYPE_MACRO, ENUM_ENTRY_MACRO) \
static ConstArrayView<HierarchicalEnumEntry> _getEntries##ENUM_TYPE() \
{ \
    static const HierarchicalEnumEntry values[] = { ENUM_TYPE_MACRO(ENUM_ENTRY_MACRO) }; \
    return makeConstArrayView(values); \
}

#define SLANG_HIERARCHICAL_ENUM(ENUM_TYPE, ENUM_TYPE_MACRO, ENUM_VALUE_MACRO) \
SLANG_HIERARCHICAL_ENUM_GET_VALUES(ENUM_TYPE, ENUM_TYPE_MACRO, ENUM_VALUE_MACRO) \
\
static const HierarchicalEnumTable<ENUM_TYPE> g_table##ENUM_TYPE(_getEntries##ENUM_TYPE()); \
\
ENUM_TYPE getParent(ENUM_TYPE kind) { return g_table##ENUM_TYPE.getParent(kind); } \
UnownedStringSlice getName(ENUM_TYPE kind) { return g_table##ENUM_TYPE.getName(kind); } \
bool isDerivedFrom(ENUM_TYPE kind, ENUM_TYPE base) { return g_table##ENUM_TYPE.isDerivedFrom(kind, base); }

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactKind !!!!!!!!!!!!!!!!!!!!!!! */

#define SLANG_ARTIFACT_KIND(x) \
    x(Invalid, Invalid) \
    x(Base, Invalid) \
        x(None, Base) \
        x(Unknown, Base) \
        x(Container, Base) \
            x(Zip, Container) \
            x(Riff, Container) \
        x(Text, Base) \
            x(HumanText, Text) \
            x(Source, Text) \
            x(Assembly, Text) \
        x(BinaryLike, Base) \
            x(ObjectCode, BinaryLike) \
            x(Library, BinaryLike) \
            x(Executable, BinaryLike) \
            x(SharedLibrary, BinaryLike) \
            x(HostCallable, BinaryLike) \
        x(Instance, Base)

#define SLANG_ARTIFACT_KIND_ENTRY(TYPE, PARENT) { Index(ArtifactKind::TYPE), Index(ArtifactKind::PARENT), #TYPE },

SLANG_HIERARCHICAL_ENUM(ArtifactKind, SLANG_ARTIFACT_KIND, SLANG_ARTIFACT_KIND_ENTRY)

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactPayload !!!!!!!!!!!!!!!!!!!!!!! */

#define SLANG_ARTIFACT_PAYLOAD(x) \
    x(Invalid, Invalid) \
    x(Base, Invalid) \
        x(None, Base) \
        x(Unknown, Base) \
        x(Source, Base) \
            x(C, Source) \
            x(Cpp, Source) \
            x(HLSL, Source) \
            x(GLSL, Source) \
            x(CUDA, Source) \
            x(Metal, Source) \
            x(Slang, Source) \
        x(KernelLike, Base) \
            x(DXIL, KernelLike) \
            x(DXBC, KernelLike) \
            x(SPIRV, KernelLike) \
            x(PTX, KernelLike) \
            x(CuBin, KernelLike) \
            x(MetalAIR, KernelLike) \
        x(CPULike, Base) \
            x(UnknownCPU, CPULike) \
            x(X86, CPULike) \
            x(X86_64, CPULike) \
            x(Aarch, CPULike) \
            x(Aarch64, CPULike) \
            x(HostCPU, CPULike) \
            x(UniversalCPU, CPULike) \
        x(GeneralIR, Base) \
            x(SlangIR, GeneralIR) \
            x(LLVMIR, GeneralIR) \
        x(AST, Base) \
            x(SlangAST, AST) \
        x(DebugInfo, Base) \
        x(Diagnostics, Base) \
        x(CompileResults, Base)

#define SLANG_ARTIFACT_PAYLOAD_ENTRY(TYPE, PARENT) { Index(ArtifactPayload::TYPE), Index(ArtifactPayload::PARENT), #TYPE },

SLANG_HIERARCHICAL_ENUM(ArtifactPayload, SLANG_ARTIFACT_PAYLOAD, SLANG_ARTIFACT_PAYLOAD_ENTRY)

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactStyle !!!!!!!!!!!!!!!!!!!!!!! */

#define SLANG_ARTIFACT_STYLE(x) \
    x(Invalid, Invalid) \
    x(Base, Invalid) \
        x(Unknown, Base) \
        x(Kernel, Base) \
        x(Host, Base) 

#define SLANG_ARTIFACT_STYLE_ENTRY(TYPE, PARENT) { Index(ArtifactStyle::TYPE), Index(ArtifactStyle::PARENT), #TYPE },

SLANG_HIERARCHICAL_ENUM(ArtifactStyle, SLANG_ARTIFACT_STYLE, SLANG_ARTIFACT_STYLE_ENTRY)


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactDescUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */ArtifactDesc ArtifactDescUtil::makeDescFromCompileTarget(SlangCompileTarget target)
{
    switch (target)
    {
        case SLANG_TARGET_UNKNOWN:                return Desc::make(Kind::Unknown, Payload::None, Style::Unknown, 0);
        case SLANG_TARGET_NONE:                   return Desc::make(Kind::None, Payload::None, Style::Unknown, 0);
        case SLANG_GLSL_VULKAN:
        case SLANG_GLSL_VULKAN_ONE_DESC:
        case SLANG_GLSL:
        {
            // For the moment we Desc::make all just map to GLSL, but we could use flags
            // or some other mechanism to distinguish the types
            return Desc::make(Kind::Source, Payload::GLSL, Style::Kernel, 0);
        }
        case SLANG_HLSL:                    return Desc::make(Kind::Source, Payload::HLSL, Style::Kernel, 0);
        case SLANG_SPIRV:                   return Desc::make(Kind::Executable, Payload::SPIRV, Style::Kernel, 0);
        case SLANG_SPIRV_ASM:               return Desc::make(Kind::Assembly, Payload::SPIRV, Style::Kernel, 0);
        case SLANG_DXBC:                    return Desc::make(Kind::Executable, Payload::DXBC, Style::Kernel, 0);
        case SLANG_DXBC_ASM:                return Desc::make(Kind::Assembly, Payload::DXBC, Style::Kernel, 0);
        case SLANG_DXIL:                    return Desc::make(Kind::Executable, Payload::DXIL, Style::Kernel, 0);
        case SLANG_DXIL_ASM:                return Desc::make(Kind::Assembly, Payload::DXIL, Style::Kernel, 0);
        case SLANG_C_SOURCE:                return Desc::make(Kind::Source, Payload::C, Style::Kernel, 0);
        case SLANG_CPP_SOURCE:              return Desc::make(Kind::Source, Payload::Cpp, Style::Kernel, 0);
        case SLANG_HOST_CPP_SOURCE:         return Desc::make(Kind::Source, Payload::Cpp, Style::Host, 0);
        case SLANG_HOST_EXECUTABLE:         return Desc::make(Kind::Executable, Payload::HostCPU, Style::Host, 0);
        case SLANG_SHADER_SHARED_LIBRARY:   return Desc::make(Kind::SharedLibrary, Payload::HostCPU, Style::Kernel, 0);
        case SLANG_SHADER_HOST_CALLABLE:    return Desc::make(Kind::HostCallable, Payload::HostCPU, Style::Kernel, 0);
        case SLANG_CUDA_SOURCE:             return Desc::make(Kind::Source, Payload::CUDA, Style::Kernel, 0);
            // TODO(JS):
            // Not entirely clear how best to represent PTX here. We could mark as 'Assembly'. Saying it is 
            // 'Executable' implies it is Binary (which PTX isn't). Executable also implies 'complete for executation', 
            // irrespective of it being text.
        case SLANG_PTX:                     return Desc::make(Kind::Executable, Payload::PTX, Style::Kernel, 0);
        case SLANG_OBJECT_CODE:             return Desc::make(Kind::ObjectCode, Payload::HostCPU, Style::Kernel, 0);
        case SLANG_HOST_HOST_CALLABLE:      return Desc::make(Kind::HostCallable, Payload::HostCPU, Style::Host, 0);
        default: break;
    }

    SLANG_UNEXPECTED("Unhandled type");
}

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

/* static */ bool ArtifactDescUtil::isCpuBinary(const ArtifactDesc& desc)
{
    return isDerivedFrom(desc.kind, ArtifactKind::BinaryLike) && isDerivedFrom(desc.payload, ArtifactPayload::CPULike);
}

/* static */bool ArtifactDescUtil::isText(const ArtifactDesc& desc)
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

/* static */bool ArtifactDescUtil::isGpuUsable(const ArtifactDesc& desc)
{
    if (isDerivedFrom(desc.kind, ArtifactKind::BinaryLike))
    {
        return isDerivedFrom(desc.payload, ArtifactPayload::KernelLike);
    }

    // PTX is a kind of special case, it's an 'assembly' (low level text represention) that can be passed 
    // to CUDA runtime
    return desc.kind == ArtifactKind::Assembly && desc.payload == ArtifactPayload::PTX;
}

/* static */bool ArtifactDescUtil::isKindBinaryLinkable(Kind kind)
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

/* static */bool ArtifactDescUtil::isLinkable(const ArtifactDesc& desc)
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

/* static */bool ArtifactDescUtil::isCpuLikeTarget(const ArtifactDesc& desc)
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

/* static */ArtifactDesc ArtifactDescUtil::getDescFromExtension(const UnownedStringSlice& slice)
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

    return makeDescFromCompileTarget(target);
}

/* static */ArtifactDesc ArtifactDescUtil::getDescFromPath(const UnownedStringSlice& slice)
{
    auto extension = Path::getPathExt(slice);
    return getDescFromExtension(extension);
}

/* static*/ UnownedStringSlice ArtifactDescUtil::getCpuExtensionForKind(Kind kind)
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

UnownedStringSlice ArtifactDescUtil::getAssemblyExtensionForPayload(ArtifactPayload payload)
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

UnownedStringSlice ArtifactDescUtil::getDefaultExtension(const ArtifactDesc& desc)
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

    if (ArtifactDescUtil::isCpuLikeTarget(desc))
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

/* static */String ArtifactDescUtil::getBaseNameFromPath(const ArtifactDesc& desc, const UnownedStringSlice& path)
{
    String name = Path::getFileName(path);

    const bool isSharedLibraryPrefixPlatform = SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY;
    if (isSharedLibraryPrefixPlatform)
    {
        // Strip lib prefix
        if (isCpuBinary(desc) &&
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
        auto descExt = getDefaultExtension(desc);
        // Strip the extension if it's a match
        if (descExt.getLength() &&
            Path::getPathExt(name) == descExt)
        {
            name = Path::getFileNameWithoutExt(name);
        }
    }

    return name;
}

/* static */String ArtifactDescUtil::getBaseName(const ArtifactDesc& desc, IFileArtifactRepresentation* fileRep)
{
    UnownedStringSlice path(fileRep->getPath());
    return getBaseNameFromPath(desc, path);
}

/* static */String ArtifactDescUtil::getBaseName(IArtifact* artifact)
{
    if (auto fileRep = findRepresentation<IFileArtifactRepresentation>(artifact))
    {
        return getBaseName(artifact->getDesc(), fileRep);
    }
    // Else use the name
    return artifact->getName();
}

/* static */String ArtifactDescUtil::getParentPath(IFileArtifactRepresentation* fileRep)
{
    UnownedStringSlice path(fileRep->getPath());
    return Path::getParentDirectory(path);
}

/* static */String ArtifactDescUtil::getParentPath(IArtifact* artifact)
{
    if (auto fileRep = findRepresentation<IFileArtifactRepresentation>(artifact))
    {
        return getParentPath(fileRep);
    }
    return String();
}

/* static */SlangResult ArtifactDescUtil::calcPathForDesc(const ArtifactDesc& desc, const UnownedStringSlice& basePath, StringBuilder& outPath)
{
    outPath.Clear();

    UnownedStringSlice baseName;

    // Append the directory
    Index pos = Path::findLastSeparatorIndex(basePath);
    if (pos >= 0)
    {
        // Keep the stem including the delimiter
        outPath.append(basePath.head(pos + 1));
        // Get the baseName
        baseName = basePath.tail(pos + 1);
    }

    if (baseName.getLength() == 0)
    {
        baseName = toSlice("unknown");
    }

    if (isCpuBinary(desc) &&
        (desc.kind == ArtifactKind::SharedLibrary ||
            desc.kind == ArtifactKind::Library))
    {
        const bool isSharedLibraryPrefixPlatform = SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY;
        if (isSharedLibraryPrefixPlatform)
        {
            outPath << "lib";
            outPath << baseName;
        }
    }

    // If there is an extension append it
    const UnownedStringSlice ext = getDefaultExtension(desc);

    if (ext.getLength())
    {
        outPath.appendChar('.');
        outPath.append(ext);
    }

    return SLANG_OK;
}

} // namespace Slang
