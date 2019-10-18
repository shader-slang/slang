// slang-state-serialize.cpp
#include "slang-state-serialize.h"

#include "../core/slang-text-io.h"

#include "../core/slang-math.h"

#include "slang-source-loc.h"

namespace Slang {


namespace { // anonymous

struct StoreContext
{
    StoreContext(RelativeContainer* container)
    {
        m_container = container;
    }

    Safe32Ptr<StateSerializeUtil::SourceFileState> fromSourceFile(SourceFile* sourceFile)
    {
        typedef StateSerializeUtil::SourceFileState SourceFileState;
        Safe32Ptr<SourceFileState> value;
        if (sourceFile)
        {
            if (!m_sourceFileMap.TryGetValue(sourceFile, value))
            {
                value = m_container->allocate<SourceFileState>();

                const auto& pathInfo = sourceFile->getPathInfo();

                auto foundPath = fromString(pathInfo.foundPath);
                auto uniqueIdentity = fromString(pathInfo.uniqueIdentity);
                auto content = fromBlob(sourceFile->getContentBlob());

                SourceFileState& dst = *value;

                dst.type = pathInfo.type;
                dst.foundPath = foundPath;
                dst.uniqueIdentity = uniqueIdentity;
                dst.content = content;

                m_sourceFileMap.Add(sourceFile, value);
            }
        }
        return value;
    }

    Safe32Ptr<RelativeString> fromBlob(ISlangBlob* blob)
    {
        Safe32Ptr<RelativeString> value;
        if (blob)
        {        
            if (m_blobMap.TryGetValue(blob, value))
            {
                return value;
            }
            value = newStringFromBlob(blob);
            m_blobMap.Add(blob, value);
        }
        return value;
    }
    Safe32Ptr<RelativeString> newStringFromBlob(ISlangBlob* blob)
    {
        if (!blob)
        {
            return Safe32Ptr<RelativeString>();
        }

        // If it has terminating 0, we should remove it
        UnownedStringSlice slice((const char*)blob->getBufferPointer(), blob->getBufferSize());
        return m_container->newString(slice);
    }

    Safe32Ptr<RelativeString> fromString(const String& in)
    {
        Safe32Ptr<RelativeString> value;
        
        if (m_stringMap.TryGetValue(in, value))
        {
            return value;
        }
        value = m_container->newString(in.getUnownedSlice());
        m_stringMap.Add(in, value);
        return value;
    }
    Safe32Ptr<RelativeString> fromName(Name* name)
    {
        if (name)
        {
            return fromString(name->text);
        }
        return Safe32Ptr<RelativeString>();
    }

    const Safe32Array<StateSerializeUtil::Define> calcDefines(const Dictionary<String, String>& srcDefines)
    {
        typedef StateSerializeUtil::Define Define;

        Safe32Array<Define> dstDefines = m_container->allocateArray<Define>(srcDefines.Count());

        Index index = 0;
        for (const auto& srcDefine : srcDefines)
        {
            // Do allocation before setting
            auto key = fromString(srcDefine.Key);
            auto value = fromString(srcDefine.Value);

            auto& dstDefine = dstDefines[index];
            dstDefine.key = key;
            dstDefine.value = value;

            index++;
        }

        return dstDefines;
    }

    Dictionary<ISlangBlob*, Safe32Ptr<RelativeString> > m_blobMap;
    Dictionary<SourceFile*, Safe32Ptr<StateSerializeUtil::SourceFileState> > m_sourceFileMap;
    Dictionary<String, Safe32Ptr<RelativeString> > m_stringMap;

    RelativeContainer* m_container;
};

struct LoadContext
{
    typedef StateSerializeUtil::SourceFileState SourceFileState;

    SourceFile* getSourceFile(SourceFileState* srcFile)
    {
        if (srcFile == nullptr)
        {
            return nullptr;
        }

        SourceFile* dstFile;
        if (!m_sourceFileMap.TryGetValue(srcFile, dstFile))
        {
            PathInfo pathInfo;

            UnownedStringSlice slice = srcFile->content->getSlice();

            pathInfo.type = srcFile->type;
            pathInfo.foundPath = srcFile->foundPath->getSlice();
            pathInfo.uniqueIdentity = srcFile->uniqueIdentity->getSlice();

            dstFile = new SourceFile(m_sourceManager, pathInfo, slice.size());

            ISlangBlob* contents = getBlob(srcFile->content);
            dstFile->setContents(contents);

            // Add to map
            m_sourceFileMap.Add(srcFile, dstFile);

            // Add to manager
            m_sourceManager->addSourceFile(pathInfo.uniqueIdentity, dstFile);
        }
        return dstFile;
    }
    ISlangBlob* getBlob(RelativeString* src)
    {
        if (src == nullptr)
        {
            return nullptr;
        }
        ComPtr<ISlangBlob> dstBlob;
        if (!m_contentsMap.TryGetValue(src, dstBlob))
        {
            dstBlob = new StringBlob(src->getSlice());
            m_contentsMap.Add(src, dstBlob);
        }

        return dstBlob;
    }

    LoadContext(SourceManager* sourceManger):
        m_sourceManager(sourceManger)
    {
    }

    SourceManager* m_sourceManager;
    Dictionary<SourceFileState*, SourceFile*> m_sourceFileMap;
    Dictionary<RelativeString*, ComPtr<ISlangBlob>> m_contentsMap;
};

} // anonymous

static void _loadDefines(const Relative32Array<StateSerializeUtil::Define>& in, Dictionary<String, String>& out)
{
    out.Clear();

    for (const auto& define : in)
    {
        out.Add(define.key->getSlice(), define.value->getSlice());
    }
}


/* static */SlangResult StateSerializeUtil::load(RequestState* requestState, EndToEndCompileRequest* request)
{
    auto externalRequest = asExternal(request);

    auto linkage = request->getLinkage();

    LoadContext context(linkage->getSourceManager());

    // Try to set state through API - as doing so means if state stored in multiple places it will be ok

    {
        spSetCompileFlags(externalRequest, (SlangCompileFlags)requestState->compileFlags);
        spSetDumpIntermediates(externalRequest, int(requestState->shouldDumpIntermediates));
        spSetLineDirectiveMode(externalRequest, SlangLineDirectiveMode(requestState->lineDirectiveMode));
        spSetDebugInfoLevel(externalRequest, SlangDebugInfoLevel(requestState->debugInfoLevel));
        spSetOptimizationLevel(externalRequest, SlangOptimizationLevel(requestState->optimizationLevel));
        spSetOutputContainerFormat(externalRequest, SlangContainerFormat(requestState->containerFormat));
        spSetPassThrough(externalRequest, SlangPassThrough(request->passThrough));

        linkage->setMatrixLayoutMode(requestState->defaultMatrixLayoutMode);
    }

    {
        for (Index i = 0; i < requestState->targetRequests.getCount(); ++i)
        {
            TargetRequestState& src = requestState->targetRequests[i];
            int index = spAddCodeGenTarget(externalRequest, SlangCompileTarget(src.target));
            SLANG_ASSERT(index == i);

            auto dstTarget = linkage->targets[i];

            SLANG_ASSERT(dstTarget->getTarget() == src.target);
            dstTarget->targetProfile = src.profile;
            dstTarget->targetFlags = src.targetFlags;
            dstTarget->floatingPointMode = src.floatingPointMode;
        }
    }

    {
        const auto& srcPaths = requestState->searchPaths;
        auto& dstPaths = linkage->searchDirectories.searchDirectories;
        dstPaths.setCount(srcPaths.getCount());
        for (Index i = 0; i < srcPaths.getCount(); ++i)
        {
            dstPaths[i].path = srcPaths[i]->getSlice();
        }
    }

    _loadDefines(requestState->preprocessorDefinitions, linkage->preprocessorDefinitions);

    {
        auto frontEndReq = request->getFrontEndReq();

        const auto& srcTranslationUnits = requestState->translationUnits;
        auto& dstTranslationUnits = frontEndReq->translationUnits;

        dstTranslationUnits.clear();
        
        for (Index i = 0; i < srcTranslationUnits.getCount(); ++i)
        {
            const auto& srcTranslationUnit = srcTranslationUnits[i];

            int index = frontEndReq->addTranslationUnit(srcTranslationUnit.language);
            SLANG_ASSERT(index == i);

            TranslationUnitRequest* dstTranslationUnit = dstTranslationUnits[i];

            _loadDefines(srcTranslationUnit.preprocessorDefinitions, dstTranslationUnit->preprocessorDefinitions);

            Name* moduleName = nullptr;
            if (srcTranslationUnit.moduleName)
            {
                moduleName = request->getNamePool()->getName(srcTranslationUnit.moduleName->getSlice());
            }

            dstTranslationUnit->moduleName = moduleName;

            const auto& srcSourceFiles = srcTranslationUnit.sourceFiles;
            auto& dstSourceFiles = dstTranslationUnit->m_sourceFiles;

            dstSourceFiles.clear();

            for (Index j = 0; j < srcSourceFiles.getCount(); ++j)
            {
                SourceFile* sourceFile = context.getSourceFile(srcSourceFiles[i]);
                // Add to translation unit
                dstTranslationUnit->addSourceFile(sourceFile);
            }
        }
    }

    {
        RefPtr<CacheFileSystem> cacheFileSystem = new CacheFileSystem(nullptr);

        // Put all the files in the cache system

        {
            auto& dstFiles = cacheFileSystem->getFileMap();
            const auto&  srcFiles = requestState->fileSystemFiles;

            for (auto& srcFile : srcFiles)
            {
                String uniqueIdentity = srcFile->uniqueIdentity->getSlice();
                CacheFileSystem::PathInfo* dstInfo = new CacheFileSystem::PathInfo(uniqueIdentity);

                if (srcFile->canonicalPath)
                {
                    String canonicalPath(srcFile->canonicalPath->getSlice());
                    dstInfo->m_canonicalPath = new StringBlob(canonicalPath);
                }

                dstInfo->m_fileBlob = context.getBlob(srcFile->contents);
                dstInfo->m_getCanonicalPathResult = srcFile->getCanonicalPathResult;
                dstInfo->m_getPathTypeResult = srcFile->getPathTypeResult;
                dstInfo->m_loadFileResult = srcFile->loadFileResult;
                dstInfo->m_pathType = srcFile->pathType;
                dstInfo->m_uniqueIdentity = new StringBlob(uniqueIdentity);
                
                dstFiles.Add(uniqueIdentity, dstInfo);
            }
        }

        // We need the references
        {
            auto& files = cacheFileSystem->getFileMap();

            auto& dstPaths = cacheFileSystem->getPathMap();
            const auto& srcPaths = requestState->pathToUniqueMap;

            dstPaths.Clear();

            for (const auto& pair : srcPaths)
            {
                String uniqueIdentifier = String(pair.second->getSlice());
                CacheFileSystem::PathInfo* pathInfo = nullptr;

                files.TryGetValue(uniqueIdentifier, pathInfo);
                dstPaths.Add(pair.first->getSlice(), pathInfo);
            }
        }

        // This is a bit of a hack, we are going to replace the file system, with our one which is filled in
        // with what was read from the file. 

        linkage->fileSystemExt = cacheFileSystem;
        linkage->cacheFileSystem = cacheFileSystem;
    }

    return SLANG_OK;
}

/* static */SlangResult StateSerializeUtil::store(EndToEndCompileRequest* request, RelativeContainer& inOutContainer, Safe32Ptr<RequestState>& outRequest)
{
    StoreContext context(&inOutContainer);

    auto linkage = request->getLinkage();

    Safe32Ptr<RequestState> requestState = inOutContainer.allocate<RequestState>();

    {
        RequestState* dst = requestState; 

        dst->compileFlags = request->getFrontEndReq()->compileFlags;
        dst->shouldDumpIntermediates = request->getBackEndReq()->shouldDumpIntermediates;
        dst->lineDirectiveMode = request->getBackEndReq()->lineDirectiveMode;

        dst->debugInfoLevel = linkage->debugInfoLevel;
        dst->optimizationLevel = linkage->optimizationLevel;
        dst->containerFormat = request->containerFormat;
        dst->passThroughMode = request->passThrough;

        dst->defaultMatrixLayoutMode = linkage->defaultMatrixLayoutMode;
    }

    {
        
        Safe32Array<TargetRequestState> dstTargets = inOutContainer.allocateArray<TargetRequestState>(linkage->targets.getCount());

        for (Index i = 0; i < linkage->targets.getCount(); ++i)
        {
            auto& dst = dstTargets[i];
            TargetRequest* targetRequest = linkage->targets[i];

            dst.target = targetRequest->getTarget();
            dst.profile = targetRequest->getTargetProfile();
            dst.targetFlags = targetRequest->targetFlags;
            dst.floatingPointMode = targetRequest->floatingPointMode;
        }

        requestState->targetRequests = dstTargets;
    }

    {
        const auto& srcPaths = linkage->searchDirectories.searchDirectories;
        Safe32Array<Relative32Ptr<RelativeString> > dstPaths = inOutContainer.allocateArray<Relative32Ptr<RelativeString> >(srcPaths.getCount());

        // We don't handle parents here
        SLANG_ASSERT(linkage->searchDirectories.parent == nullptr);

        for (Index i = 0; i < srcPaths.getCount(); ++i)
        {
            dstPaths[i] = context.fromString(srcPaths[i].path);
        }

        requestState->searchPaths = dstPaths;
    }

    requestState->preprocessorDefinitions = context.calcDefines(linkage->preprocessorDefinitions);

    {
        const auto& srcTranslationUnits = request->getFrontEndReq()->translationUnits;
        Safe32Array<TranslationUnitRequestState> dstTranslationUnits = inOutContainer.allocateArray<TranslationUnitRequestState>(srcTranslationUnits.getCount());

        for (Index i = 0; i < srcTranslationUnits.getCount(); ++i)
        {
            TranslationUnitRequest* srcTranslationUnit = srcTranslationUnits[i];

            // Do before setting, because this can allocate, and therefore break, the following section
            auto defines = context.calcDefines(srcTranslationUnit->preprocessorDefinitions);
            auto moduleName = context.fromName(srcTranslationUnit->moduleName);

            Safe32Array<Relative32Ptr<SourceFileState>> dstSourceFiles;
            {
                const auto& srcFiles = srcTranslationUnit->getSourceFiles();
                dstSourceFiles = inOutContainer.allocateArray<Relative32Ptr<SourceFileState> >(srcFiles.getCount());

                for (Index j = 0; j < srcFiles.getCount(); ++j)
                {
                    dstSourceFiles[j] = context.fromSourceFile(srcFiles[j]);
                }
            }

            TranslationUnitRequestState& dstTranslationUnit = dstTranslationUnits[i];

            dstTranslationUnit.language = srcTranslationUnit->sourceLanguage;
            dstTranslationUnit.moduleName = moduleName;
            dstTranslationUnit.sourceFiles = dstSourceFiles;
            dstTranslationUnit.preprocessorDefinitions = defines;
        }

        requestState->translationUnits = dstTranslationUnits;
    }

    {
        CacheFileSystem* cacheFileSystem = linkage->cacheFileSystem;
        
        {
            const auto& srcFiles = cacheFileSystem->getFileMap();

            Safe32Array<Relative32Ptr<FileState>> dstFiles = inOutContainer.allocateArray<Relative32Ptr<FileState>>(srcFiles.Count());

            Index index = 0;
            for (auto& pair : srcFiles)
            {
                //const String& uniquePath = pair.Key;
                CacheFileSystem::PathInfo* srcInfo = pair.Value;

                Safe32Ptr<RelativeString> dstUnique = context.fromString(srcInfo->getUniqueIdentity());
                Safe32Ptr<RelativeString> dstCanonicalPath;
                if (srcInfo->m_canonicalPath)
                {
                    dstCanonicalPath = context.fromString(srcInfo->m_canonicalPath->getString());
                }
                Safe32Ptr<RelativeString> dstContents = context.fromBlob(srcInfo->m_fileBlob);

                Safe32Ptr<FileState> file = inOutContainer.allocate<FileState>();

                FileState& dst = *file;

                dst.canonicalPath = dstCanonicalPath;
                dst.contents = dstContents;
                dst.uniqueIdentity = dstUnique;

                dst.getCanonicalPathResult = srcInfo->m_getCanonicalPathResult;
                dst.loadFileResult = srcInfo->m_loadFileResult;
                dst.getPathTypeResult = srcInfo->m_getPathTypeResult;
                dst.pathType = srcInfo->m_pathType;

                dstFiles[index] = file;

                index++;
            }

            // Set the files
            requestState->fileSystemFiles = dstFiles;
        }

        // We need the references
        {
            const auto& srcFiles = cacheFileSystem->getPathMap();

            Safe32Array<StringPair> pathUniqueMap = inOutContainer.allocateArray<StringPair>(srcFiles.Count());

            Index index = 0;
            for (const auto& pair : srcFiles)
            {
                Safe32Ptr<RelativeString> path = context.fromString(pair.Key);
                Safe32Ptr<RelativeString> uniqueIdentity = context.fromString(pair.Value->getUniqueIdentity());

                StringPair& dst = pathUniqueMap[index];
                dst.first = path;
                dst.second = uniqueIdentity;

                index++;
            }

            requestState->pathToUniqueMap = pathUniqueMap;
        }
    }

    outRequest = requestState;
    return SLANG_OK;
}

/* static */SlangResult StateSerializeUtil::saveState(EndToEndCompileRequest* request, Stream* stream)
{
    RelativeContainer container;
    Safe32Ptr<RequestState> requestState;
    SLANG_RETURN_ON_FAIL(store(request, container, requestState));
    return RiffUtil::writeData(kSlangStateFourCC, container.getData(), container.getDataCount(), stream);
}

/* static */SlangResult StateSerializeUtil::saveState(EndToEndCompileRequest* request, const String& filename)
{
    RefPtr<Stream> stream(new FileStream(filename, FileMode::Create, FileAccess::Write, FileShare::ReadWrite));
    return saveState(request, stream);
}

/* static */ SlangResult StateSerializeUtil::loadState(const String& filename, List<uint8_t>& outBuffer)
{
    RefPtr<Stream> stream;
    try
    {
        stream = new FileStream(filename, FileMode::Open, FileAccess::Read, FileShare::ReadWrite);
    }
    catch (IOException&)
    {
    	return SLANG_FAIL;
    }

    return loadState(stream, outBuffer);
}

/* static */ SlangResult StateSerializeUtil::loadState(Stream* stream, List<uint8_t>& buffer)
{
    RiffChunk chunk;
    SLANG_RETURN_ON_FAIL(RiffUtil::readData(stream, chunk, buffer));

    if (chunk.m_type != kSlangStateFourCC)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */SlangResult StateSerializeUtil::loadState(const uint8_t* data, size_t size, List<uint8_t>& outBuffer)
{
    MemoryStream stream(FileAccess::Read);

    stream.m_contents.setCount(size);
    ::memcpy(stream.m_contents.getBuffer(), data, size);

    return loadState(&stream, outBuffer);
}

/* static */ StateSerializeUtil::RequestState* StateSerializeUtil::getRequest(const List<uint8_t>& buffer)
{
    return (StateSerializeUtil::RequestState*)buffer.getBuffer();
}


} // namespace Slang
