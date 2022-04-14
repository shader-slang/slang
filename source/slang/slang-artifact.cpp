// slang-artifact.cpp
#include "slang-artifact.h"
#include <assert.h>

#include "../core/slang-blob.h"
#include "../core/slang-riff.h"

#include "../core/slang-type-text-util.h"

// Serialization
#include "slang-serialize-ir.h"
#include "slang-serialize-container.h"

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

    const auto target = (CodeGenTarget)TypeTextUtil::findCompileTargetFromExtension(slice);

    return make(target);
}

/* static */ArtifactDesc ArtifactDesc::make(CodeGenTarget target)
{
    switch (target)
    {
        case CodeGenTarget::Unknown:                return make(Kind::Unknown, Payload::None, Style::Unknown, 0);
        case CodeGenTarget::None:                   return make(Kind::None, Payload::None, Style::Unknown, 0);
        case CodeGenTarget::GLSL_Vulkan:
        case CodeGenTarget::GLSL_Vulkan_OneDesc:
        case CodeGenTarget::GLSL:
        {
            // For the moment we make all just map to GLSL, but we could use flags
            // or some other mechanism to distinguish the types
            return make(Kind::Text, Payload::GLSL, Style::Kernel, 0);
        }
        case CodeGenTarget::HLSL:                   return make(Kind::Text, Payload::HLSL, Style::Kernel, 0);
        case CodeGenTarget::SPIRV:                  return make(Kind::Executable, Payload::SPIRV, Style::Kernel, 0);
        case CodeGenTarget::SPIRVAssembly:          return make(Kind::Text, Payload::SPIRVAssembly, Style::Kernel, 0);
        case CodeGenTarget::DXBytecode:             return make(Kind::Executable, Payload::DXBC, Style::Kernel, 0);
        case CodeGenTarget::DXBytecodeAssembly:     return make(Kind::Text, Payload::DXBCAssembly, Style::Kernel, 0);
        case CodeGenTarget::DXIL:                   return make(Kind::Executable, Payload::DXIL, Style::Kernel, 0);
        case CodeGenTarget::DXILAssembly:           return make(Kind::Text, Payload::DXILAssembly, Style::Kernel, 0);
        case CodeGenTarget::CSource:                return make(Kind::Text, Payload::C, Style::Kernel, 0);
        case CodeGenTarget::CPPSource:              return make(Kind::Text, Payload::CPP, Style::Kernel, 0);
        case CodeGenTarget::HostCPPSource:          return make(Kind::Text, Payload::CPP, Style::Host, 0);
        case CodeGenTarget::HostExecutable:         return make(Kind::Executable, Payload::HostCPU, Style::Host, 0);
        case CodeGenTarget::ShaderSharedLibrary:    return make(Kind::SharedLibrary, Payload::HostCPU, Style::Kernel, 0);
        case CodeGenTarget::ShaderHostCallable:     return make(Kind::Callable, Payload::HostCPU, Style::Kernel, 0);
        case CodeGenTarget::CUDASource:             return make(Kind::Text, Payload::CUDA, Style::Kernel, 0);
        case CodeGenTarget::PTX:                    return make(Kind::Executable, Payload::PTX, Style::Kernel, 0);
        case CodeGenTarget::ObjectCode:             return make(Kind::ObjectCode, Payload::HostCPU, Style::Kernel, 0);
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
        return isPayloadCpuBinary(payload) || isPayloadGpuBinaryLinkable(payload);
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

void Artifact::add(RefObject* obj)
{
    SLANG_ASSERT(obj);

    Entry entry;
    entry.type = Entry::Type::ObjectInstance;
    entry.object = obj;

    obj->addReference();
    
    m_entries.add(entry);
}

void Artifact::add(ISlangUnknown* intf)
{
    // Can't be nullptr
    SLANG_ASSERT(intf);

    Entry entry;
    entry.type = Entry::Type::InterfaceInstance;
    intf->addRef();

    entry.intf = intf;
    m_entries.add(entry);
}

bool Artifact::exists() const
{
    // If we have an associated entry something exists
    // TODO(JS):
    // We may need in the future to distinguish between an Entry that is 'useful', but doesn't
    // represent the artifact. In that case we'll need to check each entry
    if (m_entries.getCount() > 0)
    {
        return true;
    }

    if (m_blob)
    {
        // If we have a blob it exists
        return true;
    }

    // If we don't have a path then it can't exist
    if (m_pathType == PathType::None)
    {
        return false;
    }

    // If the file exists we assume it exists
    return File::exists(m_path);
}

SlangResult Artifact::requireFilePath(String& outFilePath)
{
    if (m_pathType != PathType::None)
    {
        outFilePath = m_path; 
        return SLANG_OK;
    }

    ComPtr<ISlangBlob> blob;

    // Get the contents as a blob. If we can't do that, then we can't write anything...
    SLANG_RETURN_ON_FAIL(loadBlob(Cache::No, blob));

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
                // Prefix the lib
                path = Path::combine(parentDir, buf);
            }
            else
            {
                path = buf;
            }
        }
    }

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

SlangResult Artifact::loadBlob(Cache cacheBehavior, ComPtr<ISlangBlob>& outBlob)
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
    if (cacheBehavior == Cache::Yes)
    {
        setBlob(blob);
    }

    outBlob = blob;
    return SLANG_OK;
}

SlangResult loadModuleLibrary(const Byte* inBytes, size_t bytesCount, EndToEndCompileRequest* req, RefPtr<ModuleLibrary>& outLibrary)
{
    RefPtr<ModuleLibrary> library = new ModuleLibrary;

    // Load up the module
    MemoryStreamBase memoryStream(FileAccess::Read, inBytes, bytesCount);

    RiffContainer riffContainer;
    SLANG_RETURN_ON_FAIL(RiffUtil::read(&memoryStream, riffContainer));

    auto linkage = req->getLinkage();

    // TODO(JS): May be better to have a ITypeComponent that encapsulates a collection of modules
    // For now just add to the linkage

    {
        SerialContainerData containerData;

        SerialContainerUtil::ReadOptions options;
        options.namePool = req->getNamePool();
        options.session = req->getSession();
        options.sharedASTBuilder = linkage->getASTBuilder()->getSharedASTBuilder();
        options.sourceManager = linkage->getSourceManager();
        options.linkage = req->getLinkage();
        options.sink = req->getSink();

        SLANG_RETURN_ON_FAIL(SerialContainerUtil::read(&riffContainer, options, containerData));

        for (const auto& module : containerData.modules)
        {
            // If the irModule is set, add it
            if (module.irModule)
            {
                library->m_modules.add(module.irModule);
            }
        }

        for (const auto& entryPoint : containerData.entryPoints)
        {
            FrontEndCompileRequest::ExtraEntryPointInfo dst;
            dst.mangledName = entryPoint.mangledName;
            dst.name = entryPoint.name;
            dst.profile = entryPoint.profile;

            // Add entry point
            library->m_entryPoints.add(dst);
        }
    }

    outLibrary = library;
    return SLANG_OK;
}

SlangResult loadModuleLibrary(Artifact::Cache cacheBehavior, Artifact* product, EndToEndCompileRequest* req, RefPtr<ModuleLibrary>& outLibrary)
{
    if (auto foundLibrary = product->findObjectInstance<ModuleLibrary>())
    {
        outLibrary = foundLibrary;
        return SLANG_OK;
    }

    ComPtr<ISlangBlob> blob;

    // Load but don't require caching
    SLANG_RETURN_ON_FAIL(product->loadBlob(Artifact::Cache::No, blob));

    RefPtr<ModuleLibrary> library;
    SLANG_RETURN_ON_FAIL(loadModuleLibrary((const Byte*)blob->getBufferPointer(), blob->getBufferSize(), req, library));
    
    if (cacheBehavior == Artifact::Cache::Yes)
    {
        product->add(library);
    }

    outLibrary = library;
    return SLANG_OK;
}

} // namespace Slang
