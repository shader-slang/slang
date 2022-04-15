// slang-artifact.cpp
#include "slang-artifact.h"

#include "../core/slang-type-text-util.h"
#include "../core/slang-io.h"

namespace Slang {

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

/* static */ArtifactDesc ArtifactDesc::fromPath(const UnownedStringSlice& slice)
{
    auto extension = Path::getPathExt(slice);
    return fromExtension(extension);
}

/* static */ ArtifactDesc ArtifactDesc::fromExtension(const UnownedStringSlice& slice)
{
    if (slice == "slang-module" ||
        slice == "slang-lib")
    {
        return make(ArtifactKind::Library, ArtifactPayload::SlangIR, ArtifactStyle::Unknown);
    }

    for (const auto& kindExt : g_cpuKindExts)
    {
        if (slice == kindExt.ext)
        {
            // We'll assume it's for the host CPU for now..
            return make(kindExt.kind, Payload::HostCPU, Style::Unknown);
        }
    }

    const auto target = TypeTextUtil::findCompileTargetFromExtension(slice);

    return makeFromCompileTarget(target);
}

/* static */ArtifactDesc ArtifactDesc::makeFromCompileTarget(SlangCompileTarget target)
{
    switch (target)
    {
        case SLANG_TARGET_UNKNOWN:                return make(Kind::Unknown, Payload::None, Style::Unknown, 0);
        case SLANG_TARGET_NONE:                   return make(Kind::None, Payload::None, Style::Unknown, 0);
        case SLANG_GLSL_VULKAN:
        case SLANG_GLSL_VULKAN_ONE_DESC:
        case SLANG_GLSL:
        {
            // For the moment we make all just map to GLSL, but we could use flags
            // or some other mechanism to distinguish the types
            return make(Kind::Text, Payload::GLSL, Style::Kernel, 0);
        }
        case SLANG_HLSL:                    return make(Kind::Text, Payload::HLSL, Style::Kernel, 0);
        case SLANG_SPIRV:                   return make(Kind::Executable, Payload::SPIRV, Style::Kernel, 0);
        case SLANG_SPIRV_ASM:               return make(Kind::Text, Payload::SPIRVAssembly, Style::Kernel, 0);
        case SLANG_DXBC:                    return make(Kind::Executable, Payload::DXBC, Style::Kernel, 0);
        case SLANG_DXBC_ASM:                return make(Kind::Text, Payload::DXBCAssembly, Style::Kernel, 0);
        case SLANG_DXIL:                    return make(Kind::Executable, Payload::DXIL, Style::Kernel, 0);
        case SLANG_DXIL_ASM:                return make(Kind::Text, Payload::DXILAssembly, Style::Kernel, 0);
        case SLANG_C_SOURCE:                return make(Kind::Text, Payload::C, Style::Kernel, 0);
        case SLANG_CPP_SOURCE:              return make(Kind::Text, Payload::CPP, Style::Kernel, 0);
        case SLANG_HOST_CPP_SOURCE:         return make(Kind::Text, Payload::CPP, Style::Host, 0);
        case SLANG_HOST_EXECUTABLE:         return make(Kind::Executable, Payload::HostCPU, Style::Host, 0);
        case SLANG_SHADER_SHARED_LIBRARY:   return make(Kind::SharedLibrary, Payload::HostCPU, Style::Kernel, 0);
        case SLANG_SHADER_HOST_CALLABLE:    return make(Kind::Callable, Payload::HostCPU, Style::Kernel, 0);
        case SLANG_CUDA_SOURCE:             return make(Kind::Text, Payload::CUDA, Style::Kernel, 0);
        case SLANG_PTX:                     return make(Kind::Executable, Payload::PTX, Style::Kernel, 0);
        case SLANG_OBJECT_CODE:             return make(Kind::ObjectCode, Payload::HostCPU, Style::Kernel, 0);
        default: break;
    }

    SLANG_UNEXPECTED("Unhandled type");
}

/* static */bool ArtifactDesc::isPayloadGpuBinary(Payload payloadType)
{
    switch (payloadType)
    {
        case Payload::DXIL:
        case Payload::DXBC:
        case Payload::SPIRV:
        case Payload::PTX:
        {
            return true;
        }
        default: break;
    }
    return false;
}

/* static */bool ArtifactDesc::isPayloadCpuBinary(Payload payloadType)
{
    switch (payloadType)
    {
        case Payload::X86:
        case Payload::X86_64:
        case Payload::AARCH:
        case Payload::AARCH64:
        case Payload::HostCPU:
        {
            return true;
        }
        default: break;
    }
    return false;
}

/* static */bool ArtifactDesc::isPayloadGpuBinaryLinkable(Payload payload)
{
    switch (payload)
    {
        case Payload::DXBC:
        {
            // It seems as if DXBC is potentially linkable from
            // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-appendix-keywords#export
            return true;
        }

        case Payload::DXIL:
        case Payload::PTX:
        case Payload::SPIRV:
        {
            // We can't *actually* link PTX or SPIR-V currently but it is in principal possible
            // so let's say we accept for now
            return true;
        }
        default: break;
    }
    return false;
}

bool ArtifactDesc::isBinaryLinkable() const
{
    if (isKindBinaryLinkable(kind))
    {
        return isPayloadCpuBinary(payload) || isPayloadGpuBinaryLinkable(payload) ||  payload == ArtifactPayload::SlangIR;
    }
    
    return false;
}

/* static */bool ArtifactDesc::isKindBinaryLinkable(Kind kind)
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

/* static*/ UnownedStringSlice ArtifactDesc::getCpuExtensionForKind(Kind kind)
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

UnownedStringSlice ArtifactDesc::getDefaultExtension()
{
    if (isPayloadCpuBinary(payload))
    {
        return getCpuExtensionForKind(kind);
    }
    else
    {
        return getDefaultExtensionForPayload(payload);
    }
}

/* static */UnownedStringSlice ArtifactDesc::getDefaultExtensionForPayload(Payload payload)
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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Artifact !!!!!!!!!!!!!!!!!!!!!!!!!!! */

Artifact::~Artifact()
{
    if (m_pathType == PathType::Temporary)
    {
        File::remove(m_path);
    }

    for (const auto& entry : m_entries)
    {
        // Release the associated data
        switch (entry.type)
        {
            case Entry::Type::ObjectInstance:
            {
                entry.object->releaseReference();
                break;
            }
            case Entry::Type::InterfaceInstance:
            {
                entry.intf->release();
                break;
            }
            default: break;
        }
    }
}

Index Artifact::indexOf(Entry::Type type) const
{
    return m_entries.findFirstIndex([&](const Entry& entry) -> bool { return entry.type == type; });
}

void Artifact::add(Entry::Style style, RefObject* obj)
{
    SLANG_ASSERT(obj);

    Entry entry;
    entry.type = Entry::Type::ObjectInstance;
    entry.style = style;
    entry.object = obj;

    obj->addReference();
    
    m_entries.add(entry);
}

void Artifact::add(Entry::Style style, ISlangUnknown* intf)
{
    // Can't be nullptr
    SLANG_ASSERT(intf);

    Entry entry;
    entry.type = Entry::Type::InterfaceInstance;
    entry.style = style;

    intf->addRef();

    entry.intf = intf;
    m_entries.add(entry);
}

bool Artifact::exists() const
{
    // If we have a blob it exists
    if (m_blob)
    {
        return true;
    }

    // If we have an associated entry that represents the artifact it exists
    for (const auto& entry : m_entries)
    {
        if (entry.style == Entry::Style::Artifact)
        {
            // There is a representation that 'is' the artifact
            return true;
        }
    }

    // If we don't have a path then it can't exist
    if (m_pathType == PathType::None)
    {
        return false;
    }

    // If the file exists we assume it exists
    return File::exists(m_path);
}

ISlangUnknown* Artifact::findInterfaceInstance(const Guid& guid)
{
    for (const auto& entry : m_entries)
    {
        if (entry.type == Entry::Type::InterfaceInstance)
        {
            ISlangUnknown* intf = nullptr;
            if (SLANG_SUCCEEDED(entry.intf->queryInterface(guid, (void**)&intf)) && intf)
            {
                // NOTE! This assumes we *DONT* need to ref count to keep an interface in scope
                // (as strict COM requires so as to allow on demand interfaces).
                intf->release();

                return intf;
            }
        }
    }

    return nullptr;
}

SlangResult Artifact::requireFilePath(Keep keep, String& outFilePath)
{
    if (m_pathType != PathType::None)
    {
        outFilePath = m_path; 
        return SLANG_OK;
    }

    ComPtr<ISlangBlob> blob;

    // Get the contents as a blob. If we can't do that, then we can't write anything...
    SLANG_RETURN_ON_FAIL(loadBlob(getIntermediateKeep(keep), blob));

    const UnownedStringSlice ext = m_desc.getDefaultExtension();

    // TODO(JS): NOTE! This isn't strictly correct, as the generated filename is not guarenteed to be unique
    // if we change it with an extension (or prefix).
    // This doesn't change the previous behavior though.
    String path;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice::fromLiteral("slang-generated"), path));

    if (m_desc.isCpuBinary() && m_desc.kind == ArtifactKind::SharedLibrary)
    {
        const bool isSharedLibraryPrefixPlatform = SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY;
        if (isSharedLibraryPrefixPlatform)
        {
            StringBuilder buf;
            buf << "lib";
            buf << Path::getFileName(path);

            auto parentDir = Path::getParentDirectory(path);
            if (parentDir.getLength())
            {
                // Combine the name with path if their is a parent 
                path = Path::combine(parentDir, buf);
            }
            else
            {
                // Just use the name as is
                path = buf;
            }
        }
    }

    // If there is an extension append it
    if (ext.getLength())
    {
        path.appendChar('.');
        path.append(ext);
    }

    SLANG_RETURN_ON_FAIL(File::writeAllBytes(path, blob->getBufferPointer(), blob->getBufferSize()));

    // Okay we can now add this as temporary path too
    setPath(PathType::Temporary, path);

    return SLANG_OK;
}

SlangResult Artifact::loadBlob(Keep keep, ComPtr<ISlangBlob>& outBlob)
{
    if (m_blob)
    {
        outBlob = m_blob;
        return SLANG_OK;
    }

    // TODO(JS): 
    // Strictly speaking we could *potentially* convert some other representation into
    // a blob by serializing it, but we don't worry about any of that here
    if (m_pathType == PathType::None)
    {
        return SLANG_E_NOT_FOUND;
    }

    // Read into a blob
    ScopedAllocation alloc;
    SLANG_RETURN_ON_FAIL(File::readAllBytes(m_path, alloc));

    // Create as a blob
    RefPtr<RawBlob> blob = RawBlob::moveCreate(alloc);

    // Put in cache 
    if (canKeep(keep))
    {
        setBlob(blob);
    }

    outBlob = blob;
    return SLANG_OK;
}

} // namespace Slang
