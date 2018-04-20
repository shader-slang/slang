#include "../../slang.h"

#include "../core/slang-io.h"
#include "parameter-binding.h"
#include "lower-to-ir.h"
#include "../slang/parser.h"
#include "../slang/preprocessor.h"
#include "../slang/reflection.h"
#include "syntax-visitors.h"
#include "../slang/type-layout.h"

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
    builtinSourceManager.initialize(nullptr);

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
    CompileRequest* request;

    virtual IncludeResult TryToFindIncludeFile(
        String const& pathToInclude,
        String const& pathIncludedFrom,
        String* outFoundPath,
        String* outFoundSource) override
    {
        String path = Path::Combine(Path::GetDirectoryName(pathIncludedFrom), pathToInclude);
        if (File::Exists(path))
        {
            *outFoundPath = path;
            *outFoundSource = File::ReadAllText(path);

            request->mDependencyFilePaths.Add(path);

            return IncludeResult::Found;
        }

        for (auto & dir : request->searchDirectories)
        {
            path = Path::Combine(dir.path, pathToInclude);
            if (File::Exists(path))
            {
                *outFoundPath = path;
                *outFoundSource = File::ReadAllText(path);

                request->mDependencyFilePaths.Add(path);

                return IncludeResult::Found;
            }
        }
        return IncludeResult::NotFound;
    }
};

//


Profile getEffectiveProfile(EntryPointRequest* entryPoint, TargetRequest* target)
{
    auto entryPointProfile = entryPoint->profile;
    auto targetProfile = target->targetProfile;

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
            stageMinVersion = ProfileVersion::DX_6_1;
            break;

        //  TODO: Add equivalent logic for geometry, tessellation, and compute stages.
        }
        break;

    // TODO: Add equivalent logic for the GL/VK case.

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

CompileRequest::CompileRequest(Session* session)
    : mSession(session)
{
    getNamePool()->setRootNamePool(session->getRootNamePool());

    setSourceManager(&sourceManagerStorage);

    sourceManager->initialize(session->getBuiltinSourceManager());
}

CompileRequest::~CompileRequest()
{
    // delete things that may reference IR objects first
    targets = decltype(targets)();
    translationUnits = decltype(translationUnits)();
    entryPoints = decltype(entryPoints)();
    types = decltype(types)();
}


RefPtr<Expr> CompileRequest::parseTypeString(TranslationUnitRequest * translationUnit, String typeStr, RefPtr<Scope> scope)
{
    Slang::SourceFile srcFile;
    srcFile.content = typeStr;
    DiagnosticSink sink;
    sink.sourceManager = sourceManager;
    auto tokens = preprocessSource(
        &srcFile,
        &sink,
        nullptr,
        Dictionary<String,String>(),
        translationUnit);
    return parseTypeFromSourceFile(translationUnit, tokens, &sink, scope);
}

RefPtr<Type> checkProperType(TranslationUnitRequest * tu, TypeExp typeExp);
Type* CompileRequest::getTypeFromString(String typeStr)
{
    RefPtr<Type> type;
    if (types.TryGetValue(typeStr, type))
        return type;
    auto translationUnit = translationUnits.First();
    List<RefPtr<Scope>> scopesToTry;
    for (auto tu : translationUnits)
        scopesToTry.Add(tu->SyntaxNode->scope);
    for (auto & module : loadedModulesList)
        scopesToTry.Add(module->moduleDecl->scope);
    // parse type name
    for (auto & s : scopesToTry)
    {
        RefPtr<Expr> typeExpr = parseTypeString(translationUnit,
            typeStr, s);
        type = checkProperType(translationUnit, TypeExp(typeExpr));
        if (type)
            break;
    }
    if (type)
    {
        types[typeStr] = type;
    }
    return type.Ptr();
}

void CompileRequest::parseTranslationUnit(
    TranslationUnitRequest* translationUnit)
{
    IncludeHandlerImpl includeHandler;
    includeHandler.request = this;

    RefPtr<Scope> languageScope;
    switch (translationUnit->sourceLanguage)
    {
    case SourceLanguage::HLSL:
        languageScope = mSession->hlslLanguageScope;
        break;

    case SourceLanguage::Slang:
    default:
        languageScope = mSession->slangLanguageScope;
        break;
    }

    Dictionary<String, String> combinedPreprocessorDefinitions;
    for(auto& def : preprocessorDefinitions)
        combinedPreprocessorDefinitions.Add(def.Key, def.Value);
    for(auto& def : translationUnit->preprocessorDefinitions)
        combinedPreprocessorDefinitions.Add(def.Key, def.Value);

    RefPtr<ModuleDecl> translationUnitSyntax = new ModuleDecl();
    translationUnit->SyntaxNode = translationUnitSyntax;

    for (auto sourceFile : translationUnit->sourceFiles)
    {
        auto tokens = preprocessSource(
            sourceFile,
            &mSink,
            &includeHandler,
            combinedPreprocessorDefinitions,
            translationUnit);

        parseSourceFile(
            translationUnit,
            tokens,
            &mSink,
            languageScope);
    }
}

void validateEntryPoints(CompileRequest*);

void CompileRequest::checkAllTranslationUnits()
{
    // Iterate over all translation units and
    // apply the semantic checking logic.
    for( auto& translationUnit : translationUnits )
    {
        checkTranslationUnit(translationUnit.Ptr());
    }

    // Next, do follow-up validation on any entry points.
    validateEntryPoints(this);
}

void CompileRequest::generateIR()
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
        translationUnit->irModule = generateIRForTranslationUnit(translationUnit);
    }
}

// Try to infer a single common source language for a request
static SourceLanguage inferSourceLanguage(CompileRequest* request)
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

int CompileRequest::executeActionsInner()
{
    // Do some cleanup on settings specified by user.
    // In particular, we want to propagate flags from the overall request down to
    // each translation unit.
    for (auto& translationUnit : translationUnits)
    {
        translationUnit->compileFlags |= compileFlags;
    }

    // If no code-generation target was specified, then try to infer one from the source language,
    // just to make sure we can do something reasonable when invoked from the command line.
    if (targets.Count() == 0)
    {
        auto language = inferSourceLanguage(this);
        switch (language)
        {
        case SourceLanguage::HLSL:
            addTarget(CodeGenTarget::DXBytecode);
            break;

        case SourceLanguage::GLSL:
            addTarget(CodeGenTarget::SPIRV);
            break;

        default:
            break;
        }
    }

    // We only do parsing and semantic checking if we *aren't* doing
    // a pass-through compilation.
    //
    // Note that we *do* perform output generation as normal in pass-through mode.
    if (passThrough == PassThroughMode::None)
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
                mSink.diagnose(SourceLoc(), Diagnostics::glslIsNotSupported);
                return 1;
            }
        }


        // Parse everything from the input files requested
        for (auto& translationUnit : translationUnits)
        {
            parseTranslationUnit(translationUnit.Ptr());
        }
        if (mSink.GetErrorCount() != 0)
            return 1;

        // Perform semantic checking on the whole collection
        checkAllTranslationUnits();
        if (mSink.GetErrorCount() != 0)
            return 1;

        if ((compileFlags & SLANG_COMPILE_FLAG_NO_CODEGEN) == 0)
        {
            // Generate initial IR for all the translation
            // units, if we are in a mode where IR is called for.
            generateIR();
        }

        if (mSink.GetErrorCount() != 0)
            return 1;

        // For each code generation target generate
        // parameter binding information.
        // This step is done globaly, because all translation
        // units and entry points need to agree on where
        // parameters are allocated.
        for (auto targetReq : targets)
        {
            generateParameterBindings(targetReq);
            if (mSink.GetErrorCount() != 0)
                return 1;
        }
    }

    // If command line specifies to skip codegen, we exit here.
    // Note: this is a debugging option.
    if (shouldSkipCodegen ||
        ((compileFlags & SLANG_COMPILE_FLAG_NO_CODEGEN) != 0))
        return 0;

    // Generate output code, in whatever format was requested
    generateOutput(this);
    if (mSink.GetErrorCount() != 0)
        return 1;

    return 0;
}

// Act as expected of the API-based compiler
int CompileRequest::executeActions()
{
    int err = executeActionsInner();

    mDiagnosticOutput = mSink.outputBuffer.ProduceString();

    return err;
}

int CompileRequest::addTranslationUnit(SourceLanguage language, String const&)
{
    UInt result = translationUnits.Count();

    RefPtr<TranslationUnitRequest> translationUnit = new TranslationUnitRequest();
    translationUnit->compileRequest = this;
    translationUnit->sourceLanguage = SourceLanguage(language);

    translationUnits.Add(translationUnit);

    return (int) result;
}

void CompileRequest::addTranslationUnitSourceFile(
    int             translationUnitIndex,
    SourceFile*     sourceFile)
{
    translationUnits[translationUnitIndex]->sourceFiles.Add(sourceFile);
}

void CompileRequest::addTranslationUnitSourceString(
    int             translationUnitIndex,
    String const&   path,
    String const&   source)
{
    RefPtr<SourceFile> sourceFile = getSourceManager()->allocateSourceFile(path, source);

    addTranslationUnitSourceFile(translationUnitIndex, sourceFile);
}

void CompileRequest::addTranslationUnitSourceFile(
    int             translationUnitIndex,
    String const&   path)
{
    String source;
    try
    {
        source = File::ReadAllText(path);
    }
    catch (...)
    {
        // Emit a diagnostic!
        mSink.diagnose(
            SourceLoc(),
            Diagnostics::cannotOpenFile,
            path);
        return;
    }

    addTranslationUnitSourceString(
        translationUnitIndex,
        path,
        source);

    mDependencyFilePaths.Add(path);
}

int CompileRequest::addEntryPoint(
    int                     translationUnitIndex,
    String const&           name,
    Profile                 entryPointProfile,
    List<String> const &    genericTypeNames)
{
    RefPtr<EntryPointRequest> entryPoint = new EntryPointRequest();
    entryPoint->compileRequest = this;
    entryPoint->name = getNamePool()->getName(name);
    entryPoint->profile = entryPointProfile;
    entryPoint->translationUnitIndex = translationUnitIndex;
    for (auto typeName : genericTypeNames)
        entryPoint->genericParameterTypeNames.Add(typeName);
    auto translationUnit = translationUnits[translationUnitIndex].Ptr();
    translationUnit->entryPoints.Add(entryPoint);

    UInt result = entryPoints.Count();
    entryPoints.Add(entryPoint);
    return (int) result;
}

UInt CompileRequest::addTarget(
    CodeGenTarget   target)
{
    RefPtr<TargetRequest> targetReq = new TargetRequest();
    targetReq->compileRequest = this;
    targetReq->target = target;

    UInt result = targets.Count();
    targets.Add(targetReq);
    return (int) result;
}

void CompileRequest::loadParsedModule(
    RefPtr<TranslationUnitRequest> const&   translationUnit,
    Name*                                   name,
    String const&                           path)
{
    // Note: we add the loaded module to our name->module listing
    // before doing semantic checking, so that if it tries to
    // recursively `import` itself, we can detect it.
    RefPtr<LoadedModule> loadedModule = new LoadedModule();
    mapPathToLoadedModule.Add(path, loadedModule);
    mapNameToLoadedModules.Add(name, loadedModule);

    int errorCountBefore = mSink.GetErrorCount();
    checkTranslationUnit(translationUnit.Ptr());
    int errorCountAfter = mSink.GetErrorCount();

    RefPtr<ModuleDecl> moduleDecl = translationUnit->SyntaxNode;
    loadedModule->moduleDecl = moduleDecl;

    if (errorCountAfter != errorCountBefore)
    {
        // There must have been an error in the loaded module.
    }
    else
    {
        // If we didn't run into any errors, then try to generate
        // IR code for the imported module.
        SLANG_ASSERT(errorCountAfter == 0);
        loadedModule->irModule = generateIRForTranslationUnit(translationUnit);
    }
    loadedModulesList.Add(loadedModule);
}

RefPtr<ModuleDecl> CompileRequest::loadModule(
    Name*               name,
    String const&       path,
    String const&       source,
    SourceLoc const&    srcLoc)
{
    RefPtr<TranslationUnitRequest> translationUnit = new TranslationUnitRequest();
    translationUnit->compileRequest = this;

    // We don't want to use the same options that the user specified
    // for loading modules on-demand. In particular, we always want
    // semantic checking to be enabled.
    //
    // TODO: decide which options, if any, should be inherited.
    translationUnit->compileFlags = 0;

    RefPtr<SourceFile> sourceFile = getSourceManager()->allocateSourceFile(path, source);

    translationUnit->sourceFiles.Add(sourceFile);


    int errorCountBefore = mSink.GetErrorCount();
    parseTranslationUnit(translationUnit.Ptr());
    int errorCountAfter = mSink.GetErrorCount();

    if( errorCountAfter != errorCountBefore )
    {
        mSink.diagnose(srcLoc, Diagnostics::errorInImportedModule);
        // Something went wrong during the parsing, so we should bail out.
        return nullptr;
    }

    loadParsedModule(
        translationUnit,
        name,
        path);

    errorCountAfter = mSink.GetErrorCount();

    if (errorCountAfter != errorCountBefore)
    {
        mSink.diagnose(srcLoc, Diagnostics::errorInImportedModule);
        // Something went wrong during the parsing, so we should bail out.
        return nullptr;
    }

    return translationUnit->SyntaxNode;
}

RefPtr<ModuleDecl> CompileRequest::findOrImportModule(
    Name*               name,
    SourceLoc const&    loc)
{
    // Have we already loaded a module matching this name?
    // If so, return it.
    RefPtr<LoadedModule> loadedModule;
    if (mapNameToLoadedModules.TryGetValue(name, loadedModule))
    {
        if (!loadedModule)
            return nullptr;

        if (!loadedModule->moduleDecl)
        {
            // We seem to be in the middle of loading this module
            mSink.diagnose(loc, Diagnostics::recursiveModuleImport, name);
            return nullptr;
        }

        return loadedModule->moduleDecl;
    }

    // Derive a file name for the module, by taking the given
    // identifier, replacing all occurences of `_` with `-`,
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
    includeHandler.request = this;

    auto expandedLoc = getSourceManager()->expandSourceLoc(loc);

    String pathIncludedFrom = expandedLoc.getSpellingPath();

    String foundPath;
    String foundSource;
    IncludeResult includeResult = includeHandler.TryToFindIncludeFile(fileName, pathIncludedFrom, &foundPath, &foundSource);
    switch( includeResult )
    {
    case IncludeResult::NotFound:
    case IncludeResult::Error:
        {
            this->mSink.diagnose(loc, Diagnostics::cannotFindFile, fileName);

            mapNameToLoadedModules[name] = nullptr;
            return nullptr;
        }
        break;

    default:
        break;
    }

    // Maybe this was loaded previously at a different relative name?
    if (mapPathToLoadedModule.TryGetValue(foundPath, loadedModule))
        return loadedModule->moduleDecl;


    // We've found a file that we can load for the given module, so
    // go ahead and perform the module-load action
    return loadModule(
        name,
        foundPath,
        foundSource,
        loc);
}

Decl * CompileRequest::lookupGlobalDecl(Name * name)
{
    Decl* resultDecl = nullptr;
    for (auto module : loadedModulesList)
    {
        if (module->moduleDecl->memberDictionary.TryGetValue(name, resultDecl))
            break;
    }
    for (auto transUnit : translationUnits)
    {
        if (transUnit->SyntaxNode->memberDictionary.TryGetValue(name, resultDecl))
            break;
    }
    return resultDecl;
}

RefPtr<ModuleDecl> findOrImportModule(
    CompileRequest*     request,
    Name*               name,
    SourceLoc const&    loc)
{
    return request->findOrImportModule(name, loc);
}

void Session::addBuiltinSource(
    RefPtr<Scope> const&    scope,
    String const&           path,
    String const&           source)
{
    RefPtr<CompileRequest> compileRequest = new CompileRequest(this);
    compileRequest->setSourceManager(getBuiltinSourceManager());

    auto translationUnitIndex = compileRequest->addTranslationUnit(SourceLanguage::Slang, path);

    RefPtr<SourceFile> sourceFile = builtinSourceManager.allocateSourceFile(path, source);

    compileRequest->addTranslationUnitSourceString(
        translationUnitIndex,
        path,
        source);

    int err = compileRequest->executeActions();
    if (err)
    {
        fprintf(stderr, "%s", compileRequest->mDiagnosticOutput.Buffer());

#ifdef _WIN32
        OutputDebugStringA(compileRequest->mDiagnosticOutput.Buffer());
#endif

        SLANG_UNEXPECTED("error in Slang standard library");
    }

    // Extract the AST for the code we just parsed
    auto syntax = compileRequest->translationUnits[translationUnitIndex]->SyntaxNode;

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

    builtinTypes = decltype(builtinTypes)();
    // destroy modules next
    loadedModuleCode = decltype(loadedModuleCode)();
}

}

// implementation of C interface

#define SESSION(x) reinterpret_cast<Slang::Session *>(x)
#define REQ(x) reinterpret_cast<Slang::CompileRequest*>(x)

SLANG_API SlangSession* spCreateSession(const char*)
{
    return reinterpret_cast<SlangSession *>(new Slang::Session());
}

SLANG_API void spDestroySession(
    SlangSession*   session)
{
    if(!session) return;
    delete SESSION(session);
#ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
#endif
}

SLANG_API void spAddBuiltins(
    SlangSession*   session,
    char const*     sourcePath,
    char const*     sourceString)
{
    auto s = SESSION(session);
    s->addBuiltinSource(

        // TODO(tfoley): Add ability to directly new builtins to the approriate scope
        s->coreLanguageScope,

        sourcePath,
        sourceString);
}


SLANG_API SlangCompileRequest* spCreateCompileRequest(
    SlangSession* session)
{
    auto s = SESSION(session);
    auto req = new Slang::CompileRequest(s);
    return reinterpret_cast<SlangCompileRequest*>(req);
}

/*!
@brief Destroy a compile request.
*/
SLANG_API void spDestroyCompileRequest(
    SlangCompileRequest*    request)
{
    if(!request) return;
    auto req = REQ(request);
    delete req;
}

SLANG_API void spSetCompileFlags(
    SlangCompileRequest*    request,
    SlangCompileFlags       flags)
{
    REQ(request)->compileFlags = flags;
}

SLANG_API void spSetDumpIntermediates(
    SlangCompileRequest*    request,
    int                     enable)
{
    REQ(request)->shouldDumpIntermediates = enable != 0;
}

SLANG_API void spSetLineDirectiveMode(
    SlangCompileRequest*    request,
    SlangLineDirectiveMode  mode)
{
    // TODO: validation

    REQ(request)->lineDirectiveMode = Slang::LineDirectiveMode(mode);
}

SLANG_API void spSetCommandLineCompilerMode(
    SlangCompileRequest* request)
{
    REQ(request)->isCommandLineCompile = true;

}

SLANG_API void spSetCodeGenTarget(
        SlangCompileRequest*    request,
        int target)
{
    auto req = REQ(request);
    req->targets.Clear();
    req->addTarget(Slang::CodeGenTarget(target));
}

SLANG_API int spAddCodeGenTarget(
    SlangCompileRequest*    request,
    SlangCompileTarget      target)
{
    auto req = REQ(request);
    return (int) req->addTarget(Slang::CodeGenTarget(target));
}

SLANG_API void spSetTargetProfile(
    SlangCompileRequest*    request,
    int                     targetIndex,
    SlangProfileID          profile)
{
    auto req = REQ(request);
    req->targets[targetIndex]->targetProfile = profile;
}

SLANG_API void spSetTargetFlags(
    SlangCompileRequest*    request,
    int                     targetIndex,
    SlangTargetFlags        flags)
{
    auto req = REQ(request);
    req->targets[targetIndex]->targetFlags = flags;
}

SLANG_API void spSetOutputContainerFormat(
    SlangCompileRequest*    request,
    SlangContainerFormat    format)
{
    auto req = REQ(request);
    req->containerFormat = Slang::ContainerFormat(format);
}


SLANG_API void spSetPassThrough(
    SlangCompileRequest*    request,
    SlangPassThrough        passThrough)
{
    REQ(request)->passThrough = Slang::PassThroughMode(passThrough);
}

SLANG_API void spSetDiagnosticCallback(
    SlangCompileRequest*    request,
    SlangDiagnosticCallback callback,
    void const*             userData)
{
    if(!request) return;
    auto req = REQ(request);

    req->mSink.callback = callback;
    req->mSink.callbackUserData = (void*) userData;
}

SLANG_API void spAddSearchPath(
        SlangCompileRequest*    request,
        const char*             path)
{
    REQ(request)->searchDirectories.Add(Slang::SearchDirectory(path));
}

SLANG_API void spAddPreprocessorDefine(
    SlangCompileRequest*    request,
    const char*             key,
    const char*             value)
{
    REQ(request)->preprocessorDefinitions[key] = value;
}

SLANG_API char const* spGetDiagnosticOutput(
    SlangCompileRequest*    request)
{
    if(!request) return 0;
    auto req = REQ(request);
    return req->mDiagnosticOutput.begin();
}

// New-fangled compilation API

SLANG_API int spAddTranslationUnit(
    SlangCompileRequest*    request,
    SlangSourceLanguage     language,
    char const*             name)
{
    auto req = REQ(request);

    return req->addTranslationUnit(
        Slang::SourceLanguage(language),
        name ? name : "");
}

SLANG_API void spTranslationUnit_addPreprocessorDefine(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    const char*             key,
    const char*             value)
{
    auto req = REQ(request);

    req->translationUnits[translationUnitIndex]->preprocessorDefinitions[key] = value;

}

SLANG_API void spAddTranslationUnitSourceFile(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             path)
{
    if(!request) return;
    auto req = REQ(request);
    if(!path) return;
    if(translationUnitIndex < 0) return;
    if(Slang::UInt(translationUnitIndex) >= req->translationUnits.Count()) return;

    req->addTranslationUnitSourceFile(
        translationUnitIndex,
        path);
}

// Add a source string to the given translation unit
SLANG_API void spAddTranslationUnitSourceString(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             path,
    char const*             source)
{
    if(!request) return;
    auto req = REQ(request);
    if(!source) return;
    if(translationUnitIndex < 0) return;
    if(Slang::UInt(translationUnitIndex) >= req->translationUnits.Count()) return;

    if(!path) path = "";

    req->addTranslationUnitSourceString(
        translationUnitIndex,
        path,
        source);

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
    SlangProfileID          profile)
{
    if(!request) return -1;
    auto req = REQ(request);
    if(!name) return -1;
    if(translationUnitIndex < 0) return -1;
    if(Slang::UInt(translationUnitIndex) >= req->translationUnits.Count()) return -1;

    return req->addEntryPoint(
        translationUnitIndex,
        name,
        Slang::Profile(Slang::Profile::RawVal(profile)),
        Slang::List<Slang::String>());
}

SLANG_API int spAddEntryPointEx(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             name,
    SlangProfileID          profile,
    int                     genericParamTypeNameCount,
    char const **           genericParamTypeNames)
{
    if (!request) return -1;
    auto req = REQ(request);
    if (!name) return -1;
    if (translationUnitIndex < 0) return -1;
    if (Slang::UInt(translationUnitIndex) >= req->translationUnits.Count()) return -1;
    Slang::List<Slang::String> typeNames;
    for (int i = 0; i < genericParamTypeNameCount; i++)
        typeNames.Add(genericParamTypeNames[i]);
    return req->addEntryPoint(
        translationUnitIndex,
        name,
        Slang::Profile(Slang::Profile::RawVal(profile)),
        typeNames);
}


// Compile in a context that already has its translation units specified
SLANG_API int spCompile(
    SlangCompileRequest*    request)
{
    auto req = REQ(request);

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
    int anyErrors = 1;
    try
    {
        anyErrors = req->executeActions();
    }
    catch (Slang::AbortCompilationException&)
    {
        // This situation indicates a fatal (but not necesarily internal) error
        // that forced compilation to terminate. There should already have been
        // a diagnositc produced, so we don't need to add one here.
    }
    catch (Slang::Exception& e)
    {
        // The compiler failed due to an internal error that was detected.
        // We will print out information on the exception to help out the user
        // in either filing a bug, or locating what in their code created
        // a problem.
        req->mSink.diagnose(Slang::SourceLoc(), Slang::Diagnostics::compilationAbortedDueToException, typeid(e).name(), e.Message);
    }
    catch (...)
    {
        // The compiler failed due to some exception that wasn't a sublass of
        // `Exception`, so something really fishy is going on. We want to
        // let the user know that we messed up, so they know to blame Slang
        // and not some other component in their system.
        req->mSink.diagnose(Slang::SourceLoc(), Slang::Diagnostics::compilationAborted);
    }
    req->mDiagnosticOutput = req->mSink.outputBuffer.ProduceString();
    return anyErrors;
#else
    // When debugging, we probably don't want to filter out any errors, since
    // we are probably trying to root-cause and *fix* those errors.
    {
        int anyErrors = req->executeActions();
        return anyErrors;
    }
#endif
}

SLANG_API int
spGetDependencyFileCount(
    SlangCompileRequest*    request)
{
    if(!request) return 0;
    auto req = REQ(request);
    return (int) req->mDependencyFilePaths.Count();
}

/** Get the path to a file this compilation dependend on.
*/
SLANG_API char const*
spGetDependencyFilePath(
    SlangCompileRequest*    request,
    int                     index)
{
    if(!request) return 0;
    auto req = REQ(request);
    return req->mDependencyFilePaths[index].begin();
}

SLANG_API int
spGetTranslationUnitCount(
    SlangCompileRequest*    request)
{
    auto req = REQ(request);
    return (int) req->translationUnits.Count();
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
    auto req = REQ(request);

    // TODO: We should really accept a target index in this API
    auto targetCount = req->targets.Count();
    if (targetCount == 0)
        return nullptr;
    auto targetReq = req->targets[0];

    Slang::CompileResult& result = targetReq->entryPointResults[entryPointIndex];

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
    auto req = REQ(request);

    void const* data = req->generatedBytecode.Buffer();
    size_t size = req->generatedBytecode.Count();

    if(outSize) *outSize = size;
    return data;
}

// Reflection API

SLANG_API SlangReflection* spGetReflection(
    SlangCompileRequest*    request)
{
    if( !request ) return 0;
    auto req = REQ(request);

    // Note(tfoley): The API signature doesn't let the client
    // specify which target they want to access reflection
    // information for, so for now we default to the first one.
    //
    // TODO: Add a new `spGetReflectionForTarget(req, targetIndex)`
    // so that we can do this better, and make it clear that
    // `spGetReflection()` is shorthand for `targetIndex == 0`.
    //
    auto targetCount = req->targets.Count();
    if (targetCount == 0)
        return 0;
    auto targetReq = req->targets[0];

    return (SlangReflection*) targetReq->layout.Ptr();
}

// ... rest of reflection API implementation is in `Reflection.cpp`
