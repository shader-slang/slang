// slang-state-serialize.cpp
#include "slang-state-serialize.h"

#include "../core/slang-text-io.h"

#include "../core/slang-stream.h"

#include "../core/slang-math.h"

#include "slang-source-loc.h"

namespace Slang {

/* static */const RiffSemanticVersion StateSerializeUtil::g_semanticVersion =
    RiffSemanticVersion::make(StateSerializeUtil::kMajorVersion, StateSerializeUtil::kMinorVersion, StateSerializeUtil::kPatchVersion);

// A function to calculate the hash related in list in part to how the types used are sized. Can catch crude breaking binary differences.
static uint32_t _calcTypeHash()
{
    typedef StateSerializeUtil Util;

    const size_t sizes[] =
    {
        sizeof(Util::FileState),
        sizeof(Util::PathInfoState),
            sizeof(Util::PathInfoState::CompressedResult),
            sizeof(SlangPathType),
        sizeof(Util::PathAndPathInfo),
        sizeof(Util::TargetRequestState),
            sizeof(Profile),
            sizeof(CodeGenTarget),
            sizeof(SlangTargetFlags),
            sizeof(FloatingPointMode),
        sizeof(Util::StringPair),
        sizeof(Util::SourceFileState),
            sizeof(PathInfo::Type),
        sizeof(Util::TranslationUnitRequestState),
            sizeof(SourceLanguage),
        sizeof(Util::EntryPointState),
            sizeof(Profile),
        sizeof(Util::RequestState),
            sizeof(SlangCompileFlags),
            sizeof(bool),                       //< Unfortunately bools size can change across compilers/versions
            sizeof(LineDirectiveMode),
            sizeof(DebugInfoLevel),
            sizeof(OptimizationLevel),
            sizeof(ContainerFormat),
            sizeof(PassThroughMode),
            sizeof(SlangMatrixLayoutMode),
    };

    return uint32_t(GetHashCode((const char*)&sizes, sizeof(sizes)));
}

static uint32_t _getTypeHash()
{
    static uint32_t s_hash = _calcTypeHash();
    return s_hash;
}


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
        file = m_container->newObject<FileState>();

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
        sourceFileState = m_container->newObject<SourceFileState>();

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
            pathInfo = m_container->newObject<PathInfoState>();
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

        Safe32Array<StringPair> dstDefines = m_container->newArray<StringPair>(srcDefines.Count());

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

    const Safe32Array<Relative32Ptr<RelativeString>> fromList(const List<String>& src)
    {   
        Safe32Array<Relative32Ptr<RelativeString>> dst = m_container->newArray<Relative32Ptr<RelativeString>>(src.getCount());
        for (Index j = 0; j < src.getCount(); ++j)
        {
            dst[j] = fromString(src[j]);
        }
        return dst;
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

    Safe32Ptr<RequestState> requestState = inOutContainer.newObject<RequestState>();

    {
        RequestState* dst = requestState;

        dst->compileFlags = request->getFrontEndReq()->compileFlags;
        dst->shouldDumpIntermediates = request->getBackEndReq()->shouldDumpIntermediates;
        dst->lineDirectiveMode = request->getBackEndReq()->lineDirectiveMode;

        dst->debugInfoLevel = linkage->debugInfoLevel;
        dst->optimizationLevel = linkage->optimizationLevel;
        dst->containerFormat = request->containerFormat;
        dst->passThroughMode = request->passThrough;


        dst->useUnknownImageFormatAsDefault = request->getBackEndReq()->useUnknownImageFormatAsDefault;
        dst->obfuscateCode = request->getBackEndReq()->obfuscateCode;

        dst->defaultMatrixLayoutMode = linkage->defaultMatrixLayoutMode;
    }

    // Entry points
    {
        const auto& srcEntryPoints = request->getFrontEndReq()->m_entryPointReqs;
        const auto& srcEndToEndEntryPoints = request->entryPoints;

        SLANG_ASSERT(srcEntryPoints.getCount() == srcEndToEndEntryPoints.getCount());

        Safe32Array<EntryPointState> dstEntryPoints = inOutContainer.newArray<EntryPointState>(srcEntryPoints.getCount());

        for (Index i = 0; i < srcEntryPoints.getCount(); ++i)
        {
            FrontEndEntryPointRequest* srcEntryPoint = srcEntryPoints[i];
            const auto& srcEndToEndEntryPoint = srcEndToEndEntryPoints[i];

            auto dstSpecializationArgStrings = context.fromList(srcEndToEndEntryPoint.specializationArgStrings);
            Safe32Ptr<RelativeString> dstName = context.fromName(srcEntryPoint->getName());

            EntryPointState& dst = dstEntryPoints[i];

            dst.profile = srcEntryPoint->getProfile();
            dst.translationUnitIndex = uint32_t(srcEntryPoint->getTranslationUnitIndex());
            dst.specializationArgStrings = dstSpecializationArgStrings;
            dst.name = dstName;
        }

        requestState->entryPoints = dstEntryPoints;
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
        Safe32Array<TargetRequestState> dstTargets = inOutContainer.newArray<TargetRequestState>(linkage->targets.getCount());

        for (Index i = 0; i < linkage->targets.getCount(); ++i)
        {
            TargetRequest* srcTargetRequest = linkage->targets[i];

            // Copy the simple stuff
            {
                auto& dst = dstTargets[i];
                dst.target = srcTargetRequest->getTarget();
                dst.profile = srcTargetRequest->getTargetProfile();
                dst.targetFlags = srcTargetRequest->targetFlags;
                dst.floatingPointMode = srcTargetRequest->floatingPointMode;
            }

            // Copy the entry point/target output names
            {
                const auto& srcTargetInfos = request->targetInfos;

                if (RefPtr<EndToEndCompileRequest::TargetInfo>* infosPtr = srcTargetInfos.TryGetValue(srcTargetRequest))
                {
                    EndToEndCompileRequest::TargetInfo* infos = *infosPtr;

                    const auto& entryPointOutputPaths = infos->entryPointOutputPaths;

                    Safe32Array<OutputState> dstOutputStates = inOutContainer.newArray<OutputState>(entryPointOutputPaths.Count());

                    Index index = 0;
                    for (const auto& pair : entryPointOutputPaths)
                    {
                        Safe32Ptr<RelativeString> outputPath = inOutContainer.newString(pair.Value.getUnownedSlice());

                        auto& dstOutputState = dstOutputStates[index];

                        dstOutputState.entryPointIndex = int32_t(pair.Key);
                        dstOutputState.outputPath = outputPath;

                        index++;
                    }

                    dstTargets[i].outputStates = dstOutputStates;
                }
            }
        }
    
        // Save the result
        requestState->targetRequests = dstTargets;
    }

    // Add the search paths
    {
        const auto& srcPaths = linkage->searchDirectories.searchDirectories;
        Safe32Array<Relative32Ptr<RelativeString> > dstPaths = inOutContainer.newArray<Relative32Ptr<RelativeString> >(srcPaths.getCount());

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
        Safe32Array<TranslationUnitRequestState> dstTranslationUnits = inOutContainer.newArray<TranslationUnitRequestState>(srcTranslationUnits.getCount());

        for (Index i = 0; i < srcTranslationUnits.getCount(); ++i)
        {
            TranslationUnitRequest* srcTranslationUnit = srcTranslationUnits[i];

            // Do before setting, because this can allocate, and therefore break, the following section
            auto defines = context.calcDefines(srcTranslationUnit->preprocessorDefinitions);
            auto moduleName = context.fromName(srcTranslationUnit->moduleName);

            Safe32Array<Relative32Ptr<SourceFileState>> dstSourceFiles;
            {
                const auto& srcFiles = srcTranslationUnit->getSourceFiles();
                dstSourceFiles = inOutContainer.newArray<Relative32Ptr<SourceFileState> >(srcFiles.getCount());

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
        CacheFileSystem* cacheFileSystem = linkage->getCacheFileSystem();
        if (!cacheFileSystem)
        {
            return SLANG_FAIL;
        }

        // Traverse the references (in process we will construct the map from PathInfo)        
        {
            const auto& srcFiles = cacheFileSystem->getPathMap();

            Safe32Array<PathAndPathInfo> pathMap = inOutContainer.newArray<PathAndPathInfo>(srcFiles.Count());

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

        auto files = inOutContainer.newArray<Relative32Ptr<FileState>>(context.m_files.getCount());
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
        auto dstSourceFiles = inOutContainer.newArray<Relative32Ptr<SourceFileState>>(srcSourceFiles.Count());

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
        if (!file)
        {
            return nullptr;
        }

        ComPtr<ISlangBlob> blob;
        if (!m_fileToBlobMap.TryGetValue(file, blob))
        {
            if (m_fileSystem && file->uniqueName)
            {
                // Try loading from the file system
                m_fileSystem->loadFile(file->uniqueName->getCstr(), blob.writeRef());
            }

            // If wasn't loaded, and has contents, use that
            if (!blob && file->contents)
            {
                blob = new StringBlob(file->contents->getSlice());
            }

            // Add to map, even if the blob is nullptr (say from a failed read)
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

    static List<const char*> toList(const Relative32Array<Relative32Ptr<RelativeString>>& src)
    {
        List<const char*> dst;
        dst.setCount(src.getCount());
        for (Index i = 0; i < src.getCount(); ++i)
        {
            RelativeString* srcString = src[i];
            dst[i] = srcString ? srcString->getCstr() : nullptr;
        }
        return dst;
    }

    LoadContext(SourceManager* sourceManger, ISlangFileSystem* fileSystem):
        m_sourceManager(sourceManger),
        m_fileSystem(fileSystem)
    {
    }

    ISlangFileSystem* m_fileSystem;

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


/* static */SlangResult StateSerializeUtil::load(RequestState* requestState, ISlangFileSystem* fileSystem, EndToEndCompileRequest* request)
{
    auto externalRequest = asExternal(request);

    
    auto linkage = request->getLinkage();

    // TODO(JS): Really should be more exhaustive here, and set up to initial state ideally
    // Reset state
    {
        request->targetInfos.Clear();
        // Remove any requests
        linkage->targets.clear();
    }

    LoadContext context(linkage->getSourceManager(), fileSystem);

    // Try to set state through API - as doing so means if state stored in multiple places it will be ok

    {
        spSetCompileFlags(externalRequest, (SlangCompileFlags)requestState->compileFlags);
        spSetDumpIntermediates(externalRequest, int(requestState->shouldDumpIntermediates));
        spSetLineDirectiveMode(externalRequest, SlangLineDirectiveMode(requestState->lineDirectiveMode));
        spSetDebugInfoLevel(externalRequest, SlangDebugInfoLevel(requestState->debugInfoLevel));
        spSetOptimizationLevel(externalRequest, SlangOptimizationLevel(requestState->optimizationLevel));
        spSetOutputContainerFormat(externalRequest, SlangContainerFormat(requestState->containerFormat));
        spSetPassThrough(externalRequest, SlangPassThrough(request->passThrough));

        request->getBackEndReq()->useUnknownImageFormatAsDefault = requestState->useUnknownImageFormatAsDefault;
        request->getBackEndReq()->obfuscateCode = requestState->obfuscateCode;

        linkage->setMatrixLayoutMode(requestState->defaultMatrixLayoutMode);
    }

    // Add the target requests
    {
        for (Index i = 0; i < requestState->targetRequests.getCount(); ++i)
        {
            TargetRequestState& src = requestState->targetRequests[i];
            int index = spAddCodeGenTarget(externalRequest, SlangCompileTarget(src.target));
            SLANG_ASSERT(index == i);

            auto dstTarget = linkage->targets[index];

            SLANG_ASSERT(dstTarget->getTarget() == src.target);
            dstTarget->targetProfile = src.profile;
            dstTarget->targetFlags = src.targetFlags;
            dstTarget->floatingPointMode = src.floatingPointMode;

            // If there is output state (like output filenames) add here
            if (src.outputStates.getCount())
            {
                RefPtr<EndToEndCompileRequest::TargetInfo> dstTargetInfo(new EndToEndCompileRequest::TargetInfo);
                request->targetInfos[dstTarget] = dstTargetInfo;

                for (const auto& srcOutputState : src.outputStates)
                {
                    SLANG_ASSERT(srcOutputState.entryPointIndex < requestState->entryPoints.getCount());

                    String entryPointPath;
                    if (srcOutputState.outputPath)
                    {
                        entryPointPath = srcOutputState.outputPath->getSlice();
                    }
                    
                    dstTargetInfo->entryPointOutputPaths.Add(srcOutputState.entryPointIndex, entryPointPath);
                }
            }
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
            SLANG_UNUSED(index);
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

    // Entry points
    {
        // Check there aren't any set entry point
        SLANG_ASSERT(request->getFrontEndReq()->m_entryPointReqs.getCount() == 0);

        for (const auto& srcEntryPoint : requestState->entryPoints)
        {
            const char* name = srcEntryPoint.name ? srcEntryPoint.name->getCstr() : nullptr;

            Stage stage = srcEntryPoint.profile.GetStage();

            List<const char*> args = context.toList(srcEntryPoint.specializationArgStrings);

            spAddEntryPointEx(externalRequest, int(srcEntryPoint.translationUnitIndex), name, SlangStage(stage), int(args.getCount()), args.getBuffer());
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

        linkage->m_fileSystemExt = cacheFileSystem;
        linkage->m_cacheFileSystem = cacheFileSystem;
    }

    return SLANG_OK;
}


/* static */SlangResult StateSerializeUtil::saveState(EndToEndCompileRequest* request, Stream* stream)
{
    RelativeContainer container;
    Safe32Ptr<RequestState> requestState;
    SLANG_RETURN_ON_FAIL(store(request, container, requestState));

    Header header;
    header.m_chunk.m_type = kSlangStateFourCC;
    header.m_semanticVersion = g_semanticVersion;
    header.m_typeHash = _getTypeHash();

    return RiffUtil::writeData(&header.m_chunk, sizeof(header),container.getData(), container.getDataCount(), stream);
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
    Header header;

    SLANG_RETURN_ON_FAIL(RiffUtil::readData(stream, &header.m_chunk, sizeof(header), buffer));
    if (header.m_chunk.m_type != kSlangStateFourCC)
    {
        return SLANG_FAIL;
    }

    if (!RiffSemanticVersion::areCompatible(g_semanticVersion, header.m_semanticVersion))
    {
        return SLANG_FAIL;
    }

    if (header.m_typeHash != _getTypeHash())
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */SlangResult StateSerializeUtil::loadState(const uint8_t* data, size_t size, List<uint8_t>& outBuffer)
{
    MemoryStreamBase stream(FileAccess::Read, data, size);
    return loadState(&stream, outBuffer);
}

/* static */ StateSerializeUtil::RequestState* StateSerializeUtil::getRequest(const List<uint8_t>& buffer)
{
    return (StateSerializeUtil::RequestState*)buffer.getBuffer();
}

/* static */SlangResult StateSerializeUtil::calcDirectoryPathFromFilename(const String& filename, String& outPath)
{
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

    outPath = Path::combine(parentDir, baseName);
    return SLANG_OK;
}

/* static */SlangResult StateSerializeUtil::extractFilesToDirectory(const String& filename)
{
    List<uint8_t> buffer;
    SLANG_RETURN_ON_FAIL(StateSerializeUtil::loadState(filename, buffer));

    RequestState* requestState = StateSerializeUtil::getRequest(buffer);

    String dirPath;
    SLANG_RETURN_ON_FAIL(StateSerializeUtil::calcDirectoryPathFromFilename(filename, dirPath));

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
            builder << pathInfo->file->uniqueName->getSlice();
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

static SlangResult _findFirstSourcePath(EndToEndCompileRequest* request, String& outFilename)
{
    // We are going to look through all of the srcTranlationUnits, looking for the first filename

    auto frontEndReq = request->getFrontEndReq();
    const auto& srcTranslationUnits = frontEndReq->translationUnits;

    for (Index i = 0; i < srcTranslationUnits.getCount(); ++i)
    {
        TranslationUnitRequest* srcTranslationUnit = srcTranslationUnits[i];
        const auto& srcSourceFiles = srcTranslationUnit->getSourceFiles();

        for (Index j = 0; j < srcSourceFiles.getCount(); ++j)
        {
            SourceFile* sourceFile = srcSourceFiles[j];

            const PathInfo& pathInfo = sourceFile->getPathInfo();

            if (pathInfo.foundPath.getLength())
            {
                outFilename = pathInfo.foundPath;
                return SLANG_OK;
            }
        }
    }
    return SLANG_FAIL;
}

/* static */SlangResult StateSerializeUtil::findUniqueReproDumpStream(EndToEndCompileRequest* request, String& outFileName, RefPtr<Stream>& outStream)
{
    String sourcePath;

    if (SLANG_FAILED(_findFirstSourcePath(request, sourcePath)))
    {
        sourcePath = "unknown.slang";
    }

    String sourceFileName = Path::getFileName(sourcePath);
    String sourceBaseName = Path::getFileNameWithoutExt(sourceFileName);

    // Okay we need a unique number to make sure the name is unique
    const int maxTries = 100;
    for (int triesCount = 0; triesCount < maxTries; ++triesCount)
    {
        // We could include the count in some way perhaps, but for now let's just go with ticks
        auto tick = ProcessUtil::getClockTick();

        StringBuilder builder;
        builder << sourceBaseName << "-" << tick << ".slang-repro";

        // We write out the file name tried even if it fails, as might be useful in reporting
        outFileName = builder;

        // We could have clashes, as we use ticks, we should get to a point where the clashes stop
        try
        {
            outStream = new FileStream(builder, FileMode::CreateNew, FileAccess::Write, FileShare::WriteOnly);
            return SLANG_OK;
        }
        catch (IOException&)
        {
        }

        // TODO(JS): 
        // Might make sense to sleep here - but don't seem to have cross platform func for that yet.
    }

    return SLANG_FAIL;
}

} // namespace Slang
