#include "../../slang.h"

#include "../core/slang-io.h"
#include "../core/slang-string-util.h"
#include "../core/slang-shared-library.h"

#include "parameter-binding.h"
#include "lower-to-ir.h"
#include "../slang/parser.h"
#include "../slang/preprocessor.h"
#include "../slang/reflection.h"
#include "syntax-visitors.h"
#include "../slang/type-layout.h"

#include "slang-file-system.h"
#include "../core/slang-writer.h"

#include "source-loc.h"

#include "ir-serialize.h"

// Used to print exception type names in internal-compiler-error messages
#include <typeinfo>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#endif

namespace Slang {


Session::Session()
{
    // Initialize name pool
    getNamePool()->setRootNamePool(getRootNamePool());

    sharedLibraryLoader = DefaultSharedLibraryLoader::getSingleton();
    // Set all the shared library function pointers to nullptr
    ::memset(sharedLibraryFunctions, 0, sizeof(sharedLibraryFunctions));

    // Initialize the lookup table of syntax classes:

    #define SYNTAX_CLASS(NAME, BASE) \
        mapNameToSyntaxClass.Add(getNamePool()->getName(#NAME), getClass<NAME>());

#include "object-meta-begin.h"
#include "syntax-base-defs.h"
#include "expr-defs.h"
#include "decl-defs.h"
#include "modifier-defs.h"
#include "stmt-defs.h"
#include "type-defs.h"
#include "val-defs.h"
#include "object-meta-end.h"

    // Make sure our source manager is initialized
    builtinSourceManager.initialize(nullptr, nullptr);

    m_builtinLinkage = new Linkage(this);

    // Initialize representations of some very basic types:
    initializeTypes();

    // Create scopes for various language builtins.
    //
    // TODO: load these on-demand to avoid parsing
    // stdlib code for languages the user won't use.

    baseLanguageScope = new Scope();

    auto baseModuleDecl = populateBaseLanguageModule(
        this,
        baseLanguageScope);
    loadedModuleCode.Add(baseModuleDecl);

    coreLanguageScope = new Scope();
    coreLanguageScope->nextSibling = baseLanguageScope;

    hlslLanguageScope = new Scope();
    hlslLanguageScope->nextSibling = coreLanguageScope;

    slangLanguageScope = new Scope();
    slangLanguageScope->nextSibling = hlslLanguageScope;

    addBuiltinSource(coreLanguageScope, "core", getCoreLibraryCode());
    addBuiltinSource(hlslLanguageScope, "hlsl", getHLSLLibraryCode());
}

struct IncludeHandlerImpl : IncludeHandler
{
    Linkage*    linkage;
    SearchDirectoryList*    searchDirectories;

    ISlangFileSystemExt* _getFileSystemExt()
    {
        return linkage->getFileSystemExt();
    }

    SlangResult _findFile(SlangPathType fromPathType, const String& fromPath, const String& path, PathInfo& pathInfoOut)
    {
        ISlangFileSystemExt* fileSystemExt = _getFileSystemExt();

        // Get relative path
        ComPtr<ISlangBlob> combinedPathBlob;
        SLANG_RETURN_ON_FAIL(fileSystemExt->calcCombinedPath(fromPathType, fromPath.begin(), path.begin(), combinedPathBlob.writeRef()));
        String combinedPath(StringUtil::getString(combinedPathBlob));
        if (combinedPath.Length() <= 0)
        {
            return SLANG_FAIL;
        }
     
        SlangPathType pathType;
        SLANG_RETURN_ON_FAIL(fileSystemExt->getPathType(combinedPath.begin(), &pathType));
        if (pathType != SLANG_PATH_TYPE_FILE)
        {
            return SLANG_E_NOT_FOUND;
        }

        // Get the uniqueIdentity
        ComPtr<ISlangBlob> uniqueIdentityBlob;
        SLANG_RETURN_ON_FAIL(fileSystemExt->getFileUniqueIdentity(combinedPath.begin(), uniqueIdentityBlob.writeRef()));

        // If the rel path exists -> a uniqueIdentity MUST exists too
        String uniqueIdentity(StringUtil::getString(uniqueIdentityBlob));
        if (uniqueIdentity.Length() <= 0)
        {   
            // Unique identity can't be empty
            return SLANG_FAIL;
        }
        
        pathInfoOut.type = PathInfo::Type::Normal;
        pathInfoOut.foundPath = combinedPath;
        pathInfoOut.uniqueIdentity = uniqueIdentity;
        return SLANG_OK;     
    }

    virtual SlangResult findFile(
        String const& pathToInclude,
        String const& pathIncludedFrom,
        PathInfo& pathInfoOut) override
    {
        pathInfoOut.type = PathInfo::Type::Unknown;

        // Try just relative to current path
        {
            SlangResult res = _findFile(SLANG_PATH_TYPE_FILE, pathIncludedFrom, pathToInclude, pathInfoOut);
            // It either succeeded or wasn't found, anything else is a failure passed back
            if (SLANG_SUCCEEDED(res) || res != SLANG_E_NOT_FOUND)
            {
                return res;
            }
        }

        // Search all the searchDirectories
        for(auto sd = searchDirectories; sd; sd = sd->parent)
        {
            for(auto& dir : sd->searchDirectories)
            {
                SlangResult res = _findFile(SLANG_PATH_TYPE_DIRECTORY, dir.path, pathToInclude, pathInfoOut);
                if (SLANG_SUCCEEDED(res) || res != SLANG_E_NOT_FOUND)
                {
                    return res;
                }
            }
        }

        return SLANG_E_NOT_FOUND;
    }

#if 0
    virtual SlangResult readFile(const String& path,
        ISlangBlob** blobOut) override
    {
        ISlangFileSystem* fileSystemExt = _getFileSystemExt();
        SLANG_RETURN_ON_FAIL(fileSystemExt->loadFile(path.begin(), blobOut));

        request->mDependencyFilePaths.Add(path);

        return SLANG_OK;
    }
#endif

    virtual String simplifyPath(const String& path) override
    {
        ISlangFileSystemExt* fileSystemExt = _getFileSystemExt();
        ComPtr<ISlangBlob> simplifiedPath;
        if (SLANG_FAILED(fileSystemExt->getSimplifiedPath(path.Buffer(), simplifiedPath.writeRef())))
        {
            return path;
        }
        return StringUtil::getString(simplifiedPath);
    }

};

//


Profile getEffectiveProfile(EntryPoint* entryPoint, TargetRequest* target)
{
    auto entryPointProfile = entryPoint->getProfile();
    auto targetProfile = target->targetProfile;

    // Depending on the target *format* we might have to restrict the
    // profile family to one that makes sense.
    //
    // TODO: Some of this should really be handled as validation at
    // the front-end. People shouldn't be allowed to ask for SPIR-V
    // output with Shader Model 5.0...
    switch(target->target)
    {
    default:
        break;

    case CodeGenTarget::GLSL:
    case CodeGenTarget::GLSL_Vulkan:
    case CodeGenTarget::GLSL_Vulkan_OneDesc:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        if(targetProfile.getFamily() != ProfileFamily::GLSL)
        {
            targetProfile.setVersion(ProfileVersion::GLSL_110);
        }
        break;

    case CodeGenTarget::HLSL:
    case CodeGenTarget::DXBytecode:
    case CodeGenTarget::DXBytecodeAssembly:
    case CodeGenTarget::DXIL:
    case CodeGenTarget::DXILAssembly:
        if(targetProfile.getFamily() != ProfileFamily::DX)
        {
            targetProfile.setVersion(ProfileVersion::DX_4_0);
        }
        break;
    }

    auto entryPointProfileVersion = entryPointProfile.GetVersion();
    auto targetProfileVersion = targetProfile.GetVersion();

    // Default to the entry point profile, since we know that has the right stage.
    Profile effectiveProfile = entryPointProfile;

    // Ignore the input from the target profile if it is missing.
    if( targetProfile.getFamily() != ProfileFamily::Unknown )
    {
        // If the target comes from a different profile family, *or* it is from
        // the same family but has a greater version number, then use the target's version.
        if( targetProfile.getFamily() != entryPointProfile.getFamily()
            || (targetProfileVersion > entryPointProfileVersion) )
        {
            effectiveProfile.setVersion(targetProfileVersion);
        }
    }

    // Now consider the possibility that the chosen stage might force an "upgrade"
    // to the profile level.
    ProfileVersion stageMinVersion = ProfileVersion::Unknown;
    switch( effectiveProfile.getFamily() )
    {
    case ProfileFamily::DX:
        switch(effectiveProfile.GetStage())
        {
        default:
            break;

        case Stage::RayGeneration:
        case Stage::Intersection:
        case Stage::ClosestHit:
        case Stage::AnyHit:
        case Stage::Miss:
        case Stage::Callable:
            // The DirectX ray tracing stages implicitly
            // require Shader Model 6.3 or later.
            //
            stageMinVersion = ProfileVersion::DX_6_3;
            break;

        //  TODO: Add equivalent logic for geometry, tessellation, and compute stages.
        }
        break;

    case ProfileFamily::GLSL:
        switch(effectiveProfile.GetStage())
        {
        default:
            break;

        case Stage::RayGeneration:
        case Stage::Intersection:
        case Stage::ClosestHit:
        case Stage::AnyHit:
        case Stage::Miss:
        case Stage::Callable:
            stageMinVersion = ProfileVersion::GLSL_460;
            break;

        //  TODO: Add equivalent logic for geometry, tessellation, and compute stages.
        }
        break;

    default:
        break;
    }

    if( stageMinVersion > effectiveProfile.GetVersion() )
    {
        effectiveProfile.setVersion(stageMinVersion);
    }

    return effectiveProfile;
}


//

Linkage::Linkage(Session* session)
    : m_session(session)
    , m_sourceManager(&m_defaultSourceManager)
{
    getNamePool()->setRootNamePool(session->getRootNamePool());

    m_defaultSourceManager.initialize(session->getBuiltinSourceManager(), nullptr);

    setFileSystem(nullptr);
}

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangBlob    = SLANG_UUID_ISlangBlob;

/** Base class for simple blobs.
*/
class BlobBase : public ISlangBlob, public RefObject
{
public:
    // ISlangUnknown
    SLANG_REF_OBJECT_IUNKNOWN_ALL

protected:
    SLANG_FORCE_INLINE ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == IID_ISlangUnknown || guid == IID_ISlangBlob) ? static_cast<ISlangBlob*>(this) : nullptr;
    }
};

/** A blob that manages some raw data that it owns.
*/
class RawBlob : public BlobBase
{
public:
    // ISlangBlob
    SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE { return m_data; }
    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE { return m_size; }

    // Ctor
    RawBlob(const void* data, size_t size):
        m_size(size)
    {
        m_data = malloc(size);
        memcpy(m_data, data, size);
    }
    ~RawBlob()
    {
        free(m_data);
    }

protected:
    void* m_data;
    size_t m_size;
};

ComPtr<ISlangBlob> createRawBlob(void const* inData, size_t size)
{
    return ComPtr<ISlangBlob>(new RawBlob(inData, size));
}

//
// TargetRequest
//

Session* TargetRequest::getSession()
{
    return linkage->getSession();
}

MatrixLayoutMode TargetRequest::getDefaultMatrixLayoutMode()
{
    return linkage->getDefaultMatrixLayoutMode();
}

//
// TranslationUnitRequest
//

TranslationUnitRequest::TranslationUnitRequest(
    FrontEndCompileRequest* compileRequest)
    : compileRequest(compileRequest)
{
    module = new Module(compileRequest->getLinkage());
}


Session* TranslationUnitRequest::getSession()
{
    return compileRequest->getSession();
}

NamePool* TranslationUnitRequest::getNamePool()
{
    return compileRequest->getNamePool();
}

SourceManager* TranslationUnitRequest::getSourceManager()
{
    return compileRequest->getSourceManager();
}

void TranslationUnitRequest::addSourceFile(SourceFile* sourceFile)
{
    m_sourceFiles.Add(sourceFile);

    // We want to record that the compiled module has a dependency
    // on the path of the source file, but we also need to account
    // for cases where the user added a source string/blob without
    // an associated path (so that the API passes along an empty
    // string).
    //
    auto path = sourceFile->getPathInfo().foundPath;
    if(path.Length())
    {
        getModule()->addFilePathDependency(path);
    }
}


//

static ISlangWriter* _getDefaultWriter(WriterChannel chan)
{
    static FileWriter stdOut(stdout, WriterFlag::IsStatic | WriterFlag::IsUnowned);
    static FileWriter stdError(stderr, WriterFlag::IsStatic | WriterFlag::IsUnowned);
    static NullWriter nullWriter(WriterFlag::IsStatic | WriterFlag::IsConsole);

    switch (chan)
    {
        case WriterChannel::StdError:    return &stdError;
        case WriterChannel::StdOutput:   return &stdOut;
        case WriterChannel::Diagnostic:  return &nullWriter;
        default:
        {
            SLANG_ASSERT(!"Unknown type");
            return &stdError;
        }
    }
}

void EndToEndCompileRequest::setWriter(WriterChannel chan, ISlangWriter* writer)
{
    // If the user passed in null, we will use the default writer on that channel
    m_writers[int(chan)] = writer ? writer : _getDefaultWriter(chan);

    // For diagnostic output, if the user passes in nullptr, we set on mSink.writer as that enables buffering on DiagnosticSink
    if (chan == WriterChannel::Diagnostic)
    {
        m_sink.writer = writer; 
    }
}

SlangResult Linkage::loadFile(String const& path, ISlangBlob** outBlob)
{
    return fileSystemExt->loadFile(path.Buffer(), outBlob);
}

RefPtr<Expr> Linkage::parseTypeString(String typeStr, RefPtr<Scope> scope)
{
    // Create a SourceManager on the stack, so any allocations for 'SourceFile'/'SourceView' etc will be cleaned up
    SourceManager localSourceManager;
    localSourceManager.initialize(getSourceManager(), nullptr);
        
    Slang::SourceFile* srcFile = localSourceManager.createSourceFileWithString(PathInfo::makeTypeParse(), typeStr);
    
    // We'll use a temporary diagnostic sink  
    DiagnosticSink sink;
    sink.sourceManager = &localSourceManager;

    // RAII type to make make sure current SourceManager is restored after parse.
    // Use RAII - to make sure everything is reset even if an exception is thrown.
    struct ScopeReplaceSourceManager
    {
        ScopeReplaceSourceManager(Linkage* linkage, SourceManager* replaceManager):
            m_linkage(linkage),
            m_originalSourceManager(linkage->getSourceManager())
        {
            linkage->setSourceManager(replaceManager);
        }

        ~ScopeReplaceSourceManager()
        {
            m_linkage->setSourceManager(m_originalSourceManager);
        }

        private:
        Linkage* m_linkage;
        SourceManager* m_originalSourceManager;
    };

    // We need to temporarily replace the SourceManager for this CompileRequest
    ScopeReplaceSourceManager scopeReplaceSourceManager(this, &localSourceManager);

    auto tokens = preprocessSource(
        srcFile,
        &sink,
        nullptr,
        Dictionary<String,String>(),
        this,
        nullptr);

    return parseTypeFromSourceFile(
        getSession(),
        tokens, &sink, scope, getNamePool(), SourceLanguage::Slang);
}

RefPtr<Type> checkProperType(
    Linkage*        linkage,
    TypeExp         typeExp,
    DiagnosticSink* sink);

Type* Program::getTypeFromString(String typeStr, DiagnosticSink* sink)
{
    // If we've looked up this type name before,
    // then we can re-use it.
    //
    RefPtr<Type> type;
    if(m_types.TryGetValue(typeStr, type))
        return type;

    // Otherwise, we need to start looking in
    // the modules that were directly or
    // indirectly referenced.
    //
    // TODO: This `scopesToTry` idiom appears
    // all over the code, and isn't really
    // how we should be handling this kind of
    // lookup at all.
    //
    List<RefPtr<Scope>> scopesToTry;
    for(auto module : getModuleDependencies())
        scopesToTry.Add(module->getModuleDecl()->scope);

    auto linkage = getLinkage();
    for(auto& s : scopesToTry)
    {
        RefPtr<Expr> typeExpr = linkage->parseTypeString(
            typeStr, s);
        type = checkProperType(linkage, TypeExp(typeExpr), sink);
        if(type)
            break;
    }
    if( type )
    {
        m_types[typeStr] = type;
    }
    return type;
}

CompileRequestBase::CompileRequestBase(
    Linkage*        linkage,
    DiagnosticSink* sink)
    : m_linkage(linkage)
    , m_sink(sink)
{}


FrontEndCompileRequest::FrontEndCompileRequest(
    Linkage*        linkage,
    DiagnosticSink* sink)
    : CompileRequestBase(linkage, sink)
{
}

void FrontEndCompileRequest::parseTranslationUnit(
    TranslationUnitRequest* translationUnit)
{
    IncludeHandlerImpl includeHandler;
    includeHandler.linkage = getLinkage();
    includeHandler.searchDirectories = &searchDirectories;

    RefPtr<Scope> languageScope;
    switch (translationUnit->sourceLanguage)
    {
    case SourceLanguage::HLSL:
        languageScope = getSession()->hlslLanguageScope;
        break;

    case SourceLanguage::Slang:
    default:
        languageScope = getSession()->slangLanguageScope;
        break;
    }

    Dictionary<String, String> combinedPreprocessorDefinitions;
    for(auto& def : getLinkage()->preprocessorDefinitions)
        combinedPreprocessorDefinitions.Add(def.Key, def.Value);
    for(auto& def : preprocessorDefinitions)
        combinedPreprocessorDefinitions.Add(def.Key, def.Value);
    for(auto& def : translationUnit->preprocessorDefinitions)
        combinedPreprocessorDefinitions.Add(def.Key, def.Value);

    auto module = translationUnit->getModule();
    RefPtr<ModuleDecl> translationUnitSyntax = new ModuleDecl();
    translationUnitSyntax->nameAndLoc.name = translationUnit->moduleName;
    translationUnitSyntax->module = module;
    module->setModuleDecl(translationUnitSyntax);

    for (auto sourceFile : translationUnit->getSourceFiles())
    {
        auto tokens = preprocessSource(
            sourceFile,
            getSink(),
            &includeHandler,
            combinedPreprocessorDefinitions,
            getLinkage(),
            module);

        parseSourceFile(
            translationUnit,
            tokens,
            getSink(),
            languageScope);
    }
}

RefPtr<Program> createUnspecializedProgram(
        FrontEndCompileRequest* compileRequest);

RefPtr<Program> createSpecializedProgram(
    EndToEndCompileRequest* endToEndReq);

void FrontEndCompileRequest::checkAllTranslationUnits()
{
    // Iterate over all translation units and
    // apply the semantic checking logic.
    for( auto& translationUnit : translationUnits )
    {
        checkTranslationUnit(translationUnit.Ptr());
    }
}

void FrontEndCompileRequest::generateIR()
{
    // Our task in this function is to generate IR code
    // for all of the declarations in the translation
    // units that were loaded.

    // Each translation unit is its own little world
    // for code generation (we are not trying to
    // replicate the GLSL linkage model), and so
    // we will generate IR for each (if needed)
    // in isolation.
    for( auto& translationUnit : translationUnits )
    {
        // We want to only run generateIRForTranslationUnit once here. This is for two side effects:
        // * it can dump ir 
        // * it can generate diagnostics

        /// Generate IR for translation unit
        RefPtr<IRModule> irModule(generateIRForTranslationUnit(translationUnit));

        if (verifyDebugSerialization)
        {
            // Verify debug information
            if (SLANG_FAILED(IRSerialUtil::verifySerialize(irModule, getSession(), getSourceManager(), IRSerialBinary::CompressionType::None, IRSerialWriter::OptionFlag::DebugInfo)))
            {
                getSink()->diagnose(irModule->moduleInst->sourceLoc, Diagnostics::serialDebugVerificationFailed);
            }
        }

        if (useSerialIRBottleneck)
        {              
            IRSerialData serialData;
            {
                // Write IR out to serialData - copying over SourceLoc information directly
                IRSerialWriter writer;
                writer.write(irModule, getSourceManager(), IRSerialWriter::OptionFlag::RawSourceLocation, &serialData);

                // Destroy irModule such that memory can be used for newly constructed read irReadModule  
                irModule = nullptr;
            }
            RefPtr<IRModule> irReadModule;
            {
                // Read IR back from serialData
                IRSerialReader reader;
                reader.read(serialData, getSession(), nullptr, irReadModule);
            }

            // Set irModule to the read module
            irModule = irReadModule;
        }

        // Set the module on the translation unit
        translationUnit->getModule()->setIRModule(irModule);
    }
}

// Try to infer a single common source language for a request
static SourceLanguage inferSourceLanguage(FrontEndCompileRequest* request)
{
    SourceLanguage language = SourceLanguage::Unknown;
    for (auto& translationUnit : request->translationUnits)
    {
        // Allow any other language to overide Slang as a choice
        if (language == SourceLanguage::Unknown
            || language == SourceLanguage::Slang)
        {
            language = translationUnit->sourceLanguage;
        }
        else if (language == translationUnit->sourceLanguage)
        {
            // same language as we currently have, so keep going
        }
        else
        {
            // we found a mismatch, so inference fails
            return SourceLanguage::Unknown;
        }
    }
    return language;
}

SlangResult FrontEndCompileRequest::executeActionsInner()
{
    // We currently allow GlSL files on the command line so that we can
    // drive our "pass-through" mode, but we really want to issue an error
    // message if the user is seriously asking us to compile them.
    for (auto& translationUnit : translationUnits)
    {
        switch(translationUnit->sourceLanguage)
        {
        default:
            break;

        case SourceLanguage::GLSL:
            getSink()->diagnose(SourceLoc(), Diagnostics::glslIsNotSupported);
            return SLANG_FAIL;
        }
    }


    // Parse everything from the input files requested
    for (auto& translationUnit : translationUnits)
    {
        parseTranslationUnit(translationUnit.Ptr());
    }
    if (getSink()->GetErrorCount() != 0)
        return SLANG_FAIL;

    // Perform semantic checking on the whole collection
    checkAllTranslationUnits();
    if (getSink()->GetErrorCount() != 0)
        return SLANG_FAIL;


    // Look up all the entry points that are expected,
    // and use them to populate the `program` member.
    //
    m_program = createUnspecializedProgram(this);
    if (getSink()->GetErrorCount() != 0)
        return SLANG_FAIL;

    if ((compileFlags & SLANG_COMPILE_FLAG_NO_CODEGEN) == 0)
    {
        // Generate initial IR for all the translation
        // units, if we are in a mode where IR is called for.
        generateIR();
    }

    if (getSink()->GetErrorCount() != 0)
        return SLANG_FAIL;

    // Do parameter binding generation, for each compilation target.
    //
    for(auto targetReq : getLinkage()->targets)
    {
        auto targetProgram = m_program->getTargetProgram(targetReq);
        targetProgram->getOrCreateLayout(getSink());
    }
    if (getSink()->GetErrorCount() != 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

BackEndCompileRequest::BackEndCompileRequest(
    Linkage*        linkage,
    DiagnosticSink* sink,
    Program*        program)
    : CompileRequestBase(linkage, sink)
    , m_program(program)
{}

EndToEndCompileRequest::EndToEndCompileRequest(
    Session* session)
    : m_session(session)
{
    m_linkage = new Linkage(session);

    m_sink.sourceManager = m_linkage->getSourceManager();

    // Set all the default writers
    for (int i = 0; i < int(WriterChannel::CountOf); ++i)
    {
        setWriter(WriterChannel(i), nullptr);
    }

    m_frontEndReq = new FrontEndCompileRequest(getLinkage(), getSink());

    m_backEndReq = new BackEndCompileRequest(getLinkage(), getSink());
}

SlangResult EndToEndCompileRequest::executeActionsInner()
{
    // If no code-generation target was specified, then try to infer one from the source language,
    // just to make sure we can do something reasonable when invoked from the command line.
    //
    // TODO: This logic should be moved into `options.cpp` or somewhere else
    // specific to the command-line tool.
    //
    if (getLinkage()->targets.Count() == 0)
    {
        auto language = inferSourceLanguage(getFrontEndReq());
        switch (language)
        {
        case SourceLanguage::HLSL:
            getLinkage()->addTarget(CodeGenTarget::DXBytecode);
            break;

        case SourceLanguage::GLSL:
            getLinkage()->addTarget(CodeGenTarget::SPIRV);
            break;

        default:
            break;
        }
    }

    // We only do parsing and semantic checking if we *aren't* doing
    // a pass-through compilation.
    //
    if (passThrough == PassThroughMode::None)
    {
        SLANG_RETURN_ON_FAIL(getFrontEndReq()->executeActionsInner());
    }

    // If command line specifies to skip codegen, we exit here.
    // Note: this is a debugging option.
    //
    if (shouldSkipCodegen ||
        ((getFrontEndReq()->compileFlags & SLANG_COMPILE_FLAG_NO_CODEGEN) != 0))
    {
        // We will use the program (and matching layout information)
        // that was computed in the front-end for all subsequent
        // reflection queries, etc.
        //
        m_specializedProgram = getUnspecializedProgram();

        return SLANG_OK;
    }

    // If codegen is enabled, we need to move along to
    // apply any generic specialization that the user asked for.
    //
    if (passThrough == PassThroughMode::None)
    {
        m_specializedProgram = createSpecializedProgram(this);
        if (getSink()->GetErrorCount() != 0)
            return SLANG_FAIL;

        // For each code generation target, we will generate specialized
        // parameter binding information (taking global generic
        // arguments into account at this time).
        //
        for (auto targetReq : getLinkage()->targets)
        {
            auto targetProgram = m_specializedProgram->getTargetProgram(targetReq);
            targetProgram->getOrCreateLayout(getSink());
        }
        if (getSink()->GetErrorCount() != 0)
            return SLANG_FAIL;
    }
    else
    {
        // We need to create dummy `EntryPoint` objects
        // to make sure that the logic in `generateOutput`
        // sees something worth processing.
        //
        auto specializedProgram = new Program(getLinkage());
        m_specializedProgram = specializedProgram;
        for(auto entryPointReq : getFrontEndReq()->getEntryPointReqs())
        {
            RefPtr<EntryPoint> entryPoint = EntryPoint::createDummyForPassThrough(
                entryPointReq->getName(),
                entryPointReq->getProfile());

            specializedProgram->addEntryPoint(entryPoint);
        }
    }

    // Generate output code, in whatever format was requested
    getBackEndReq()->setProgram(getSpecializedProgram());
    generateOutput(this);
    if (getSink()->GetErrorCount() != 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

// Act as expected of the API-based compiler
SlangResult EndToEndCompileRequest::executeActions()
{
    SlangResult res = executeActionsInner();
    mDiagnosticOutput = getSink()->outputBuffer.ProduceString();
    return res;
}

int FrontEndCompileRequest::addTranslationUnit(SourceLanguage language, Name* moduleName)
{
    UInt result = translationUnits.Count();

    RefPtr<TranslationUnitRequest> translationUnit = new TranslationUnitRequest(this);
    translationUnit->compileRequest = this;
    translationUnit->sourceLanguage = SourceLanguage(language);

    translationUnit->moduleName = moduleName;

    translationUnits.Add(translationUnit);

    return (int) result;
}

int FrontEndCompileRequest::addTranslationUnit(SourceLanguage language)
{
    // We want to ensure that symbols defined in different translation
    // units get unique mangled names, so that we can, e.g., tell apart
    // a `main()` function in `vertex.slang` and a `main()` in `fragment.slang`,
    // even when they are being compiled together.
    //
    String generatedName = "tu";
    generatedName.append(translationUnits.Count());
    return addTranslationUnit(language,  getNamePool()->getName(generatedName));
}

void FrontEndCompileRequest::addTranslationUnitSourceFile(
    int             translationUnitIndex,
    SourceFile*     sourceFile)
{
    translationUnits[translationUnitIndex]->addSourceFile(sourceFile);
}

void FrontEndCompileRequest::addTranslationUnitSourceBlob(
    int             translationUnitIndex,
    String const&   path,
    ISlangBlob*     sourceBlob)
{
    PathInfo pathInfo = PathInfo::makePath(path);
    SourceFile* sourceFile = getSourceManager()->createSourceFileWithBlob(pathInfo, sourceBlob);
    
    addTranslationUnitSourceFile(translationUnitIndex, sourceFile);
}

void FrontEndCompileRequest::addTranslationUnitSourceString(
    int             translationUnitIndex,
    String const&   path,
    String const&   source)
{
    PathInfo pathInfo = PathInfo::makePath(path);
    SourceFile* sourceFile = getSourceManager()->createSourceFileWithString(pathInfo, source);
    
    addTranslationUnitSourceFile(translationUnitIndex, sourceFile);
}

void FrontEndCompileRequest::addTranslationUnitSourceFile(
    int             translationUnitIndex,
    String const&   path)
{
    // TODO: We need to consider whether a relative `path` should cause
    // us to look things up using the registered search paths.
    //
    // This behavior wouldn't make sense for command-line invocations
    // of `slangc`, but at least one API user wondered by the search
    // paths were not taken into account by this function.
    //

    ComPtr<ISlangBlob> sourceBlob;
    SlangResult result = loadFile(path, sourceBlob.writeRef());
    if(SLANG_FAILED(result))
    {
        // Emit a diagnostic!
        getSink()->diagnose(
            SourceLoc(),
            Diagnostics::cannotOpenFile,
            path);
        return;
    }

    addTranslationUnitSourceBlob(
        translationUnitIndex,
        path,
        sourceBlob);
}

int FrontEndCompileRequest::addEntryPoint(
    int                     translationUnitIndex,
    String const&           name,
    Profile                 entryPointProfile)
{
    auto translationUnitReq = translationUnits[translationUnitIndex];

    UInt result = m_entryPointReqs.Count();

    RefPtr<FrontEndEntryPointRequest> entryPointReq = new FrontEndEntryPointRequest(
        this,
        translationUnitIndex,
        getNamePool()->getName(name),
        entryPointProfile);

    m_entryPointReqs.Add(entryPointReq);
//    translationUnitReq->entryPoints.Add(entryPointReq);

    return int(result);
}

int EndToEndCompileRequest::addEntryPoint(
    int                     translationUnitIndex,
    String const&           name,
    Profile                 entryPointProfile,
    List<String> const &    genericTypeNames)
{
    getFrontEndReq()->addEntryPoint(translationUnitIndex, name, entryPointProfile);

    EntryPointInfo entryPointInfo;
    for (auto typeName : genericTypeNames)
        entryPointInfo.genericArgStrings.Add(typeName);

    UInt result = entryPoints.Count();
    entryPoints.Add(_Move(entryPointInfo));
    return (int) result;
}

UInt Linkage::addTarget(
    CodeGenTarget   target)
{
    RefPtr<TargetRequest> targetReq = new TargetRequest();
    targetReq->linkage = this;
    targetReq->target = target;

    UInt result = targets.Count();
    targets.Add(targetReq);
    return (int) result;
}

void Linkage::loadParsedModule(
    RefPtr<TranslationUnitRequest>  translationUnit,
    Name*                           name,
    const PathInfo&                 pathInfo)
{
    // Note: we add the loaded module to our name->module listing
    // before doing semantic checking, so that if it tries to
    // recursively `import` itself, we can detect it.
    //
    RefPtr<Module> loadedModule = translationUnit->getModule();

    // Get a path
    String mostUniqueIdentity = pathInfo.getMostUniqueIdentity();
    SLANG_ASSERT(mostUniqueIdentity.Length() > 0);

    mapPathToLoadedModule.Add(mostUniqueIdentity, loadedModule);
    mapNameToLoadedModules.Add(name, loadedModule);

    auto sink = translationUnit->compileRequest->getSink();

    int errorCountBefore = sink->GetErrorCount();
    checkTranslationUnit(translationUnit.Ptr());
    int errorCountAfter = sink->GetErrorCount();

    if (errorCountAfter != errorCountBefore)
    {
        // There must have been an error in the loaded module.
    }
    else
    {
        // If we didn't run into any errors, then try to generate
        // IR code for the imported module.
        SLANG_ASSERT(errorCountAfter == 0);
        loadedModule->setIRModule(generateIRForTranslationUnit(translationUnit));
    }
    loadedModulesList.Add(loadedModule);
}

Module* Linkage::loadModule(String const& name)
{
    // TODO: We either need to have a diagnostics sink
    // get passed into this operation, or associate
    // one with the linkage.
    //
    DiagnosticSink* sink = nullptr;
    return findOrImportModule(
        getNamePool()->getName(name),
        SourceLoc(),
        sink);
}


RefPtr<Module> Linkage::loadModule(
    Name*               name,
    const PathInfo&     filePathInfo,
    ISlangBlob*         sourceBlob, 
    SourceLoc const&    srcLoc,
    DiagnosticSink*     sink)
{
    RefPtr<FrontEndCompileRequest> frontEndReq = new FrontEndCompileRequest(this, sink);

    RefPtr<TranslationUnitRequest> translationUnit = new TranslationUnitRequest(frontEndReq);
    translationUnit->compileRequest = frontEndReq;
    translationUnit->moduleName = name;

    auto module = translationUnit->getModule();

    ModuleBeingImportedRAII moduleBeingImported(
        this,
        module);

    // Create with the 'friendly' name
    SourceFile* sourceFile = getSourceManager()->createSourceFileWithBlob(filePathInfo, sourceBlob);
    
    translationUnit->addSourceFile(sourceFile);

    int errorCountBefore = sink->GetErrorCount();
    frontEndReq->parseTranslationUnit(translationUnit);
    int errorCountAfter = sink->GetErrorCount();

    if( errorCountAfter != errorCountBefore )
    {
        sink->diagnose(srcLoc, Diagnostics::errorInImportedModule);
    }
    if (errorCountAfter)
    {
        // Something went wrong during the parsing, so we should bail out.
        return nullptr;
    }

    loadParsedModule(
        translationUnit,
        name,
        filePathInfo);

    errorCountAfter = sink->GetErrorCount();

    if (errorCountAfter != errorCountBefore)
    {
        sink->diagnose(srcLoc, Diagnostics::errorInImportedModule);
        // Something went wrong during the parsing, so we should bail out.
        return nullptr;
    }

    return module;
}

bool Linkage::isBeingImported(Module* module)
{
    for(auto ii = m_modulesBeingImported; ii; ii = ii->next)
    {
        if(module == ii->module)
            return true;
    }
    return false;
}

RefPtr<Module> Linkage::findOrImportModule(
    Name*               name,
    SourceLoc const&    loc,
    DiagnosticSink*     sink)
{
    // Have we already loaded a module matching this name?
    //
    RefPtr<LoadedModule> loadedModule;
    if (mapNameToLoadedModules.TryGetValue(name, loadedModule))
    {
        // If the map shows a null module having been loaded,
        // then that means there was a prior load attempt,
        // but it failed, so we won't bother trying again.
        //
        if (!loadedModule)
            return nullptr;

        // If state shows us that the module is already being
        // imported deeper on the call stack, then we've
        // hit a recursive case, and that is an error.
        //
        if(isBeingImported(loadedModule))
        {
            // We seem to be in the middle of loading this module
            sink->diagnose(loc, Diagnostics::recursiveModuleImport, name);
            return nullptr;
        }

        return loadedModule;
    }

    // Derive a file name for the module, by taking the given
    // identifier, replacing all occurrences of `_` with `-`,
    // and then appending `.slang`.
    //
    // For example, `foo_bar` becomes `foo-bar.slang`.

    StringBuilder sb;
    for (auto c : getText(name))
    {
        if (c == '_')
            c = '-';

        sb.Append(c);
    }
    sb.Append(".slang");

    String fileName = sb.ProduceString();

    // Next, try to find the file of the given name,
    // using our ordinary include-handling logic.

    IncludeHandlerImpl includeHandler;
    includeHandler.linkage = this;
    includeHandler.searchDirectories = &searchDirectories;

    // Get the original path info
    PathInfo pathIncludedFromInfo = getSourceManager()->getPathInfo(loc, SourceLocType::Actual);
    PathInfo filePathInfo;

    // We have to load via the found path - as that is how file was originally loaded 
    if (SLANG_FAILED(includeHandler.findFile(fileName, pathIncludedFromInfo.foundPath, filePathInfo)))
    {
        sink->diagnose(loc, Diagnostics::cannotFindFile, fileName);
        mapNameToLoadedModules[name] = nullptr;
        return nullptr;
    }

    // Maybe this was loaded previously at a different relative name?
    if (mapPathToLoadedModule.TryGetValue(filePathInfo.getMostUniqueIdentity(), loadedModule))
        return loadedModule;

    // Try to load it
    ComPtr<ISlangBlob> fileContents;
    if(SLANG_FAILED(getFileSystemExt()->loadFile(filePathInfo.foundPath.Buffer(), fileContents.writeRef())))
    {
        sink->diagnose(loc, Diagnostics::cannotOpenFile, fileName);
        mapNameToLoadedModules[name] = nullptr;
        return nullptr;
    }

    // We've found a file that we can load for the given module, so
    // go ahead and perform the module-load action
    return loadModule(
        name,
        filePathInfo,
        fileContents,
        loc,
        sink);
}

//
// ModuleDependencyList
//

void ModuleDependencyList::addDependency(Module* module)
{
    // If we depend on a module, then we depend on everything it depends on.
    //
    // Note: We are processing these sub-depenencies before adding the
    // `module` itself, so that in the common case a module will always
    // appear *after* everything it depends on.
    //
    // However, this rule is being violated in the compiler right now because
    // the modules for hte top-level translation units of a compile request
    // will be added to the list first (using `addLeafDependency`) to
    // maintain compatibility with old behavior. This may be fixed later.
    //
    for(auto subDependency : module->getModuleDependencyList())
    {
        _addDependency(subDependency);
    }
    _addDependency(module);
}

void ModuleDependencyList::addLeafDependency(Module* module)
{
    _addDependency(module);
}

void ModuleDependencyList::_addDependency(Module* module)
{
    if(m_moduleSet.Contains(module))
        return;

    m_moduleList.Add(module);
    m_moduleSet.Add(module);
}

//
// FilePathDependencyList
//

void FilePathDependencyList::addDependency(String const& path)
{
    if(m_filePathSet.Contains(path))
        return;

    m_filePathList.Add(path);
    m_filePathSet.Add(path);
}

void FilePathDependencyList::addDependency(Module* module)
{
    for(auto& path : module->getFilePathDependencyList())
    {
        addDependency(path);
    }
}



//
// Module
//

Module::Module(Linkage* linkage)
    : m_linkage(linkage)
{}


void Module::addModuleDependency(Module* module)
{
    m_moduleDependencyList.addDependency(module);
    m_filePathDependencyList.addDependency(module);
}

void Module::addFilePathDependency(String const& path)
{
    m_filePathDependencyList.addDependency(path);
}

// Program

Program::Program(Linkage* linkage)
    : m_linkage(linkage)
{}

void Program::addReferencedModule(Module* module)
{
    m_moduleDependencyList.addDependency(module);
    m_filePathDependencyList.addDependency(module);
}

void Program::addReferencedLeafModule(Module* module)
{
    m_moduleDependencyList.addLeafDependency(module);
    m_filePathDependencyList.addDependency(module);
}

void Program::addEntryPoint(EntryPoint* entryPoint)
{
    m_entryPoints.Add(entryPoint);

    for(auto module : entryPoint->getModuleDependencies())
    {
        addReferencedModule(module);
    }
}

RefPtr<IRModule> Program::getOrCreateIRModule(DiagnosticSink* sink)
{
    if(!m_irModule)
    {
        m_irModule = generateIRForProgram(
            m_linkage->getSession(),
            this,
            sink);
    }
    return m_irModule;
}


TargetProgram* Program::getTargetProgram(TargetRequest* target)
{
    RefPtr<TargetProgram> targetProgram;
    if(!m_targetPrograms.TryGetValue(target, targetProgram))
    {
        targetProgram = new TargetProgram(this, target);
        m_targetPrograms[target] = targetProgram;
    }
    return targetProgram;
}

//
// TargetProgram
//

TargetProgram::TargetProgram(
    Program*        program,
    TargetRequest*  targetReq)
    : m_program(program)
    , m_targetReq(targetReq)
{
    m_entryPointResults.SetSize(program->getEntryPoints().Count());
}

//

void DiagnosticSink::noteInternalErrorLoc(SourceLoc const& loc)
{
    // Don't consider invalid source locations.
    if(!loc.isValid())
        return;

    // If this is the first source location being noted,
    // then emit a message to help the user isolate what
    // code might have confused the compiler.
    if(internalErrorLocsNoted == 0)
    {
        diagnose(loc, Diagnostics::noteLocationOfInternalError);
    }
    internalErrorLocsNoted++;
}

Session* CompileRequestBase::getSession()
{
    return getLinkage()->getSession();
}

static const Slang::Guid IID_ISlangFileSystemExt = SLANG_UUID_ISlangFileSystemExt;

void Linkage::setFileSystem(ISlangFileSystem* inFileSystem)
{
    // Set the fileSystem
    fileSystem = inFileSystem;

    // Set up fileSystemExt appropriately
    if (inFileSystem == nullptr)
    {
        fileSystemExt = new Slang::CacheFileSystem(Slang::OSFileSystem::getSingleton());
    }
    else
    {
        // See if we have the interface 
        inFileSystem->queryInterface(IID_ISlangFileSystemExt, (void**)fileSystemExt.writeRef());

        // If not wrap with WrapFileSytem that keeps the old behavior
        if (!fileSystemExt)
        {
            // Construct a wrapper to emulate the extended interface behavior
            fileSystemExt = new Slang::CacheFileSystem(fileSystem);
        }
    }

    // Set the file system used on the source manager
    getSourceManager()->setFileSystemExt(fileSystemExt);
}

RefPtr<Module> findOrImportModule(
    Linkage*            linkage,
    Name*               name,
    SourceLoc const&    loc,
    DiagnosticSink*     sink)
{
    return linkage->findOrImportModule(name, loc, sink);
}

void Session::addBuiltinSource(
    RefPtr<Scope> const&    scope,
    String const&           path,
    String const&           source)
{
    DiagnosticSink sink;
    RefPtr<FrontEndCompileRequest> compileRequest = new FrontEndCompileRequest(
        m_builtinLinkage,
        &sink);

    Name* moduleName = getNamePool()->getName(path);
    auto translationUnitIndex = compileRequest->addTranslationUnit(SourceLanguage::Slang, moduleName);

    compileRequest->addTranslationUnitSourceString(
        translationUnitIndex,
        path,
        source);

    SlangResult res = compileRequest->executeActionsInner();
    if (SLANG_FAILED(res))
    {
        char const* diagnostics = sink.outputBuffer.Buffer();
        fprintf(stderr, "%s", diagnostics);

#ifdef _WIN32
        OutputDebugStringA(diagnostics);
#endif

        SLANG_UNEXPECTED("error in Slang standard library");
    }

    // Extract the AST for the code we just parsed
    auto syntax = compileRequest->translationUnits[translationUnitIndex]->getModuleDecl();

    // HACK(tfoley): mark all declarations in the "stdlib" so
    // that we can detect them later (e.g., so we don't emit them)
    for (auto m : syntax->Members)
    {
        auto fromStdLibModifier = new FromStdLibModifier();

        fromStdLibModifier->next = m->modifiers.first;
        m->modifiers.first = fromStdLibModifier;
    }

    // Add the resulting code to the appropriate scope
    if (!scope->containerDecl)
    {
        // We are the first chunk of code to be loaded for this scope
        scope->containerDecl = syntax.Ptr();
    }
    else
    {
        // We need to create a new scope to link into the whole thing
        auto subScope = new Scope();
        subScope->containerDecl = syntax.Ptr();
        subScope->nextSibling = scope->nextSibling;
        scope->nextSibling = subScope;
    }

    // We need to retain this AST so that we can use it in other code
    // (Note that the `Scope` type does not retain the AST it points to)
    loadedModuleCode.Add(syntax);
}

Session::~Session()
{
    // free all built-in types first
    errorType = nullptr;
    initializerListType = nullptr;
    overloadedType = nullptr;
    irBasicBlockType = nullptr;
    constExprRate = nullptr;

    destroyTypeCheckingCache();

    builtinTypes = decltype(builtinTypes)();
    // destroy modules next
    loadedModuleCode = decltype(loadedModuleCode)();
}

}

// implementation of C interface

static SlangSession* convert(Slang::Session* session)
{ return reinterpret_cast<SlangSession*>(session); }

static Slang::Session* convert(SlangSession* session)
{ return reinterpret_cast<Slang::Session*>(session); }

static SlangCompileRequest* convert(Slang::EndToEndCompileRequest* request)
{ return reinterpret_cast<SlangCompileRequest*>(request); }

static Slang::EndToEndCompileRequest* convert(SlangCompileRequest* request)
{ return reinterpret_cast<Slang::EndToEndCompileRequest*>(request); }

static SlangLinkage* convert(Slang::Linkage* linkage)
{ return reinterpret_cast<SlangLinkage*>(linkage); }

static Slang::Linkage* convert(SlangLinkage* linkage)
{ return reinterpret_cast<Slang::Linkage*>(linkage); }

static SlangModule* convert(Slang::Module* module)
{ return reinterpret_cast<SlangModule*>(module); }

SLANG_API SlangSession* spCreateSession(const char*)
{
    return convert(new Slang::Session());
}

SLANG_API void spDestroySession(
    SlangSession*   session)
{
    if(!session) return;
    delete convert(session);
}

SLANG_API void spAddBuiltins(
    SlangSession*   session,
    char const*     sourcePath,
    char const*     sourceString)
{
    auto s = convert(session);
    s->addBuiltinSource(

        // TODO(tfoley): Add ability to directly new builtins to the approriate scope
        s->coreLanguageScope,

        sourcePath,
        sourceString);
}

SLANG_API void spSessionSetSharedLibraryLoader(
    SlangSession*               session,
    ISlangSharedLibraryLoader* loader)
{
    auto s = convert(session);

    if (!loader)
    {
        // If null set the default
        loader = Slang::DefaultSharedLibraryLoader::getSingleton();
    }

    if (s->sharedLibraryLoader != loader)
    {
        // Need to clear all of the libraries
        for (int i = 0; i < SLANG_COUNT_OF(s->sharedLibraries); ++i)
        {
            s->sharedLibraries[i].setNull();
        }

        // Clear all of the functions
        ::memset(s->sharedLibraryFunctions, 0, sizeof(s->sharedLibraryFunctions));

        // Set the loader
        s->sharedLibraryLoader = loader;
    }
}

SLANG_API ISlangSharedLibraryLoader* spSessionGetSharedLibraryLoader(
    SlangSession*               session)
{
    auto s = convert(session);
    return (s->sharedLibraryLoader == Slang::DefaultSharedLibraryLoader::getSingleton()) ? nullptr : s->sharedLibraryLoader.get();
}

SLANG_API SlangResult spSessionCheckCompileTargetSupport(
    SlangSession*                session,
    SlangCompileTarget           target)
{
    auto s = convert(session);
    return Slang::checkCompileTargetSupport(s, Slang::CodeGenTarget(target));
}

SLANG_API SlangResult spSessionCheckPassThroughSupport(
    SlangSession*       session,
    SlangPassThrough    passThrough)
{
    auto s = convert(session);
    return Slang::checkExternalCompilerSupport(s, Slang::PassThroughMode(passThrough));
}


SLANG_API SlangLinkage* spCreateLinkage(
    SlangSession* session)
{
    auto s = convert(session);
    auto linkage = new Slang::Linkage(s);
    return convert(linkage);
}

SLANG_API void spDestroyLinkage(
    SlangLinkage* linkage)
{
    if(!linkage) return;
    auto lnk = convert(linkage);
    delete lnk;
}

SLANG_API SlangModule* spLoadModule(
    SlangLinkage* linkage,
    char const* moduleName)
{
    if(!linkage) return nullptr;
    auto lnk = convert(linkage);

    auto mod = lnk->loadModule(moduleName);
    return convert(mod);
}


SLANG_API SlangCompileRequest* spCreateCompileRequest(
    SlangSession* session)
{
    auto s = convert(session);
    auto req = new Slang::EndToEndCompileRequest(s);
    return convert(req);
}

/*!
@brief Destroy a compile request.
*/
SLANG_API void spDestroyCompileRequest(
    SlangCompileRequest*    request)
{
    if(!request) return;
    auto req = convert(request);
    delete req;
}

SLANG_API void spSetFileSystem(
    SlangCompileRequest*    request,
    ISlangFileSystem*       fileSystem)
{
    if(!request) return;
    convert(request)->getLinkage()->setFileSystem(fileSystem);
}

SLANG_API void spSetCompileFlags(
    SlangCompileRequest*    request,
    SlangCompileFlags       flags)
{
    convert(request)->getFrontEndReq()->compileFlags = flags;
}

SLANG_API void spSetDumpIntermediates(
    SlangCompileRequest*    request,
    int                     enable)
{
    convert(request)->getBackEndReq()->shouldDumpIntermediates = enable != 0;
}

SLANG_API void spSetLineDirectiveMode(
    SlangCompileRequest*    request,
    SlangLineDirectiveMode  mode)
{
    // TODO: validation

    convert(request)->getBackEndReq()->lineDirectiveMode = Slang::LineDirectiveMode(mode);
}

SLANG_API void spSetCommandLineCompilerMode(
    SlangCompileRequest* request)
{
    convert(request)->isCommandLineCompile = true;

}

SLANG_API void spSetCodeGenTarget(
        SlangCompileRequest*    request,
        SlangCompileTarget target)
{
    auto req = convert(request);
    auto linkage = req->getLinkage();
    linkage->targets.Clear();
    linkage->addTarget(Slang::CodeGenTarget(target));
}

SLANG_API int spAddCodeGenTarget(
    SlangCompileRequest*    request,
    SlangCompileTarget      target)
{
    auto req = convert(request);
    auto linkage = req->getLinkage();
    return (int) linkage->addTarget(Slang::CodeGenTarget(target));
}

SLANG_API void spSetTargetProfile(
    SlangCompileRequest*    request,
    int                     targetIndex,
    SlangProfileID          profile)
{
    auto req = convert(request);
    auto linkage = req->getLinkage();
    linkage->targets[targetIndex]->targetProfile = Slang::Profile(profile);
}

SLANG_API void spSetTargetFlags(
    SlangCompileRequest*    request,
    int                     targetIndex,
    SlangTargetFlags        flags)
{
    auto req = convert(request);
    auto linkage = req->getLinkage();
    linkage->targets[targetIndex]->targetFlags = flags;
}

SLANG_API void spSetTargetFloatingPointMode(
    SlangCompileRequest*    request,
    int                     targetIndex,
    SlangFloatingPointMode  mode)
{
    auto req = convert(request);
    auto linkage = req->getLinkage();
    linkage->targets[targetIndex]->floatingPointMode = Slang::FloatingPointMode(mode);
}

SLANG_API void spSetMatrixLayoutMode(
    SlangCompileRequest*    request,
    SlangMatrixLayoutMode   mode)
{
    auto req = convert(request);
    auto linkage = req->getLinkage();
    linkage->defaultMatrixLayoutMode = Slang::MatrixLayoutMode(mode);
}

SLANG_API void spSetTargetMatrixLayoutMode(
    SlangCompileRequest*    request,
    int                     targetIndex,
    SlangMatrixLayoutMode   mode)
{
    SLANG_UNUSED(targetIndex);
    spSetMatrixLayoutMode(request, mode);
}


SLANG_API void spSetOutputContainerFormat(
    SlangCompileRequest*    request,
    SlangContainerFormat    format)
{
    auto req = convert(request);
    req->containerFormat = Slang::ContainerFormat(format);
}


SLANG_API void spSetPassThrough(
    SlangCompileRequest*    request,
    SlangPassThrough        passThrough)
{
    convert(request)->passThrough = Slang::PassThroughMode(passThrough);
}

SLANG_API void spSetDiagnosticCallback(
    SlangCompileRequest*    request,
    SlangDiagnosticCallback callback,
    void const*             userData)
{
    using namespace Slang;

    if(!request) return;
    auto req = convert(request);

    ComPtr<ISlangWriter> writer(new CallbackWriter(callback, userData, WriterFlag::IsConsole));
    req->setWriter(WriterChannel::Diagnostic, writer);
}

SLANG_API void spSetWriter(
    SlangCompileRequest*    request,
    SlangWriterChannel      chan, 
    ISlangWriter*           writer)
{
    if (!request) return;
    auto req = convert(request);

    req->setWriter(Slang::WriterChannel(chan), writer);
}

SLANG_API ISlangWriter* spGetWriter(
    SlangCompileRequest*    request,
    SlangWriterChannel      chan)
{
    if (!request) return nullptr;
    auto req = convert(request);
    return req->getWriter(Slang::WriterChannel(chan));
}

SLANG_API void spAddSearchPath(
    SlangCompileRequest*    request,
    const char*             path)
{
    auto req = convert(request);
    auto linkage = req->getLinkage();
    linkage->searchDirectories.searchDirectories.Add(Slang::SearchDirectory(path));
}

SLANG_API void spAddPreprocessorDefine(
    SlangCompileRequest*    request,
    const char*             key,
    const char*             value)
{
    auto req = convert(request);
    auto linkage = req->getLinkage();
    linkage->preprocessorDefinitions[key] = value;
}

SLANG_API char const* spGetDiagnosticOutput(
    SlangCompileRequest*    request)
{
    if(!request) return 0;
    auto req = convert(request);
    return req->mDiagnosticOutput.begin();
}

SLANG_API SlangResult spGetDiagnosticOutputBlob(
    SlangCompileRequest*    request,
    ISlangBlob**            outBlob)
{
    if(!request) return SLANG_ERROR_INVALID_PARAMETER;
    if(!outBlob) return SLANG_ERROR_INVALID_PARAMETER;

    auto req = convert(request);

    if(!req->diagnosticOutputBlob)
    {
        req->diagnosticOutputBlob = Slang::StringUtil::createStringBlob(req->mDiagnosticOutput);
    }

    Slang::ComPtr<ISlangBlob> resultBlob = req->diagnosticOutputBlob;
    *outBlob = resultBlob.detach();
    return SLANG_OK;
}

// New-fangled compilation API

SLANG_API int spAddTranslationUnit(
    SlangCompileRequest*    request,
    SlangSourceLanguage     language,
    char const*             name)
{
    SLANG_UNUSED(name);

    auto req = convert(request);
    auto frontEndReq = req->getFrontEndReq();

    return frontEndReq->addTranslationUnit(
        Slang::SourceLanguage(language));
}

SLANG_API void spTranslationUnit_addPreprocessorDefine(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    const char*             key,
    const char*             value)
{
    auto req = convert(request);
    auto frontEndReq = req->getFrontEndReq();

    frontEndReq->translationUnits[translationUnitIndex]->preprocessorDefinitions[key] = value;
}

SLANG_API void spAddTranslationUnitSourceFile(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             path)
{
    if(!request) return;
    auto req = convert(request);
    auto frontEndReq = req->getFrontEndReq();
    if(!path) return;
    if(translationUnitIndex < 0) return;
    if(Slang::UInt(translationUnitIndex) >= frontEndReq->translationUnits.Count()) return;

    frontEndReq->addTranslationUnitSourceFile(
        translationUnitIndex,
        path);
}

SLANG_API void spAddTranslationUnitSourceString(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             path,
    char const*             source)
{
    if(!source) return;
    spAddTranslationUnitSourceStringSpan(
        request,
        translationUnitIndex,
        path,
        source,
        source + strlen(source));
}

SLANG_API void spAddTranslationUnitSourceStringSpan(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             path,
    char const*             sourceBegin,
    char const*             sourceEnd)
{
    if(!request) return;
    auto req = convert(request);
    auto frontEndReq = req->getFrontEndReq();
    if(!sourceBegin) return;
    if(translationUnitIndex < 0) return;
    if(Slang::UInt(translationUnitIndex) >= frontEndReq->translationUnits.Count()) return;

    if(!path) path = "";

    frontEndReq->addTranslationUnitSourceString(
        translationUnitIndex,
        path,
        Slang::UnownedStringSlice(sourceBegin, sourceEnd));
}

SLANG_API void spAddTranslationUnitSourceBlob(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             path,
    ISlangBlob*             sourceBlob)
{
    if(!request) return;
    auto req = convert(request);
    auto frontEndReq = req->getFrontEndReq();
    if(!sourceBlob) return;
    if(translationUnitIndex < 0) return;
    if(Slang::UInt(translationUnitIndex) >= frontEndReq->translationUnits.Count()) return;

    if(!path) path = "";

    frontEndReq->addTranslationUnitSourceBlob(
        translationUnitIndex,
        path,
        sourceBlob);
}






SLANG_API SlangProfileID spFindProfile(
    SlangSession*,
    char const*     name)
{
    return Slang::Profile::LookUp(name).raw;
}

SLANG_API int spAddEntryPoint(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             name,
    SlangStage              stage)
{
    return spAddEntryPointEx(
        request,
        translationUnitIndex,
        name,
        stage,
        0,
        nullptr);
}

SLANG_API int spAddEntryPointEx(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             name,
    SlangStage              stage,
    int                     genericParamTypeNameCount,
    char const **           genericParamTypeNames)
{
    if (!request) return -1;
    auto req = convert(request);
    auto frontEndReq = req->getFrontEndReq();
    if (!name) return -1;
    if (translationUnitIndex < 0) return -1;
    if (Slang::UInt(translationUnitIndex) >= frontEndReq->translationUnits.Count()) return -1;
    Slang::List<Slang::String> typeNames;
    for (int i = 0; i < genericParamTypeNameCount; i++)
        typeNames.Add(genericParamTypeNames[i]);
    return req->addEntryPoint(
        translationUnitIndex,
        name,
        Slang::Profile(Slang::Stage(stage)),
        typeNames);
}

SLANG_API SlangResult spSetGlobalGenericArgs(
    SlangCompileRequest*    request,
    int                     genericArgCount,
    char const**            genericArgs)
{
    if (!request) return SLANG_FAIL;
    auto req = convert(request);

    auto& genericArgStrings = req->globalGenericArgStrings;
    genericArgStrings.Clear();
    for (int i = 0; i < genericArgCount; i++)
        genericArgStrings.Add(genericArgs[i]);

    return SLANG_OK;
}


// Compile in a context that already has its translation units specified
SLANG_API SlangResult spCompile(
    SlangCompileRequest*    request)
{
    auto req = convert(request);

#if !defined(SLANG_DEBUG_INTERNAL_ERROR)
    // By default we'd like to catch as many internal errors as possible,
    // and report them to the user nicely (rather than just crash their
    // application). Internally Slang currently uses exceptions for this.
    //
    // TODO: Consider using `setjmp()`-style escape so that we can work
    // with applications that disable exceptions.
    //
    // TODO: Consider supporting Windows "Structured Exception Handling"
    // so that we can also recover from a wider class of crashes.
    SlangResult res = SLANG_FAIL; 
    try
    {
        res = req->executeActions();
    }
    catch (Slang::AbortCompilationException&)
    {
        // This situation indicates a fatal (but not necessarily internal) error
        // that forced compilation to terminate. There should already have been
        // a diagnostic produced, so we don't need to add one here.
    }
    catch (Slang::Exception& e)
    {
        // The compiler failed due to an internal error that was detected.
        // We will print out information on the exception to help out the user
        // in either filing a bug, or locating what in their code created
        // a problem.
        req->getSink()->diagnose(Slang::SourceLoc(), Slang::Diagnostics::compilationAbortedDueToException, typeid(e).name(), e.Message);
    }
    catch (...)
    {
        // The compiler failed due to some exception that wasn't a sublass of
        // `Exception`, so something really fishy is going on. We want to
        // let the user know that we messed up, so they know to blame Slang
        // and not some other component in their system.
        req->getSink()->diagnose(Slang::SourceLoc(), Slang::Diagnostics::compilationAborted);
    }
    req->mDiagnosticOutput = req->getSink()->outputBuffer.ProduceString();
    return res;
#else
    // When debugging, we probably don't want to filter out any errors, since
    // we are probably trying to root-cause and *fix* those errors.
    {
        return req->executeActions();
    }
#endif
}

SLANG_API int
spGetDependencyFileCount(
    SlangCompileRequest*    request)
{
    if(!request) return 0;
    auto req = convert(request);
    auto frontEndReq = req->getFrontEndReq();
    auto program = frontEndReq->getProgram();
    return (int) program->getFilePathDependencies().Count();
}

/** Get the path to a file this compilation dependend on.
*/
SLANG_API char const*
spGetDependencyFilePath(
    SlangCompileRequest*    request,
    int                     index)
{
    if(!request) return 0;
    auto req = convert(request);
    auto frontEndReq = req->getFrontEndReq();
    auto program = frontEndReq->getProgram();
    return program->getFilePathDependencies()[index].begin();
}

SLANG_API int
spGetTranslationUnitCount(
    SlangCompileRequest*    request)
{
    auto req = convert(request);
    auto frontEndReq = req->getFrontEndReq();
    return (int) frontEndReq->translationUnits.Count();
}

// Get the output code associated with a specific translation unit
SLANG_API char const* spGetTranslationUnitSource(
    SlangCompileRequest*    /*request*/,
    int                     /*translationUnitIndex*/)
{
    fprintf(stderr, "DEPRECATED: spGetTranslationUnitSource()\n");
    return nullptr;
}

SLANG_API void const* spGetEntryPointCode(
    SlangCompileRequest*    request,
    int                     entryPointIndex,
    size_t*                 outSize)
{
    auto req = convert(request);
    auto linkage = req->getLinkage();
    auto program = req->getSpecializedProgram();

    // TODO: We should really accept a target index in this API
    Slang::UInt targetIndex = 0;
    auto targetCount = linkage->targets.Count();
    if (targetIndex >= targetCount)
        return nullptr;
    auto targetReq = linkage->targets[targetIndex];


    if(entryPointIndex < 0) return nullptr;
    if(Slang::UInt(entryPointIndex) >= req->entryPoints.Count()) return nullptr;
    auto entryPoint = program->getEntryPoint(entryPointIndex);

    auto targetProgram = program->getTargetProgram(targetReq);
    if(!targetProgram)
        return nullptr;
    Slang::CompileResult& result = targetProgram->getExistingEntryPointResult(entryPointIndex);

    void const* data = nullptr;
    size_t size = 0;

    switch (result.format)
    {
    case Slang::ResultFormat::None:
    default:
        break;

    case Slang::ResultFormat::Binary:
        data = result.outputBinary.Buffer();
        size = result.outputBinary.Count();
        break;

    case Slang::ResultFormat::Text:
        data = result.outputString.Buffer();
        size = result.outputString.Length();
        break;
    }

    if(outSize) *outSize = size;
    return data;
}

SLANG_API SlangResult spGetEntryPointCodeBlob(
        SlangCompileRequest*    request,
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangBlob**            outBlob)
{
    if(!request) return SLANG_ERROR_INVALID_PARAMETER;
    if(!outBlob) return SLANG_ERROR_INVALID_PARAMETER;

    auto req = convert(request);
    auto linkage = req->getLinkage();
    auto program = req->getSpecializedProgram();

    int targetCount = (int) linkage->targets.Count();
    if((targetIndex < 0) || (targetIndex >= targetCount))
    {
        return SLANG_ERROR_INVALID_PARAMETER;
    }
    auto targetReq = linkage->targets[targetIndex];

    int entryPointCount = (int) req->entryPoints.Count();
    if((entryPointIndex < 0) || (entryPointIndex >= entryPointCount))
    {
        return SLANG_ERROR_INVALID_PARAMETER;
    }
    auto entryPointReq = program->getEntryPoint(entryPointIndex);


    auto targetProgram = program->getTargetProgram(targetReq);
    if(!targetProgram)
        return SLANG_FAIL;
    Slang::CompileResult& result = targetProgram->getExistingEntryPointResult(entryPointIndex);

    auto blob = result.getBlob();
    *outBlob = blob.detach();
    return SLANG_OK;
}

SLANG_API char const* spGetEntryPointSource(
    SlangCompileRequest*    request,
    int                     entryPointIndex)
{
    return (char const*) spGetEntryPointCode(request, entryPointIndex, nullptr);
}

SLANG_API void const* spGetCompileRequestCode(
    SlangCompileRequest*    request,
    size_t*                 outSize)
{
    SLANG_UNUSED(request);
    SLANG_UNUSED(outSize);
    return nullptr;
}

// Reflection API

SLANG_API SlangReflection* spGetReflection(
    SlangCompileRequest*    request)
{
    if( !request ) return 0;
    auto req = convert(request);
    auto linkage = req->getLinkage();
    auto program = req->getSpecializedProgram();

    // Note(tfoley): The API signature doesn't let the client
    // specify which target they want to access reflection
    // information for, so for now we default to the first one.
    //
    // TODO: Add a new `spGetReflectionForTarget(req, targetIndex)`
    // so that we can do this better, and make it clear that
    // `spGetReflection()` is shorthand for `targetIndex == 0`.
    //
    Slang::UInt targetIndex = 0;
    auto targetCount = linkage->targets.Count();
    if (targetIndex >= targetCount)
        return nullptr;

    auto targetReq = linkage->targets[targetIndex];
    auto targetProgram = program->getTargetProgram(targetReq);
    auto programLayout = targetProgram->getExistingLayout();

    return (SlangReflection*) programLayout;
}

// ... rest of reflection API implementation is in `Reflection.cpp`
