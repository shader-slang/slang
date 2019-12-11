// slang-check.cpp
#include "slang-check.h"

// This file provides general facilities related to semantic
// checking that don't cleanly land in one of the more
// specialized `slang-check-*` files.

#include "slang-check-impl.h"

namespace Slang
{
    namespace { // anonymous
    struct FunctionInfo
    {
        const char* name;
        PassThroughMode compilerType;
    };

    const Guid IID_ISlangSharedLibraryLoader = SLANG_UUID_ISlangSharedLibraryLoader;
    const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;

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
                    m_sink->diagnose(SourceLoc(), Diagnostics::failedToLoadDynamicLibrary, path);
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
            return (guid == IID_ISlangUnknown || guid == IID_ISlangSharedLibraryLoader) ? static_cast<ISlangSharedLibraryLoader*>(this) : nullptr;
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
            case FuncType::Glslang_Compile:   return { "glslang_compile", PassThroughMode::Glslang} ;
            case FuncType::Fxc_D3DCompile:     return { "D3DCompile", PassThroughMode::Fxc};
            case FuncType::Fxc_D3DDisassemble: return { "D3DDisassemble", PassThroughMode::Fxc };
            case FuncType::Dxc_DxcCreateInstance:  return { "DxcCreateInstance", PassThroughMode::Dxc };
            default: return { nullptr, PassThroughMode::None };
        } 
    }

    void Session::setSharedLibraryLoader(ISlangSharedLibraryLoader* loader)
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
            // try loading all C/C++ compilers
            getOrLoadDownstreamCompiler(PassThroughMode::Clang, sink);
            getOrLoadDownstreamCompiler(PassThroughMode::Gcc, sink);
            getOrLoadDownstreamCompiler(PassThroughMode::VisualStudio, sink);
        }

        // Mark that we have tried to load it
        m_downstreamCompilerInitialized |= (1 << int(type));
        m_downstreamCompilers[int(type)].setNull();

        // Do we have a locator
        auto locator = m_downstreamCompilerLocators[int(type)];
        if (!locator)
        {
            return nullptr;
        }

        m_downstreamCompilerSet->remove(SlangPassThrough(type));

        SinkSharedLibraryLoader loader(m_sharedLibraryLoader, sink);
        locator(m_downstreamCompilerPaths[int(type)], &loader, m_downstreamCompilerSet);

        DownstreamCompilerUtil::updateDefaults(m_downstreamCompilerSet);

        if (type == PassThroughMode::GenericCCpp)
        {
            m_downstreamCompilers[int(type)] = m_downstreamCompilerSet->getDefaultCompiler(DownstreamCompiler::SourceType::CPP);
        }
        else
        {
            DownstreamCompiler::Desc desc;
            desc.type = SlangPassThrough(type);

            m_downstreamCompilers[int(type)] = DownstreamCompilerUtil::findCompiler(m_downstreamCompilerSet, DownstreamCompilerUtil::MatchType::Newest, desc);
        }

        return m_downstreamCompilers[int(type)];
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
            UnownedStringSlice compilerName = DownstreamCompiler::getCompilerTypeAsText(SlangPassThrough(info.compilerType));
            sink->diagnose(SourceLoc(), Diagnostics::failedToFindFunctionForCompiler, info.name, compilerName);
            return nullptr;
        }

        // Store in the function cache
        m_sharedLibraryFunctions[int(type)] = func;
        return func;
    }

    TypeCheckingCache* Session::getTypeCheckingCache()
    {
        if (!typeCheckingCache)
            typeCheckingCache = new TypeCheckingCache();
        return typeCheckingCache;
    }

    void Session::destroyTypeCheckingCache()
    {
        delete typeCheckingCache;
        typeCheckingCache = nullptr;
    }

    void checkTranslationUnit(
        TranslationUnitRequest* translationUnit)
    {
        SharedSemanticsContext sharedSemanticsContext(
            translationUnit->compileRequest->getLinkage(),
            translationUnit->compileRequest->getSink());

        SemanticsDeclVisitorBase visitor(&sharedSemanticsContext);

        // Apply the visitor to do the main semantic
        // checking that is required on all declarations
        // in the translation unit.

        visitor.checkModule(translationUnit->getModuleDecl());

        translationUnit->getModule()->_collectShaderParams();
    }

    void SemanticsVisitor::dispatchStmt(Stmt* stmt, FuncDecl* parentFunc, OuterStmtInfo* outerStmts)
    {
        SemanticsStmtVisitor visitor(getShared(), parentFunc, outerStmts);
        try
        {
            visitor.dispatch(stmt);
        }
        catch(AbortCompilationException&) { throw; }
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
        catch(AbortCompilationException&) { throw; }
        catch(...)
        {
            getSink()->noteInternalErrorLoc(expr->loc);
            throw;
        }
    }
}
