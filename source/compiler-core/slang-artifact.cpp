// slang-artifact.cpp
#include "slang-artifact.h"

#include "slang-artifact-info.h"

#include "../core/slang-type-text-util.h"
#include "../core/slang-io.h"

namespace Slang {

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
        case SLANG_HOST_HOST_CALLABLE:      return make(Kind::Callable, Payload::HostCPU, Style::Host, 0);
        default: break;
    }

    SLANG_UNEXPECTED("Unhandled type");
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Artifact !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* Artifact::getInterface(const Guid& uuid)
{
    if (uuid == ISlangUnknown::getTypeGuid() || uuid == IArtifact::getTypeGuid())
    {
        return static_cast<IArtifact*>(this);
    }
    return nullptr;
}

Artifact::~Artifact()
{
    // Remove the temporary
    if (m_pathType == PathType::Temporary)
    {
        File::remove(m_path);
    }
    // If there is a temporary lock path, remove that
    if (m_temporaryLockPath.getLength())
    {
        File::remove(m_temporaryLockPath);
    }
}

bool Artifact::exists()
{
    // If we have a blob it exists
    if (m_blob)
    {
        return true;
    }

    // If we have an associated entry that represents the artifact it exists
    if (findElement(IArtifactInstance::getTypeGuid()))
    {
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

ISlangUnknown* Artifact::findElement(const Guid& guid)
{
    for (auto const& element : m_elements)
    {
        ISlangUnknown* value = element.value;

        ISlangUnknown* intf = nullptr;
        if (SLANG_SUCCEEDED(value->queryInterface(guid, (void**)&intf)) && intf)
        {
            // NOTE! This assumes we *DONT* need to ref count to keep an interface in scope
            // (as strict COM requires so as to allow on demand interfaces).
            intf->release();
            return intf;
        }
    }

    return nullptr;
}

void Artifact::addElement(const Desc& desc, ISlangUnknown* intf) 
{ 
    SLANG_ASSERT(intf); 
    Element element{ desc, ComPtr<ISlangUnknown>(intf) };
    m_elements.add(element); 
}

void Artifact::removeElementAt(Index i)
{
    m_elements.removeAt(i);
}

void* Artifact::findElementObject(const Guid& classGuid)
{
    ComPtr<IArtifactInstance> instance;
    for (const auto& element : m_elements)
    {
        ISlangUnknown* value = element.value;

        if (SLANG_SUCCEEDED(value->queryInterface(IArtifactInstance::getTypeGuid(), (void**)instance.writeRef())) && instance)
        {
            void* classInstance = instance->queryObject(classGuid);
            if (classInstance)
            {
                return classInstance;
            }
        }
    }

    return nullptr;
}

SlangResult Artifact::requireFileLike(Keep keep)
{
    // If there is no path set and no blob we still need a name. 
    // If the artifact is a library we can assume it's a system level library, 
    // or it can be found by appropriate search paths. 
    if (m_pathType == PathType::None && 
        m_blob == nullptr && 
        (m_desc.kind == ArtifactKind::Library || 
         m_desc.kind == ArtifactKind::SharedLibrary))
    {
        if (m_name.getLength() > 0)
        {
            return SLANG_OK;
        }

        // TODO(JS): If we could serialize, we could turn some other representation into a file, and therefore 
        // a name, but currently that's not supported
        return SLANG_E_NOT_FOUND;
    }

    // Will turn into a file if necessary
    SLANG_RETURN_ON_FAIL(requireFile(keep));
    return SLANG_OK;
}

SlangResult Artifact::requireFile(Keep keep)
{
    if (m_pathType != PathType::None)
    {
        return SLANG_OK;
    }

    ComPtr<ISlangBlob> blob;

    // Get the contents as a blob. If we can't do that, then we can't write anything...
    SLANG_RETURN_ON_FAIL(loadBlob(getIntermediateKeep(keep), blob.writeRef()));

    // If we have a name, make the generated name based on that name
    // Else just use 'slang-generated' the basis

    UnownedStringSlice nameBase;
    if (m_name.getLength() > 0)
    {
        nameBase = m_name.getUnownedSlice();
    }
    else
    {
        nameBase = UnownedStringSlice::fromLiteral("slang-generated");
    }

    // TODO(JS): NOTE! This isn't strictly correct, as the generated filename is not guarenteed to be unique
    // if we change it with an extension (or prefix).
    // This doesn't change the previous behavior though.
    String temporaryLockPath;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(nameBase, temporaryLockPath));

    String path = temporaryLockPath;

    if (ArtifactInfoUtil::isCpuBinary(m_desc) && 
        (m_desc.kind == ArtifactKind::SharedLibrary ||
         m_desc.kind == ArtifactKind::Library))
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
    const UnownedStringSlice ext = ArtifactInfoUtil::getDefaultExtension(m_desc);

    if (ext.getLength())
    {
        path.appendChar('.');
        path.append(ext);
    }

    // If the final path is different from the lock path save that path
    if (path != temporaryLockPath)
    {
        m_temporaryLockPath = temporaryLockPath;
    }

    // Write the contents
    SLANG_RETURN_ON_FAIL(File::writeAllBytes(path, blob->getBufferPointer(), blob->getBufferSize()));

    // Okay we can now add this as temporary path too
    _setPath(PathType::Temporary, path);

    return SLANG_OK;
}

SlangResult Artifact::loadBlob(Keep keep, ISlangBlob** outBlob)
{
    ComPtr<ISlangBlob> blob(m_blob);

    if (!blob)
    {
        if (m_pathType != PathType::None)
        {
            // Read into a blob
            ScopedAllocation alloc;
            SLANG_RETURN_ON_FAIL(File::readAllBytes(m_path, alloc));

            // Create as a blob
            blob = RawBlob::moveCreate(alloc);
        }
        else
        {
            // Look for a representation that we can serialize into a blob
            for (const auto& element : m_elements)
            {
                ISlangUnknown* intf = element.value;

                ComPtr<IArtifactInstance> inst;
                if (SLANG_SUCCEEDED(intf->queryInterface(IArtifactInstance::getTypeGuid(), (void**)inst.writeRef())) && inst)
                {
                    SlangResult res = inst->writeToBlob(blob.writeRef());
                    if (SLANG_SUCCEEDED(res) && blob)
                    {
                        break;
                    }
                }
            }
        }

        // Wasn't able to construct
        if (!blob)
        {
            return SLANG_E_NOT_FOUND;
        }

        // Put in cache 
        if (canKeep(keep))
        {
            setBlob(blob);
        }
    }

    *outBlob = blob.detach();
    return SLANG_OK;
}

} // namespace Slang
