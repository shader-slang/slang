#include "../../slang.h"
#include "../../slang-tag-version.h"

#include "../core/slang-io.h"
#include "../core/slang-string-util.h"
#include "../core/slang-shared-library.h"

#include "slang-check.h"
#include "slang-parameter-binding.h"
#include "slang-lower-to-ir.h"
#include "slang-mangle.h"
#include "slang-parser.h"
#include "slang-preprocessor.h"
#include "slang-reflection.h"
#include "slang-type-layout.h"

#include "slang-ast-dump.h"
#include "slang-ast-serialize.h"

#include "slang-repro.h"

#include "slang-file-system.h"

#include "../core/slang-writer.h"

#include "slang-source-loc.h"

#include "slang-ir-serialize.h"

#include "slang-check-impl.h"

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

/* static */const BaseTypeInfo BaseTypeInfo::s_info[Index(BaseType::CountOf)] = 
{
    { 0, 0, uint8_t(BaseType::Void) },
    { uint8_t(sizeof(bool)),   0, uint8_t(BaseType::Bool) },
    { uint8_t(sizeof(int8_t)),   BaseTypeInfo::Flag::Signed | BaseTypeInfo::Flag::Integer , uint8_t(BaseType::Int8) },
    { uint8_t(sizeof(int16_t)),  BaseTypeInfo::Flag::Signed | BaseTypeInfo::Flag::Integer , uint8_t(BaseType::Int16) },
    { uint8_t(sizeof(int32_t)),  BaseTypeInfo::Flag::Signed | BaseTypeInfo::Flag::Integer , uint8_t(BaseType::Int) },
    { uint8_t(sizeof(int64_t)),  BaseTypeInfo::Flag::Signed | BaseTypeInfo::Flag::Integer , uint8_t(BaseType::Int64) },
    { uint8_t(sizeof(uint8_t)),                               BaseTypeInfo::Flag::Integer , uint8_t(BaseType::UInt8) },
    { uint8_t(sizeof(uint16_t)),                              BaseTypeInfo::Flag::Integer , uint8_t(BaseType::UInt16) },
    { uint8_t(sizeof(uint32_t)),                              BaseTypeInfo::Flag::Integer , uint8_t(BaseType::UInt) },
    { uint8_t(sizeof(uint64_t)),                              BaseTypeInfo::Flag::Integer, uint8_t(BaseType::UInt64) },
    { uint8_t(sizeof(uint16_t)), BaseTypeInfo::Flag::FloatingPoint , uint8_t(BaseType::Half) },
    { uint8_t(sizeof(float)),    BaseTypeInfo::Flag::FloatingPoint , uint8_t(BaseType::Float) },
    { uint8_t(sizeof(double)),   BaseTypeInfo::Flag::FloatingPoint , uint8_t(BaseType::Double) },
};

/* static */bool BaseTypeInfo::check()
{
    for (Index i = 0; i < SLANG_COUNT_OF(s_info); ++i)
    {
        if (s_info[i].baseType != i)
        {
            SLANG_ASSERT(!"Inconsistency between the s_info table and BaseInfo");
            return false;
        }
    }
    return true;
}

/* static */UnownedStringSlice BaseTypeInfo::asText(BaseType baseType)
{
    switch (baseType)
    {
        case BaseType::Void:            return UnownedStringSlice::fromLiteral("void");
        case BaseType::Bool:            return UnownedStringSlice::fromLiteral("bool");
        case BaseType::Int8:            return UnownedStringSlice::fromLiteral("int8_t");
        case BaseType::Int16:           return UnownedStringSlice::fromLiteral("int16_t");
        case BaseType::Int:             return UnownedStringSlice::fromLiteral("int");
        case BaseType::Int64:           return UnownedStringSlice::fromLiteral("int64_t");
        case BaseType::UInt8:           return UnownedStringSlice::fromLiteral("uint8_t");
        case BaseType::UInt16:          return UnownedStringSlice::fromLiteral("uint16_t");
        case BaseType::UInt:            return UnownedStringSlice::fromLiteral("uint");
        case BaseType::UInt64:          return UnownedStringSlice::fromLiteral("uint64_t");
        case BaseType::Half:            return UnownedStringSlice::fromLiteral("half");
        case BaseType::Float:           return UnownedStringSlice::fromLiteral("float");
        case BaseType::Double:          return UnownedStringSlice::fromLiteral("double");
        default:
        {
            SLANG_ASSERT(!"Unknown basic type");
            return UnownedStringSlice();
        }
    }
}

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_IComponentType    = SLANG_UUID_IComponentType;
static const Guid IID_IEntryPoint       = SLANG_UUID_IEntryPoint;
static const Guid IID_IGlobalSession    = SLANG_UUID_IGlobalSession;
static const Guid IID_IModule           = SLANG_UUID_IModule;
static const Guid IID_ISession          = SLANG_UUID_ISession;
static const Guid IID_ISlangBlob        = SLANG_UUID_ISlangBlob;
static const Guid IID_ISlangUnknown     = SLANG_UUID_ISlangUnknown;

void Session::init()
{
    SLANG_ASSERT(BaseTypeInfo::check());

    ::memset(m_downstreamCompilerLocators, 0, sizeof(m_downstreamCompilerLocators));
    DownstreamCompilerUtil::setDefaultLocators(m_downstreamCompilerLocators);
    m_downstreamCompilerSet = new DownstreamCompilerSet;

    // Initialize name pool
    getNamePool()->setRootNamePool(getRootNamePool());

    m_sharedLibraryLoader = DefaultSharedLibraryLoader::getSingleton();
    // Set all the shared library function pointers to nullptr
    ::memset(m_sharedLibraryFunctions, 0, sizeof(m_sharedLibraryFunctions));

    // Set up shared AST builder
    m_sharedASTBuilder = new SharedASTBuilder;
    m_sharedASTBuilder->init(this);

    //  Use to create a ASTBuilder
    RefPtr<ASTBuilder> builtinAstBuilder(new ASTBuilder(m_sharedASTBuilder, "m_builtInLinkage::m_astBuilder"));

    // And the global ASTBuilder
    globalAstBuilder = new ASTBuilder(m_sharedASTBuilder, "globalAstBuilder");


    // Make sure our source manager is initialized
    builtinSourceManager.initialize(nullptr, nullptr);

    // Built in linkage uses the built in builder
    m_builtinLinkage = new Linkage(this, builtinAstBuilder);

    
    // Because the `Session` retains the builtin `Linkage`,
    // we need to make sure that the parent pointer inside
    // `Linkage` doesn't create a retain cycle.
    //
    // This operation ensures that the parent pointer will
    // just be a raw pointer, so that the builtin linkage
    // doesn't keep the parent session alive.
    //
    m_builtinLinkage->_stopRetainingParentSession();

  
    // Create scopes for various language builtins.
    //
    // TODO: load these on-demand to avoid parsing
    // stdlib code for languages the user won't use.

    baseLanguageScope = new Scope();

    // Will stay in scope as long as ASTBuilder
    baseModuleDecl = populateBaseLanguageModule(
        m_builtinLinkage->getASTBuilder(),
        baseLanguageScope);

    coreLanguageScope = new Scope();
    coreLanguageScope->nextSibling = baseLanguageScope;

    hlslLanguageScope = new Scope();
    hlslLanguageScope->nextSibling = coreLanguageScope;

    slangLanguageScope = new Scope();
    slangLanguageScope->nextSibling = hlslLanguageScope;

    addBuiltinSource(coreLanguageScope, "core", getCoreLibraryCode());
    addBuiltinSource(hlslLanguageScope, "hlsl", getHLSLLibraryCode());

    {
        for (Index i = 0; i < Index(SourceLanguage::CountOf); ++i)
        {
            m_defaultDownstreamCompilers[i] = PassThroughMode::None;
        }
        m_defaultDownstreamCompilers[Index(SourceLanguage::C)] = PassThroughMode::GenericCCpp;
        m_defaultDownstreamCompilers[Index(SourceLanguage::CPP)] = PassThroughMode::GenericCCpp;
        m_defaultDownstreamCompilers[Index(SourceLanguage::CUDA)] = PassThroughMode::NVRTC;
    }
}

ISlangUnknown* Session::getInterface(const Guid& guid)
{
    if(guid == IID_ISlangUnknown || guid == IID_IGlobalSession)
        return asExternal(this);
    return nullptr;
}

SLANG_NO_THROW SlangResult SLANG_MCALL Session::createSession(
    slang::SessionDesc const&  desc,
    slang::ISession**          outSession)
{
    RefPtr<ASTBuilder> astBuilder(new ASTBuilder(m_sharedASTBuilder, "Session::astBuilder"));
    RefPtr<Linkage> linkage = new Linkage(this, astBuilder);

    Int targetCount = desc.targetCount;
    for(Int ii = 0; ii < targetCount; ++ii)
    {
        linkage->addTarget(desc.targets[ii]);
    }

    if(desc.flags & slang::kSessionFlag_FalcorCustomSharedKeywordSemantics)
    {
        linkage->m_useFalcorCustomSharedKeywordSemantics = true;
    }

    linkage->setMatrixLayoutMode(desc.defaultMatrixLayoutMode);

    Int searchPathCount = desc.searchPathCount;
    for(Int ii = 0; ii < searchPathCount; ++ii)
    {
        linkage->addSearchPath(desc.searchPaths[ii]);
    }

    Int macroCount = desc.preprocessorMacroCount;
    for(Int ii = 0; ii < macroCount; ++ii)
    {
        auto& macro = desc.preprocessorMacros[ii];
        linkage->addPreprocessorDefine(macro.name, macro.value);
    }

    *outSession = asExternal(linkage.detach());
    return SLANG_OK;
}

SLANG_NO_THROW SlangProfileID SLANG_MCALL Session::findProfile(
    char const*     name)
{
    return Slang::Profile::lookUp(name).raw;
}

SLANG_NO_THROW void SLANG_MCALL Session::setDownstreamCompilerPath(
    SlangPassThrough inPassThrough,
    char const* path)
{
    PassThroughMode passThrough = PassThroughMode(inPassThrough);
    SLANG_ASSERT(int(passThrough) > int(PassThroughMode::None) && int(passThrough) < int(PassThroughMode::CountOf));
    
    if (m_downstreamCompilerPaths[int(passThrough)] != path)
    {
        // Make access redetermine compiler
        resetDownstreamCompiler(passThrough);
        // Set the path
        m_downstreamCompilerPaths[int(passThrough)] = path;
    }
}

SLANG_NO_THROW void SLANG_MCALL Session::setDownstreamCompilerPrelude(
    SlangPassThrough inPassThrough,
    char const* prelude)
{
    PassThroughMode downstreamCompiler = PassThroughMode(inPassThrough);
    SLANG_ASSERT(int(downstreamCompiler) > int(PassThroughMode::None) && int(downstreamCompiler) < int(PassThroughMode::CountOf));
    const SourceLanguage sourceLanguage = getDefaultSourceLanguageForDownstreamCompiler(downstreamCompiler);
    setLanguagePrelude(SlangSourceLanguage(sourceLanguage), prelude);
}

SLANG_NO_THROW void SLANG_MCALL Session::getDownstreamCompilerPrelude(
    SlangPassThrough inPassThrough,
    ISlangBlob** outPrelude)
{
    PassThroughMode downstreamCompiler = PassThroughMode(inPassThrough);
    SLANG_ASSERT(int(downstreamCompiler) > int(PassThroughMode::None) && int(downstreamCompiler) < int(PassThroughMode::CountOf));
    const SourceLanguage sourceLanguage = getDefaultSourceLanguageForDownstreamCompiler(downstreamCompiler);
    getLanguagePrelude(SlangSourceLanguage(sourceLanguage), outPrelude);
}

SLANG_NO_THROW void SLANG_MCALL Session::setLanguagePrelude(
    SlangSourceLanguage inSourceLanguage,
    char const* prelude)
{
    SourceLanguage sourceLanguage = SourceLanguage(inSourceLanguage);
    SLANG_ASSERT(int(sourceLanguage) > int(SourceLanguage::Unknown) && int(sourceLanguage) < int(SourceLanguage::CountOf));

    SLANG_ASSERT(sourceLanguage != SourceLanguage::Unknown);

    if (sourceLanguage != SourceLanguage::Unknown)
    {
        m_languagePreludes[int(sourceLanguage)] = prelude;
    }
}

SLANG_NO_THROW void SLANG_MCALL Session::getLanguagePrelude(
    SlangSourceLanguage inSourceLanguage,
    ISlangBlob** outPrelude)
{
    SourceLanguage sourceLanguage = SourceLanguage(inSourceLanguage);
    SLANG_ASSERT(int(sourceLanguage) > int(SourceLanguage::Unknown) && int(sourceLanguage) < int(SourceLanguage::CountOf));

    SLANG_ASSERT(sourceLanguage != SourceLanguage::Unknown);

    *outPrelude = nullptr;
    if (sourceLanguage != SourceLanguage::Unknown)
    {
        *outPrelude = Slang::StringUtil::createStringBlob(m_languagePreludes[int(sourceLanguage)]).detach();
    }
}

SLANG_NO_THROW const char* SLANG_MCALL Session::getBuildTagString()
{
    return spGetBuildTagString();
}

SLANG_NO_THROW SlangResult SLANG_MCALL Session::setDefaultDownstreamCompiler(SlangSourceLanguage sourceLanguage, SlangPassThrough defaultCompiler)
{
    if (DownstreamCompiler::canCompile(defaultCompiler, sourceLanguage))
    {
        m_defaultDownstreamCompilers[int(sourceLanguage)] = PassThroughMode(defaultCompiler);
        return SLANG_OK;
    }
    return SLANG_FAIL;
}

SlangPassThrough SLANG_MCALL Session::getDefaultDownstreamCompiler(SlangSourceLanguage inSourceLanguage)
{
    SLANG_ASSERT(inSourceLanguage >= 0 && inSourceLanguage < SLANG_SOURCE_LANGUAGE_COUNT_OF);
    auto sourceLanguage = SourceLanguage(inSourceLanguage);
    return SlangPassThrough(m_defaultDownstreamCompilers[int(sourceLanguage)]);
}

DownstreamCompiler* Session::getDefaultDownstreamCompiler(SourceLanguage sourceLanguage)
{
    return getOrLoadDownstreamCompiler(m_defaultDownstreamCompilers[int(sourceLanguage)], nullptr);
}

ISlangFileSystemExt* IncludeHandlerImpl::_getFileSystemExt()
{
    return linkage->getFileSystemExt();
}

SlangResult IncludeHandlerImpl::_findFile(SlangPathType fromPathType, const String& fromPath, const String& path, PathInfo& pathInfoOut)
{
    ISlangFileSystemExt* fileSystemExt = _getFileSystemExt();

    // Get relative path
    ComPtr<ISlangBlob> combinedPathBlob;
    SLANG_RETURN_ON_FAIL(fileSystemExt->calcCombinedPath(fromPathType, fromPath.begin(), path.begin(), combinedPathBlob.writeRef()));
    String combinedPath(StringUtil::getString(combinedPathBlob));
    if (combinedPath.getLength() <= 0)
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
    if (uniqueIdentity.getLength() <= 0)
    {   
        // Unique identity can't be empty
        return SLANG_FAIL;
    }
        
    pathInfoOut.type = PathInfo::Type::Normal;
    pathInfoOut.foundPath = combinedPath;
    pathInfoOut.uniqueIdentity = uniqueIdentity;
    return SLANG_OK;     
}

SlangResult IncludeHandlerImpl::findFile(
    String const& pathToInclude,
    String const& pathIncludedFrom,
    PathInfo& pathInfoOut)
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

String IncludeHandlerImpl::simplifyPath(const String& path)
{
    ISlangFileSystemExt* fileSystemExt = _getFileSystemExt();
    ComPtr<ISlangBlob> simplifiedPath;
    if (SLANG_FAILED(fileSystemExt->getSimplifiedPath(path.getBuffer(), simplifiedPath.writeRef())))
    {
        return path;
    }
    return StringUtil::getString(simplifiedPath);
}

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

    auto entryPointProfileVersion = entryPointProfile.getVersion();
    auto targetProfileVersion = targetProfile.getVersion();

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
        switch(effectiveProfile.getStage())
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
        switch(effectiveProfile.getStage())
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

    if( stageMinVersion > effectiveProfile.getVersion() )
    {
        effectiveProfile.setVersion(stageMinVersion);
    }

    return effectiveProfile;
}


//

Linkage::Linkage(Session* session, ASTBuilder* astBuilder)
    : m_session(session)
    , m_retainedSession(session)
    , m_sourceManager(&m_defaultSourceManager)
    , m_astBuilder(astBuilder)
{
    getNamePool()->setRootNamePool(session->getRootNamePool());

    m_defaultSourceManager.initialize(session->getBuiltinSourceManager(), nullptr);

    setFileSystem(nullptr);
}

ISlangUnknown* Linkage::getInterface(const Guid& guid)
{
    if(guid == IID_ISlangUnknown || guid == IID_ISession)
        return asExternal(this);

    return nullptr;
}

Linkage::~Linkage()
{
    destroyTypeCheckingCache();
}

TypeCheckingCache* Linkage::getTypeCheckingCache()
{
    if (!m_typeCheckingCache)
    {
        m_typeCheckingCache = new TypeCheckingCache();
    }
    return m_typeCheckingCache;
}

void Linkage::destroyTypeCheckingCache()
{
    delete m_typeCheckingCache;
    m_typeCheckingCache = nullptr;
}

SLANG_NO_THROW slang::IGlobalSession* SLANG_MCALL Linkage::getGlobalSession()
{
    return asExternal(getSessionImpl());
}

void Linkage::addTarget(
    slang::TargetDesc const&  desc)
{
    auto targetIndex = addTarget(CodeGenTarget(desc.format));
    auto target = targets[targetIndex];

    target->floatingPointMode = FloatingPointMode(desc.floatingPointMode);
    target->targetFlags = desc.flags;
    target->targetProfile = Profile(desc.profile);
}

#if 0
SLANG_NO_THROW SlangInt SLANG_MCALL Linkage::getTargetCount()
{
    return targets.getCount();
}

SLANG_NO_THROW slang::ITarget* SLANG_MCALL Linkage::getTargetByIndex(SlangInt index)
{
    if(index < 0) return nullptr;
    if(index >= targets.getCount()) return nullptr;
    return asExternal(targets[index]);
}
#endif

SLANG_NO_THROW slang::IModule* SLANG_MCALL Linkage::loadModule(
    const char*     moduleName,
    slang::IBlob**  outDiagnostics)
{
    auto name = getNamePool()->getName(moduleName);

    DiagnosticSink sink(getSourceManager());
    auto module = findOrImportModule(name, SourceLoc(), &sink);
    sink.getBlobIfNeeded(outDiagnostics);

    return asExternal(module);
}

SLANG_NO_THROW SlangResult SLANG_MCALL Linkage::createCompositeComponentType(
    slang::IComponentType* const*   componentTypes,
    SlangInt                        componentTypeCount,
    slang::IComponentType**         outCompositeComponentType,
    ISlangBlob**                    outDiagnostics)
{
    // Attempting to create a "composite" of just one component type should
    // just return the component type itself, to avoid redundant work.
    //
    if( componentTypeCount == 1)
    {
        auto componentType = componentTypes[0];
        componentType->addRef();
        *outCompositeComponentType = componentType;
        return SLANG_OK;
    }

    DiagnosticSink sink(getSourceManager());

    List<RefPtr<ComponentType>> childComponents;
    for( Int cc = 0; cc < componentTypeCount; ++cc )
    {
        childComponents.add(asInternal(componentTypes[cc]));
    }

    RefPtr<ComponentType> composite = CompositeComponentType::create(
        this,
        childComponents);

    sink.getBlobIfNeeded(outDiagnostics);

    *outCompositeComponentType = asExternal(composite.detach());
    return SLANG_OK;
}

SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL Linkage::specializeType(
    slang::TypeReflection*          inUnspecializedType,
    slang::SpecializationArg const* specializationArgs,
    SlangInt                        specializationArgCount,
    ISlangBlob**                    outDiagnostics)
{
    auto unspecializedType = asInternal(inUnspecializedType);

    List<Type*> typeArgs;

    for(Int ii = 0; ii < specializationArgCount; ++ii)
    {
        auto& arg = specializationArgs[ii];
        if(arg.kind != slang::SpecializationArg::Kind::Type)
            return nullptr;

        typeArgs.add(asInternal(arg.type));
    }

    DiagnosticSink sink(getSourceManager());
    auto specializedType = specializeType(unspecializedType, typeArgs.getCount(), typeArgs.getBuffer(), &sink);
    sink.getBlobIfNeeded(outDiagnostics);

    return asExternal(specializedType);
}

SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL Linkage::getTypeLayout(
    slang::TypeReflection*  inType,
    SlangInt                targetIndex,
    slang::LayoutRules      rules,
    ISlangBlob**            outDiagnostics)
{
    auto type = asInternal(inType);

    if(targetIndex < 0 || targetIndex >= targets.getCount())
        return nullptr;

    auto target = targets[targetIndex];

    // TODO: We need a way to pass through the layout rules
    // that the user requested (e.g., constant buffers vs.
    // structured buffer rules). Right now the API only
    // exposes a single case, so this isn't a big deal.
    //
    SLANG_UNUSED(rules);

    auto typeLayout = target->getTypeLayout(type);

    // TODO: We currently don't have a path for capturing
    // errors that occur during layout (e.g., types that
    // are invalid because of target-specific layout constraints).
    //
    SLANG_UNUSED(outDiagnostics);

    return asExternal(typeLayout);
}

SLANG_NO_THROW SlangResult SLANG_MCALL Linkage::createCompileRequest(
    SlangCompileRequest**   outCompileRequest)
{
    auto compileRequest = new EndToEndCompileRequest(this);
    *outCompileRequest = asExternal(compileRequest);
    return SLANG_OK;
}

SlangResult Linkage::addSearchPath(
    char const* path)
{
    searchDirectories.searchDirectories.add(Slang::SearchDirectory(path));
    return SLANG_OK;
}

SlangResult Linkage::addPreprocessorDefine(
    char const* name,
    char const* value)
{
    preprocessorDefinitions[name] = value;
    return SLANG_OK;
}

SlangResult Linkage::setMatrixLayoutMode(
    SlangMatrixLayoutMode mode)
{
    defaultMatrixLayoutMode = MatrixLayoutMode(mode);
    return SLANG_OK;
}


//
// TargetRequest
//

Session* TargetRequest::getSession()
{
    return linkage->getSessionImpl();
}

MatrixLayoutMode TargetRequest::getDefaultMatrixLayoutMode()
{
    return linkage->getDefaultMatrixLayoutMode();
}

TypeLayout* TargetRequest::getTypeLayout(Type* type)
{
    // TODO: We are not passing in a `ProgramLayout` here, although one
    // is nominally required to establish the global ordering of
    // generic type parameters, which might be referenced from field types.
    //
    // The solution here is to make sure that the reflection data for
    // uses of global generic/existential types does *not* include any
    // kind of index in that global ordering, and just refers to the
    // parameter instead (leaving the user to figure out how that
    // maps to the ordering via some API on the program layout).
    //
    auto layoutContext = getInitialLayoutContextForTarget(this, nullptr);

    RefPtr<TypeLayout> result;
    if (getTypeLayouts().TryGetValue(type, result))
        return result.Ptr();
    result = createTypeLayout(layoutContext, type);
    getTypeLayouts()[type] = result;
    return result.Ptr();
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
    m_sourceFiles.add(sourceFile);

    // We want to record that the compiled module has a dependency
    // on the path of the source file, but we also need to account
    // for cases where the user added a source string/blob without
    // an associated path and/or wasn't from a file.

    auto pathInfo = sourceFile->getPathInfo();
    if (pathInfo.hasFileFoundPath())
    {
        getModule()->addFilePathDependency(pathInfo.foundPath);
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

SlangResult Linkage::loadFile(String const& path, PathInfo& outPathInfo, ISlangBlob** outBlob)
{
    outPathInfo.type = PathInfo::Type::Unknown;

    SLANG_RETURN_ON_FAIL(m_fileSystemExt->loadFile(path.getBuffer(), outBlob));

    ComPtr<ISlangBlob> uniqueIdentity;
    // Get the unique identity
    SLANG_RETURN_ON_FAIL(m_fileSystemExt->getFileUniqueIdentity(path.getBuffer(), uniqueIdentity.writeRef()));

    outPathInfo.foundPath = path;
    outPathInfo.type = PathInfo::Type::FoundPath;
    outPathInfo.uniqueIdentity = StringUtil::getString(uniqueIdentity);

    return SLANG_OK;
}

Expr* Linkage::parseTermString(String typeStr, RefPtr<Scope> scope)
{
    // Create a SourceManager on the stack, so any allocations for 'SourceFile'/'SourceView' etc will be cleaned up
    SourceManager localSourceManager;
    localSourceManager.initialize(getSourceManager(), nullptr);
        
    Slang::SourceFile* srcFile = localSourceManager.createSourceFileWithString(PathInfo::makeTypeParse(), typeStr);
    
    // We'll use a temporary diagnostic sink  
    DiagnosticSink sink(&localSourceManager);

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

    return parseTermFromSourceFile(
        getASTBuilder(),
        tokens, &sink, scope, getNamePool(), SourceLanguage::Slang);
}

Type* checkProperType(
    Linkage*        linkage,
    TypeExp         typeExp,
    DiagnosticSink* sink);

Type* ComponentType::getTypeFromString(
        String const&   typeStr,
        DiagnosticSink* sink)
{
    // If we've looked up this type name before,
    // then we can re-use it.
    //
    Type* type = nullptr;
    if(m_types.TryGetValue(typeStr, type))
        return type;

    // Otherwise, we need to start looking in
    // the modules that were directly or
    // indirectly referenced.
    //
    RefPtr<Scope> scope = _createScopeForLegacyLookup();

    auto linkage = getLinkage();
    Expr* typeExpr = linkage->parseTermString(
        typeStr, scope);
    type = checkProperType(linkage, TypeExp(typeExpr), sink);

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

    auto linkage = getLinkage();

    // TODO(JS): NOTE! Here we are using the searchDirectories on the linkage. This is because
    // currently the API only allows the setting search paths on linkage.
    // 
    // Here we should probably be using the searchDirectories on the FrontEndCompileRequest.
    // If searchDirectories.parent pointed to the one in the Linkage would mean linkage paths
    // would be checked too (after those on the FrontEndCompileRequest). 

    includeHandler.linkage = linkage;
    includeHandler.searchDirectories = &linkage->searchDirectories;

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

    ASTBuilder* astBuilder = module->getASTBuilder();

    //ASTBuilder* astBuilder = linkage->getASTBuilder();

    ModuleDecl* translationUnitSyntax = astBuilder->create<ModuleDecl>();

    translationUnitSyntax->nameAndLoc.name = translationUnit->moduleName;
    translationUnitSyntax->module = module;
    module->setModuleDecl(translationUnitSyntax);

    // When compiling a module of code that belongs to the Slang
    // standard library, we add a modifier to the module to act
    // as a marker, so that downstream code can detect declarations
    // that came from the standard library (by walking up their
    // chain of ancestors and looking for the marker), and treat
    // them differently from user declarations.
    //
    // We are adding the marker here, before we even parse the
    // code in the module, in case the subsequent steps would
    // like to treat the standard library differently. Alternatively
    // we could pass down the `m_isStandardLibraryCode` flag to
    // these passes.
    //
    if( m_isStandardLibraryCode )
    {
        translationUnitSyntax->modifiers.first = astBuilder->create<FromStdLibModifier>();
    }

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
            astBuilder,
            translationUnit,
            tokens,
            getSink(),
            languageScope);

        // Let's try dumping

        if (shouldDumpAST)
        {
            StringBuilder buf;
            SourceWriter writer(linkage->getSourceManager(), LineDirectiveMode::None);

            ASTDumpUtil::dump(translationUnit->getModuleDecl(), ASTDumpUtil::Style::Flat, 0, &writer);

            const String& path = sourceFile->getPathInfo().foundPath;
            if (path.getLength())
            {
                String fileName = Path::getFileNameWithoutExt(path);
                fileName.append(".slang-ast");

                File::writeAllText(fileName, writer.getContent());
            }
        }

#if 0
        // Test serialization
        {
            ASTSerialTestUtil::testSerialize(translationUnit->getModuleDecl(), getSession()->getRootNamePool(), getLinkage()->getASTBuilder()->getSharedASTBuilder(), getSourceManager());
        }
#endif

    }
}

RefPtr<ComponentType> createUnspecializedGlobalComponentType(
        FrontEndCompileRequest* compileRequest);

RefPtr<ComponentType> createUnspecializedGlobalAndEntryPointsComponentType(
        FrontEndCompileRequest*         compileRequest,
        List<RefPtr<ComponentType>>&    outUnspecializedEntryPoints);

RefPtr<ComponentType> createSpecializedGlobalComponentType(
    EndToEndCompileRequest* endToEndReq);

RefPtr<ComponentType> createSpecializedGlobalAndEntryPointsComponentType(
    EndToEndCompileRequest*         endToEndReq,
    List<RefPtr<ComponentType>>&    outSpecializedEntryPoints);

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

        /// Generate IR for translation unit.
        /// TODO(JS): Use the linkage ASTBuilder, because it seems possible that cross module constructs are possible in
        /// ir lowering.
        RefPtr<IRModule> irModule(generateIRForTranslationUnit(getLinkage()->getASTBuilder(), translationUnit));

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

    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    // Perform semantic checking on the whole collection
    checkAllTranslationUnits();
    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;


    // Look up all the entry points that are expected,
    // and use them to populate the `program` member.
    //
    m_globalComponentType = createUnspecializedGlobalComponentType(this);
    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    m_globalAndEntryPointsComponentType = createUnspecializedGlobalAndEntryPointsComponentType(
        this,
        m_unspecializedEntryPoints);
    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    // We always generate IR for all the translation units.
    //
    // TODO: We may eventually have a mode where we skip
    // IR codegen and only produce an AST (e.g., for use when
    // debugging problems in the parser or semantic checking),
    // but for now there are no cases where not having IR
    // makes sense.
    //
    generateIR();
    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    // Do parameter binding generation, for each compilation target.
    //
    for(auto targetReq : getLinkage()->targets)
    {
        auto targetProgram = m_globalAndEntryPointsComponentType->getTargetProgram(targetReq);
        targetProgram->getOrCreateLayout(getSink());
        targetProgram->getOrCreateIRModuleForLayout(getSink());
    }
    if (getSink()->getErrorCount() != 0)
        return SLANG_FAIL;

    return SLANG_OK;
}

BackEndCompileRequest::BackEndCompileRequest(
    Linkage*        linkage,
    DiagnosticSink* sink,
    ComponentType*  program)
    : CompileRequestBase(linkage, sink)
    , m_program(program)
    , m_dumpIntermediatePrefix("slang-dump-")
{}

EndToEndCompileRequest::EndToEndCompileRequest(
    Session* session)
    : m_session(session)
    , m_sink(nullptr)
{
    RefPtr<ASTBuilder> astBuilder(new ASTBuilder(session->m_sharedASTBuilder, "EndToEnd::Linkage::astBuilder"));
    m_linkage = new Linkage(session, astBuilder);
    init();
}

EndToEndCompileRequest::EndToEndCompileRequest(
    Linkage* linkage)
    : m_session(linkage->getSessionImpl())
    , m_linkage(linkage)
    , m_sink(nullptr)
{
    init();
}

void EndToEndCompileRequest::init()
{
    m_sink.setSourceManager(m_linkage->getSourceManager());

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
    if (getLinkage()->targets.getCount() == 0)
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
        m_specializedGlobalComponentType = getUnspecializedGlobalComponentType();
        m_specializedGlobalAndEntryPointsComponentType = getUnspecializedGlobalAndEntryPointsComponentType();
        m_specializedEntryPoints = getFrontEndReq()->getUnspecializedEntryPoints();

        SLANG_RETURN_ON_FAIL(maybeCreateContainer());
        SLANG_RETURN_ON_FAIL(maybeWriteContainer(m_containerOutputPath));

        return SLANG_OK;
    }

    // If codegen is enabled, we need to move along to
    // apply any generic specialization that the user asked for.
    //
    if (passThrough == PassThroughMode::None)
    {
        m_specializedGlobalComponentType = createSpecializedGlobalComponentType(this);
        if (getSink()->getErrorCount() != 0)
            return SLANG_FAIL;

        m_specializedGlobalAndEntryPointsComponentType = createSpecializedGlobalAndEntryPointsComponentType(
            this,
            m_specializedEntryPoints);
        if (getSink()->getErrorCount() != 0)
            return SLANG_FAIL;

        // For each code generation target, we will generate specialized
        // parameter binding information (taking global generic
        // arguments into account at this time).
        //
        for (auto targetReq : getLinkage()->targets)
        {
            auto targetProgram = m_specializedGlobalAndEntryPointsComponentType->getTargetProgram(targetReq);
            targetProgram->getOrCreateLayout(getSink());
        }
        if (getSink()->getErrorCount() != 0)
            return SLANG_FAIL;
    }
    else
    {
        // We need to create dummy `EntryPoint` objects
        // to make sure that the logic in `generateOutput`
        // sees something worth processing.
        //
        List<RefPtr<ComponentType>> dummyEntryPoints;
        for(auto entryPointReq : getFrontEndReq()->getEntryPointReqs())
        {
            RefPtr<EntryPoint> dummyEntryPoint = EntryPoint::createDummyForPassThrough(
                getLinkage(),
                entryPointReq->getName(),
                entryPointReq->getProfile());

            dummyEntryPoints.add(dummyEntryPoint);
        }

        RefPtr<ComponentType> composedProgram = CompositeComponentType::create(
            getLinkage(),
            dummyEntryPoints);

        m_specializedGlobalComponentType = getUnspecializedGlobalComponentType();
        m_specializedGlobalAndEntryPointsComponentType = composedProgram;
        m_specializedEntryPoints = getFrontEndReq()->getUnspecializedEntryPoints();
    }

    // Generate output code, in whatever format was requested
    getBackEndReq()->setProgram(getSpecializedGlobalAndEntryPointsComponentType());
    generateOutput(this);
    if (getSink()->getErrorCount() != 0)
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
    Index result = translationUnits.getCount();

    if (!moduleName)
    {
        // We want to ensure that symbols defined in different translation
        // units get unique mangled names, so that we can, e.g., tell apart
        // a `main()` function in `vertex.slang` and a `main()` in `fragment.slang`,
        // even when they are being compiled together.
        //
        String generatedName = "tu";
        generatedName.append(translationUnits.getCount());
        moduleName = getNamePool()->getName(generatedName);
    }

    RefPtr<TranslationUnitRequest> translationUnit = new TranslationUnitRequest(this);
    translationUnit->compileRequest = this;
    translationUnit->sourceLanguage = SourceLanguage(language);

    translationUnit->moduleName = moduleName;

    translationUnits.add(translationUnit);

    return (int) result;
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
    // The path specified may or may not be a file path - mark as being constructed 'FromString'.
    SourceFile* sourceFile = getSourceManager()->createSourceFileWithBlob(PathInfo::makeFromString(path), sourceBlob);
    
    addTranslationUnitSourceFile(translationUnitIndex, sourceFile);
}

void FrontEndCompileRequest::addTranslationUnitSourceString(
    int             translationUnitIndex,
    String const&   path,
    String const&   source)
{
    // The path specified may or may not be a file path - mark as being constructed 'FromString'.
    SourceFile* sourceFile = getSourceManager()->createSourceFileWithString(PathInfo::makeFromString(path), source);

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

    PathInfo pathInfo;

    ComPtr<ISlangBlob> sourceBlob;
    SlangResult result = loadFile(path, pathInfo, sourceBlob.writeRef());
    if(SLANG_FAILED(result))
    {
        // Emit a diagnostic!
        getSink()->diagnose(
            SourceLoc(),
            Diagnostics::cannotOpenFile,
            path);
        return;
    }

    // Was loaded from the specified path
    SourceFile* sourceFile = getSourceManager()->createSourceFileWithBlob(pathInfo, sourceBlob);
    addTranslationUnitSourceFile(translationUnitIndex, sourceFile);
}

int FrontEndCompileRequest::addEntryPoint(
    int                     translationUnitIndex,
    String const&           name,
    Profile                 entryPointProfile)
{
    auto translationUnitReq = translationUnits[translationUnitIndex];

    Index result = m_entryPointReqs.getCount();

    RefPtr<FrontEndEntryPointRequest> entryPointReq = new FrontEndEntryPointRequest(
        this,
        translationUnitIndex,
        getNamePool()->getName(name),
        entryPointProfile);

    m_entryPointReqs.add(entryPointReq);
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
        entryPointInfo.specializationArgStrings.add(typeName);

    Index result = entryPoints.getCount();
    entryPoints.add(_Move(entryPointInfo));
    return (int) result;
}

UInt Linkage::addTarget(
    CodeGenTarget   target)
{
    RefPtr<TargetRequest> targetReq = new TargetRequest();
    targetReq->linkage = this;
    targetReq->target = target;

    Index result = targets.getCount();
    targets.add(targetReq);
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
    SLANG_ASSERT(mostUniqueIdentity.getLength() > 0);

    mapPathToLoadedModule.Add(mostUniqueIdentity, loadedModule);
    mapNameToLoadedModules.Add(name, loadedModule);

    auto sink = translationUnit->compileRequest->getSink();

    int errorCountBefore = sink->getErrorCount();
    checkTranslationUnit(translationUnit.Ptr());
    int errorCountAfter = sink->getErrorCount();

    if (errorCountAfter != errorCountBefore)
    {
        // There must have been an error in the loaded module.
    }
    else
    {
        // If we didn't run into any errors, then try to generate
        // IR code for the imported module.
        SLANG_ASSERT(errorCountAfter == 0);
        loadedModule->setIRModule(generateIRForTranslationUnit(getASTBuilder(), translationUnit));
    }
    loadedModulesList.add(loadedModule);
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
    translationUnit->sourceLanguage = SourceLanguage::Slang;

    auto module = translationUnit->getModule();

    ModuleBeingImportedRAII moduleBeingImported(
        this,
        module);

    // Create with the 'friendly' name
    SourceFile* sourceFile = getSourceManager()->createSourceFileWithBlob(filePathInfo, sourceBlob);
    
    translationUnit->addSourceFile(sourceFile);

    int errorCountBefore = sink->getErrorCount();
    frontEndReq->parseTranslationUnit(translationUnit);
    int errorCountAfter = sink->getErrorCount();

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

    errorCountAfter = sink->getErrorCount();

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
    if(SLANG_FAILED(getFileSystemExt()->loadFile(filePathInfo.foundPath.getBuffer(), fileContents.writeRef())))
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

    m_moduleList.add(module);
    m_moduleSet.Add(module);
}

//
// FilePathDependencyList
//

void FilePathDependencyList::addDependency(String const& path)
{
    if(m_filePathSet.Contains(path))
        return;

    m_filePathList.add(path);
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
    : ComponentType(linkage)
    , m_astBuilder(linkage->getASTBuilder()->getSharedASTBuilder(), "Module")
{
    addModuleDependency(this);
}

ISlangUnknown* Module::getInterface(const Guid& guid)
{
    if(guid == IID_IModule)
        return asExternal(this);
    return Super::getInterface(guid);
}

void Module::addModuleDependency(Module* module)
{
    m_moduleDependencyList.addDependency(module);
    m_filePathDependencyList.addDependency(module);
}

void Module::addFilePathDependency(String const& path)
{
    m_filePathDependencyList.addDependency(path);
}

void Module::setModuleDecl(ModuleDecl* moduleDecl)
{
    m_moduleDecl = moduleDecl;
}

RefPtr<EntryPoint> Module::findEntryPointByName(UnownedStringSlice const& name)
{
    // TODO: We should consider having this function be expanded to be able
    // to look up and validate possible entry-point functions in teh module
    // even if they were not marked with `[shader(...)]` in the source code.
    //
    // With such a change the function would probably need to accept a stage
    // to use and a sink to write validation errors to.

    for(auto entryPoint : m_entryPoints)
    {
        if(entryPoint->getName()->text.getUnownedSlice() == name)
            return entryPoint;
    }

    return nullptr;
}

void Module::_addEntryPoint(EntryPoint* entryPoint)
{
    m_entryPoints.add(entryPoint);
}


// ComponentType

ComponentType::ComponentType(Linkage* linkage)
    : m_linkage(linkage)
{}

ComponentType* asInternal(slang::IComponentType* inComponentType)
{
    // Note: we use a `queryInterface` here instead of just a `static_cast`
    // to ensure that the `IComponentType` we get is the preferred/canonical
    // one, which shares its address with the `ComponentType`.
    //
    // TODO: An alternative choice here would be to have a "magic" IID that
    // we pass into `queryInterface` that returns the `ComponentType` directly
    // (without even `addRef`-ing it).
    //
    ComPtr<slang::IComponentType> componentType;
    inComponentType->queryInterface(IID_IComponentType, (void**) componentType.writeRef());
    return static_cast<ComponentType*>(componentType.get());
}

ISlangUnknown* ComponentType::getInterface(Guid const& guid)
{
    if(guid == IID_ISlangUnknown
        || guid == IID_IComponentType)
    {
        return static_cast<slang::IComponentType*>(this);
    }

    return nullptr;
}

SLANG_NO_THROW slang::ISession* SLANG_MCALL ComponentType::getSession()
{
    return m_linkage;
}

SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL ComponentType::getLayout(
    Int             targetIndex,
    slang::IBlob**  outDiagnostics)
{
    auto linkage = getLinkage();
    if(targetIndex < 0 || targetIndex >= linkage->targets.getCount())
        return nullptr;
    auto target = linkage->targets[targetIndex];

    DiagnosticSink sink(linkage->getSourceManager());
    auto programLayout = getTargetProgram(target)->getOrCreateLayout(&sink);
    sink.getBlobIfNeeded(outDiagnostics);

    return asExternal(programLayout);
}

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::getEntryPointCode(
    SlangInt        entryPointIndex,
    Int             targetIndex,
    slang::IBlob**  outCode,
    slang::IBlob**  outDiagnostics)
{
    auto linkage = getLinkage();
    if(targetIndex < 0 || targetIndex >= linkage->targets.getCount())
        return SLANG_E_INVALID_ARG;
    auto target = linkage->targets[targetIndex];

    auto targetProgram = getTargetProgram(target);

    DiagnosticSink sink(linkage->getSourceManager());
    auto& entryPointResult = targetProgram->getOrCreateEntryPointResult(entryPointIndex, &sink);
    sink.getBlobIfNeeded(outDiagnostics);

    if(entryPointResult.format == ResultFormat::None )
        return SLANG_FAIL;

    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(entryPointResult.getBlob(blob));
    *outCode = blob.detach();
    return SLANG_OK;
}

RefPtr<ComponentType> ComponentType::specialize(
    SpecializationArg const*    inSpecializationArgs,
    SlangInt                    specializationArgCount,
    DiagnosticSink*             sink)
{
    if(specializationArgCount == 0)
    {
        return this;
    }

    List<SpecializationArg> specializationArgs;
    specializationArgs.addRange(
        inSpecializationArgs,
        specializationArgCount);

    // We next need to validate that the specialization arguments
    // make sense, and also expand them to include any derived data
    // (e.g., interface conformance witnesses) that doesn't get
    // passed explicitly through the API interface.
    //
    RefPtr<SpecializationInfo> specializationInfo = _validateSpecializationArgs(
        specializationArgs.getBuffer(),
        specializationArgCount,
        sink);

    return new SpecializedComponentType(
        this,
        specializationInfo,
        specializationArgs,
        sink);
}

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::specialize(
    slang::SpecializationArg const* specializationArgs,
    SlangInt                        specializationArgCount,
    slang::IComponentType**         outSpecializedComponentType,
    ISlangBlob**                    outDiagnostics)
{
    DiagnosticSink sink(getLinkage()->getSourceManager());

    // First let's check if the number of arguments given matches
    // the number of parameters that are present on this component type.
    //
    auto specializationParamCount = getSpecializationParamCount();
    if( specializationArgCount != specializationParamCount )
    {
        // TODO: diagnose
        sink.getBlobIfNeeded(outDiagnostics);
        return SLANG_FAIL;
    }

    List<SpecializationArg> expandedArgs;
    for( Int aa = 0; aa < specializationArgCount; ++aa )
    {
        auto apiArg = specializationArgs[aa];

        SpecializationArg expandedArg;
        switch(apiArg.kind)
        {
        case slang::SpecializationArg::Kind::Type:
            expandedArg.val = asInternal(apiArg.type);
            break;

        default:
            sink.getBlobIfNeeded(outDiagnostics);
            return SLANG_FAIL;
        }
        expandedArgs.add(expandedArg);
    }

    auto specializedComponentType = specialize(
        expandedArgs.getBuffer(),
        expandedArgs.getCount(),
        &sink);

    sink.getBlobIfNeeded(outDiagnostics);

    *outSpecializedComponentType = specializedComponentType.detach();

    return SLANG_OK;
}

RefPtr<ComponentType> fillRequirements(
    ComponentType* inComponentType);

SLANG_NO_THROW SlangResult SLANG_MCALL ComponentType::link(
    slang::IComponentType**         outLinkedComponentType,
    ISlangBlob**                    outDiagnostics)
{
    // TODO: It should be possible for `fillRequirements` to fail,
    // in cases where we have a dependency that can't be automatically
    // resolved.
    //
    SLANG_UNUSED(outDiagnostics);

    auto linked = fillRequirements(this);
    if(!linked)
        return SLANG_FAIL;

    *outLinkedComponentType = ComPtr<slang::IComponentType>(linked).detach();
    return SLANG_OK;
}


    /// Visitor used by `ComponentType::enumerateModules`
struct EnumerateModulesVisitor : ComponentTypeVisitor
{
    EnumerateModulesVisitor(ComponentType::EnumerateModulesCallback callback, void* userData)
        : m_callback(callback)
        , m_userData(userData)
    {}

    ComponentType::EnumerateModulesCallback m_callback;
    void* m_userData;

    void visitEntryPoint(EntryPoint*, EntryPoint::EntryPointSpecializationInfo*) SLANG_OVERRIDE {}

    void visitModule(Module* module, Module::ModuleSpecializationInfo*) SLANG_OVERRIDE
    {
        m_callback(module, m_userData);
    }

    void visitComposite(CompositeComponentType* composite, CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        visitChildren(composite, specializationInfo);
    }

    void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
    {
        visitChildren(specialized);
    }
};


void ComponentType::enumerateModules(EnumerateModulesCallback callback, void* userData)
{
    EnumerateModulesVisitor visitor(callback, userData);
    acceptVisitor(&visitor, nullptr);
}

    /// Visitor used by `ComponentType::enumerateIRModules`
struct EnumerateIRModulesVisitor : ComponentTypeVisitor
{
    EnumerateIRModulesVisitor(ComponentType::EnumerateIRModulesCallback callback, void* userData)
        : m_callback(callback)
        , m_userData(userData)
    {}

    ComponentType::EnumerateIRModulesCallback m_callback;
    void* m_userData;

    void visitEntryPoint(EntryPoint*, EntryPoint::EntryPointSpecializationInfo*) SLANG_OVERRIDE {}

    void visitModule(Module* module, Module::ModuleSpecializationInfo*) SLANG_OVERRIDE
    {
        m_callback(module->getIRModule(), m_userData);
    }

    void visitComposite(CompositeComponentType* composite, CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        visitChildren(composite, specializationInfo);
    }

    void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
    {
        visitChildren(specialized);

        m_callback(specialized->getIRModule(), m_userData);
    }
};

void ComponentType::enumerateIRModules(EnumerateIRModulesCallback callback, void* userData)
{
    EnumerateIRModulesVisitor visitor(callback, userData);
    acceptVisitor(&visitor, nullptr);
}

//
// CompositeComponentType
//

RefPtr<ComponentType> CompositeComponentType::create(
    Linkage*                            linkage,
    List<RefPtr<ComponentType>> const&  childComponents)
{
    // TODO: We should ideally be caching the results of
    // composition on the `linkage`, so that if we get
    // asked for the same composite again later we re-use
    // it rather than re-create it.
    //
    // Similarly, we might want to do some amount of
    // work to "canonicalize" the input for composition.
    // E.g., if the user does:
    //
    //    X = compose(A,B);
    //    Y = compose(C,D);
    //    Z = compose(X,Y);
    //
    //    W = compose(A, B, C, D);
    //
    // Then there is no observable difference between
    // Z and W, so we might prefer to have them be identical.

    // If there is only a single child, then we should
    // just return that child rather than create a dummy composite.
    //
    if( childComponents.getCount() == 1 )
    {
        return childComponents[0];
    }

    return new CompositeComponentType(linkage, childComponents);
}


CompositeComponentType::CompositeComponentType(
    Linkage*                            linkage,
    List<RefPtr<ComponentType>> const&  childComponents)
    : ComponentType(linkage)
    , m_childComponents(childComponents)
{
    HashSet<ComponentType*> requirementsSet;
    for(auto child : childComponents )
    {
        child->enumerateModules([&](Module* module)
        {
            requirementsSet.Add(module);
        });
    }

    for(auto child : childComponents )
    {
        auto childEntryPointCount = child->getEntryPointCount();
        for(Index cc = 0; cc < childEntryPointCount; ++cc)
        {
            m_entryPoints.add(child->getEntryPoint(cc));
            m_entryPointMangledNames.add(child->getEntryPointMangledName(cc));
        }

        auto childShaderParamCount = child->getShaderParamCount();
        for(Index pp = 0; pp < childShaderParamCount; ++pp)
        {
            m_shaderParams.add(child->getShaderParam(pp));
        }

        auto childSpecializationParamCount = child->getSpecializationParamCount();
        for(Index pp = 0; pp < childSpecializationParamCount; ++pp)
        {
            m_specializationParams.add(child->getSpecializationParam(pp));
        }

        for(auto module : child->getModuleDependencies())
        {
            m_moduleDependencyList.addDependency(module);
        }
        for(auto filePath : child->getFilePathDependencies())
        {
            m_filePathDependencyList.addDependency(filePath);
        }

        auto childRequirementCount = child->getRequirementCount();
        for(Index rr = 0; rr < childRequirementCount; ++rr)
        {
            auto childRequirement = child->getRequirement(rr);
            if(!requirementsSet.Contains(childRequirement))
            {
                requirementsSet.Add(childRequirement);
                m_requirements.add(childRequirement);
            }
        }
    }
}

Index CompositeComponentType::getEntryPointCount()
{
    return m_entryPoints.getCount();
}

RefPtr<EntryPoint> CompositeComponentType::getEntryPoint(Index index)
{
    return m_entryPoints[index];
}

String CompositeComponentType::getEntryPointMangledName(Index index)
{
    return m_entryPointMangledNames[index];
}

Index CompositeComponentType::getShaderParamCount()
{
    return m_shaderParams.getCount();
}

ShaderParamInfo CompositeComponentType::getShaderParam(Index index)
{
    return m_shaderParams[index];
}

Index CompositeComponentType::getSpecializationParamCount()
{
    return m_specializationParams.getCount();
}

SpecializationParam const& CompositeComponentType::getSpecializationParam(Index index)
{
    return m_specializationParams[index];
}

Index CompositeComponentType::getRequirementCount()
{
    return m_requirements.getCount();
}

RefPtr<ComponentType> CompositeComponentType::getRequirement(Index index)
{
    return m_requirements[index];
}

List<Module*> const& CompositeComponentType::getModuleDependencies()
{
    return m_moduleDependencyList.getModuleList();
}

List<String> const& CompositeComponentType::getFilePathDependencies()
{
    return m_filePathDependencyList.getFilePathList();
}

void CompositeComponentType::acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo)
{
    visitor->visitComposite(this, as<CompositeSpecializationInfo>(specializationInfo));
}


RefPtr<ComponentType::SpecializationInfo> CompositeComponentType::_validateSpecializationArgsImpl(
    SpecializationArg const*    args,
    Index                       argCount,
    DiagnosticSink*             sink)
{
    SLANG_UNUSED(argCount);

    RefPtr<CompositeSpecializationInfo> specializationInfo = new CompositeSpecializationInfo();

    Index offset = 0;
    for(auto child : m_childComponents)
    {
        auto childParamCount = child->getSpecializationParamCount();
        SLANG_ASSERT(offset + childParamCount <= argCount);

        auto childInfo = child->_validateSpecializationArgs(
            args + offset,
            childParamCount,
            sink);

        specializationInfo->childInfos.add(childInfo);

        offset += childParamCount;
    }
    return specializationInfo;
}

//
// SpecializedComponentType
//

SpecializedComponentType::SpecializedComponentType(
    ComponentType*                      base,
    ComponentType::SpecializationInfo*  specializationInfo,
    List<SpecializationArg> const&      specializationArgs,
    DiagnosticSink*                     sink)
    : ComponentType(base->getLinkage())
    , m_base(base)
    , m_specializationInfo(specializationInfo)
    , m_specializationArgs(specializationArgs)
{
    m_irModule = generateIRForSpecializedComponentType(this, sink);

    // The following is a bit of a hack.
    //
    // Back-end code generation relies on us having computed layouts for all tagged
    // unions that end up being used in the code, which means we need a way to find
    // all such types that get used in a program (and the stuff it imports).
    //
    // For now we are assuming a tagged union type only comes into existence
    // as a (top-level) argument for a generic type parameter, so that we
    // can check for them here and cache them on the entry point.
    //
    // A longer-term strategy might need to consider any (tagged or untagged)
    // union types that get used inside of a module, and also take
    // those lists into account.
    //
    // An even longer-term strategy would be to allow type layout to
    // be performed on IR types, so taht we don't need to have front-end
    // code worrying about this stuff.
    // 
    for(auto arg : specializationArgs)
    {
        auto argType = as<Type>(arg.val);
        if(!argType)
            continue;

        auto taggedUnionType = as<TaggedUnionType>(argType);
        if(!taggedUnionType)
            continue;

        m_taggedUnionTypes.add(taggedUnionType);
    }

    // Because we are specializing shader code, the mangled entry
    // point names for this component type may be different than
    // for the base component type (e.g., the mangled name for `f<int>`
    // is different than that that of the generic `f` function
    // itself).
    //
    // We will compute the mangled names of all the entry points and
    // store them here, so that we don't have to do it on the fly.
    // Because the `ComponentType` structure is hierarchical, we
    // need to use a recursive visitor to compute the names,
    // and we will define that visitor locally:
    //
    struct EntryPointMangledNameCollector : ComponentTypeVisitor
    {
        List<String>* mangledEntryPointNames;

        void visitEntryPoint(EntryPoint* entryPoint, EntryPoint::EntryPointSpecializationInfo* specializationInfo)  SLANG_OVERRIDE
        {
            auto funcDeclRef = entryPoint->getFuncDeclRef();
            if(specializationInfo)
                funcDeclRef = specializationInfo->specializedFuncDeclRef;

            (*mangledEntryPointNames).add(getMangledName(m_astBuilder, funcDeclRef));
        }

        void visitModule(Module*, Module::ModuleSpecializationInfo*) SLANG_OVERRIDE
        {}
        void visitComposite(CompositeComponentType* composite, CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
        { visitChildren(composite, specializationInfo); }
        void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
        { visitChildren(specialized); }

        EntryPointMangledNameCollector(ASTBuilder* astBuilder):
            m_astBuilder(astBuilder)
        {
        }
        ASTBuilder* m_astBuilder;
    };

    // With the visitor defined, we apply it to ourself to compute
    // and collect the mangled entry point names.
    //
    EntryPointMangledNameCollector collector(getLinkage()->getASTBuilder());
    collector.mangledEntryPointNames = &m_entryPointMangledNames;
    collector.visitSpecialized(this);
}

void SpecializedComponentType::acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo)
{
    SLANG_ASSERT(specializationInfo == nullptr);
    SLANG_UNUSED(specializationInfo);
    visitor->visitSpecialized(this);
}

Index SpecializedComponentType::getRequirementCount()
{
    // TODO: A specialized component type may have *more* requirements
    // than the original, because it also needs to include the module(s)
    // that define the types used for specialization arguments.

    return m_base->getRequirementCount();
}

RefPtr<ComponentType> SpecializedComponentType::getRequirement(Index index)
{
    return m_base->getRequirement(index);
}

String SpecializedComponentType::getEntryPointMangledName(Index index)
{
    return m_entryPointMangledNames[index];
}

void ComponentTypeVisitor::visitChildren(CompositeComponentType* composite, CompositeComponentType::CompositeSpecializationInfo* specializationInfo)
{
    auto childCount = composite->getChildComponentCount();
    for(Index ii = 0; ii < childCount; ++ii)
    {
        auto child = composite->getChildComponent(ii);
        auto childSpecializationInfo = specializationInfo
            ? specializationInfo->childInfos[ii]
            : nullptr;

        child->acceptVisitor(this, childSpecializationInfo);
    }
}

void ComponentTypeVisitor::visitChildren(SpecializedComponentType* specialized)
{
    specialized->getBaseComponentType()->acceptVisitor(this, specialized->getSpecializationInfo());
}

TargetProgram* ComponentType::getTargetProgram(TargetRequest* target)
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
    ComponentType*  componentType,
    TargetRequest*  targetReq)
    : m_program(componentType)
    , m_targetReq(targetReq)
{
    m_entryPointResults.setCount(componentType->getEntryPointCount());
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
    if(m_internalErrorLocsNoted == 0)
    {
        diagnose(loc, Diagnostics::noteLocationOfInternalError);
    }
    m_internalErrorLocsNoted++;
}

SlangResult DiagnosticSink::getBlobIfNeeded(ISlangBlob** outBlob)
{
    // If the client doesn't want an output blob, there is nothing to do.
    //
    if(!outBlob) return SLANG_OK;

    // For outputBuffer to be valid and hold diagnostics, writer must not be set
    SLANG_ASSERT(writer == nullptr);

    // If there were no errors, and there was no diagnostic output, there is nothing to do.
    if(getErrorCount() == 0 && outputBuffer.getLength() == 0)
    {
        return SLANG_OK;
    }

    Slang::ComPtr<ISlangBlob> blob = Slang::StringUtil::createStringBlob(outputBuffer);
    *outBlob = blob.detach();

    return SLANG_OK;
}


Session* CompileRequestBase::getSession()
{
    return getLinkage()->getSessionImpl();
}

static const Slang::Guid IID_ISlangFileSystemExt = SLANG_UUID_ISlangFileSystemExt;
static const Slang::Guid IID_SlangCacheFileSystem = SLANG_UUID_CacheFileSystem;

void Linkage::setFileSystem(ISlangFileSystem* inFileSystem)
{
    // Set the fileSystem
    m_fileSystem = inFileSystem;

    // Release what's there
    m_fileSystemExt.setNull();
    m_cacheFileSystem.setNull();

    // If nullptr passed in set up default 
    if (inFileSystem == nullptr)
    {
        m_cacheFileSystem = new Slang::CacheFileSystem(Slang::OSFileSystemExt::getSingleton());
        m_fileSystemExt = m_cacheFileSystem;
    }
    else
    {
        CacheFileSystem* cacheFileSystemPtr = nullptr;   
        inFileSystem->queryInterface(IID_SlangCacheFileSystem, (void**)&cacheFileSystemPtr);
        if (cacheFileSystemPtr)
        {
            m_cacheFileSystem = cacheFileSystemPtr;
            m_fileSystemExt = cacheFileSystemPtr;
        }
        else 
        {
            if (m_requireCacheFileSystem)
            {
                m_cacheFileSystem = new Slang::CacheFileSystem(inFileSystem);
                m_fileSystemExt = m_cacheFileSystem;
            }
            else
            {
                // See if we have the full ISlangFileSystemExt interface, if we do just use it
                inFileSystem->queryInterface(IID_ISlangFileSystemExt, (void**)m_fileSystemExt.writeRef());

                // If not wrap with CacheFileSystem that emulates ISlangFileSystemExt from the ISlangFileSystem interface
                if (!m_fileSystemExt)
                {
                    // Construct a wrapper to emulate the extended interface behavior
                    m_cacheFileSystem = new Slang::CacheFileSystem(m_fileSystem);
                    m_fileSystemExt = m_cacheFileSystem;
                }
            }
        }
    }

    // Set the file system used on the source manager
    getSourceManager()->setFileSystemExt(m_fileSystemExt);
}

void Linkage::setRequireCacheFileSystem(bool requireCacheFileSystem)
{
    if (requireCacheFileSystem == m_requireCacheFileSystem)
    {
        return;
    }

    ComPtr<ISlangFileSystem> scopeFileSystem(m_fileSystem);
    m_requireCacheFileSystem = requireCacheFileSystem;

    setFileSystem(scopeFileSystem);
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
    SourceManager* sourceManager = getBuiltinSourceManager();

    DiagnosticSink sink(sourceManager);
    RefPtr<FrontEndCompileRequest> compileRequest = new FrontEndCompileRequest(
        m_builtinLinkage,
        &sink);
    compileRequest->m_isStandardLibraryCode = true;

    // Set the source manager on the sink
    sink.setSourceManager(sourceManager);
    // Make the linkage use the builtin source manager
    Linkage* linkage = compileRequest->getLinkage();
    linkage->setSourceManager(sourceManager);

    Name* moduleName = getNamePool()->getName(path);
    auto translationUnitIndex = compileRequest->addTranslationUnit(SourceLanguage::Slang, moduleName);

    compileRequest->addTranslationUnitSourceString(
        translationUnitIndex,
        path,
        source);

    SlangResult res = compileRequest->executeActionsInner();
    if (SLANG_FAILED(res))
    {
        char const* diagnostics = sink.outputBuffer.getBuffer();
        fprintf(stderr, "%s", diagnostics);

#ifdef _WIN32
        OutputDebugStringA(diagnostics);
#endif

        SLANG_UNEXPECTED("error in Slang standard library");
    }

    // Extract the AST for the code we just parsed
    auto module = compileRequest->translationUnits[translationUnitIndex]->getModule();
    auto moduleDecl = module->getModuleDecl();

    // Add the resulting code to the appropriate scope
    if (!scope->containerDecl)
    {
        // We are the first chunk of code to be loaded for this scope
        scope->containerDecl = moduleDecl;
    }
    else
    {
        // We need to create a new scope to link into the whole thing
        auto subScope = new Scope();
        subScope->containerDecl = moduleDecl;
        subScope->nextSibling = scope->nextSibling;
        scope->nextSibling = subScope;
    }

    // We need to retain this AST so that we can use it in other code
    // (Note that the `Scope` type does not retain the AST it points to)
    stdlibModules.add(module);
}

Session::~Session()
{
    // destroy modules next
    stdlibModules = decltype(stdlibModules)();
}

}

// implementation of C interface

SLANG_API SlangSession* spCreateSession(const char*)
{
    Slang::RefPtr<Slang::Session> session(new Slang::Session());
    session->init();
    // Will be returned with a refcount of 1
    return asExternal(session.detach());
}

SLANG_API SlangResult slang_createGlobalSession(
    SlangInt                apiVersion,
    slang::IGlobalSession** outGlobalSession)
{
    if(apiVersion != 0)
        return SLANG_E_NOT_IMPLEMENTED;

    Slang::RefPtr<Slang::Session> globalSession(new Slang::Session());
    globalSession->init();
    Slang::ComPtr<slang::IGlobalSession> result(Slang::asExternal(globalSession));
    *outGlobalSession = result.detach();
    return SLANG_OK;
}

SLANG_API void spDestroySession(
    SlangSession*   inSession)
{
    if(!inSession) return;

    Slang::Session* session = Slang::asInternal(inSession);
    // It is assumed there is only a single reference on the session (the one placed
    // with spCreateSession) if this function is called
    SLANG_ASSERT(session->debugGetReferenceCount() == 1);
    // Release
    session->release();
}

SLANG_API const char* spGetBuildTagString()
{
    return SLANG_TAG_VERSION;
}

SLANG_API void spAddBuiltins(
    SlangSession*   session,
    char const*     sourcePath,
    char const*     sourceString)
{
    auto s = Slang::asInternal(session);
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
    auto s = Slang::asInternal(session);
    loader = loader ? loader : Slang::DefaultSharedLibraryLoader::getSingleton();
    s->setSharedLibraryLoader(loader);
}

SLANG_API ISlangSharedLibraryLoader* spSessionGetSharedLibraryLoader(
    SlangSession*               session)
{
    auto s = Slang::asInternal(session);
    return (s->m_sharedLibraryLoader == Slang::DefaultSharedLibraryLoader::getSingleton()) ? nullptr : s->m_sharedLibraryLoader.get();
}

SLANG_API SlangResult spSessionCheckCompileTargetSupport(
    SlangSession*                session,
    SlangCompileTarget           target)
{
    auto s = Slang::asInternal(session);
    return Slang::checkCompileTargetSupport(s, Slang::CodeGenTarget(target));
}

SLANG_API SlangResult spSessionCheckPassThroughSupport(
    SlangSession*       session,
    SlangPassThrough    passThrough)
{
    auto s = Slang::asInternal(session);
    return Slang::checkExternalCompilerSupport(s, Slang::PassThroughMode(passThrough));
}

SLANG_API SlangCompileRequest* spCreateCompileRequest(
    SlangSession* session)
{
    auto s = Slang::asInternal(session);
    auto req = new Slang::EndToEndCompileRequest(s);
    return asExternal(req);
}

/*!
@brief Destroy a compile request.
*/
SLANG_API void spDestroyCompileRequest(
    SlangCompileRequest*    request)
{
    if(!request) return;
    auto req = Slang::asInternal(request);

    delete req;
}

SLANG_API void spSetFileSystem(
    SlangCompileRequest*    request,
    ISlangFileSystem*       fileSystem)
{
    if(!request) return;
    Slang::asInternal(request)->getLinkage()->setFileSystem(fileSystem);
}

SLANG_API void spSetCompileFlags(
    SlangCompileRequest*    request,
    SlangCompileFlags       flags)
{
    Slang::asInternal(request)->getFrontEndReq()->compileFlags = flags;
}

SLANG_API void spSetDumpIntermediates(
    SlangCompileRequest*    request,
    int                     enable)
{
    Slang::asInternal(request)->getBackEndReq()->shouldDumpIntermediates = enable != 0;
}

SLANG_API void spSetDumpIntermediatePrefix(
    SlangCompileRequest*    request,
    const char* prefix)
{
    Slang::asInternal(request)->getBackEndReq()->m_dumpIntermediatePrefix = prefix; 
}

SLANG_API void spSetLineDirectiveMode(
    SlangCompileRequest*    request,
    SlangLineDirectiveMode  mode)
{
    // TODO: validation

    Slang::asInternal(request)->getBackEndReq()->lineDirectiveMode = Slang::LineDirectiveMode(mode);
}

SLANG_API void spSetCommandLineCompilerMode(
    SlangCompileRequest* request)
{
    Slang::asInternal(request)->isCommandLineCompile = true;

}

SLANG_API void spSetCodeGenTarget(
        SlangCompileRequest*    request,
        SlangCompileTarget target)
{
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    linkage->targets.clear();
    linkage->addTarget(Slang::CodeGenTarget(target));
}

SLANG_API int spAddCodeGenTarget(
    SlangCompileRequest*    request,
    SlangCompileTarget      target)
{
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    return (int) linkage->addTarget(Slang::CodeGenTarget(target));
}

SLANG_API void spSetTargetProfile(
    SlangCompileRequest*    request,
    int                     targetIndex,
    SlangProfileID          profile)
{
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    linkage->targets[targetIndex]->targetProfile = Slang::Profile(profile);
}

SLANG_API void spSetTargetFlags(
    SlangCompileRequest*    request,
    int                     targetIndex,
    SlangTargetFlags        flags)
{
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    linkage->targets[targetIndex]->targetFlags = flags;
}

SLANG_API void spSetTargetFloatingPointMode(
    SlangCompileRequest*    request,
    int                     targetIndex,
    SlangFloatingPointMode  mode)
{
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    linkage->targets[targetIndex]->floatingPointMode = Slang::FloatingPointMode(mode);
}

SLANG_API void spSetMatrixLayoutMode(
    SlangCompileRequest*    request,
    SlangMatrixLayoutMode   mode)
{
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    linkage->setMatrixLayoutMode(mode);
}

SLANG_API void spSetTargetMatrixLayoutMode(
    SlangCompileRequest*    request,
    int                     targetIndex,
    SlangMatrixLayoutMode   mode)
{
    SLANG_UNUSED(targetIndex);
    spSetMatrixLayoutMode(request, mode);
}

/*!
@brief Set the level of debug information to produce.
*/
SLANG_API void spSetDebugInfoLevel(
    SlangCompileRequest*    request,
    SlangDebugInfoLevel     level)
{
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    linkage->debugInfoLevel = Slang::DebugInfoLevel(level);
}

/*!
@brief Set the level of optimization to perform.
*/
SLANG_API void spSetOptimizationLevel(
    SlangCompileRequest*    request,
    SlangOptimizationLevel  level)
{
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    linkage->optimizationLevel = Slang::OptimizationLevel(level);
}


SLANG_API void spSetOutputContainerFormat(
    SlangCompileRequest*    request,
    SlangContainerFormat    format)
{
    auto req = Slang::asInternal(request);
    req->m_containerFormat = Slang::ContainerFormat(format);
}


SLANG_API void spSetPassThrough(
    SlangCompileRequest*    request,
    SlangPassThrough        passThrough)
{
    Slang::asInternal(request)->passThrough = Slang::PassThroughMode(passThrough);
}

SLANG_API void spSetDiagnosticCallback(
    SlangCompileRequest*    request,
    SlangDiagnosticCallback callback,
    void const*             userData)
{
    using namespace Slang;

    if(!request) return;
    auto req = Slang::asInternal(request);

    ComPtr<ISlangWriter> writer(new CallbackWriter(callback, userData, WriterFlag::IsConsole));
    req->setWriter(WriterChannel::Diagnostic, writer);
}

SLANG_API void spSetWriter(
    SlangCompileRequest*    request,
    SlangWriterChannel      chan, 
    ISlangWriter*           writer)
{
    if (!request) return;
    auto req = Slang::asInternal(request);

    req->setWriter(Slang::WriterChannel(chan), writer);
}

SLANG_API ISlangWriter* spGetWriter(
    SlangCompileRequest*    request,
    SlangWriterChannel      chan)
{
    if (!request) return nullptr;
    auto req = Slang::asInternal(request);
    return req->getWriter(Slang::WriterChannel(chan));
}

SLANG_API void spAddSearchPath(
    SlangCompileRequest*    request,
    const char*             path)
{
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    linkage->addSearchPath(path);
}

SLANG_API void spAddPreprocessorDefine(
    SlangCompileRequest*    request,
    const char*             key,
    const char*             value)
{
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    linkage->addPreprocessorDefine(key, value);
}

SLANG_API char const* spGetDiagnosticOutput(
    SlangCompileRequest*    request)
{
    if(!request) return 0;
    auto req = Slang::asInternal(request);
    return req->mDiagnosticOutput.begin();
}

SLANG_API SlangResult spGetDiagnosticOutputBlob(
    SlangCompileRequest*    request,
    ISlangBlob**            outBlob)
{
    if(!request) return SLANG_ERROR_INVALID_PARAMETER;
    if(!outBlob) return SLANG_ERROR_INVALID_PARAMETER;

    auto req = Slang::asInternal(request);

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
    char const*             inName)
{
    auto req = Slang::asInternal(request);
    auto frontEndReq = req->getFrontEndReq();

    Slang::NamePool* namePool = req->getFrontEndReq()->getNamePool();

    // Work out a module name. Can be nullptr if so will generate a name
    Slang::Name* moduleName = inName ? namePool->getName(inName) : frontEndReq->m_defaultModuleName;

    // If moduleName is nullptr a name will be generated
    
    return frontEndReq->addTranslationUnit(
        Slang::SourceLanguage(language),
        moduleName);
}

SLANG_API void spSetDefaultModuleName(
    SlangCompileRequest*    request,
    const char* defaultModuleName)
{
    auto req = Slang::asInternal(request);
    auto frontEndReq = req->getFrontEndReq();

    Slang::NamePool* namePool = req->getFrontEndReq()->getNamePool();

    frontEndReq->m_defaultModuleName = namePool->getName(defaultModuleName);
}

namespace Slang
{
SlangResult _addLibraryReference(EndToEndCompileRequest* req, Stream* stream)
{
    // Read all of the contained modules
    List<RefPtr<IRModule>> irModules;
    List<FrontEndCompileRequest::ExtraEntryPointInfo> entryPointMangledNames;
    if (SLANG_FAILED(IRSerialReader::readStreamModules(stream, req->getSession(), req->getFrontEndReq()->getSourceManager(), irModules, entryPointMangledNames)))
    {
        req->getSink()->diagnose(SourceLoc(), Diagnostics::unableToAddReferenceToModuleContainer);
        return SLANG_FAIL;
    }

    // TODO(JS): May be better to have a ITypeComponent that encapsulates a collection of modules
    // For now just add to the linkage
    auto linkage = req->getLinkage();
    linkage->m_libModules.addRange(irModules);

    req->getFrontEndReq()->m_extraEntryPoints.addRange(entryPointMangledNames);

    return SLANG_OK;
}
}


SLANG_API SlangResult spAddLibraryReference(
    SlangCompileRequest*    request,
    const void* libData,
    size_t libDataSize)
{
    using namespace Slang;
    auto req = Slang::asInternal(request);

    // We need to deserialize and add the modules
    MemoryStreamBase fileStream(FileAccess::Read, libData, libDataSize);

    return _addLibraryReference(req, &fileStream);
}

SLANG_API void spTranslationUnit_addPreprocessorDefine(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    const char*             key,
    const char*             value)
{
    auto req = Slang::asInternal(request);
    auto frontEndReq = req->getFrontEndReq();

    frontEndReq->translationUnits[translationUnitIndex]->preprocessorDefinitions[key] = value;
}

SLANG_API void spAddTranslationUnitSourceFile(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             path)
{
    if(!request) return;
    auto req = Slang::asInternal(request);
    auto frontEndReq = req->getFrontEndReq();
    if(!path) return;
    if(translationUnitIndex < 0) return;
    if(Slang::Index(translationUnitIndex) >= frontEndReq->translationUnits.getCount()) return;

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
    using namespace Slang;
    if(!request) return;
    auto req = Slang::asInternal(request);
    auto frontEndReq = req->getFrontEndReq();
    if(!sourceBegin) return;
    if(translationUnitIndex < 0) return;
    if(Index(translationUnitIndex) >= frontEndReq->translationUnits.getCount()) return;

    if(!path) path = "";

    frontEndReq->addTranslationUnitSourceString(
        translationUnitIndex,
        path,
        UnownedStringSlice(sourceBegin, sourceEnd));
}

SLANG_API void spAddTranslationUnitSourceBlob(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             path,
    ISlangBlob*             sourceBlob)
{
    if(!request) return;
    auto req = Slang::asInternal(request);
    auto frontEndReq = req->getFrontEndReq();
    if(!sourceBlob) return;
    if(translationUnitIndex < 0) return;
    if(Slang::Index(translationUnitIndex) >= frontEndReq->translationUnits.getCount()) return;

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
    return Slang::Profile::lookUp(name).raw;
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
    using namespace Slang;
    if (!request) return -1;
    auto req = Slang::asInternal(request);
    auto frontEndReq = req->getFrontEndReq();
    if (!name) return -1;
    if (translationUnitIndex < 0) return -1;
    if (Index(translationUnitIndex) >= frontEndReq->translationUnits.getCount()) return -1;
    List<String> typeNames;
    for (int i = 0; i < genericParamTypeNameCount; i++)
        typeNames.add(genericParamTypeNames[i]);
    return req->addEntryPoint(
        translationUnitIndex,
        name,
        Profile(Stage(stage)),
        typeNames);
}

SLANG_API SlangResult spSetGlobalGenericArgs(
    SlangCompileRequest*    request,
    int                     genericArgCount,
    char const**            genericArgs)
{
    if (!request) return SLANG_FAIL;
    auto req = Slang::asInternal(request);

    auto& argStrings = req->globalSpecializationArgStrings;
    argStrings.clear();
    for (int i = 0; i < genericArgCount; i++)
        argStrings.add(genericArgs[i]);

    return SLANG_OK;
}

SLANG_API SlangResult spSetTypeNameForGlobalExistentialTypeParam(
    SlangCompileRequest*    request,
    int                     slotIndex,
    char const*             typeName)
{
    using namespace Slang;
    if(!request)        return SLANG_FAIL;
    if(slotIndex < 0)   return SLANG_FAIL;
    if(!typeName)       return SLANG_FAIL;

    auto req = Slang::asInternal(request);
    auto& typeArgStrings = req->globalSpecializationArgStrings;
    if(Index(slotIndex) >= typeArgStrings.getCount())
        typeArgStrings.setCount(slotIndex+1);
    typeArgStrings[slotIndex] = String(typeName);
    return SLANG_OK;
}

SLANG_API SlangResult spSetTypeNameForEntryPointExistentialTypeParam(
    SlangCompileRequest*    request,
    int                     entryPointIndex,
    int                     slotIndex,
    char const*             typeName)
{
    using namespace Slang;
    if(!request)            return SLANG_FAIL;
    if(entryPointIndex < 0) return SLANG_FAIL;
    if(slotIndex < 0)       return SLANG_FAIL;
    if(!typeName)           return SLANG_FAIL;

    auto req = Slang::asInternal(request);
    if(Index(entryPointIndex) >= req->entryPoints.getCount())
        return SLANG_FAIL;

    auto& entryPointInfo = req->entryPoints[entryPointIndex];
    auto& typeArgStrings = entryPointInfo.specializationArgStrings;
    if(Index(slotIndex) >= typeArgStrings.getCount())
        typeArgStrings.setCount(slotIndex+1);
    typeArgStrings[slotIndex] = String(typeName);
    return SLANG_OK;
}

// Compile in a context that already has its translation units specified
SLANG_API SlangResult spCompile(
    SlangCompileRequest*    request)
{
    using namespace Slang;
    auto req = asInternal(request);

    SlangResult res = SLANG_FAIL;

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
    
    try
    {
        res = req->executeActions();
    }
    catch (const AbortCompilationException&)
    {
        // This situation indicates a fatal (but not necessarily internal) error
        // that forced compilation to terminate. There should already have been
        // a diagnostic produced, so we don't need to add one here.
    }
    catch (const Exception& e)
    {
        // The compiler failed due to an internal error that was detected.
        // We will print out information on the exception to help out the user
        // in either filing a bug, or locating what in their code created
        // a problem.
        req->getSink()->diagnose(SourceLoc(), Diagnostics::compilationAbortedDueToException, typeid(e).name(), e.Message);
    }
    catch (...)
    {
        // The compiler failed due to some exception that wasn't a sublass of
        // `Exception`, so something really fishy is going on. We want to
        // let the user know that we messed up, so they know to blame Slang
        // and not some other component in their system.
        req->getSink()->diagnose(SourceLoc(), Diagnostics::compilationAborted);
    }
    req->mDiagnosticOutput = req->getSink()->outputBuffer.ProduceString();
    
#else
    // When debugging, we probably don't want to filter out any errors, since
    // we are probably trying to root-cause and *fix* those errors.
    {
        res = req->executeActions();
    }
#endif

    // Repro dump handling
    {
        if (req->dumpRepro.getLength())
        {
            SlangResult saveRes = ReproUtil::saveState(req, req->dumpRepro);
            if (SLANG_FAILED(saveRes))
            {
                req->getSink()->diagnose(SourceLoc(), Diagnostics::unableToWriteReproFile, req->dumpRepro);
                return saveRes;
            }
        }
        else if (req->dumpReproOnError && SLANG_FAILED(res))
        {
            String reproFileName;
            SlangResult saveRes = SLANG_FAIL;

            RefPtr<Stream> stream;
            if (SLANG_SUCCEEDED(ReproUtil::findUniqueReproDumpStream(req, reproFileName, stream)))
            {
                saveRes = ReproUtil::saveState(req, stream);
            }

            if (SLANG_FAILED(saveRes))
            {
                req->getSink()->diagnose(SourceLoc(), Diagnostics::unableToWriteReproFile, reproFileName);
            }
        }
    }

    return res;
}

SLANG_API int
spGetDependencyFileCount(
    SlangCompileRequest*    request)
{
    if(!request) return 0;
    auto req = Slang::asInternal(request);
    auto frontEndReq = req->getFrontEndReq();
    auto program = frontEndReq->getGlobalAndEntryPointsComponentType();
    return (int) program->getFilePathDependencies().getCount();
}

/** Get the path to a file this compilation dependend on.
*/
SLANG_API char const*
spGetDependencyFilePath(
    SlangCompileRequest*    request,
    int                     index)
{
    if(!request) return 0;
    auto req = Slang::asInternal(request);
    auto frontEndReq = req->getFrontEndReq();
    auto program = frontEndReq->getGlobalAndEntryPointsComponentType();
    return program->getFilePathDependencies()[index].begin();
}

SLANG_API int
spGetTranslationUnitCount(
    SlangCompileRequest*    request)
{
    auto req = Slang::asInternal(request);
    auto frontEndReq = req->getFrontEndReq();
    return (int) frontEndReq->translationUnits.getCount();
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
    using namespace Slang;
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    auto program = req->getSpecializedGlobalAndEntryPointsComponentType();

    // TODO: We should really accept a target index in this API
    Index targetIndex = 0;
    auto targetCount = linkage->targets.getCount();
    if (targetIndex >= targetCount)
        return nullptr;
    auto targetReq = linkage->targets[targetIndex];


    if(entryPointIndex < 0) return nullptr;
    if(Index(entryPointIndex) >= program->getEntryPointCount()) return nullptr;
    auto entryPoint = program->getEntryPoint(entryPointIndex);

    auto targetProgram = program->getTargetProgram(targetReq);
    if(!targetProgram)
        return nullptr;
    CompileResult& result = targetProgram->getExistingEntryPointResult(entryPointIndex);

    void const* data = nullptr;
    size_t size = 0;

    ComPtr<ISlangBlob> blob;
    if (SLANG_SUCCEEDED(result.getBlob(blob)))
    {
        data = blob->getBufferPointer();
        size = blob->getBufferSize();
    }

    if (outSize)
    {
        *outSize = size;
    }

    return data;
}

static SlangResult _getEntryPointResult(
    SlangCompileRequest*    request,
    int                     entryPointIndex,
    int                     targetIndex,
    Slang::CompileResult**  outCompileResult)
{
    using namespace Slang;
    if (!request) return SLANG_ERROR_INVALID_PARAMETER;
    
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    auto program = req->getSpecializedGlobalAndEntryPointsComponentType();

    Index targetCount = linkage->targets.getCount();
    if ((targetIndex < 0) || (targetIndex >= targetCount))
    {
        return SLANG_ERROR_INVALID_PARAMETER;
    }
    auto targetReq = linkage->targets[targetIndex];

    Index entryPointCount = req->entryPoints.getCount();
    if ((entryPointIndex < 0) || (entryPointIndex >= entryPointCount))
    {
        return SLANG_ERROR_INVALID_PARAMETER;
    }
    auto entryPointReq = program->getEntryPoint(entryPointIndex);


    auto targetProgram = program->getTargetProgram(targetReq);
    if (!targetProgram)
        return SLANG_FAIL;
    *outCompileResult = &targetProgram->getExistingEntryPointResult(entryPointIndex);
    return SLANG_OK;
}

SLANG_API SlangResult spGetEntryPointCodeBlob(
        SlangCompileRequest*    request,
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangBlob**            outBlob)
{
    using namespace Slang;
    if(!outBlob) return SLANG_ERROR_INVALID_PARAMETER;
    Slang::CompileResult* compileResult = nullptr;
    SLANG_RETURN_ON_FAIL(_getEntryPointResult(request, entryPointIndex, targetIndex, &compileResult));

    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(compileResult->getBlob(blob));
    *outBlob = blob.detach();
    return SLANG_OK;
}

SLANG_API SlangResult spGetEntryPointHostCallable(
    SlangCompileRequest*    request,
    int                     entryPointIndex,
    int                     targetIndex,
    ISlangSharedLibrary**   outSharedLibrary)
{
    using namespace Slang;
    if (!outSharedLibrary) return SLANG_ERROR_INVALID_PARAMETER;

    Slang::CompileResult* compileResult = nullptr;
    SLANG_RETURN_ON_FAIL(_getEntryPointResult(request, entryPointIndex, targetIndex, &compileResult));

    ComPtr<ISlangSharedLibrary> sharedLibrary;
    SLANG_RETURN_ON_FAIL(compileResult->getSharedLibrary(sharedLibrary));
    *outSharedLibrary = sharedLibrary.detach();
    return SLANG_OK;
}

SLANG_API char const* spGetEntryPointSource(
    SlangCompileRequest*    request,
    int                     entryPointIndex)
{
    return (char const*) spGetEntryPointCode(request, entryPointIndex, nullptr);
}

SLANG_API void const* spGetCompileRequestCode(
    SlangCompileRequest*    inRequest,
    size_t*                 outSize)
{
    using namespace Slang;
    auto request = asInternal(inRequest);

    if (request->m_containerBlob)
    {
        *outSize = request->m_containerBlob->getBufferSize();
        return request->m_containerBlob->getBufferPointer();
    }

    // Container blob does not have any contents
    *outSize = 0;
    return nullptr;
}

SLANG_API SlangResult spGetContainerCode(
    SlangCompileRequest*    inRequest,
    ISlangBlob**            outBlob)
{
    using namespace Slang;
    auto request = asInternal(inRequest);

    ISlangBlob* containerBlob = request->m_containerBlob;
    if (containerBlob)
    {
        containerBlob->addRef();
        *outBlob = containerBlob;
        return SLANG_OK;
    }

    return SLANG_FAIL;
}

SLANG_API SlangResult spLoadRepro(
    SlangCompileRequest* inRequest,
    ISlangFileSystem* fileSystem,
    const void* data,
    size_t size)
{
    using namespace Slang;
    auto request = asInternal(inRequest);

    List<uint8_t> buffer;
    SLANG_RETURN_ON_FAIL(ReproUtil::loadState((const uint8_t*)data, size, buffer));

    MemoryOffsetBase base;
    base.set(buffer.getBuffer(), buffer.getCount());

    ReproUtil::RequestState* requestState = ReproUtil::getRequest(buffer);

    SLANG_RETURN_ON_FAIL(ReproUtil::load(base, requestState, fileSystem, request));
    return SLANG_OK;
}

SLANG_API SlangResult spSaveRepro(
    SlangCompileRequest* inRequest,
    ISlangBlob** outBlob)
{
    using namespace Slang;
    auto request = asInternal(inRequest);

    OwnedMemoryStream stream(FileAccess::Write);

    SLANG_RETURN_ON_FAIL(ReproUtil::saveState(request, &stream));

    RefPtr<ListBlob> listBlob(new ListBlob);

    // Put the content of the stream in the blob
    stream.swapContents(listBlob->m_data);

    *outBlob = listBlob.detach();
    return SLANG_OK;
}

SLANG_API SlangResult spEnableReproCapture(
    SlangCompileRequest* inRequest)
{
    using namespace Slang;
    auto request = asInternal(inRequest);

    request->getLinkage()->setRequireCacheFileSystem(true);
    return SLANG_OK;
}

SLANG_API SlangResult spExtractRepro(SlangSession* session, const void* reproData, size_t reproDataSize, ISlangFileSystemExt* fileSystem)
{
    using namespace Slang;
    SLANG_UNUSED(session);

    List<uint8_t> buffer;
    {
        MemoryStreamBase memoryStream(FileAccess::Read, reproData, reproDataSize);
        SLANG_RETURN_ON_FAIL(ReproUtil::loadState(&memoryStream, buffer));
    }

    MemoryOffsetBase base;
    base.set(buffer.getBuffer(), buffer.getCount());

    ReproUtil::RequestState* requestState = ReproUtil::getRequest(buffer);
    return ReproUtil::extractFiles(base, requestState, fileSystem);
}

SLANG_API SlangResult spLoadReproAsFileSystem(
    SlangSession* session,
    const void* reproData,
    size_t reproDataSize,
    ISlangFileSystem* replaceFileSystem,
    ISlangFileSystemExt** outFileSystem)
{
    using namespace Slang;

    SLANG_UNUSED(session);
    
    MemoryStreamBase stream(FileAccess::Read, reproData, reproDataSize);

    List<uint8_t> buffer;
    SLANG_RETURN_ON_FAIL(ReproUtil::loadState(&stream, buffer));

    auto requestState = ReproUtil::getRequest(buffer);
    MemoryOffsetBase base;
    base.set(buffer.getBuffer(), buffer.getCount());

    RefPtr<CacheFileSystem> cacheFileSystem;
    SLANG_RETURN_ON_FAIL(ReproUtil::loadFileSystem(base, requestState, replaceFileSystem, cacheFileSystem));

    *outFileSystem = cacheFileSystem.detach();
    return SLANG_OK;
}

// Reflection API

SLANG_API SlangResult spCompileRequest_getProgram(
    SlangCompileRequest*    request,
    slang::IComponentType** outProgram)
{
    if( !request ) return SLANG_ERROR_INVALID_PARAMETER;
    auto req = Slang::asInternal(request);
    auto program = req->getSpecializedGlobalComponentType();

    *outProgram = Slang::ComPtr<slang::IComponentType>(program).detach();
    return SLANG_OK;
}

SLANG_API SlangResult spCompileRequest_getModule(
    SlangCompileRequest*    request,
    SlangInt                translationUnitIndex,
    slang::IModule**        outModule)
{
    if( !request ) return SLANG_ERROR_INVALID_PARAMETER;
    auto req = Slang::asInternal(request);

    auto module = req->getFrontEndReq()->getTranslationUnit(translationUnitIndex)->getModule();

    *outModule = Slang::ComPtr<slang::IModule>(module).detach();
    return SLANG_OK;
}

SLANG_API SlangResult spCompileRequest_getEntryPoint(
    SlangCompileRequest*    request,
    SlangInt                entryPointIndex,
    slang::IComponentType** outEntryPoint)
{
    if( !request ) return SLANG_ERROR_INVALID_PARAMETER;
    auto req = Slang::asInternal(request);

    auto entryPoint = req->getSpecializedEntryPointComponentType(entryPointIndex);

    *outEntryPoint = Slang::ComPtr<slang::IComponentType>(entryPoint).detach();
    return SLANG_OK;
}


SLANG_API SlangReflection* spGetReflection(
    SlangCompileRequest*    request)
{
    if( !request ) return 0;
    auto req = Slang::asInternal(request);
    auto linkage = req->getLinkage();
    auto program = req->getSpecializedGlobalAndEntryPointsComponentType();

    // Note(tfoley): The API signature doesn't let the client
    // specify which target they want to access reflection
    // information for, so for now we default to the first one.
    //
    // TODO: Add a new `spGetReflectionForTarget(req, targetIndex)`
    // so that we can do this better, and make it clear that
    // `spGetReflection()` is shorthand for `targetIndex == 0`.
    //
    Slang::Index targetIndex = 0;
    auto targetCount = linkage->targets.getCount();
    if (targetIndex >= targetCount)
        return nullptr;

    auto targetReq = linkage->targets[targetIndex];
    auto targetProgram = program->getTargetProgram(targetReq);
    auto programLayout = targetProgram->getExistingLayout();

    return (SlangReflection*) programLayout;
}

// ... rest of reflection API implementation is in `Reflection.cpp`
