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
        SharedLibraryType libraryType;
    };
    } // anonymous

    static FunctionInfo _getFunctionInfo(Session::SharedLibraryFuncType funcType)
    {
        typedef Session::SharedLibraryFuncType FuncType;
        typedef SharedLibraryType LibType;

        switch (funcType)
        {
            case FuncType::Glslang_Compile:   return { "glslang_compile", LibType::Glslang } ;
            case FuncType::Fxc_D3DCompile:     return { "D3DCompile", LibType::Fxc };
            case FuncType::Fxc_D3DDisassemble: return { "D3DDisassemble", LibType::Fxc };
            case FuncType::Dxc_DxcCreateInstance:  return { "DxcCreateInstance", LibType::Dxc };
            default: return { nullptr, LibType::Unknown };
        } 
    }

    static PassThroughMode _toPassThroughMode(SharedLibraryType type)
    {
        switch (type)
        {
            case SharedLibraryType::Dxil:
            case SharedLibraryType::Dxc:
            {
                return PassThroughMode::Dxc;
            }
            case SharedLibraryType::Fxc:        return PassThroughMode::Fxc;
            case SharedLibraryType::Glslang:    return PassThroughMode::Glslang;
            default: break;
        }

        return PassThroughMode::None;    
    }

    void Session::setSharedLibrary(SharedLibraryType type, ISlangSharedLibrary* library)
    {
        sharedLibraries[int(type)] = library;
    }

    ISlangSharedLibrary* Session::getOrLoadSharedLibrary(SharedLibraryType type, DiagnosticSink* sink)
    {
        // If not loaded, try loading it
        if (!sharedLibraries[int(type)])
        {
            // Try to preload dxil first, if loading dxc
            if (type == SharedLibraryType::Dxc)
            {
                // Pass nullptr as the sink, because if it fails we don't want to report as error
                getOrLoadSharedLibrary(SharedLibraryType::Dxil, nullptr);
            }

            const char* libName = DefaultSharedLibraryLoader::getSharedLibraryNameFromType(type);

            StringBuilder builder;
            PassThroughMode passThrough = _toPassThroughMode(type);
            if (passThrough != PassThroughMode::None && m_downstreamCompilerPaths[int(passThrough)].getLength() > 0)
            {
                Path::combineIntoBuilder(m_downstreamCompilerPaths[int(passThrough)].getUnownedSlice(), UnownedStringSlice(libName), builder);
                libName = builder.getBuffer();
            }

            if (SLANG_FAILED(sharedLibraryLoader->loadSharedLibrary(libName, sharedLibraries[int(type)].writeRef())))
            {
                if (sink)
                {
                    sink->diagnose(SourceLoc(), Diagnostics::failedToLoadDynamicLibrary, libName);
                }
                return nullptr;
            }
        }
        return sharedLibraries[int(type)];
    }

    SlangFuncPtr Session::getSharedLibraryFunc(SharedLibraryFuncType type, DiagnosticSink* sink)
    {
        if (sharedLibraryFunctions[int(type)])
        {
            return sharedLibraryFunctions[int(type)];
        }
        // do we have the library
        FunctionInfo info = _getFunctionInfo(type);
        if (info.name == nullptr)
        {
            return nullptr;
        }
        // Try loading the library
        ISlangSharedLibrary* sharedLib = getOrLoadSharedLibrary(info.libraryType, sink);
        if (!sharedLib)
        {
            return nullptr;
        }

        // Okay now access the func
        SlangFuncPtr func = sharedLib->findFuncByName(info.name);
        if (!func)
        {
            const char* libName = DefaultSharedLibraryLoader::getSharedLibraryNameFromType(info.libraryType);
            sink->diagnose(SourceLoc(), Diagnostics::failedToFindFunctionInSharedLibrary, info.name, libName);
            return nullptr;
        }

        // Store in the function cache
        sharedLibraryFunctions[int(type)] = func;
        return func;
    }

    CPPCompilerSet* Session::requireCPPCompilerSet()
    {
        if (cppCompilerSet == nullptr)
        {
            cppCompilerSet = new CPPCompilerSet;

            typedef CPPCompiler::CompilerType CompilerType;
            CPPCompilerUtil::InitializeSetDesc desc;
       
            desc.paths[int(CompilerType::GCC)] = m_downstreamCompilerPaths[int(PassThroughMode::Gcc)];
            desc.paths[int(CompilerType::Clang)] = m_downstreamCompilerPaths[int(PassThroughMode::Clang)];
            desc.paths[int(CompilerType::VisualStudio)] = m_downstreamCompilerPaths[int(PassThroughMode::VisualStudio)];

            CPPCompilerUtil::initializeSet(desc, cppCompilerSet);
        }
        SLANG_ASSERT(cppCompilerSet);
        return cppCompilerSet;
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
        SemanticsVisitor visitor(
            translationUnit->compileRequest->getLinkage(),
            translationUnit->compileRequest->getSink());

        // Apply the visitor to do the main semantic
        // checking that is required on all declarations
        // in the translation unit.
        visitor.checkDecl(translationUnit->getModuleDecl());

        translationUnit->getModule()->_collectShaderParams();
    }

    void SemanticsVisitor::dispatchDecl(DeclBase* decl)
    {
        try
        {
            DeclVisitor::dispatch(decl);
        }
        // Don't emit any context message for an explicit `AbortCompilationException`
        // because it should only happen when an error is already emitted.
        catch(AbortCompilationException&) { throw; }
        catch(...)
        {
            getSink()->noteInternalErrorLoc(decl->loc);
            throw;
        }
    }

    void SemanticsVisitor::dispatchStmt(Stmt* stmt)
    {
        try
        {
            StmtVisitor::dispatch(stmt);
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
        try
        {
            ExprVisitor::dispatch(expr);
        }
        catch(AbortCompilationException&) { throw; }
        catch(...)
        {
            getSink()->noteInternalErrorLoc(expr->loc);
            throw;
        }
    }
}
