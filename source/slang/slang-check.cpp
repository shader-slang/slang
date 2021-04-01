// slang-check.cpp
#include "slang-check.h"

// This file provides general facilities related to semantic
// checking that don't cleanly land in one of the more
// specialized `slang-check-*` files.

#include "slang-check-impl.h"

#include "../core/slang-type-text-util.h"

namespace Slang
{
    namespace { // anonymous
    struct FunctionInfo
    {
        const char* name;
        PassThroughMode compilerType;
    };

    class SinkSharedLibraryLoader : public RefObject, public ISlangSharedLibraryLoader
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL

            virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadSharedLibrary(
                const char*     path,
                ISlangSharedLibrary** outSharedLibrary) SLANG_OVERRIDE
        {
            SlangResult res = m_loader->loadSharedLibrary(path, outSharedLibrary);

            // Special handling for failure...
            if (SLANG_FAILED(res) && m_sink)
            {
                String filename = Path::getFileNameWithoutExt(path);
                if (filename == "dxil")
                {
                    m_sink->diagnose(SourceLoc(), Diagnostics::dxilNotFound);
                }
                else
                {
                    m_sink->diagnose(SourceLoc(), Diagnostics::noteFailedToLoadDynamicLibrary, path);
                }
            }
            return res;
        }

        SinkSharedLibraryLoader(ISlangSharedLibraryLoader* loader, DiagnosticSink* sink) :
            m_loader(loader),
            m_sink(sink)
        {
        }

    protected:
        ISlangUnknown* getInterface(const Guid& guid)
        {
            return (guid == ISlangUnknown::getTypeGuid() || guid == ISlangSharedLibraryLoader::getTypeGuid()) ? static_cast<ISlangSharedLibraryLoader*>(this) : nullptr;
        }
        ISlangSharedLibraryLoader* m_loader;
        DiagnosticSink* m_sink;
    };

    } // anonymous

    static FunctionInfo _getFunctionInfo(Session::SharedLibraryFuncType funcType)
    {
        typedef Session::SharedLibraryFuncType FuncType;
        
        switch (funcType)
        {
            case FuncType::Glslang_Compile_1_0:   return { "glslang_compile", PassThroughMode::Glslang} ;
            case FuncType::Glslang_Compile_1_1:   return { "glslang_compile_1_1", PassThroughMode::Glslang} ;
            case FuncType::Fxc_D3DCompile:     return { "D3DCompile", PassThroughMode::Fxc};
            case FuncType::Fxc_D3DDisassemble: return { "D3DDisassemble", PassThroughMode::Fxc };
            case FuncType::Dxc_DxcCreateInstance:  return { "DxcCreateInstance", PassThroughMode::Dxc };
            default: return { nullptr, PassThroughMode::None };
        } 
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

            // Clear all of the functions
            ::memset(m_sharedLibraryFunctions, 0, sizeof(m_sharedLibraryFunctions));

            // Set the loader
            m_sharedLibraryLoader = loader;
        }
    }

    void Session::resetDownstreamCompiler(PassThroughMode type)
    {
        // Mark as initialized
        m_downstreamCompilerInitialized &= ~(1 << int(type));
        m_downstreamCompilers[int(type)].setNull();
    }

    DownstreamCompiler* Session::getOrLoadDownstreamCompiler(PassThroughMode type, DiagnosticSink* sink)
    {
        if (m_downstreamCompilerInitialized & (1 << int(type)))
        {
            return m_downstreamCompilers[int(type)];
        }

        if (type == PassThroughMode::GenericCCpp)
        {
            // try testing for availability on all C/C++ compilers
            getOrLoadDownstreamCompiler(PassThroughMode::Clang, sink);
            getOrLoadDownstreamCompiler(PassThroughMode::Gcc, sink);
            getOrLoadDownstreamCompiler(PassThroughMode::VisualStudio, sink);
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
            if (SLANG_FAILED(locator(m_downstreamCompilerPaths[int(type)], m_sharedLibraryLoader, m_downstreamCompilerSet)))
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
                if( sink )
                {
                    sink->diagnose(SourceLoc(), Diagnostics::failedToLoadDownstreamCompiler, type);
                }
                SinkSharedLibraryLoader loader(m_sharedLibraryLoader, sink);
                locator(m_downstreamCompilerPaths[int(type)], &loader, m_downstreamCompilerSet);
            }

            DownstreamCompilerUtil::updateDefaults(m_downstreamCompilerSet);
        }

        DownstreamCompiler* compiler = nullptr;

        if (type == PassThroughMode::GenericCCpp)
        {
            compiler = m_downstreamCompilerSet->getDefaultCompiler(SLANG_SOURCE_LANGUAGE_CPP);
        }
        else
        {
            DownstreamCompiler::Desc desc;
            desc.type = SlangPassThrough(type);
            compiler = DownstreamCompilerUtil::findCompiler(m_downstreamCompilerSet, DownstreamCompilerUtil::MatchType::Newest, desc);
        }
        m_downstreamCompilers[int(type)] = compiler;
        return compiler;
    }

    SlangFuncPtr Session::getSharedLibraryFunc(SharedLibraryFuncType type, DiagnosticSink* sink)
    {
        if (m_sharedLibraryFunctions[int(type)])
        {
            return m_sharedLibraryFunctions[int(type)];
        }
        // do we have the library
        FunctionInfo info = _getFunctionInfo(type);
        if (info.name == nullptr)
        {
            return nullptr;
        }
        // Try loading the library
        DownstreamCompiler* compiler = getOrLoadDownstreamCompiler(info.compilerType, sink);
        if (!compiler)
        {
            return nullptr;
        }
        ISlangSharedLibrary* sharedLibrary = compiler->getSharedLibrary();
        if (!sharedLibrary)
        {
            return nullptr;
        }

        // Okay now access the func
        SlangFuncPtr func = sharedLibrary->findFuncByName(info.name);
        if (!func)
        {
            if (sink)
            {
                UnownedStringSlice compilerName = TypeTextUtil::getPassThroughName(SlangPassThrough(info.compilerType));
                sink->diagnose(SourceLoc(), Diagnostics::failedToFindFunctionForCompiler, info.name, compilerName);
            }
            return nullptr;
        }

        // Store in the function cache
        m_sharedLibraryFunctions[int(type)] = func;
        return func;
    }

    void checkTranslationUnit(
        TranslationUnitRequest* translationUnit)
    {
        SharedSemanticsContext sharedSemanticsContext(
            translationUnit->compileRequest->getLinkage(),
            translationUnit->getModule(),
            translationUnit->compileRequest->getSink());

        SemanticsDeclVisitorBase visitor(&sharedSemanticsContext);

        // Apply the visitor to do the main semantic
        // checking that is required on all declarations
        // in the translation unit.

        visitor.checkModule(translationUnit->getModuleDecl());

        translationUnit->getModule()->_collectShaderParams();
    }

    void SemanticsVisitor::dispatchStmt(Stmt* stmt, FunctionDeclBase* parentFunc, OuterStmtInfo* outerStmts)
    {
        SemanticsStmtVisitor visitor(getShared(), parentFunc, outerStmts);
        try
        {
            visitor.dispatch(stmt);
        }
        catch(const AbortCompilationException&) { throw; }
        catch(...)
        {
            getSink()->noteInternalErrorLoc(stmt->loc);
            throw;
        }
    }

    void SemanticsVisitor::dispatchExpr(Expr* expr)
    {
        SemanticsExprVisitor visitor(getShared());
        try
        {
            visitor.dispatch(expr);
        }
        catch(const AbortCompilationException&) { throw; }
        catch(...)
        {
            getSink()->noteInternalErrorLoc(expr->loc);
            throw;
        }
    }
}
