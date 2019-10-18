// slang-state-serialize.cpp
#include "slang-state-serialize.h"

#include "../core/slang-text-io.h"

#include "../core/slang-math.h"

#include "slang-source-loc.h"

namespace Slang {


namespace { // anonymous

struct StoreContext
{
    typedef StateSerializeUtil::FileState FileState;
    typedef StateSerializeUtil::SourceFileState SourceFileState;
    typedef StateSerializeUtil::PathInfoState PathInfoState;

    StoreContext(RelativeContainer* container)
    {
        m_container = container;
    }

    Safe32Ptr<FileState> findFile(const String& uniqueIdentity)
    {
        Safe32Ptr<FileState> file;
        m_uniqueToFileMap.TryGetValue(uniqueIdentity, file);
        return file;
    }

    Safe32Ptr<FileState> addFile(const String& uniqueIdentity, const UnownedStringSlice* content)
    {
        Safe32Ptr<FileState> file;

        // Get the file, if it has an identity
        if (uniqueIdentity.getLength() && m_uniqueToFileMap.TryGetValue(uniqueIdentity, file))
        {
            return file;
        }

        // If file was not found create it
        // Create the file
        file = m_container->allocate<FileState>();

        if (content)
        {
            file->contents = m_container->newString(*content);
        }
        if (uniqueIdentity.getLength())
        {
            file->uniqueIdentity = m_container->newString(uniqueIdentity.getUnownedSlice());
            m_uniqueToFileMap.Add(uniqueIdentity, file);
        }

        m_files.add(file);
        return file;
    }

    Safe32Ptr<SourceFileState> addSourceFile(SourceFile* sourceFile)
    {
        if (!sourceFile)
        {
            return Safe32Ptr<SourceFileState>();
        }

        Safe32Ptr<StateSerializeUtil::SourceFileState> sourceFileState;
        if (m_sourceFileMap.TryGetValue(sourceFile, sourceFileState))
        {
            return sourceFileState;
        }

        const PathInfo& pathInfo = sourceFile->getPathInfo();

        UnownedStringSlice content = sourceFile->getContent();
        Safe32Ptr<FileState> file = addFile(pathInfo.uniqueIdentity, &content);

        Safe32Ptr<RelativeString> foundPath;

        if (pathInfo.foundPath.getLength() && file->foundPath == nullptr)
        {
            foundPath = fromString(pathInfo.foundPath.getUnownedSlice());
        }
        // Set on the file
        file->foundPath = foundPath;

        // Create the source file
        sourceFileState = m_container->allocate<SourceFileState>();

        sourceFileState->file = file;
        sourceFileState->foundPath = foundPath;
        sourceFileState->type = pathInfo.type;

        m_sourceFileMap.Add(sourceFile, sourceFileState);

        return sourceFileState;
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

    Safe32Ptr<PathInfoState> addPathInfo(const CacheFileSystem::PathInfo* srcPathInfo)
    {
        if (!srcPathInfo)
        {
            return Safe32Ptr<PathInfoState>();
        }

        Safe32Ptr<PathInfoState> pathInfo;
        if (!m_pathInfoMap.TryGetValue(srcPathInfo, pathInfo))
        {
            // Get the associated file
            Safe32Ptr<FileState> fileState;

            // Only store as file if we have the contents
            if(srcPathInfo->m_fileBlob)
            {
                fileState = addFile(srcPathInfo->getUniqueIdentity(), nullptr);
            }

            // Save the rest of the state
            pathInfo = m_container->allocate<PathInfoState>();
            PathInfoState& dst = *pathInfo;

            pathInfo->file = fileState;

            // Save any other info
            dst.getCanonicalPathResult = srcPathInfo->m_getCanonicalPathResult;
            dst.getPathTypeResult = srcPathInfo->m_getPathTypeResult;
            dst.loadFileResult = srcPathInfo->m_loadFileResult;
            dst.pathType = srcPathInfo->m_pathType;

            m_pathInfoMap.Add(srcPathInfo, pathInfo);
        }

        // Fill in info on the file
        Safe32Ptr<FileState> fileState(m_container->toSafe(pathInfo->file.get()));

        // If have fileState add any missing element
        if (fileState)
        {
            if (srcPathInfo->m_fileBlob && fileState->contents == nullptr)
            {
                UnownedStringSlice contents((const char*)srcPathInfo->m_fileBlob->getBufferPointer(), srcPathInfo->m_fileBlob->getBufferSize());
                fileState->contents = m_container->newString(contents);
            }

            if (srcPathInfo->m_canonicalPath && fileState->canonicalPath == nullptr)
            {
                fileState->canonicalPath = fromString(srcPathInfo->m_canonicalPath->getString());
            }

            if (srcPathInfo->m_uniqueIdentity && fileState->uniqueIdentity == nullptr)
            {
                fileState->uniqueIdentity = fromString(srcPathInfo->m_uniqueIdentity->getString());
            }
        }

        return pathInfo;
    }

    const Safe32Array<StateSerializeUtil::StringPair> calcDefines(const Dictionary<String, String>& srcDefines)
    {
        typedef StateSerializeUtil::StringPair StringPair;

        Safe32Array<StringPair> dstDefines = m_container->allocateArray<StringPair>(srcDefines.Count());

        Index index = 0;
        for (const auto& srcDefine : srcDefines)
        {
            // Do allocation before setting
            auto key = fromString(srcDefine.Key);
            auto value = fromString(srcDefine.Value);

            auto& dstDefine = dstDefines[index];
            dstDefine.first = key;
            dstDefine.second = value;

            index++;
        }

        return dstDefines;
    }

    Dictionary<String, Safe32Ptr<RelativeString> > m_stringMap;

    Dictionary<SourceFile*, Safe32Ptr<StateSerializeUtil::SourceFileState> > m_sourceFileMap;
    
    Dictionary<String, Safe32Ptr<StateSerializeUtil::FileState> > m_uniqueToFileMap;

    Dictionary<const CacheFileSystem::PathInfo*, Safe32Ptr<PathInfoState> > m_pathInfoMap;

    List<Safe32Ptr<StateSerializeUtil::FileState> > m_files; 

    RelativeContainer* m_container;
};

} //

static bool _isStorable(const PathInfo::Type type)
{
    switch (type)
    {
        case PathInfo::Type::Unknown:
        case PathInfo::Type::Normal: 
        case PathInfo::Type::FoundPath: 
        case PathInfo::Type::FromString:
        {
            return true;
        }
        default: return false;
    }
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

    // Add all of the source files
    {
        SourceManager* sourceManager = request->getFrontEndReq()->getSourceManager();
        const List<SourceFile*>& sourceFiles = sourceManager->getSourceFiles();

        for (SourceFile* sourceFile : sourceFiles)
        {
            const PathInfo& pathInfo = sourceFile->getPathInfo();
            if (_isStorable(pathInfo.type))
            {
                context.addSourceFile(sourceFile);
            }
        }
    }

    // Add all the target requests
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

    // Add the search paths
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

    // Add preprocessor definitions
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
                    dstSourceFiles[j] = context.addSourceFile(srcFiles[j]);
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

    // Find files from the file system, and mapping paths to files
    {
        CacheFileSystem* cacheFileSystem = linkage->cacheFileSystem;
        // Traverse the references (in process we will construct the map from PathInfo)        
        {
            const auto& srcFiles = cacheFileSystem->getPathMap();

            Safe32Array<PathAndPathInfo> pathMap = inOutContainer.allocateArray<PathAndPathInfo>(srcFiles.Count());

            Index index = 0;
            for (const auto& pair : srcFiles)
            {
                Safe32Ptr<RelativeString> path = context.fromString(pair.Key);
                Safe32Ptr<PathInfoState> pathInfo = context.addPathInfo(pair.Value);

                PathAndPathInfo& dstInfo = pathMap[index];
                dstInfo.path = path;
                dstInfo.pathInfo = pathInfo;

                index++;
            }

            requestState->pathInfoMap = pathMap;
        }
    }

    // Save all of the files 
    {
        Dictionary<String, int> uniqueNameMap;

        auto files = inOutContainer.allocateArray<Relative32Ptr<FileState>>(context.m_files.getCount());
        for (Index i = 0; i < context.m_files.getCount(); ++i)
        {
            Safe32Ptr<FileState> file = context.m_files[i];

            // Need to come up with unique names
            String path;

            if (file->canonicalPath)
            {
                path = file->canonicalPath->getSlice();
            }
            else if (file->foundPath)
            {
                path = file->foundPath->getSlice();
            }
            else if (file->uniqueIdentity)
            {
                path = file->uniqueIdentity->getSlice();
            }

            if (path.getLength() == 0)
            {
                StringBuilder builder;
                builder << "unnamed" << i;
                path = builder;
            }

            String filename = Path::getFileNameWithoutExt(path);
            String ext = Path::getFileExt(path);

            StringBuilder uniqueName;
            for (Index j = 0; j < 0x10000; j++)
            {
                uniqueName.Clear();
                uniqueName << filename;

                if (j > 0)
                {
                    uniqueName << "-" << j;
                }

                if (ext.getLength())
                {
                    uniqueName << "." << ext;
                }

                int dummy = 0;
                if (!uniqueNameMap.TryGetValueOrAdd(uniqueName, dummy))
                {
                    // It was added so we are done
                    break;
                }
            }

            // Save the unique generated name
            file->uniqueName = inOutContainer.newString(uniqueName.getUnownedSlice());

            files[i] = file;
        }

        requestState->files = files;
    }

    // Save all the SourceFile state
    {
        const auto& srcSourceFiles = context.m_sourceFileMap;
        auto dstSourceFiles = inOutContainer.allocateArray<Relative32Ptr<SourceFileState>>(srcSourceFiles.Count());

        Index index = 0;
        for (const auto& pair : srcSourceFiles)
        {
            dstSourceFiles[index] = pair.Value; 
            index++;
        }
        requestState->sourceFiles = dstSourceFiles;
    }

    outRequest = requestState;
    return SLANG_OK;
}

namespace { // anonymous 

struct LoadContext
{
    typedef StateSerializeUtil::SourceFileState SourceFileState;
    typedef StateSerializeUtil::FileState FileState;
    typedef StateSerializeUtil::PathInfoState PathInfoState;

    ISlangBlob* getFileBlob(FileState* file)
    {
        if (!file || file->contents == nullptr)
        {
            return nullptr;
        }

        ComPtr<ISlangBlob> blob;
        if (!m_fileToBlobMap.TryGetValue(file, blob))
        {
            blob = new StringBlob(file->contents->getSlice());
            m_fileToBlobMap.Add(file, blob);
        }

        return blob;
    }

    SourceFile* getSourceFile(SourceFileState* sourceFile)
    {
        if (sourceFile == nullptr)
        {
            return nullptr;
        }

        SourceFile* dstFile;
        if (!m_sourceFileMap.TryGetValue(sourceFile, dstFile))
        {
            FileState* file = sourceFile->file;
            ISlangBlob* blob = getFileBlob(file);

            PathInfo pathInfo;

            pathInfo.type = sourceFile->type;

            if (sourceFile->foundPath)
            {
                pathInfo.foundPath = sourceFile->foundPath->getSlice();
            }
            else if (file->foundPath)
            {
                pathInfo.foundPath = file->foundPath->getSlice();
            }

            if (file->uniqueIdentity)
            {
                pathInfo.uniqueIdentity = file->uniqueIdentity->getSlice();
            }

            dstFile = new SourceFile(m_sourceManager, pathInfo, blob->getBufferSize());
            dstFile->setContents(blob);

            // Add to map
            m_sourceFileMap.Add(sourceFile, dstFile);

            // Add to manager
            m_sourceManager->addSourceFile(pathInfo.uniqueIdentity, dstFile);
        }
        return dstFile;
    }

    CacheFileSystem::PathInfo* addPathInfo(const PathInfoState* srcInfo)
    {
        CacheFileSystem::PathInfo* pathInfo;
        if (m_pathInfoMap.TryGetValue(srcInfo, pathInfo))
        {
            return pathInfo;
        }

        CacheFileSystem::PathInfo* dstInfo = new CacheFileSystem::PathInfo(String());
        FileState* file = srcInfo->file;
        if (file)
        {
            if (file->uniqueIdentity)
            {
                String uniqueIdentity = file->uniqueIdentity->getSlice();
                dstInfo->m_uniqueIdentity = new StringBlob(uniqueIdentity);
            }

            if (file->canonicalPath)
            {
                dstInfo->m_canonicalPath = new StringBlob(file->canonicalPath->getSlice());
            }

            dstInfo->m_fileBlob = getFileBlob(file);
        }

        dstInfo->m_getCanonicalPathResult = srcInfo->getCanonicalPathResult;
        dstInfo->m_getPathTypeResult = srcInfo->getPathTypeResult;
        dstInfo->m_loadFileResult = srcInfo->loadFileResult;
        dstInfo->m_pathType = srcInfo->pathType;

        m_pathInfoMap.Add(srcInfo, dstInfo);
        return dstInfo;
    }

    LoadContext(SourceManager* sourceManger):
        m_sourceManager(sourceManger)
    {
    }

    SourceManager* m_sourceManager;
    Dictionary<SourceFileState*, SourceFile*> m_sourceFileMap;
    Dictionary<FileState*, ComPtr<ISlangBlob> > m_fileToBlobMap;
    Dictionary<const PathInfoState*, CacheFileSystem::PathInfo*> m_pathInfoMap;
};

} // anonymous

static void _loadDefines(const Relative32Array<StateSerializeUtil::StringPair>& in, Dictionary<String, String>& out)
{
    out.Clear();

    for (const auto& define : in)
    {
        out.Add(define.first->getSlice(), define.second->getSlice());
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
        auto& dstUniqueMap = cacheFileSystem->getUniqueMap();
        auto& dstPathMap = cacheFileSystem->getPathMap();

        // Put all the paths to path info
        {
            for (const auto& pair : requestState->pathInfoMap)
            {
                CacheFileSystem::PathInfo* pathInfo = context.addPathInfo(pair.pathInfo);
                dstPathMap.Add(pair.path->getSlice(), pathInfo);
            }
        }
        // Put all the path infos in the cache system
        {
            for (const auto& pair : context.m_pathInfoMap)
            {
                CacheFileSystem::PathInfo* pathInfo = pair.Value;
                SLANG_ASSERT(pathInfo->m_uniqueIdentity);
                dstUniqueMap.Add(pathInfo->m_uniqueIdentity->getString(), pathInfo);
            }
        }
    
        // This is a bit of a hack, we are going to replace the file system, with our one which is filled in
        // with what was read from the file. 

        linkage->fileSystemExt = cacheFileSystem;
        linkage->cacheFileSystem = cacheFileSystem;
    }

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

/* static */SlangResult StateSerializeUtil::extractFilesToDirectory(const String& filename)
{
    List<uint8_t> buffer;
    SLANG_RETURN_ON_FAIL(StateSerializeUtil::loadState(filename, buffer));

    RequestState* requestState = StateSerializeUtil::getRequest(buffer);

    String absPath;
    SLANG_RETURN_ON_FAIL(Path::getCanonical(filename, absPath));

    String parentDir = Path::getParentDirectory(absPath);

    String baseName = Path::getFileNameWithoutExt(filename);
    String ext = Path::getFileExt(filename);

    if (ext.getLength() == 0)
    {
        StringBuilder builder;
        builder << baseName << "-files";
        baseName = builder;
    }

    String dirPath = Path::combine(parentDir, baseName);

    Path::createDirectory(dirPath);
    // Set up a file system to write into this directory
    RelativeFileSystem relFileSystem(OSFileSystemExt::getSingleton(), dirPath);

    return extractFiles(requestState, &relFileSystem);
}

/* static */SlangResult StateSerializeUtil::extractFiles(RequestState* requestState, ISlangFileSystemExt* fileSystem)
{
    StringBuilder builder;

    builder << "[files]\n";

    for (FileState* file : requestState->files)
    {
        if (file->contents)
        {
            UnownedStringSlice contents = file->contents->getSlice();

            SLANG_RETURN_ON_FAIL(fileSystem->saveFile(file->uniqueName->getCstr(), contents.begin(), contents.size()));

            RelativeString* originalName = nullptr;
            if (file->canonicalPath)
            {
                originalName = file->canonicalPath;
            }
            else if (file->foundPath)
            {
                originalName = file->foundPath;
            }
            else if (file->uniqueIdentity)
            {
                originalName = file->uniqueIdentity;
            }

            builder << file->uniqueName->getSlice() << " -> ";
            if (originalName)
            {
                builder << originalName->getSlice();
            }
            else
            {
                builder << "?";
            }
            builder << "\n";
        }
    }

    builder << "[paths]\n";
    for (const PathAndPathInfo& path : requestState->pathInfoMap)
    {
        builder << path.path->getSlice() << " -> ";

        const auto pathInfo = path.pathInfo.get();

        if (pathInfo->file)
        {
            builder << pathInfo->file->uniqueIdentity->getSlice();
        }
        else
        {
            typedef CacheFileSystem::CompressedResult CompressedResult;
            if (pathInfo->getPathTypeResult == CompressedResult::Ok)
            {
                switch (pathInfo->pathType)
                {
                    case SLANG_PATH_TYPE_FILE: builder << "file "; break;
                    case SLANG_PATH_TYPE_DIRECTORY: builder << "directory "; break;
                    default: builder << "?"; break;
                }
            }

            CompressedResult curRes =  pathInfo->getCanonicalPathResult;
            CompressedResult results[] =
            {
                pathInfo->getPathTypeResult,
                pathInfo->loadFileResult,
            };

            for (auto compRes : results)
            {
                if (int(compRes) > int(curRes))
                {
                    curRes = compRes;
                }
            }

            switch (curRes)
            {
                default:
                case CompressedResult::Uninitialized: break;
                case CompressedResult::Ok: break;
                
                case CompressedResult::NotFound:    builder << " [not found]"; break;
                case CompressedResult::CannotOpen:  builder << "[cannot open]"; break;
                case CompressedResult::Fail:        builder << "[fail]"; break;
            }

            
        }

        builder << "\n";
    }

    SLANG_RETURN_ON_FAIL(fileSystem->saveFile("manifest.txt", builder.getBuffer(), builder.getLength()));
    return SLANG_OK;
}

} // namespace Slang
