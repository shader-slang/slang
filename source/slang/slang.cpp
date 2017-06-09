#include "../../slang.h"

#include "../core/slang-io.h"
#include "../slang/slang-stdlib.h"
#include "../slang/parser.h"
#include "../slang/preprocessor.h"
#include "../slang/reflection.h"
#include "../slang/type-layout.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#endif

using namespace CoreLib::Basic;
using namespace CoreLib::IO;
using namespace Slang::Compiler;

namespace SlangLib
{
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
        CoreLib::String cacheDir;

        RefPtr<ShaderCompiler> compiler;

        RefPtr<Scope>   slangLanguageScope;
        RefPtr<Scope>   hlslLanguageScope;
        RefPtr<Scope>   glslLanguageScope;

        List<RefPtr<ProgramSyntaxNode>> loadedModuleCode;


        Session(bool /*pUseCache*/, CoreLib::String /*pCacheDir*/)
        {
            compiler = CreateShaderCompiler();

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
                CoreLib::String const& pathToInclude,
                CoreLib::String const& pathIncludedFrom,
                CoreLib::String* outFoundPath,
                CoreLib::String* outFoundSource) override
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
            TranslationUnitOptions const&   translationUnitOptions)
        {
            auto& options = Options;

            IncludeHandlerImpl includeHandler;
            includeHandler.request = this;

            CompileUnit translationUnit;

            RefPtr<Scope> languageScope;
            switch( translationUnitOptions.sourceLanguage )
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


            auto& preprocesorDefinitions = options.PreprocessorDefinitions;

            RefPtr<ProgramSyntaxNode> translationUnitSyntax = new ProgramSyntaxNode();

            for( auto sourceFile : translationUnitOptions.sourceFiles )
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
                    preprocesorDefinitions,
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

        int executeCompilerDriverActions()
        {
            // If we are being asked to do pass-through, then we need to do that here...
            if (Options.passThrough != PassThroughMode::None)
            {
                for( auto& translationUnitOptions : Options.translationUnits )
                {
                    switch( translationUnitOptions.sourceLanguage )
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

                    mSession->compiler->PassThrough(
                        source,
                        sourceFilePath,
                        Options,
                        translationUnitOptions);
                }
                return 0;
            }

            // TODO: load the stdlib

            mCollectionOfTranslationUnits = new CollectionOfTranslationUnits();

            // Parse everything from the input files requested
            //
            // TODO: this may trigger the loading and/or compilation of additional modules.
            for( auto& translationUnitOptions : Options.translationUnits )
            {
                auto translationUnit = parseTranslationUnit(translationUnitOptions);
                mCollectionOfTranslationUnits->translationUnits.Add(translationUnit);
            }
            if( mResult.GetErrorCount() != 0 )
                return 1;

            // Now perform semantic checks, emit output, etc.
            mSession->compiler->Compile(
                mResult, mCollectionOfTranslationUnits.Ptr(), Options);
            if(mResult.GetErrorCount() != 0)
                return 1;

            mReflectionData = mCollectionOfTranslationUnits->layout;

            return 0;
        }

        // Act as expected of the API-based compiler
        int executeAPIActions()
        {
            mResult.mSink = &mSink;

            int err = executeCompilerDriverActions();

            mDiagnosticOutput = mSink.outputBuffer.ProduceString();

            if(mSink.GetErrorCount() != 0)
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
            catch( ... )
            {
                // Emit a diagnostic!
                mSink.diagnose(
                    CodePosition(0,0,0,path),
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
    };

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
        if(err)
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
        if( !scope->containerDecl )
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

using namespace SlangLib;

// implementation of C interface

#define SESSION(x) reinterpret_cast<SlangLib::Session *>(x)
#define REQ(x) reinterpret_cast<SlangLib::CompileRequest*>(x)

SLANG_API SlangSession* spCreateSession(const char * cacheDir)
{
    return reinterpret_cast<SlangSession *>(new SlangLib::Session((cacheDir ? true : false), cacheDir));
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
    auto req = new SlangLib::CompileRequest(s);
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
    REQ(request)->Options.Target = (CodeGenTarget)target;
}

SLANG_API void spSetPassThrough(
    SlangCompileRequest*    request,
    SlangPassThrough        passThrough)
{
    REQ(request)->Options.passThrough = PassThroughMode(passThrough);
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
    REQ(request)->Options.PreprocessorDefinitions[key] = value;
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
        SourceLanguage(language),
        name ? name : "");
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
    return Profile::LookUp(name).raw;
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
        Profile(Profile::RawVal(profile)));
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
