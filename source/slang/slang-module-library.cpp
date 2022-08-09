// slang-module-library.cpp
#include "slang-module-library.h"
#include <assert.h>

#include "../core/slang-blob.h"
#include "../core/slang-riff.h"

#include "../core/slang-type-text-util.h"

// Serialization
#include "slang-serialize-ir.h"
#include "slang-serialize-container.h"

namespace Slang {

void* ModuleLibrary::getInterface(const Guid& uuid)
{
    if (uuid == ISlangUnknown::getTypeGuid() || 
        uuid == ICastable::getTypeGuid() || 
        uuid == IArtifactRepresentation::getTypeGuid() ||
        uuid == IModuleLibrary::getTypeGuid())
    {
        return static_cast<IModuleLibrary*>(this);
    }
    return nullptr;
}

void* ModuleLibrary::getObject(const Guid& uuid)
{
    return uuid == getTypeGuid() ? this : nullptr;
}

void* ModuleLibrary::castAs(const Guid& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

SlangResult loadModuleLibrary(const Byte* inBytes, size_t bytesCount, EndToEndCompileRequest* req, ComPtr<IModuleLibrary>& outLibrary)
{
    auto library = new ModuleLibrary;
    ComPtr<IModuleLibrary> scopeLibrary(library);

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

    outLibrary.swap(scopeLibrary);
    return SLANG_OK;
}

SlangResult loadModuleLibrary(ArtifactKeep keep, IArtifact* artifact, EndToEndCompileRequest* req, ComPtr<IModuleLibrary>& outLibrary)
{
    if (auto foundLibrary = findRepresentation<IModuleLibrary>(artifact))
    {
        outLibrary = foundLibrary;
        return SLANG_OK;
    }

    // Load the blob
    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(artifact->loadBlob(getIntermediateKeep(keep), blob.writeRef()));

    // Load the module
    ComPtr<IModuleLibrary> library;
    SLANG_RETURN_ON_FAIL(loadModuleLibrary((const Byte*)blob->getBufferPointer(), blob->getBufferSize(), req, library));
    
    if (canKeep(keep))
    {
        artifact->addRepresentation(library);
    }

    outLibrary.swap(library);
    return SLANG_OK;
}

} // namespace Slang
