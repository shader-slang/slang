// slang-state-serialize.cpp
#include "slang-state-serialize.h"

#include "../core/slang-text-io.h"

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

    StoreContext(OffsetContainer* container)
    {
        m_container = container;
    }

    Offset32Ptr<FileState> findFile(const String& uniqueIdentity)
    {
        Offset32Ptr<FileState> file;
        m_uniqueToFileMap.TryGetValue(uniqueIdentity, file);
        return file;
    }

    Offset32Ptr<FileState> addFile(const String& uniqueIdentity, const UnownedStringSlice* content)
    {
        Offset32Ptr<FileState> file;

        auto base = m_container->getBase();

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
            base->asRaw(file)->contents = m_container->newString(*content);
        }
        if (uniqueIdentity.getLength())
        {
            base->asRaw(file)->uniqueIdentity = m_container->newString(uniqueIdentity.getUnownedSlice());
            m_uniqueToFileMap.Add(uniqueIdentity, file);
        }

        m_files.add(file);
        return file;
    }

    Offset32Ptr<SourceFileState> addSourceFile(SourceFile* sourceFile)
    {
        if (!sourceFile)
        {
            return Offset32Ptr<SourceFileState>();
        }

        auto base = m_container->getBase();

        Offset32Ptr<StateSerializeUtil::SourceFileState> sourceFileState;
        if (m_sourceFileMap.TryGetValue(sourceFile, sourceFileState))
        {
            return sourceFileState;
        }

        const PathInfo& pathInfo = sourceFile->getPathInfo();

        UnownedStringSlice content = sourceFile->getContent();
        Offset32Ptr<FileState> file = addFile(pathInfo.uniqueIdentity, &content);

        Offset32Ptr<OffsetString> foundPath;

        if (pathInfo.foundPath.getLength() && base->asRaw(file)->foundPath.isNull())
        {
            foundPath = fromString(pathInfo.foundPath.getUnownedSlice());
        }
        // Set on the file
        base->asRaw(file)->foundPath = foundPath;

        // Create the source file
        sourceFileState = m_container->newObject<SourceFileState>();

        {
            auto dst = base->asRaw(sourceFileState);
            dst->file = file;
            dst->foundPath = foundPath;
            dst->type = pathInfo.type;
        }

        m_sourceFileMap.Add(sourceFile, sourceFileState);

        return sourceFileState;
    }

    Offset32Ptr<OffsetString> fromString(const String& in)
    {
        Offset32Ptr<OffsetString> value;
        
        if (m_stringMap.TryGetValue(in, value))
        {
            return value;
        }
        value = m_container->newString(in.getUnownedSlice());
        m_stringMap.Add(in, value);
        return value;
    }
    Offset32Ptr<OffsetString> fromName(Name* name)
    {
        if (name)
        {
            return fromString(name->text);
        }
        return Offset32Ptr<OffsetString>();
    }

    Offset32Ptr<PathInfoState> addPathInfo(const CacheFileSystem::PathInfo* srcPathInfo)
    {
        if (!srcPathInfo)
        {
            return Offset32Ptr<PathInfoState>();
        }

        auto base = m_container->getBase();

        Offset32Ptr<PathInfoState> pathInfo;
        if (!m_pathInfoMap.TryGetValue(srcPathInfo, pathInfo))
        {
            // Get the associated file
            Offset32Ptr<FileState> fileState;

            // Only store as file if we have the contents
            if(srcPathInfo->m_fileBlob)
            {
                fileState = addFile(srcPathInfo->getUniqueIdentity(), nullptr);
            }

            // Save the rest of the state
            pathInfo = m_container->newObject<PathInfoState>();
            PathInfoState& dst = base->asRaw(*pathInfo);

            dst.file = fileState;

            // Save any other info
            dst.getCanonicalPathResult = srcPathInfo->m_getCanonicalPathResult;
            dst.getPathTypeResult = srcPathInfo->m_getPathTypeResult;
            dst.loadFileResult = srcPathInfo->m_loadFileResult;
            dst.pathType = srcPathInfo->m_pathType;

            m_pathInfoMap.Add(srcPathInfo, pathInfo);
        }

        // Fill in info on the file
        auto fileState(base->asRaw(pathInfo)->file);

        // If have fileState add any missing element
        if (fileState)
        {
            if (srcPathInfo->m_fileBlob && base->asRaw(fileState)->contents.isNull())
            {
                UnownedStringSlice contents((const char*)srcPathInfo->m_fileBlob->getBufferPointer(), srcPathInfo->m_fileBlob->getBufferSize());
                base->asRaw(fileState)->contents = m_container->newString(contents);
            }

            if (srcPathInfo->m_canonicalPath && base->asRaw(fileState)->canonicalPath.isNull())
            {
                base->asRaw(fileState)->canonicalPath = fromString(srcPathInfo->m_canonicalPath->getString());
            }

            if (srcPathInfo->m_uniqueIdentity && base->asRaw(fileState)->uniqueIdentity.isNull())
            {
                base->asRaw(fileState)->uniqueIdentity = fromString(srcPathInfo->m_uniqueIdentity->getString());
            }
        }

        return pathInfo;
    }

    const Offset32Array<StateSerializeUtil::StringPair> calcDefines(const Dictionary<String, String>& srcDefines)
    {
        typedef StateSerializeUtil::StringPair StringPair;

        Offset32Array<StringPair> dstDefines = m_container->newArray<StringPair>(srcDefines.Count());

        auto base = m_container->getBase();

        Index index = 0;
        for (const auto& srcDefine : srcDefines)
        {
            // Do allocation before setting
            auto key = fromString(srcDefine.Key);
            auto value = fromString(srcDefine.Value);

            auto& dstDefine = base->asRaw(dstDefines[index]);
            dstDefine.first = key;
            dstDefine.second = value;

            index++;
        }

        return dstDefines;
    }

    const Offset32Array<Offset32Ptr<OffsetString>> fromList(const List<String>& src)
    {   
        Offset32Array<Offset32Ptr<OffsetString>> dst = m_container->newArray<Offset32Ptr<OffsetString>>(src.getCount());
        auto base = m_container->getBase();

        for (Index j = 0; j < src.getCount(); ++j)
        {
            base->asRaw(dst[j]) = fromString(src[j]);
        }
        return dst;
    }

    Dictionary<String, Offset32Ptr<OffsetString> > m_stringMap;

    Dictionary<SourceFile*, Offset32Ptr<StateSerializeUtil::SourceFileState> > m_sourceFileMap;
    
    Dictionary<String, Offset32Ptr<StateSerializeUtil::FileState> > m_uniqueToFileMap;

    Dictionary<const CacheFileSystem::PathInfo*, Offset32Ptr<PathInfoState> > m_pathInfoMap;

    List<Offset32Ptr<StateSerializeUtil::FileState> > m_files; 

    OffsetContainer* m_container;
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

/* static */SlangResult StateSerializeUtil::store(EndToEndCompileRequest* request, OffsetContainer& inOutContainer, Offset32Ptr<RequestState>& outRequest)
{
    StoreContext context(&inOutContainer);

    auto base = inOutContainer.getBase();

    auto linkage = request->getLinkage();

    Offset32Ptr<RequestState> requestState = inOutContainer.newObject<RequestState>();

    {
        RequestState* dst = base->asRaw(requestState);

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

        Offset32Array<EntryPointState> dstEntryPoints = inOutContainer.newArray<EntryPointState>(srcEntryPoints.getCount());

        for (Index i = 0; i < srcEntryPoints.getCount(); ++i)
        {
            FrontEndEntryPointRequest* srcEntryPoint = srcEntryPoints[i];
            const auto& srcEndToEndEntryPoint = srcEndToEndEntryPoints[i];

            auto dstSpecializationArgStrings = context.fromList(srcEndToEndEntryPoint.specializationArgStrings);
            Offset32Ptr<OffsetString> dstName = context.fromName(srcEntryPoint->getName());

            EntryPointState& dst = base->asRaw(dstEntryPoints[i]);

            dst.profile = srcEntryPoint->getProfile();
            dst.translationUnitIndex = uint32_t(srcEntryPoint->getTranslationUnitIndex());
            dst.specializationArgStrings = dstSpecializationArgStrings;
            dst.name = dstName;
        }

        base->asRaw(requestState)->entryPoints = dstEntryPoints;
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
        Offset32Array<TargetRequestState> dstTargets = inOutContainer.newArray<TargetRequestState>(linkage->targets.getCount());

        for (Index i = 0; i < linkage->targets.getCount(); ++i)
        {
            TargetRequest* srcTargetRequest = linkage->targets[i];

            // Copy the simple stuff
            {
                auto& dst = base->asRaw(dstTargets[i]);
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

                    Offset32Array<OutputState> dstOutputStates = inOutContainer.newArray<OutputState>(entryPointOutputPaths.Count());

                    Index index = 0;
                    for (const auto& pair : entryPointOutputPaths)
                    {
                        Offset32Ptr<OffsetString> outputPath = inOutContainer.newString(pair.Value.getUnownedSlice());

                        auto& dstOutputState = base->asRaw(dstOutputStates[index]);

                        dstOutputState.entryPointIndex = int32_t(pair.Key);
                        dstOutputState.outputPath = outputPath;

                        index++;
                    }

                    base->asRaw(dstTargets[i]).outputStates = dstOutputStates;
                }
            }
        }
    
        // Save the result
        base->asRaw(requestState)->targetRequests = dstTargets;
    }

    // Add the search paths
    {
        const auto& srcPaths = linkage->searchDirectories.searchDirectories;
        Offset32Array<Offset32Ptr<OffsetString> > dstPaths = inOutContainer.newArray<Offset32Ptr<OffsetString> >(srcPaths.getCount());

        // We don't handle parents here
        SLANG_ASSERT(linkage->searchDirectories.parent == nullptr);
        for (Index i = 0; i < srcPaths.getCount(); ++i)
        {
            base->asRaw(dstPaths[i]) = context.fromString(srcPaths[i].path);
        }
        base->asRaw(requestState)->searchPaths = dstPaths;
    }

    // Add preprocessor definitions
    base->asRaw(requestState)->preprocessorDefinitions = context.calcDefines(linkage->preprocessorDefinitions);

    {
        const auto& srcTranslationUnits = request->getFrontEndReq()->translationUnits;
        Offset32Array<TranslationUnitRequestState> dstTranslationUnits = inOutContainer.newArray<TranslationUnitRequestState>(srcTranslationUnits.getCount());

        for (Index i = 0; i < srcTranslationUnits.getCount(); ++i)
        {
            TranslationUnitRequest* srcTranslationUnit = srcTranslationUnits[i];

            // Do before setting, because this can allocate, and therefore break, the following section
            auto defines = context.calcDefines(srcTranslationUnit->preprocessorDefinitions);
            auto moduleName = context.fromName(srcTranslationUnit->moduleName);

            Offset32Array<Offset32Ptr<SourceFileState>> dstSourceFiles;
            {
                const auto& srcFiles = srcTranslationUnit->getSourceFiles();
                dstSourceFiles = inOutContainer.newArray<Offset32Ptr<SourceFileState> >(srcFiles.getCount());

                for (Index j = 0; j < srcFiles.getCount(); ++j)
                {
                    base->asRaw(dstSourceFiles[j]) = context.addSourceFile(srcFiles[j]);
                }
            }

            TranslationUnitRequestState& dstTranslationUnit = base->asRaw(dstTranslationUnits[i]);

            dstTranslationUnit.language = srcTranslationUnit->sourceLanguage;
            dstTranslationUnit.moduleName = moduleName;
            dstTranslationUnit.sourceFiles = dstSourceFiles;
            dstTranslationUnit.preprocessorDefinitions = defines;
        }

        base->asRaw(requestState)->translationUnits = dstTranslationUnits;
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

            Offset32Array<PathAndPathInfo> pathMap = inOutContainer.newArray<PathAndPathInfo>(srcFiles.Count());

            Index index = 0;
            for (const auto& pair : srcFiles)
            {
                Offset32Ptr<OffsetString> path = context.fromString(pair.Key);
                Offset32Ptr<PathInfoState> pathInfo = context.addPathInfo(pair.Value);

                PathAndPathInfo& dstInfo = base->asRaw(pathMap[index]);
                dstInfo.path = path;
                dstInfo.pathInfo = pathInfo;

                index++;
            }

            base->asRaw(requestState)->pathInfoMap = pathMap;
        }
    }

    // Save all of the files 
    {
        Dictionary<String, int> uniqueNameMap;

        auto files = inOutContainer.newArray<Offset32Ptr<FileState>>(context.m_files.getCount());
        for (Index i = 0; i < context.m_files.getCount(); ++i)
        {
            Offset32Ptr<FileState> file = context.m_files[i];

            // Need to come up with unique names
            String path;

            if (auto canonicalPath = base->asRaw(file)->canonicalPath)
            {
                path = base->asRaw(canonicalPath)->getSlice();
            }
            else if (auto foundPath = base->asRaw(file)->foundPath)
            {
                path = base->asRaw(foundPath)->getSlice();
            }
            else if (auto uniqueIdentity = base->asRaw(file)->uniqueIdentity)
            {
                path = base->asRaw(uniqueIdentity)->getSlice();
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
            base->asRaw(file)->uniqueName = inOutContainer.newString(uniqueName.getUnownedSlice());

            base->asRaw(files[i]) = file;
        }

        base->asRaw(requestState)->files = files;
    }

    // Save all the SourceFile state
    {
        const auto& srcSourceFiles = context.m_sourceFileMap;
        auto dstSourceFiles = inOutContainer.newArray<Offset32Ptr<SourceFileState>>(srcSourceFiles.Count());

        Index index = 0;
        for (const auto& pair : srcSourceFiles)
        {
            base->asRaw(dstSourceFiles[index]) = pair.Value; 
            index++;
        }
        base->asRaw(requestState)->sourceFiles = dstSourceFiles;
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
                m_fileSystem->loadFile(m_base->asRaw(file->uniqueName)->getCstr(), blob.writeRef());
            }

            // If wasn't loaded, and has contents, use that
            if (!blob && file->contents)
            {
                blob = new StringBlob(m_base->asRaw(file->contents)->getSlice());
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
            FileState* file = m_base->asRaw(sourceFile->file);
            ISlangBlob* blob = getFileBlob(file);

            PathInfo pathInfo;

            pathInfo.type = sourceFile->type;

            if (sourceFile->foundPath)
            {
                pathInfo.foundPath = m_base->asRaw(sourceFile->foundPath)->getSlice();
            }
            else if (file->foundPath)
            {
                pathInfo.foundPath = m_base->asRaw(file->foundPath)->getSlice();
            }

            if (file->uniqueIdentity)
            {
                pathInfo.uniqueIdentity = m_base->asRaw(file->uniqueIdentity)->getSlice();
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
        FileState* file = m_base->asRaw(srcInfo->file);
        if (file)
        {
            if (file->uniqueIdentity)
            {
                String uniqueIdentity = m_base->asRaw(file->uniqueIdentity)->getSlice();
                dstInfo->m_uniqueIdentity = new StringBlob(uniqueIdentity);
            }

            if (file->canonicalPath)
            {
                dstInfo->m_canonicalPath = new StringBlob(m_base->asRaw(file->canonicalPath)->getSlice());
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

    List<const char*> toList(const Offset32Array<Offset32Ptr<OffsetString>>& src)
    {
        List<const char*> dst;
        dst.setCount(src.getCount());
        for (Index i = 0; i < src.getCount(); ++i)
        {
            OffsetString* srcString = m_base->asRaw(m_base->asRaw(src[i]));
            dst[i] = srcString ? srcString->getCstr() : nullptr;
        }
        return dst;
    }


    void loadDefines(const Offset32Array<StateSerializeUtil::StringPair>& in, Dictionary<String, String>& out)
    {
        out.Clear();

        for (const auto& define : in)
        {
            out.Add(m_base->asRaw(m_base->asRaw(define).first)->getSlice(), m_base->asRaw(m_base->asRaw(define).second)->getSlice());
        }
    }

    LoadContext(SourceManager* sourceManger, ISlangFileSystem* fileSystem, OffsetBase* base):
        m_sourceManager(sourceManger),
        m_fileSystem(fileSystem),
        m_base(base)
    {
    }

    ISlangFileSystem* m_fileSystem;

    OffsetBase* m_base;

    SourceManager* m_sourceManager;
    Dictionary<SourceFileState*, SourceFile*> m_sourceFileMap;
    Dictionary<FileState*, ComPtr<ISlangBlob> > m_fileToBlobMap;
    Dictionary<const PathInfoState*, CacheFileSystem::PathInfo*> m_pathInfoMap;
};

} // anonymous



/* static */SlangResult StateSerializeUtil::load(OffsetBase& base, RequestState* requestState, ISlangFileSystem* fileSystem, EndToEndCompileRequest* request)
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

    LoadContext context(linkage->getSourceManager(), fileSystem, &base);

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
            TargetRequestState& src = base.asRaw(requestState->targetRequests[i]);
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

                for (const auto& srcOutputStateOffset : src.outputStates)
                {
                    const auto& srcOutputState = base.asRaw(srcOutputStateOffset);

                    SLANG_ASSERT(srcOutputState.entryPointIndex < requestState->entryPoints.getCount());

                    String entryPointPath;
                    if (srcOutputState.outputPath)
                    {
                        entryPointPath = base.asRaw(srcOutputState.outputPath)->getSlice();
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
            dstPaths[i].path = base.asRaw(base.asRaw(srcPaths[i]))->getSlice();
        }
    }

    context.loadDefines(requestState->preprocessorDefinitions, linkage->preprocessorDefinitions);

    {
        auto frontEndReq = request->getFrontEndReq();

        const auto& srcTranslationUnits = requestState->translationUnits;
        auto& dstTranslationUnits = frontEndReq->translationUnits;

        dstTranslationUnits.clear();
        
        for (Index i = 0; i < srcTranslationUnits.getCount(); ++i)
        {
            const auto& srcTranslationUnit = base.asRaw(srcTranslationUnits[i]);

            int index = frontEndReq->addTranslationUnit(srcTranslationUnit.language);
            SLANG_UNUSED(index);
            SLANG_ASSERT(index == i);

            TranslationUnitRequest* dstTranslationUnit = dstTranslationUnits[i];

            context.loadDefines(srcTranslationUnit.preprocessorDefinitions, dstTranslationUnit->preprocessorDefinitions);

            Name* moduleName = nullptr;
            if (srcTranslationUnit.moduleName)
            {
                moduleName = request->getNamePool()->getName(base.asRaw(srcTranslationUnit.moduleName)->getSlice());
            }

            dstTranslationUnit->moduleName = moduleName;

            const auto& srcSourceFiles = srcTranslationUnit.sourceFiles;
            auto& dstSourceFiles = dstTranslationUnit->m_sourceFiles;

            dstSourceFiles.clear();

            for (Index j = 0; j < srcSourceFiles.getCount(); ++j)
            {
                SourceFile* sourceFile = context.getSourceFile(base.asRaw(base.asRaw(srcSourceFiles[i])));
                // Add to translation unit
                dstTranslationUnit->addSourceFile(sourceFile);
            }
        }
    }

    // Entry points
    {
        // Check there aren't any set entry point
        SLANG_ASSERT(request->getFrontEndReq()->m_entryPointReqs.getCount() == 0);

        for (const auto& srcEntryPointOffset : requestState->entryPoints)
        {
            const auto srcEntryPoint = base.asRaw(srcEntryPointOffset);

            const char* name = srcEntryPoint.name ? base.asRaw(srcEntryPoint.name)->getCstr() : nullptr;

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
            for (const auto& pairOffset : requestState->pathInfoMap)
            {
                const auto& pair = base.asRaw(pairOffset);
                CacheFileSystem::PathInfo* pathInfo = context.addPathInfo(base.asRaw(pair.pathInfo));
                dstPathMap.Add(base.asRaw(pair.path)->getSlice(), pathInfo);
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
    OffsetContainer container;
    Offset32Ptr<RequestState> requestState;
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
    return (StateSerializeUtil::RequestState*)(buffer.getBuffer() + kStartOffset);
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

    OffsetBase base;
    base.set(buffer.getBuffer(), buffer.getCount());

    RequestState* requestState = StateSerializeUtil::getRequest(buffer);

    String dirPath;
    SLANG_RETURN_ON_FAIL(StateSerializeUtil::calcDirectoryPathFromFilename(filename, dirPath));

    Path::createDirectory(dirPath);
    // Set up a file system to write into this directory
    RelativeFileSystem relFileSystem(OSFileSystemExt::getSingleton(), dirPath);

    return extractFiles(base, requestState, &relFileSystem);
}

/* static */SlangResult StateSerializeUtil::extractFiles(OffsetBase& base, RequestState* requestState, ISlangFileSystemExt* fileSystem)
{
    StringBuilder builder;

    builder << "[files]\n";

    for (auto fileOffset : requestState->files)
    {
        auto file = base.asRaw(base.asRaw(fileOffset));

        if (file->contents)
        {
            UnownedStringSlice contents = base.asRaw(file->contents)->getSlice();

            SLANG_RETURN_ON_FAIL(fileSystem->saveFile(base.asRaw(file->uniqueName)->getCstr(), contents.begin(), contents.size()));

            OffsetString* originalName = nullptr;
            if (file->canonicalPath)
            {
                originalName = base.asRaw(file->canonicalPath);
            }
            else if (file->foundPath)
            {
                originalName = base.asRaw(file->foundPath);
            }
            else if (file->uniqueIdentity)
            {
                originalName = base.asRaw(file->uniqueIdentity);
            }

            builder << base.asRaw(file->uniqueName)->getSlice() << " -> ";
            if (originalName)
            {
                builder << originalName->getSlice();
            }

            if (builder.getLength() == 0)
            {
                builder << "?";
            }

            builder << "\n";
        }
    }

    builder << "[paths]\n";
    for (const auto pathOffset : requestState->pathInfoMap)
    {
        const auto& path = base.asRaw(pathOffset);

        builder << base.asRaw(path.path)->getSlice() << " -> ";

        const auto pathInfo = base.asRaw(path.pathInfo);

        if (pathInfo->file)
        {
            builder << base.asRaw(base.asRaw(pathInfo->file)->uniqueName)->getSlice();
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
