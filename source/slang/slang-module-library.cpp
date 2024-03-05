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

SlangResult loadModuleLibrary(const Byte* inBytes, size_t bytesCount, String path, EndToEndCompileRequest* req, ComPtr<IModuleLibrary>& outLibrary)
{
    auto library = new ModuleLibrary;
    ComPtr<IModuleLibrary> scopeLibrary(library);

    // Load up the module
    MemoryStreamBase memoryStream(FileAccess::Read, inBytes, bytesCount);

    RiffContainer riffContainer;
    SLANG_RETURN_ON_FAIL(RiffUtil::read(&memoryStream, riffContainer));

    auto linkage = req->getLinkage();
    {
        SerialContainerData containerData;

        SerialContainerUtil::ReadOptions options;
        options.namePool = req->getNamePool();
        options.session = req->getSession();
        options.sharedASTBuilder = linkage->getASTBuilder()->getSharedASTBuilder();
        options.sourceManager = linkage->getSourceManager();
        options.linkage = req->getLinkage();
        options.sink = req->getSink();
        options.astBuilder = linkage->getASTBuilder();
        options.modulePath = path;
        SLANG_RETURN_ON_FAIL(SerialContainerUtil::read(&riffContainer, options, nullptr, containerData));
        DiagnosticSink sink;

        // Modules in the container should be serialized in its depedency order,
        // so that we always load the dependencies before the consuming module.
        for (auto& module : containerData.modules)
        {
            // If the irModule is set, add it
            if (module.irModule)
            {
                if (module.dependentFiles.getCount() == 0)
                    return SLANG_FAIL;
                if (!module.astRootNode)
                    return SLANG_FAIL;
                auto loadedModule = linkage->loadDeserializedModule(
                    as<ModuleDecl>(module.astRootNode)->getName(),
                    PathInfo::makePath(module.dependentFiles.getFirst()),
                    module, &sink);
                if (!loadedModule)
                    return SLANG_FAIL;
                library->m_modules.add(loadedModule);
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

SlangResult loadModuleLibrary(ArtifactKeep keep, IArtifact* artifact, String path, EndToEndCompileRequest* req, ComPtr<IModuleLibrary>& outLibrary)
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
    SLANG_RETURN_ON_FAIL(loadModuleLibrary((const Byte*)blob->getBufferPointer(), blob->getBufferSize(), path, req, library));
    
    if (canKeep(keep))
    {
        artifact->addRepresentation(library);
    }

    outLibrary.swap(library);
    return SLANG_OK;
}

} // namespace Slang
