// slang-compile-product.cpp
#include "slang-compile-product.h"
#include <assert.h>

#include "../core/slang-blob.h"
#include "../core/slang-riff.h"

// Serialization
#include "slang-serialize-ir.h"
#include "slang-serialize-container.h"

namespace Slang {

/* static */CompileProductDesc CompileProductDesc::make(CodeGenTarget target)
{
    switch (target)
    {
        case CodeGenTarget::Unknown:                return make(ContainerType::Unknown, PayloadType::None, Style::Unknown, 0);
        case CodeGenTarget::None:                   return make(ContainerType::None, PayloadType::None, Style::Unknown, 0);
        case CodeGenTarget::GLSL_Vulkan:
        case CodeGenTarget::GLSL_Vulkan_OneDesc:
        case CodeGenTarget::GLSL:
        {
            // For the moment we make all just map to GLSL, but we could use flags
            // or some other mechanism to distinguish the types
            return make(ContainerType::Text, PayloadType::GLSL, Style::Kernel, 0);
        }
        case CodeGenTarget::HLSL:                   return make(ContainerType::Text, PayloadType::HLSL, Style::Kernel, 0);
        case CodeGenTarget::SPIRV:                  return make(ContainerType::Executable, PayloadType::SPIRV, Style::Kernel, 0);
        case CodeGenTarget::SPIRVAssembly:          return make(ContainerType::Text, PayloadType::SPIRVAssembly, Style::Kernel, 0);
        case CodeGenTarget::DXBytecode:             return make(ContainerType::Executable, PayloadType::DXBC, Style::Kernel, 0);
        case CodeGenTarget::DXBytecodeAssembly:     return make(ContainerType::Text, PayloadType::DXBCAssembly, Style::Kernel, 0);
        case CodeGenTarget::DXIL:                   return make(ContainerType::Executable, PayloadType::DXIL, Style::Kernel, 0);
        case CodeGenTarget::DXILAssembly:           return make(ContainerType::Text, PayloadType::DXILAssembly, Style::Kernel, 0);
        case CodeGenTarget::CSource:                return make(ContainerType::Text, PayloadType::C, Style::Kernel, 0);
        case CodeGenTarget::CPPSource:              return make(ContainerType::Text, PayloadType::CPP, Style::Kernel, 0);
        case CodeGenTarget::HostCPPSource:          return make(ContainerType::Text, PayloadType::CPP, Style::Host, 0);
        case CodeGenTarget::HostExecutable:         return make(ContainerType::Executable, PayloadType::HostCPU, Style::Host, 0);
        case CodeGenTarget::ShaderSharedLibrary:    return make(ContainerType::SharedLibrary, PayloadType::HostCPU, Style::Kernel, 0);
        case CodeGenTarget::ShaderHostCallable:     return make(ContainerType::Callable, PayloadType::HostCPU, Style::Kernel, 0);
        case CodeGenTarget::CUDASource:             return make(ContainerType::Text, PayloadType::CUDA, Style::Kernel, 0);
        case CodeGenTarget::PTX:                    return make(ContainerType::Executable, PayloadType::PTX, Style::Kernel, 0);
        case CodeGenTarget::ObjectCode:             return make(ContainerType::ObjectCode, PayloadType::HostCPU, Style::Kernel, 0);
        default: break;
    }

    SLANG_UNEXPECTED("Unhandled type");
}

/* static */bool CompileProductDesc::isCpu(PayloadType payloadType)
{
    switch (payloadType)
    {
        case PayloadType::X86:
        case PayloadType::X86_64:
        case PayloadType::AARCH:
        case PayloadType::AARCH64:
        case PayloadType::HostCPU:
        {
            return true;
        }
        default: break;
    }
    return false;
}

/* static */bool CompileProductDesc::isBinaryLinkable(ContainerType kind)
{
    switch (kind)
    {
        case ContainerType::Library:
        case ContainerType::ObjectCode:
        {
            return true;
        }
        default: break;
    }
    return false;
}

UnownedStringSlice CompileProductDesc::getDefaultExtension()
{
    if (isCpu(payloadType))
    {
        switch (containerType)
        {
            case ContainerType::None:
            case ContainerType::Unknown:
            case ContainerType::Callable:
            case ContainerType::Container:
            {
                return UnownedStringSlice();
            }
            case ContainerType::Text:
            {
                auto ext = getDefaultExtensionForPayloadType(payloadType);
                if (ext.getLength())
                {
                    return ext;
                }
                // Default to txt then...
                return UnownedStringSlice::fromLiteral("txt");
            }
            case ContainerType::Library:
            {
#if SLANG_WINDOWS_FAMILY
                return UnownedStringSlice::fromLiteral("lib");
#else
                return UnownedStringSlice::fromLiteral("a");
#endif
            }
            case ContainerType::ObjectCode:
            {
#if SLANG_WINDOWS_FAMILY
                return UnownedStringSlice::fromLiteral("obj");
#else
                return UnownedStringSlice::fromLiteral("o");
#endif
            }
            case ContainerType::Executable:
            {
#if SLANG_WINDOWS_FAMILY
                return UnownedStringSlice::fromLiteral("exe");
#else
                return UnownedStringSlice();
#endif
            }
            case ContainerType::SharedLibrary:
            {
#if __CYGWIN__ || SLANG_WINDOWS_FAMILY
                return UnownedStringSlice::fromLiteral("dll");
#elif SLANG_APPLE_FAMILY
                return UnownedStringSlice::fromLiteral("dylib");
#elif SLANG_LINUX_FAMILY
                return UnownedStringSlice::fromLiteral("so");
#else
                return UnownedStringSlice();
#endif
            }
            default: break;
        }
    }
    else
    {
        return getDefaultExtensionForPayloadType(payloadType);
    }

    SLANG_UNEXPECTED("Unknown CompileProductDesc type");
}

/* static */UnownedStringSlice CompileProductDesc::getDefaultExtensionForPayloadType(PayloadType payloadType)
{
    switch (payloadType)
    {
        case PayloadType::None:         return UnownedStringSlice();
        case PayloadType::Unknown:      return UnownedStringSlice::fromLiteral("unknown");

        case PayloadType::DXIL:         return UnownedStringSlice::fromLiteral("dxil");
        case PayloadType::DXBC:         return UnownedStringSlice::fromLiteral("dxbc");
        case PayloadType::SPIRV:        return UnownedStringSlice::fromLiteral("spirv");

        case PayloadType::PTX:          return UnownedStringSlice::fromLiteral("ptx");

        case PayloadType::X86:
        case PayloadType::X86_64:
        case PayloadType::AARCH:
        case PayloadType::AARCH64:
        case PayloadType::HostCPU:
        {
            return UnownedStringSlice();
        }

        case PayloadType::SlangIR:      return UnownedStringSlice::fromLiteral("slang-ir");
        case PayloadType::LLVMIR:       return UnownedStringSlice::fromLiteral("llvm-ir");

        case PayloadType::HLSL:         return UnownedStringSlice::fromLiteral("hlsl");
        case PayloadType::GLSL:         return UnownedStringSlice::fromLiteral("glsl");

        case PayloadType::CPP:          return UnownedStringSlice::fromLiteral("cpp");
        case PayloadType::C:            return UnownedStringSlice::fromLiteral("c");

        case PayloadType::CUDA:         return UnownedStringSlice::fromLiteral("cu");

        case PayloadType::Slang:        return UnownedStringSlice::fromLiteral("slang");
        default: break;
    }

    SLANG_UNEXPECTED("Unknown content type");
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CompileProduct !!!!!!!!!!!!!!!!!!!!!!!!!!! */

CompileProduct::~CompileProduct()
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

Index CompileProduct::indexOf(Entry::Type type) const
{
    return m_entries.findFirstIndex([&](const Entry& entry) -> bool { return entry.type == type; });
}

void CompileProduct::add(RefObject* obj)
{
    SLANG_ASSERT(obj);

    Entry entry;
    entry.type = Entry::Type::ObjectInstance;
    entry.object = obj;

    obj->addReference();
    
    m_entries.add(entry);
}

void CompileProduct::add(ISlangUnknown* intf)
{
    // Can't be nullptr
    SLANG_ASSERT(intf);

    Entry entry;
    entry.type = Entry::Type::InterfaceInstance;
    intf->addRef();

    entry.intf = intf;
    m_entries.add(entry);
}

SlangResult CompileProduct::requireFilePath(String& outFilePath)
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

    if (m_desc.isCpu() && m_desc.containerType == CompileProductDesc::ContainerType::SharedLibrary)
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

SlangResult CompileProduct::loadBlob(Cache cacheBehavior, ComPtr<ISlangBlob>& outBlob)
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

SlangResult loadModuleLibrary(CompileProduct::Cache cacheBehavior, CompileProduct* product, EndToEndCompileRequest* req, RefPtr<ModuleLibrary>& outLibrary)
{
    if (auto foundLibrary = product->findObjectInstance<ModuleLibrary>())
    {
        outLibrary = foundLibrary;
        return SLANG_OK;
    }

    ComPtr<ISlangBlob> blob;

    // Load but don't require caching
    SLANG_RETURN_ON_FAIL(product->loadBlob(CompileProduct::Cache::No, blob));

    RefPtr<ModuleLibrary> library;
    SLANG_RETURN_ON_FAIL(loadModuleLibrary((const Byte*)blob->getBufferPointer(), blob->getBufferSize(), req, library));
    
    if (cacheBehavior == CompileProduct::Cache::Yes)
    {
        product->add(library);
    }

    outLibrary = library;
    return SLANG_OK;
}

} // namespace Slang
