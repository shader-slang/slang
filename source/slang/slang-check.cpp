// slang-check.cpp
#include "slang-check.h"

// This file provides general facilities related to semantic
// checking that don't cleanly land in one of the more
// specialized `slang-check-*` files.

#include "core/slang-type-text-util.h"
#include "slang-check-impl.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
namespace
{ // anonymous

class SinkSharedLibraryLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outSharedLibrary) SLANG_OVERRIDE
    {
        SlangResult res = m_loader->loadSharedLibrary(path, outSharedLibrary);

        // Special handling for failure...
        if (SLANG_FAILED(res) && m_sink)
        {
            String filename = Path::getFileNameWithoutExt(path);
            if (filename == "dxil")
            {
                m_sink->diagnose(Diagnostics::DxilNotFound{});
            }
            else
            {
                m_sink->diagnose(Diagnostics::NoteFailedToLoadDynamicLibrary{.path = path});
            }
        }
        return res;
    }

    SinkSharedLibraryLoader(ISlangSharedLibraryLoader* loader, DiagnosticSink* sink)
        : m_loader(loader), m_sink(sink)
    {
    }

protected:
    ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }
    ISlangSharedLibraryLoader* m_loader;
    DiagnosticSink* m_sink;
};

} // namespace

SlangResult SemanticsContext::ensureAutodiffModuleLoaded(SourceLoc location)
{
    auto session = getSession();
    Module* module = nullptr;
    const SlangResult loadResult = session->loadAutodiffModuleIfNeeded(module);
    if (SLANG_FAILED(loadResult))
    {
        // The load only fails if the embedded supplement blob is corrupt, or a source-only build
        // fails to compile the supplement on demand. Neither is reachable from a `.slang`
        // regression test without fault injection, so this diagnostic (38038) has no automated
        // test; the caller in `_checkHigherOrderInvokeExpr` then returns an error expression.
        m_sink->diagnose(Diagnostics::UnableToLoadAutodiffModule{.location = location});
        return loadResult;
    }

    if (module)
    {
        m_shared->addLoadedAutodiffModule(module->getModuleDecl());

        // Make the module we are checking come out of checking looking as if it had written
        // `import` for the supplement, the same way `visitImportDecl` records a written import:
        // synthesize an `ImportDecl` naming the supplement, add it as a member of the checked
        // module, and record the dependency directly for immediate use in this compilation.
        //
        // `Module::_collectShaderParams` walks a module's `ImportDecl`s to build its requirement
        // list, and AST serialization walks every cross-module `Decl*` reference -- including this
        // `ImportDecl`'s own `importedModuleDecl` field -- to record which other modules a
        // serialized module depends on (see `_findModuleDeclWasImportedFrom` in
        // slang-serialize-ast.cpp). Adding a real `ImportDecl` here, rather than only updating
        // `Module::addModuleDependency`'s bookkeeping directly, means both of those consumers pick
        // up the dependency through the one mechanism they already use for a written `import`,
        // instead of needing a second, parallel notion of "depends on" that could drift from it.
        // `Linkage::loadSerializedModuleContents` relies on exactly this when a module compiled
        // this way is later deserialized in a fresh session.
        //
        // Unlike `visitImportDecl`, we do not also call `importModuleIntoScope` here: ordinary
        // unqualified lookup is never supposed to find declarations from the supplement. The
        // eager/lazy split moved everything ordinary code can name into the eager core; what
        // remains in the supplement is derivative-registration machinery reached only through the
        // extension/associated-decl caches that `addLoadedAutodiffModule` merges above. Importing
        // it into scope would let the module that happened to trigger the load also spuriously see
        // the supplement's internal helper names by ordinary lookup.
        if (auto currentModule = m_shared->getModule())
        {
            auto syntheticImportDecl = getASTBuilder()->create<ImportDecl>();
            syntheticImportDecl->moduleNameAndLoc = NameLoc(session->autodiffModuleName, location);
            syntheticImportDecl->loc = location;
            syntheticImportDecl->importedModuleDecl = module->getModuleDecl();

            // The declaration is fully formed above; mark it checked so that the ordinary
            // decl-checking driver does not dispatch `visitImportDecl` on it, which would
            // redundantly (and, per the note above, incorrectly) call `importModuleIntoScope`.
            syntheticImportDecl->setCheckState(DeclCheckState::DefinitionChecked);

            currentModule->getModuleDecl()->addMember(syntheticImportDecl);
            currentModule->addModuleDependency(module);
        }

        return SLANG_OK;
    }

    // A successful load with no module means the recursion guard in `loadAutodiffModuleIfNeeded`
    // was taken: while source-compiling a builtin module it declines to load, because that
    // compilation already checks the supplement's declarations in the current module and there is
    // no separate late module to merge. Returning SLANG_OK with no merge is therefore correct in
    // that state, so this is a debug assertion documenting the invariant rather than a release
    // guard — promoting it would turn a benign, self-consistent path into a crash.
    SLANG_ASSERT(session->isCompilingBuiltinModule());
    return SLANG_OK;
}


void Session::_setSharedLibraryLoader(ISlangSharedLibraryLoader* loader)
{
    if (m_sharedLibraryLoader != loader)
    {
        // Need to clear all of the libraries
        m_downstreamCompilerSet->clear();
        m_downstreamCompilerInitialized = 0;

        for (Index i = 0; i < Index(SLANG_PASS_THROUGH_COUNT_OF); ++i)
        {
            m_downstreamCompilers[i].setNull();
        }

        // Set the loader
        m_sharedLibraryLoader = loader;
    }
}

void Session::resetDownstreamCompiler(PassThroughMode type)
{
    // The downstream compiler table is shared session state and may be reset concurrently.
    std::lock_guard<std::recursive_mutex> lock(m_downstreamCompilerMutex);

    // Mark as initialized
    m_downstreamCompilerInitialized &= ~(1 << int(type));
    m_downstreamCompilers[int(type)].setNull();
}

IDownstreamCompiler* Session::getOrLoadDownstreamCompiler(
    PassThroughMode type,
    DiagnosticSink* sink)
{
    // Lazy downstream compiler loading mutates shared state, and GenericCCpp can re-enter
    // this routine while probing specific C/C++ compilers.
    std::lock_guard<std::recursive_mutex> lock(m_downstreamCompilerMutex);

    if (m_downstreamCompilerInitialized & (1 << int(type)))
    {
        return m_downstreamCompilers[int(type)];
    }

    if (type == PassThroughMode::GenericCCpp)
    {
        // try testing for availability on all C/C++ compilers
        getOrLoadDownstreamCompiler(PassThroughMode::Clang, nullptr);
        getOrLoadDownstreamCompiler(PassThroughMode::Gcc, nullptr);
        getOrLoadDownstreamCompiler(PassThroughMode::VisualStudio, nullptr);
        getOrLoadDownstreamCompiler(PassThroughMode::LLVM, nullptr);
    }

    // Mark that we have tried to load it
    m_downstreamCompilerInitialized |= (1 << int(type));
    m_downstreamCompilers[int(type)].setNull();

    // Do we have a locator
    auto locator = m_downstreamCompilerLocators[int(type)];
    if (locator)
    {
        m_downstreamCompilerSet->remove(SlangPassThrough(type));

        // We want to be able to report a diagnostic to the user if a loader
        // was unable to locate the desired downstream compiler, but we
        // also need to deal with the fact that the locator might "probe"
        // multiple possible library versions/names, and failing to load
        // one library should not be taken as a hard error.
        //
        // The approach we use here is to first apply the `locator` directly
        // with our `m_sharedLibraryLoader` and see if it succeeds. If
        // it does, then we will move along.
        //
        if (SLANG_FAILED(locator(
                m_downstreamCompilerPaths[int(type)],
                m_sharedLibraryLoader,
                m_downstreamCompilerSet)))
        {
            // If the locator reported a failure the first time we invoked
            // it, then we will invoke it against with a wrapper shared library
            // loader that reported library load failures to our diagnost `sink`.
            //
            // This means that in the case of failure the user will see a listing
            // of all the libraries that the locator attempted to load but failed
            // to find. The user will know that making one or more of these libraries
            // available could fix the issue, but we cannot communicate precise
            // information to them with this approach (e.g., the difference between
            // "I need all of these libraries" vs. "I need at least one of these
            // libraries").
            //
            if (sink)
            {
                sink->diagnose(Diagnostics::FailedToLoadDownstreamCompiler{
                    .compiler = TypeTextUtil::getPassThroughAsHumanText(SlangPassThrough(type))});
            }
            SinkSharedLibraryLoader loader(m_sharedLibraryLoader, sink);
            locator(m_downstreamCompilerPaths[int(type)], &loader, m_downstreamCompilerSet);
        }

        DownstreamCompilerUtil::updateDefaults(m_downstreamCompilerSet);
    }

    IDownstreamCompiler* compiler = nullptr;

    if (type == PassThroughMode::GenericCCpp)
    {
        compiler = m_downstreamCompilerSet->getDefaultCompiler(SLANG_SOURCE_LANGUAGE_CPP);
    }
    else
    {
        DownstreamCompilerDesc desc;
        desc.type = SlangPassThrough(type);
        compiler = DownstreamCompilerUtil::findCompiler(
            m_downstreamCompilerSet,
            DownstreamCompilerUtil::MatchType::Newest,
            desc);
    }
    m_downstreamCompilers[int(type)] = compiler;
    return compiler;
}

void checkTranslationUnit(
    TranslationUnitRequest* translationUnit,
    LoadedModuleDictionary& loadedModules)
{
    SLANG_AST_BUILDER_RAII(translationUnit->compileRequest->getLinkage()->getASTBuilder());

    SharedSemanticsContext sharedSemanticsContext(
        translationUnit->compileRequest->getLinkage(),
        translationUnit->getModule(),
        translationUnit->compileRequest->getSink(),
        &loadedModules,
        translationUnit);

    SemanticsDeclVisitorBase visitor((SemanticsContext(&sharedSemanticsContext)));

    // Apply the visitor to do the main semantic
    // checking that is required on all declarations
    // in the translation unit.

    visitor.checkModule(translationUnit->getModuleDecl());

    translationUnit->getModule()->_collectShaderParams(translationUnit->compileRequest->getSink());
}

void SemanticsVisitor::dispatchStmt(Stmt* stmt, SemanticsContext const& context)
{
    SemanticsStmtVisitor visitor(context);
    try
    {
        visitor.dispatch(stmt);
    }
    catch (const AbortCompilationException&)
    {
        throw;
    }
    catch (...)
    {
        getSink()->noteInternalErrorLoc(stmt->loc);
        throw;
    }
}

Expr* SemanticsVisitor::dispatchExpr(Expr* expr, SemanticsContext const& context)
{
    SemanticsExprVisitor visitor(context);
    try
    {
        return visitor.dispatch(expr);
    }
    catch (const AbortCompilationException&)
    {
        throw;
    }
    catch (...)
    {
        getSink()->noteInternalErrorLoc(expr->loc);
        throw;
    }
}

ASTBuilder* semanticsVisitorGetASTBuilder(SemanticsVisitor* sv)
{
    return sv->getASTBuilder();
}

} // namespace Slang
