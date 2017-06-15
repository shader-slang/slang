#include "../../slang.h"

#include "../core/slang-io.h"
#include "../slang/slang-stdlib.h"
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

static void stdlibDiagnosticCallback(
    char const* message,
    void*       userData)
{
    fputs(message, stderr);
    fflush(stderr);
#ifdef WIN32
    OutputDebugStringA(message);
#endif
}

class Session
{
public:
    bool useCache = false;
    String cacheDir;

    RefPtr<Scope>   slangLanguageScope;
    RefPtr<Scope>   hlslLanguageScope;
    RefPtr<Scope>   glslLanguageScope;

    List<RefPtr<ProgramSyntaxNode>> loadedModuleCode;


    Session(bool /*pUseCache*/, String /*pCacheDir*/)
    {
        // Initialize global state
        // TODO: move this into the session instead
        BasicExpressionType::Init();

        // Create scopes for various language builtins.
        //
        // TODO: load these on-demand to avoid parsing
        // stdlib code for languages the user won't use.

        slangLanguageScope = new Scope();

        hlslLanguageScope = new Scope();
        hlslLanguageScope->parent = slangLanguageScope;

        glslLanguageScope = new Scope();
        glslLanguageScope->parent = slangLanguageScope;

        addBuiltinSource(slangLanguageScope, "stdlib", SlangStdLib::GetCode());
        addBuiltinSource(glslLanguageScope, "glsl", getGLSLLibraryCode());
    }

    ~Session()
    {
        // We need to clean up the strings for the standard library
        // code that we might have allocated and loaded into static
        // variables (TODO: don't use `static` variables for this stuff)

        SlangStdLib::Finalize();

        // Ditto for our type represnetation stuff

        ExpressionType::Finalize();
    }

    CompileUnit createPredefUnit()
    {
        CompileUnit translationUnit;


        RefPtr<ProgramSyntaxNode> translationUnitSyntax = new ProgramSyntaxNode();

        TranslationUnitOptions translationUnitOptions;
        translationUnit.options = translationUnitOptions;
        translationUnit.SyntaxNode = translationUnitSyntax;

        return translationUnit;
    }

    void addBuiltinSource(
        RefPtr<Scope> const&    scope,
        String const&           path,
        String const&           source);
};

struct CompileRequest
{
    // Pointer to parent session
    Session* mSession;

    // Input options
    CompileOptions Options;

    // Output stuff
    DiagnosticSink mSink;
    String mDiagnosticOutput;

    RefPtr<CollectionOfTranslationUnits> mCollectionOfTranslationUnits;

    RefPtr<ProgramLayout> mReflectionData;

    CompileResult mResult;

    List<String> mDependencyFilePaths;

    CompileRequest(Session* session)
        : mSession(session)
    {}

    ~CompileRequest()
    {}

    struct IncludeHandlerImpl : IncludeHandler
    {
        CompileRequest* request;

        List<String> searchDirs;

        virtual bool TryToFindIncludeFile(
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

                return true;
            }

            for (auto & dir : searchDirs)
            {
                path = Path::Combine(dir, pathToInclude);
                if (File::Exists(path))
                {
                    *outFoundPath = path;
                    *outFoundSource = File::ReadAllText(path);

                    request->mDependencyFilePaths.Add(path);

                    return true;
                }
            }
            return false;
        }
    };


    CompileUnit parseTranslationUnit(
        TranslationUnitOptions const&   translationUnitOptions,
        CompileOptions&                 options)
    {
        IncludeHandlerImpl includeHandler;
        includeHandler.request = this;

        CompileUnit translationUnit;

        RefPtr<Scope> languageScope;
        switch (translationUnitOptions.sourceLanguage)
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

        Dictionary<String, String> preprocessorDefinitions;
        for(auto& def : options.preprocessorDefinitions)
            preprocessorDefinitions.Add(def.Key, def.Value);
        for(auto& def : translationUnitOptions.preprocessorDefinitions)
            preprocessorDefinitions.Add(def.Key, def.Value);

        RefPtr<ProgramSyntaxNode> translationUnitSyntax = new ProgramSyntaxNode();

        for (auto sourceFile : translationUnitOptions.sourceFiles)
        {
            auto sourceFilePath = sourceFile->path;

            auto searchDirs = options.SearchDirectories;
            searchDirs.Reverse();
            searchDirs.Add(Path::GetDirectoryName(sourceFilePath));
            searchDirs.Reverse();
            includeHandler.searchDirs = searchDirs;

            String source = sourceFile->content;

            auto tokens = preprocessSource(
                source,
                sourceFilePath,
                mResult.GetErrorWriter(),
                &includeHandler,
                preprocessorDefinitions,
                translationUnitSyntax.Ptr());

            parseSourceFile(
                translationUnitSyntax.Ptr(),
                options,
                tokens,
                mResult.GetErrorWriter(),
                sourceFilePath,
                languageScope);
        }

        translationUnit.options = translationUnitOptions;
        translationUnit.SyntaxNode = translationUnitSyntax;

        return translationUnit;
    }

    CompileUnit parseTranslationUnit(
        TranslationUnitOptions const&   translationUnitOptions)
    {
        return parseTranslationUnit(translationUnitOptions, Options);
    }

    void checkTranslationUnit(
        CompileUnit&            translationUnit,
        RefPtr<SyntaxVisitor>   visitor)
    {
        visitor->setSourceLanguage(translationUnit.options.sourceLanguage);
        translationUnit.SyntaxNode->Accept(visitor.Ptr());
    }

    void checkTranslationUnit(
        CompileUnit&    translationUnit,
        CompileOptions& options)
    {
        RefPtr<SyntaxVisitor> visitor = CreateSemanticsVisitor(
            mResult.GetErrorWriter(),
            options,
            this);

        checkTranslationUnit(translationUnit, visitor);
    }

    void checkCollectionOfTranslationUnits(
        RefPtr<CollectionOfTranslationUnits>    collectionOfTranslationUnits)
    {
        RefPtr<SyntaxVisitor> visitor = CreateSemanticsVisitor(
            mResult.GetErrorWriter(),
            Options,
            this);

        for( auto& translationUnit : collectionOfTranslationUnits->translationUnits )
        {
            checkTranslationUnit(translationUnit, visitor);
        }
    }

    void generateOutputForCollectionOfTranslationUnits(
        RefPtr<CollectionOfTranslationUnits>    collectionOfTranslationUnits)
    {
        // Do binding generation, and then reflection (globally)
        // before we move on to any code-generation activites.
        GenerateParameterBindings(collectionOfTranslationUnits.Ptr());


        // HACK(tfoley): for right now I just want to pretty-print an AST
        // into another language, so the whole compiler back-end is just
        // getting in the way.
        //
        // I'm going to bypass it for now and see what I can do:

        ExtraContext extra;
        extra.options = &Options;
        extra.programLayout = collectionOfTranslationUnits->layout.Ptr();
        extra.compileResult = &mResult;

        generateOutput(extra, collectionOfTranslationUnits.Ptr());
    }

    int executeCompilerDriverActions()
    {
        // If we are being asked to do pass-through, then we need to do that here...
        if (Options.passThrough != PassThroughMode::None)
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

        // TODO: load the stdlib

        mCollectionOfTranslationUnits = new CollectionOfTranslationUnits();

        // Parse everything from the input files requested
        //
        // TODO: this may trigger the loading and/or compilation of additional modules.
        for (auto& translationUnitOptions : Options.translationUnits)
        {
            auto translationUnit = parseTranslationUnit(translationUnitOptions);
            mCollectionOfTranslationUnits->translationUnits.Add(translationUnit);
        }
        if (mResult.GetErrorCount() != 0)
            return 1;

        // Perform semantic checking on the whole collection
        checkCollectionOfTranslationUnits(mCollectionOfTranslationUnits);
        if (mResult.GetErrorCount() != 0)
            return 1;

        // Generate output code, in whatever format was requested
        generateOutputForCollectionOfTranslationUnits(mCollectionOfTranslationUnits);
        if (mResult.GetErrorCount() != 0)
            return 1;

        // Extract the reflection layout information so that users
        // can easily query it.
        mReflectionData = mCollectionOfTranslationUnits->layout;

        return 0;
    }

    // Act as expected of the API-based compiler
    int executeAPIActions()
    {
        mResult.mSink = &mSink;

        int err = executeCompilerDriverActions();

        mDiagnosticOutput = mSink.outputBuffer.ProduceString();

        if (mSink.GetErrorCount() != 0)
            return mSink.GetErrorCount();

        return err;
    }

    int addTranslationUnit(SourceLanguage language, String const& name)
    {
        int result = Options.translationUnits.Count();

        TranslationUnitOptions translationUnit;
        translationUnit.sourceLanguage = SourceLanguage(language);

        Options.translationUnits.Add(translationUnit);

        return result;
    }

    void addTranslationUnitSourceString(
        int             translationUnitIndex,
        String const&   path,
        String const&   source)
    {
        RefPtr<SourceFile> sourceFile = new SourceFile();
        sourceFile->path = path;
        sourceFile->content = source;

        Options.translationUnits[translationUnitIndex].sourceFiles.Add(sourceFile);
    }

    void addTranslationUnitSourceFile(
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
                CodePosition(0, 0, 0, path),
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

    int addTranslationUnitEntryPoint(
        int                     translationUnitIndex,
        String const&           name,
        Profile                 profile)
    {
        EntryPointOption entryPoint;
        entryPoint.name = name;
        entryPoint.profile = profile;

        // TODO: realistically want this to be global across all TUs...
        int result = Options.translationUnits[translationUnitIndex].entryPoints.Count();

        Options.translationUnits[translationUnitIndex].entryPoints.Add(entryPoint);
        return result;
    }

    Dictionary<String, RefPtr<ProgramSyntaxNode>> loadedModules;

    RefPtr<ProgramSyntaxNode> findOrImportModule(
        String const&       name,
        CodePosition const& loc)
    {
        // Have we already loaded a module matching this name?
        // If so, return it.
        RefPtr<ProgramSyntaxNode> moduleDecl;
        if (loadedModules.TryGetValue(name, moduleDecl))
            return moduleDecl;

        // Derive a file name for the module, by taking the given
        // identifier, replacing all occurences of `_` with `-`,
        // and then appending `.slang`.
        //
        // For example, `foo_bar` becomes `foo-bar.slang`.

        StringBuilder sb;
        for (auto c : name)
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

        String pathIncludedFrom = loc.FileName;

        String foundPath;
        String foundSource;
        bool found = includeHandler.TryToFindIncludeFile(fileName, pathIncludedFrom, &foundPath, &foundSource);
        if (!found)
        {
            this->mSink.diagnose(loc, Diagnostics::cannotFindFile, fileName);

            loadedModules[name] = nullptr;
            return nullptr;
        }

        // We've found a file that we can load for the given module, so
        // now we need to try compiling it, etc.

        // We don't want to use the same options that the user specified
        // for loading modules on-demand. In particular, we always want
        // semantic checking to be enabled.
        CompileOptions moduleOptions;
        moduleOptions.SearchDirectories = Options.SearchDirectories;
        moduleOptions.profile = Options.profile;

        RefPtr<SourceFile> sourceFile = new SourceFile();
        sourceFile->path = foundPath;
        sourceFile->content = foundSource;

        TranslationUnitOptions translationUnitOptions;
        translationUnitOptions.sourceFiles.Add(sourceFile);

        CompileUnit translationUnit = parseTranslationUnit(translationUnitOptions, moduleOptions);

        // TODO: handle errors

        checkTranslationUnit(translationUnit, moduleOptions);

        // Skip code generation

        //

        moduleDecl = translationUnit.SyntaxNode;

        loadedModules.Add(name, moduleDecl);

        return moduleDecl;
    }

};

RefPtr<ProgramSyntaxNode> findOrImportModule(
    CompileRequest*     request,
    String const&       name,
    CodePosition const& loc)
{
    return request->findOrImportModule(name, loc);
}

void Session::addBuiltinSource(
    RefPtr<Scope> const&    scope,
    String const&           path,
    String const&           source)
{
    CompileRequest compileRequest(this);

    auto translationUnitIndex = compileRequest.addTranslationUnit(SourceLanguage::Slang, path);

    compileRequest.addTranslationUnitSourceString(
        translationUnitIndex,
        path,
        source);

    int err = compileRequest.executeAPIActions();
    if (err)
    {
        fprintf(stderr, "%s", compileRequest.mDiagnosticOutput.Buffer());

#ifdef _WIN32
        OutputDebugStringA(compileRequest.mDiagnosticOutput.Buffer());
#endif

        assert(!"error in stdlib");
    }

    // Extract the AST for the code we just parsed
    auto syntax = compileRequest.mCollectionOfTranslationUnits->translationUnits[translationUnitIndex].SyntaxNode;

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

SLANG_API SlangSession* spCreateSession(const char * cacheDir)
{
    return reinterpret_cast<SlangSession *>(new Slang::Session((cacheDir ? true : false), cacheDir));
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
        s->slangLanguageScope,

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
    REQ(request)->Options.flags = flags;
}

SLANG_API void spSetCodeGenTarget(
        SlangCompileRequest*    request,
        int target)
{
    REQ(request)->Options.Target = (Slang::CodeGenTarget)target;
}

SLANG_API void spSetPassThrough(
    SlangCompileRequest*    request,
    SlangPassThrough        passThrough)
{
    REQ(request)->Options.passThrough = Slang::PassThroughMode(passThrough);
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
        const char*             searchDir)
{
    REQ(request)->Options.SearchDirectories.Add(searchDir);
}

SLANG_API void spAddPreprocessorDefine(
    SlangCompileRequest*    request,
    const char*             key,
    const char*             value)
{
    REQ(request)->Options.preprocessorDefinitions[key] = value;
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

    req->Options.translationUnits[translationUnitIndex].preprocessorDefinitions[key] = value;

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
    if(translationUnitIndex >= req->Options.translationUnits.Count()) return;

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
    if(translationUnitIndex >= req->Options.translationUnits.Count()) return;

    if(!path) path = "";

    req->addTranslationUnitSourceString(
        translationUnitIndex,
        path,
        source);

}

SLANG_API SlangProfileID spFindProfile(
    SlangSession*   session,
    char const*     name)
{
    return Slang::Profile::LookUp(name).raw;
}

SLANG_API int spAddTranslationUnitEntryPoint(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    char const*             name,
    SlangProfileID          profile)
{
    if(!request) return -1;
    auto req = REQ(request);
    if(!name) return -1;
    if(translationUnitIndex < 0) return -1;
    if(translationUnitIndex >= req->Options.translationUnits.Count()) return -1;


    return req->addTranslationUnitEntryPoint(
        translationUnitIndex,
        name,
        Slang::Profile(Slang::Profile::RawVal(profile)));
}


// Compile in a context that already has its translation units specified
SLANG_API int spCompile(
    SlangCompileRequest*    request)
{
    auto req = REQ(request);

    int anyErrors = req->executeAPIActions();
    return anyErrors;
}

SLANG_API int
spGetDependencyFileCount(
    SlangCompileRequest*    request)
{
    if(!request) return 0;
    auto req = REQ(request);
    return req->mDependencyFilePaths.Count();
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
    return req->mResult.translationUnits.Count();
}

// Get the output code associated with a specific translation unit
SLANG_API char const* spGetTranslationUnitSource(
    SlangCompileRequest*    request,
    int                     translationUnitIndex)
{
    auto req = REQ(request);
    return req->mResult.translationUnits[translationUnitIndex].outputSource.Buffer();
}

SLANG_API char const* spGetEntryPointSource(
    SlangCompileRequest*    request,
    int                     translationUnitIndex,
    int                     entryPointIndex)
{
    auto req = REQ(request);
    return req->mResult.translationUnits[translationUnitIndex].entryPoints[entryPointIndex].outputSource.Buffer();

}

// Reflection API

SLANG_API SlangReflection* spGetReflection(
    SlangCompileRequest*    request)
{
    if( !request ) return 0;

    auto req = REQ(request);
    return (SlangReflection*) req->mReflectionData.Ptr();
}


// ... rest of reflection API implementation is in `Reflection.cpp`
