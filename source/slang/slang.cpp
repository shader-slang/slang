#include "../../slang.h"

#include "../core/slang-io.h"
#include "parameter-binding.h"
#include "../slang/parser.h"
#include "../slang/preprocessor.h"
#include "../slang/reflection.h"
#include "syntax-visitors.h"
#include "../slang/type-layout.h"

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

    glslLanguageScope = new Scope();
    glslLanguageScope->nextSibling = coreLanguageScope;

    addBuiltinSource(coreLanguageScope, "core", getCoreLibraryCode());
    addBuiltinSource(hlslLanguageScope, "hlsl", getHLSLLibraryCode());
    addBuiltinSource(glslLanguageScope, "glsl", getGLSLLibraryCode());
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

CompileRequest::CompileRequest(Session* session)
    : mSession(session)
{
    getNamePool()->setRootNamePool(session->getRootNamePool());

    setSourceManager(&sourceManagerStorage);

    sourceManager->initialize(session->getBuiltinSourceManager());
}

CompileRequest::~CompileRequest()
{}

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

    case SourceLanguage::GLSL:
        languageScope = mSession->glslLanguageScope;
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

void CompileRequest::checkAllTranslationUnits()
{
    for( auto& translationUnit : translationUnits )
    {
        checkTranslationUnit(translationUnit.Ptr());
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
    for( auto& translationUnit : translationUnits )
    {
        translationUnit->compileFlags |= compileFlags;

        // However, the "no checking" flag shouldn't be applied to
        // any translation unit that is native Slang code.
        if( translationUnit->sourceLanguage == SourceLanguage::Slang )
        {
            translationUnit->compileFlags &= ~SLANG_COMPILE_FLAG_NO_CHECKING;
        }
    }

    // If no code-generation target was specified, then try to infer one from the source language,
    // just to make sure we can do something reasonable when `reflection-json` is specified
    if (Target == CodeGenTarget::Unknown)
    {
        auto language = inferSourceLanguage(this);
        switch (language)
        {
        case SourceLanguage::HLSL:
            Target = CodeGenTarget::DXBytecodeAssembly;
            break;

        case SourceLanguage::GLSL:
            Target = CodeGenTarget::SPIRVAssembly;
            break;

        default:
            break;
        }
    }

#if 0
    // If we are being asked to do pass-through, then we need to do that here...
    if (passThrough != PassThroughMode::None)
    {
        for (auto& translationUnitOptions : Options.translationUnits)
        {
            switch (translationUnitOptions.sourceLanguage)
            {
                // We can pass-through code written in a native shading language
            case SourceLanguage::GLSL:
            case SourceLanguage::HLSL:
                break;

                // All other translation units need to be skipped
            default:
                continue;
            }

            auto sourceFile = translationUnitOptions.sourceFiles[0];
            auto sourceFilePath = sourceFile->path;
            String source = sourceFile->content;

            auto translationUnitResult = passThrough(
                source,
                sourceFilePath,
                Options,
                translationUnitOptions);

            mResult.translationUnits.Add(translationUnitResult);
        }
        return 0;
    }
#endif

    // We only do parsing and semantic checking if we *aren't* doing
    // a pass-through compilation.
    //
    // Note that we *do* perform output generation as normal in pass-through mode.
    if( passThrough == PassThroughMode::None )
    {
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

        // Now do shader parameter binding generation, which
        // needs to be performed globally.
        generateParameterBindings(this);
        if (mSink.GetErrorCount() != 0)
            return 1;
    }

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
    Profile                 entryPointProfile)
{
    RefPtr<EntryPointRequest> entryPoint = new EntryPointRequest();
    entryPoint->compileRequest = this;
    entryPoint->name = getNamePool()->getName(name);
    entryPoint->profile = entryPointProfile;
    entryPoint->translationUnitIndex = translationUnitIndex;

    auto translationUnit = translationUnits[translationUnitIndex].Ptr();
    translationUnit->entryPoints.Add(entryPoint);

    UInt result = entryPoints.Count();
    entryPoints.Add(entryPoint);
    return (int) result;
}

RefPtr<ModuleDecl> CompileRequest::loadModule(
    Name*               name,
    String const&       path,
    String const&       source,
    SourceLoc const&)
{
    RefPtr<TranslationUnitRequest> translationUnit = new TranslationUnitRequest();
    translationUnit->compileRequest = this;

    // We don't want to use the same options that the user specified
    // for loading modules on-demand. In particular, we always want
    // semantic checking to be enabled.
    //
    // TODO: decide which options, if any, should be inherited.

    RefPtr<SourceFile> sourceFile = getSourceManager()->allocateSourceFile(path, source);

    translationUnit->sourceFiles.Add(sourceFile);

    parseTranslationUnit(translationUnit.Ptr());

    // TODO: handle errors

    checkTranslationUnit(translationUnit.Ptr());

    // Skip code generation

    //

    RefPtr<ModuleDecl> moduleDecl = translationUnit->SyntaxNode;

    mapPathToLoadedModule.Add(path, moduleDecl);
    mapNameToLoadedModules.Add(name, moduleDecl);
    loadedModulesList.Add(moduleDecl);

    return moduleDecl;

}

void CompileRequest::handlePoundImport(
    String const&       path,
    TokenList const&    tokens)
{
    RefPtr<TranslationUnitRequest> translationUnit = new TranslationUnitRequest();
    translationUnit->compileRequest = this;

    // Imported code is always native Slang code
    RefPtr<Scope> languageScope = mSession->slangLanguageScope;

    RefPtr<ModuleDecl> translationUnitSyntax = new ModuleDecl();
    translationUnit->SyntaxNode = translationUnitSyntax;

    parseSourceFile(
        translationUnit.Ptr(),
        tokens,
        &mSink,
        languageScope);

    // TODO: handle errors

    checkTranslationUnit(translationUnit.Ptr());

    // Skip code generation

    //

    RefPtr<ModuleDecl> moduleDecl = translationUnit->SyntaxNode;

    // TODO: It is a bit broken here that we use the module path,
    // as the "name" when registering things, but this saves
    // us the trouble of trying to special-case things when
    // checking an `import` down the road.
    //
    // Ideally we'd construct a suitable name by effectively
    // running the name->path logic in reverse (e.g., replacing
    // `-` with `_` and `/` with `.`).
    Name* name = getNamePool()->getName(path);
    mapNameToLoadedModules.Add(name, moduleDecl);

    mapPathToLoadedModule.Add(path, moduleDecl);
    loadedModulesList.Add(moduleDecl);
}

RefPtr<ModuleDecl> CompileRequest::findOrImportModule(
    Name*               name,
    SourceLoc const&    loc)
{
    // Have we already loaded a module matching this name?
    // If so, return it.
    RefPtr<ModuleDecl> moduleDecl;
    if (mapNameToLoadedModules.TryGetValue(name, moduleDecl))
        return moduleDecl;

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

    // Maybe this was loaded previously via `#import`
    if (mapPathToLoadedModule.TryGetValue(foundPath, moduleDecl))
        return moduleDecl;


    // We've found a file that we can load for the given module, so
    // go ahead and perform the module-load action
    return loadModule(
        name,
        foundPath,
        foundSource,
        loc);
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
    if (target == SLANG_REFLECTION_JSON)
    {
        // HACK: We special case this because reflection JSON is actually
        // an additional output step that layers on top of an existing
        // target
        REQ(request)->extraTarget = Slang::CodeGenTarget::ReflectionJSON;
    }
    else
    {
        REQ(request)->Target = (Slang::CodeGenTarget)target;
    }
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
        Slang::Profile(Slang::Profile::RawVal(profile)));
}


// Compile in a context that already has its translation units specified
SLANG_API int spCompile(
    SlangCompileRequest*    request)
{
    auto req = REQ(request);

#if 0
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
        int anyErrors = req->executeActions();
        return anyErrors;
    }
    catch (...)
    {
        req->mSink.diagnose(Slang::SourceLoc(), Slang::Diagnostics::compilationAborted);
        return 1;
    }
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
    SlangCompileRequest*    request,
    int                     translationUnitIndex)
{
    auto req = REQ(request);
    return req->translationUnits[translationUnitIndex]->result.outputString.Buffer();
}

SLANG_API char const* spGetEntryPointSource(
    SlangCompileRequest*    request,
    int                     entryPointIndex)
{
    auto req = REQ(request);
    return req->entryPoints[entryPointIndex]->result.outputString.Buffer();
}

SLANG_API void const* spGetEntryPointCode(
    SlangCompileRequest*    request,
    int                     entryPointIndex,
    size_t*                 outSize)
{
    auto req = REQ(request);
    Slang::CompileResult& result = req->entryPoints[entryPointIndex]->result;

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

// Reflection API

SLANG_API SlangReflection* spGetReflection(
    SlangCompileRequest*    request)
{
    if( !request ) return 0;

    auto req = REQ(request);
    return (SlangReflection*) req->layout.Ptr();
}

// ... rest of reflection API implementation is in `Reflection.cpp`
