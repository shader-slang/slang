// slang-compile-product.cpp
#include "slang-compile-product.h"
#include <assert.h>

#include "../core/slang-blob.h"

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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CompileProduct::Entry !!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */CompileProduct::Entry::DataStyle CompileProduct::Entry::getStyle(Type type)
{
    switch (type)
    {
        case Type::File:                return DataStyle::StringRep;
        case Type::TemporaryFile:       return DataStyle::StringRep;
        case Type::Blob:
        case Type::InterfaceInstance:
        {
            return DataStyle::Interface;
        }
        case Type::ObjectInstance:      return DataStyle::Object;
        default: break;
    }
    SLANG_UNEXPECTED("Unknown type");
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CompileProduct !!!!!!!!!!!!!!!!!!!!!!!!!!! */

CompileProduct::~CompileProduct()
{
    for (const auto& entry : m_entries)
    {
        // Handle special cases
        switch (entry.type)
        {
            case Entry::Type::TemporaryFile:
            {
                File::remove(String(entry.stringRep));
                break;
            }
            default: break;
        }

        // Release the associated data
        switch (Entry::getStyle(entry.type))
        {
            case Entry::DataStyle::StringRep:
            {
                // We need to special case StringRep as we can allow null ptr for an empty string
                if (entry.stringRep)
                {
                    entry.stringRep->releaseReference();
                }
                break;
            }
            case Entry::DataStyle::Object:
            {
                entry.object->releaseReference();
                break;
            }
            case Entry::DataStyle::Interface:
            {
                entry.intf->release();
                break;
            }
            default: break;
        }
    }
}

Index CompileProduct::indexOfPath() const
{
    return m_entries.findFirstIndex([&](const Entry& entry) -> bool
        { return entry.type == Entry::Type::TemporaryFile || entry.type == Entry::Type::File; });
}

Index CompileProduct::indexOf(Entry::Type type) const
{
    return m_entries.findFirstIndex([&](const Entry& entry) -> bool { return entry.type == type; });
}

SlangResult CompileProduct::getFilePath(String& outPath)
{
    const Index index = indexOfPath();
    if (index >= 0)
    {
        outPath = String(m_entries[index].stringRep);
        return SLANG_OK;
    }
    return SLANG_E_NOT_FOUND;
}

void CompileProduct::add(Entry::Type type, RefObject* obj)
{
    auto const style = Entry::getStyle(type);
    SLANG_UNUSED(style);

    // Check it's an object type
    SLANG_ASSERT(style == Entry::DataStyle::StringRep || style == Entry::DataStyle::Object);

    // Only StringRep allows null
    SLANG_ASSERT(style == Entry::DataStyle::StringRep || obj);

    if (type == Entry::Type::File ||
        type == Entry::Type::TemporaryFile)
    {
        if (indexOfPath() >= 0)
        {
            SLANG_ASSERT(!"Already has a file associated");
            return;
        }
    }

    Entry entry;
    entry.type = type;
    if (obj)
    {
        obj->addReference();
    }
    entry.object = obj;
    m_entries.add(entry);
}

void CompileProduct::add(Entry::Type type, ISlangUnknown* intf)
{
    if (type == Entry::Type::Blob &&
        indexOf(Entry::Type::Blob) >= 0)
    {
        SLANG_ASSERT(!"Already has a blob");
        return;
    }

    // Check it's an interface type
    SLANG_ASSERT(Entry::getStyle(type) == Entry::DataStyle::Interface);
    // Can't be nullptr
    SLANG_ASSERT(intf);

    Entry entry;
    entry.type = type;
    intf->addRef();

    entry.intf = intf;
    m_entries.add(entry);
}

SlangResult CompileProduct::requireFilePath(String& outFilePath)
{
    const auto index = indexOfPath();
    if (index >= 0)
    {
        outFilePath = String(m_entries[index].stringRep);
        return SLANG_OK;
    }

    ComPtr<ISlangBlob> blob;
    // Get the contents as a blob. If we can't do that, then we can't write anything...
    SLANG_RETURN_ON_FAIL(loadBlob(blob));

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
    addTemporaryFile(path);

    return SLANG_OK;
}

SlangResult CompileProduct::loadBlob(ComPtr<ISlangBlob>& outBlob)
{
    Index index = indexOf(Entry::Type::Blob);
    if (index >= 0)
    {
        outBlob = static_cast<ISlangBlob*>(m_entries[index].intf);
        return SLANG_OK;
    }

    // TODO(JS): 
    // Strictly speaking we could *potentially* convert some other representation into
    // a blob by serializing it, but we don't worry about any of that here
    index = indexOfPath();
    if (index <= 0)
    {
        return SLANG_E_NOT_FOUND;
    }

    String path(m_entries[index].stringRep);

    // Read into a blob
    ScopedAllocation alloc;
    SLANG_RETURN_ON_FAIL(File::readAllBytes(path, alloc));

    // Create as a blob
    RefPtr<RawBlob> blob = RawBlob::moveCreate(alloc);
    add(Entry::Type::Blob, static_cast<ISlangBlob*>(blob));

    outBlob = blob;
    return SLANG_OK;
}

} // namespace Slang
