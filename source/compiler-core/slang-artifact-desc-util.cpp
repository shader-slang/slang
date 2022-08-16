// slang-artifact-desc-util.cpp
#include "slang-artifact-desc-util.h"

#include "slang-artifact-representation.h"

#include "slang-artifact-impl.h"

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
        } while (Index(type) >= Index(T::Base));

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
        x(CompileResults, Base) \
        x(MetaData, Base) \
            x(DebugInfo, MetaData) \
            x(Diagnostics, MetaData)

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

/* static*/ SlangResult ArtifactDescUtil::appendCpuExtensionForKind(Kind kind, StringBuilder& out)
{
    for (const auto& kindExt : g_cpuKindExts)
    {
        if (kind == kindExt.kind)
        {
            out << kindExt.ext;
            return SLANG_OK;
        }
    }
    return SLANG_E_NOT_FOUND;
}

static UnownedStringSlice _getPayloadExtension(ArtifactPayload payload)
{
    typedef ArtifactPayload Payload;
    switch (payload)
    {
        /* Misc */
        case Payload::Unknown:      return toSlice("unknown");

        /* Source types */
        case Payload::HLSL:         return toSlice("hlsl");
        case Payload::GLSL:         return toSlice("glsl");

        case Payload::Cpp:          return toSlice("cpp");
        case Payload::C:            return toSlice("c");

        case Payload::Metal:        return toSlice("metal");

        case Payload::CUDA:         return toSlice("cu");

        case Payload::Slang:        return toSlice("slang");

        /* Binary types */
        case Payload::DXIL:         return toSlice("dxil");
        case Payload::DXBC:         return toSlice("dxbc");
        case Payload::SPIRV:        return toSlice("spv");

        case Payload::PTX:          return toSlice("ptx");

        case Payload::LLVMIR:       return toSlice("llvm-ir");

        case Payload::SlangIR:      return toSlice("slang-ir");
        
        case Payload::MetalAIR:     return toSlice("air");

        default: break;
    }
    return UnownedStringSlice();
}

SlangResult ArtifactDescUtil::appendDefaultExtension(const ArtifactDesc& desc, StringBuilder& out)
{
    switch (desc.kind)
    {
        case ArtifactKind::Library:
        {
            // Special cases
            if (desc.payload == Payload::SlangIR)
            {
                out << toSlice("slang-module");
                return SLANG_OK;
            }
            else if (desc.payload == Payload::MetalAIR)
            {
                // https://developer.apple.com/documentation/metal/shader_libraries/building_a_library_with_metal_s_command-line_tools
                out << toSlice("metallib");
                return SLANG_OK;
            }
            
            break;
        }
        case ArtifactKind::Zip:         
        {
            out << toSlice("zip");
            return SLANG_OK;
        }
        case ArtifactKind::Riff:         
        {
            out << toSlice("riff");
            return SLANG_OK;
        }
        case ArtifactKind::Assembly:
        {
            // Special case PTX, because it is assembly
            if (desc.payload == Payload::PTX)
            {
                out << _getPayloadExtension(desc.payload);
                return SLANG_OK;
            }

            // We'll just use asm for all CPU assembly type
            if (isDerivedFrom(desc.payload, ArtifactPayload::CPULike))
            {
                out << toSlice("asm");
                return SLANG_OK;
            }

            // Use the payload extension "-asm"
            out << _getPayloadExtension(desc.payload);
            out << toSlice("-asm");
            return SLANG_OK;
        }
        case ArtifactKind::Source:
        {
            out << _getPayloadExtension(desc.payload);
            return SLANG_OK;
        }
        default: break;
    }

    if (ArtifactDescUtil::isCpuLikeTarget(desc) && !isDerivedFrom(desc.payload, ArtifactPayload::Source))
    {
        return appendCpuExtensionForKind(desc.kind, out);
    }
    else
    {
        auto slice = _getPayloadExtension(desc.payload);
        if (slice.getLength())
        {
            out << slice;
            return SLANG_OK;
        }
    }

    return SLANG_E_NOT_FOUND;
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
        StringBuilder descExt;

        appendDefaultExtension(desc, descExt);

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

/* static */SlangResult ArtifactDescUtil::calcNameForDesc(const ArtifactDesc& desc, const UnownedStringSlice& inBaseName, StringBuilder& outName)
{
    UnownedStringSlice baseName(inBaseName);

    // If there is no basename, set one
    if (baseName.getLength() == 0)
    {
        baseName = toSlice("unknown");
    }

    // Prefix
    if (isCpuBinary(desc) &&
        (desc.kind == ArtifactKind::SharedLibrary ||
            desc.kind == ArtifactKind::Library))
    {
        const bool isSharedLibraryPrefixPlatform = SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY;
        if (isSharedLibraryPrefixPlatform)
        {
            outName << "lib";
        }
    }

    // Output the basename
    outName << baseName;

    // If there is an extension append it
    StringBuilder ext;
    if (SLANG_SUCCEEDED(appendDefaultExtension(desc, ext)) && ext.getLength() > 0)
    {
        outName.appendChar('.');
        outName.append(ext);
    }

    return SLANG_OK;
}

/* static */SlangResult ArtifactDescUtil::calcPathForDesc(const ArtifactDesc& desc, const UnownedStringSlice& basePath, StringBuilder& outPath)
{
    outPath.Clear();

    // Append the directory
    Index pos = Path::findLastSeparatorIndex(basePath);
    if (pos >= 0)
    {
        // Keep the stem including the delimiter
        outPath.append(basePath.head(pos + 1));

        StringBuilder buf;
        const auto baseName = basePath.tail(pos + 1);

        SLANG_RETURN_ON_FAIL(calcNameForDesc(desc, baseName, buf));
        outPath.append(buf);

        return SLANG_OK;
    }
    else
    {
        return calcNameForDesc(desc, basePath, outPath);
    }
}

} // namespace Slang
