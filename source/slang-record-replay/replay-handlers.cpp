// replay-handlers.cpp
// 
// This file registers all replay handlers for proxy methods.
// It is included in the slang library to enable automatic registration
// of handlers when the library is loaded.
//
// To add a new handler:
// 1. Implement the proxy method with RECORD_CALL(), RECORD_INPUT(), etc.
// 2. Add REPLAY_REGISTER(ProxyType, methodName) in registerAllHandlers()

#include "replay-context.h"
#include "proxy/proxy-macros.h"

// Include all proxy headers
#include "proxy/proxy-global-session.h"
#include "proxy/proxy-session.h"
#include "proxy/proxy-module.h"
#include "proxy/proxy-component-type.h"
#include "proxy/proxy-entry-point.h"
#include "proxy/proxy-type-conformance.h"
#include "proxy/proxy-compile-request.h"
#include "proxy/proxy-blob.h"
#include "proxy/proxy-shared-library.h"
#include "proxy/proxy-mutable-file-system.h"

#include <slang.h>
#include "../slang/slang-internal.h"

namespace SlangRecord {

// =============================================================================
// Static/Free Function Handlers
// =============================================================================

/// Handler for slang_createGlobalSession2
/// This is a special case - it's a free function, not a method on a proxy.
/// We call slang_createGlobalSessionImpl directly to avoid re-recording,
/// then wrap the result to match what was recorded.
static void handle_slang_createGlobalSession2(ReplayContext& ctx)
{
    // Read header
    const char* signature = nullptr;
    uint64_t thisHandle = 0;
    ctx.record(RecordFlag::Input, signature);
    ctx.recordHandle(RecordFlag::Input, thisHandle);

    // Read the input descriptor
    SlangGlobalSessionDesc desc = {};
    ctx.record(RecordFlag::Input, desc);

    // Call the implementation directly (not slang_createGlobalSession2) to avoid re-recording
    Slang::GlobalSessionInternalDesc internalDesc = {};
    slang::IGlobalSession* globalSession = nullptr;
    SlangResult result = slang_createGlobalSessionImpl(&desc, &internalDesc, &globalSession);
    
    // Wrap the session in a proxy (just like slang_createGlobalSession2 does during recording)
    if (SLANG_SUCCEEDED(result) && globalSession)
        globalSession = wrapObject(globalSession);
    
    // Read and verify the output (this will register the created session in the handle table)
    ctx.record(RecordFlag::Output, globalSession);
    
    // Read and verify the return value
    ctx.record(RecordFlag::ReturnValue, result);
}

/// Register all replay handlers.
/// This function is called during static initialization.
static void registerAllHandlers()
{
    // =========================================================================
    // Static/Free Function handlers
    // =========================================================================
    
    // slang_createGlobalSession2 - the entry point for creating global sessions
    ReplayContext::get().registerHandler("slang_createGlobalSession2", handle_slang_createGlobalSession2);
    
    // =========================================================================
    // GlobalSessionProxy handlers
    // =========================================================================
    
    REPLAY_REGISTER(GlobalSessionProxy, addRef);
    REPLAY_REGISTER(GlobalSessionProxy, release);
    REPLAY_REGISTER(GlobalSessionProxy, createSession);
    REPLAY_REGISTER(GlobalSessionProxy, findProfile);
    REPLAY_REGISTER(GlobalSessionProxy, setDownstreamCompilerPath);
    REPLAY_REGISTER(GlobalSessionProxy, setDownstreamCompilerPrelude);
    REPLAY_REGISTER(GlobalSessionProxy, getDownstreamCompilerPrelude);
    REPLAY_REGISTER(GlobalSessionProxy, getBuildTagString);
    REPLAY_REGISTER(GlobalSessionProxy, setDefaultDownstreamCompiler);
    REPLAY_REGISTER(GlobalSessionProxy, getDefaultDownstreamCompiler);
    REPLAY_REGISTER(GlobalSessionProxy, setLanguagePrelude);
    REPLAY_REGISTER(GlobalSessionProxy, getLanguagePrelude);
    REPLAY_REGISTER(GlobalSessionProxy, createCompileRequest);
    REPLAY_REGISTER(GlobalSessionProxy, addBuiltins);
    REPLAY_REGISTER(GlobalSessionProxy, setSharedLibraryLoader);
    REPLAY_REGISTER(GlobalSessionProxy, getSharedLibraryLoader);
    REPLAY_REGISTER(GlobalSessionProxy, checkCompileTargetSupport);
    REPLAY_REGISTER(GlobalSessionProxy, checkPassThroughSupport);
    REPLAY_REGISTER(GlobalSessionProxy, compileCoreModule);
    REPLAY_REGISTER(GlobalSessionProxy, loadCoreModule);
    REPLAY_REGISTER(GlobalSessionProxy, saveCoreModule);
    REPLAY_REGISTER(GlobalSessionProxy, findCapability);
    REPLAY_REGISTER(GlobalSessionProxy, setDownstreamCompilerForTransition);
    REPLAY_REGISTER(GlobalSessionProxy, getDownstreamCompilerForTransition);
    REPLAY_REGISTER(GlobalSessionProxy, getCompilerElapsedTime);
    REPLAY_REGISTER(GlobalSessionProxy, setSPIRVCoreGrammar);
    REPLAY_REGISTER(GlobalSessionProxy, parseCommandLineArguments);
    REPLAY_REGISTER(GlobalSessionProxy, getSessionDescDigest);
    REPLAY_REGISTER(GlobalSessionProxy, compileBuiltinModule);
    REPLAY_REGISTER(GlobalSessionProxy, loadBuiltinModule);
    REPLAY_REGISTER(GlobalSessionProxy, saveBuiltinModule);
    
    // =========================================================================
    // SessionProxy handlers
    // =========================================================================
    
    REPLAY_REGISTER(SessionProxy, addRef);
    REPLAY_REGISTER(SessionProxy, release);
    REPLAY_REGISTER(SessionProxy, getGlobalSession);
    REPLAY_REGISTER(SessionProxy, loadModule);
    REPLAY_REGISTER(SessionProxy, loadModuleFromSource);
    REPLAY_REGISTER(SessionProxy, loadModuleFromIRBlob);
    REPLAY_REGISTER(SessionProxy, loadModuleFromSourceString);
    REPLAY_REGISTER(SessionProxy, createCompositeComponentType);
    REPLAY_REGISTER(SessionProxy, specializeType);
    REPLAY_REGISTER(SessionProxy, getTypeLayout);
    REPLAY_REGISTER(SessionProxy, getContainerType);
    REPLAY_REGISTER(SessionProxy, getDynamicType);
    REPLAY_REGISTER(SessionProxy, getTypeRTTIMangledName);
    REPLAY_REGISTER(SessionProxy, getTypeConformanceWitnessMangledName);
    REPLAY_REGISTER(SessionProxy, getTypeConformanceWitnessSequentialID);
    REPLAY_REGISTER(SessionProxy, createCompileRequest);
    REPLAY_REGISTER(SessionProxy, createTypeConformanceComponentType);
    REPLAY_REGISTER(SessionProxy, isBinaryModuleUpToDate);
    
    // =========================================================================
    // ModuleProxy handlers  
    // =========================================================================
    
    REPLAY_REGISTER(ModuleProxy, addRef);
    REPLAY_REGISTER(ModuleProxy, release);
    REPLAY_REGISTER(ModuleProxy, getLayout);
    REPLAY_REGISTER(ModuleProxy, getSpecializationParamCount);
    REPLAY_REGISTER(ModuleProxy, specialize);
    REPLAY_REGISTER(ModuleProxy, link);
    REPLAY_REGISTER(ModuleProxy, findEntryPointByName);
    REPLAY_REGISTER(ModuleProxy, getDefinedEntryPointCount);
    REPLAY_REGISTER(ModuleProxy, getDefinedEntryPoint);
    REPLAY_REGISTER(ModuleProxy, serialize);
    REPLAY_REGISTER(ModuleProxy, writeToFile);
    REPLAY_REGISTER(ModuleProxy, getName);
    REPLAY_REGISTER(ModuleProxy, getFilePath);
    REPLAY_REGISTER(ModuleProxy, getUniqueIdentity);
    REPLAY_REGISTER(ModuleProxy, findAndCheckEntryPoint);
    REPLAY_REGISTER(ModuleProxy, getDependencyFileCount);
    REPLAY_REGISTER(ModuleProxy, getDependencyFilePath);
    REPLAY_REGISTER(ModuleProxy, getModuleReflection);
    REPLAY_REGISTER(ModuleProxy, precompileForTarget);
    
    // =========================================================================
    // ComponentTypeProxy handlers
    // =========================================================================
    
    REPLAY_REGISTER(ComponentTypeProxy, addRef);
    REPLAY_REGISTER(ComponentTypeProxy, release);
    REPLAY_REGISTER(ComponentTypeProxy, getSession);
    REPLAY_REGISTER(ComponentTypeProxy, getLayout);
    REPLAY_REGISTER(ComponentTypeProxy, getSpecializationParamCount);
    REPLAY_REGISTER(ComponentTypeProxy, getEntryPointCode);
    REPLAY_REGISTER(ComponentTypeProxy, getTargetCode);
    REPLAY_REGISTER(ComponentTypeProxy, getResultAsFileSystem);
    REPLAY_REGISTER(ComponentTypeProxy, getEntryPointHash);
    REPLAY_REGISTER(ComponentTypeProxy, specialize);
    REPLAY_REGISTER(ComponentTypeProxy, link);
    REPLAY_REGISTER(ComponentTypeProxy, getEntryPointHostCallable);
    REPLAY_REGISTER(ComponentTypeProxy, renameEntryPoint);
    REPLAY_REGISTER(ComponentTypeProxy, linkWithOptions);
    REPLAY_REGISTER(ComponentTypeProxy, getTargetHostCallable);
    REPLAY_REGISTER(ComponentTypeProxy, getEntryPointMetadata);
    REPLAY_REGISTER(ComponentTypeProxy, getTargetMetadata);
    
    // =========================================================================
    // EntryPointProxy handlers
    // =========================================================================
    
    REPLAY_REGISTER(EntryPointProxy, addRef);
    REPLAY_REGISTER(EntryPointProxy, release);
    REPLAY_REGISTER(EntryPointProxy, getSession);
    REPLAY_REGISTER(EntryPointProxy, getLayout);
    REPLAY_REGISTER(EntryPointProxy, getSpecializationParamCount);
    REPLAY_REGISTER(EntryPointProxy, getEntryPointCode);
    REPLAY_REGISTER(EntryPointProxy, getTargetCode);
    REPLAY_REGISTER(EntryPointProxy, getResultAsFileSystem);
    REPLAY_REGISTER(EntryPointProxy, getEntryPointHash);
    REPLAY_REGISTER(EntryPointProxy, specialize);
    REPLAY_REGISTER(EntryPointProxy, link);
    REPLAY_REGISTER(EntryPointProxy, getEntryPointHostCallable);
    REPLAY_REGISTER(EntryPointProxy, renameEntryPoint);
    REPLAY_REGISTER(EntryPointProxy, linkWithOptions);
    REPLAY_REGISTER(EntryPointProxy, getFunctionReflection);
    
    // =========================================================================
    // TypeConformanceProxy handlers
    // =========================================================================
    
    REPLAY_REGISTER(TypeConformanceProxy, addRef);
    REPLAY_REGISTER(TypeConformanceProxy, release);
    REPLAY_REGISTER(TypeConformanceProxy, getSession);
    REPLAY_REGISTER(TypeConformanceProxy, getLayout);
    REPLAY_REGISTER(TypeConformanceProxy, getSpecializationParamCount);
    REPLAY_REGISTER(TypeConformanceProxy, getEntryPointCode);
    REPLAY_REGISTER(TypeConformanceProxy, getTargetCode);
    REPLAY_REGISTER(TypeConformanceProxy, getResultAsFileSystem);
    REPLAY_REGISTER(TypeConformanceProxy, getEntryPointHash);
    REPLAY_REGISTER(TypeConformanceProxy, specialize);
    REPLAY_REGISTER(TypeConformanceProxy, link);
    REPLAY_REGISTER(TypeConformanceProxy, getEntryPointHostCallable);
    REPLAY_REGISTER(TypeConformanceProxy, renameEntryPoint);
    REPLAY_REGISTER(TypeConformanceProxy, linkWithOptions);
    
    // =========================================================================
    // CompileRequestProxy handlers
    // =========================================================================
    
    REPLAY_REGISTER(CompileRequestProxy, addRef);
    REPLAY_REGISTER(CompileRequestProxy, release);
    REPLAY_REGISTER(CompileRequestProxy, setFileSystem);
    REPLAY_REGISTER(CompileRequestProxy, setCompileFlags);
    REPLAY_REGISTER(CompileRequestProxy, getCompileFlags);
    REPLAY_REGISTER(CompileRequestProxy, setDumpIntermediates);
    REPLAY_REGISTER(CompileRequestProxy, setDumpIntermediatePrefix);
    REPLAY_REGISTER(CompileRequestProxy, setLineDirectiveMode);
    REPLAY_REGISTER(CompileRequestProxy, setCodeGenTarget);
    REPLAY_REGISTER(CompileRequestProxy, addCodeGenTarget);
    REPLAY_REGISTER(CompileRequestProxy, setTargetProfile);
    REPLAY_REGISTER(CompileRequestProxy, setTargetFlags);
    REPLAY_REGISTER(CompileRequestProxy, setTargetFloatingPointMode);
    REPLAY_REGISTER(CompileRequestProxy, setTargetMatrixLayoutMode);
    REPLAY_REGISTER(CompileRequestProxy, setMatrixLayoutMode);
    REPLAY_REGISTER(CompileRequestProxy, setDebugInfoLevel);
    REPLAY_REGISTER(CompileRequestProxy, setOptimizationLevel);
    REPLAY_REGISTER(CompileRequestProxy, setOutputContainerFormat);
    REPLAY_REGISTER(CompileRequestProxy, setPassThrough);
    REPLAY_REGISTER(CompileRequestProxy, setDiagnosticCallback);
    REPLAY_REGISTER(CompileRequestProxy, setWriter);
    REPLAY_REGISTER(CompileRequestProxy, addSearchPath);
    REPLAY_REGISTER(CompileRequestProxy, addPreprocessorDefine);
    REPLAY_REGISTER(CompileRequestProxy, processCommandLineArguments);
    REPLAY_REGISTER(CompileRequestProxy, addTranslationUnit);
    REPLAY_REGISTER(CompileRequestProxy, setDefaultModuleName);
    REPLAY_REGISTER(CompileRequestProxy, addTranslationUnitPreprocessorDefine);
    REPLAY_REGISTER(CompileRequestProxy, addTranslationUnitSourceFile);
    REPLAY_REGISTER(CompileRequestProxy, addTranslationUnitSourceString);
    REPLAY_REGISTER(CompileRequestProxy, addLibraryReference);
    REPLAY_REGISTER(CompileRequestProxy, addTranslationUnitSourceStringSpan);
    REPLAY_REGISTER(CompileRequestProxy, addTranslationUnitSourceBlob);
    REPLAY_REGISTER(CompileRequestProxy, addEntryPoint);
    REPLAY_REGISTER(CompileRequestProxy, addEntryPointEx);
    REPLAY_REGISTER(CompileRequestProxy, setGlobalGenericArgs);
    REPLAY_REGISTER(CompileRequestProxy, setTypeNameForGlobalExistentialTypeParam);
    REPLAY_REGISTER(CompileRequestProxy, setTypeNameForEntryPointExistentialTypeParam);
    REPLAY_REGISTER(CompileRequestProxy, compile);
    REPLAY_REGISTER(CompileRequestProxy, getDiagnosticOutput);
    REPLAY_REGISTER(CompileRequestProxy, getDiagnosticOutputBlob);
    REPLAY_REGISTER(CompileRequestProxy, getDependencyFileCount);
    REPLAY_REGISTER(CompileRequestProxy, getDependencyFilePath);
    REPLAY_REGISTER(CompileRequestProxy, getTranslationUnitCount);
    REPLAY_REGISTER(CompileRequestProxy, getEntryPointSource);
    REPLAY_REGISTER(CompileRequestProxy, getEntryPointCode);
    REPLAY_REGISTER(CompileRequestProxy, getEntryPointCodeBlob);
    REPLAY_REGISTER(CompileRequestProxy, getCompileRequestCode);
    REPLAY_REGISTER(CompileRequestProxy, getTargetCodeBlob);
    REPLAY_REGISTER(CompileRequestProxy, getTargetHostCallable);
    REPLAY_REGISTER(CompileRequestProxy, getContainerCode);
    REPLAY_REGISTER(CompileRequestProxy, loadRepro);
    REPLAY_REGISTER(CompileRequestProxy, saveRepro);
    REPLAY_REGISTER(CompileRequestProxy, enableReproCapture);
    REPLAY_REGISTER(CompileRequestProxy, getProgram);
    REPLAY_REGISTER(CompileRequestProxy, getProgramWithEntryPoints);
    REPLAY_REGISTER(CompileRequestProxy, isParameterLocationUsed);
    REPLAY_REGISTER(CompileRequestProxy, setTargetLineDirectiveMode);
    REPLAY_REGISTER(CompileRequestProxy, setTargetForceGLSLScalarBufferLayout);
    REPLAY_REGISTER(CompileRequestProxy, overrideDiagnosticSeverity);
    REPLAY_REGISTER(CompileRequestProxy, getDiagnosticFlags);
    REPLAY_REGISTER(CompileRequestProxy, setDiagnosticFlags);
    REPLAY_REGISTER(CompileRequestProxy, setDebugInfoFormat);
    REPLAY_REGISTER(CompileRequestProxy, setEnableEffectAnnotations);
    REPLAY_REGISTER(CompileRequestProxy, setReportDownstreamTime);
    REPLAY_REGISTER(CompileRequestProxy, setReportPerfBenchmark);
    REPLAY_REGISTER(CompileRequestProxy, setSkipSPIRVValidation);
    REPLAY_REGISTER(CompileRequestProxy, setTargetUseMinimumSlangOptimization);
    REPLAY_REGISTER(CompileRequestProxy, setIgnoreCapabilityCheck);
    REPLAY_REGISTER(CompileRequestProxy, getReflection);
    REPLAY_REGISTER(CompileRequestProxy, setCommandLineCompilerMode);
    // setSourceEmbedStyle, setSourceEmbedName, setSourceEmbedLanguage not implemented in proxy
    // setEmitSpirvMethod not implemented in proxy
    REPLAY_REGISTER(CompileRequestProxy, getModule);
    REPLAY_REGISTER(CompileRequestProxy, getSession);
    REPLAY_REGISTER(CompileRequestProxy, getEntryPoint);
    // setEmitSpirvDirectly not implemented in proxy
    
    // =========================================================================
    // BlobProxy handlers
    // =========================================================================
    
    REPLAY_REGISTER(BlobProxy, addRef);
    REPLAY_REGISTER(BlobProxy, release);
    REPLAY_REGISTER(BlobProxy, getBufferPointer);
    REPLAY_REGISTER(BlobProxy, getBufferSize);
    
    // =========================================================================
    // SharedLibraryProxy handlers
    // =========================================================================
    
    REPLAY_REGISTER(SharedLibraryProxy, addRef);
    REPLAY_REGISTER(SharedLibraryProxy, release);
    REPLAY_REGISTER(SharedLibraryProxy, findSymbolAddressByName);
    
    // =========================================================================
    // MutableFileSystemProxy handlers
    // =========================================================================
    
    REPLAY_REGISTER(MutableFileSystemProxy, addRef);
    REPLAY_REGISTER(MutableFileSystemProxy, release);
    REPLAY_REGISTER(MutableFileSystemProxy, loadFile);
    REPLAY_REGISTER(MutableFileSystemProxy, getFileUniqueIdentity);
    REPLAY_REGISTER(MutableFileSystemProxy, calcCombinedPath);
    REPLAY_REGISTER(MutableFileSystemProxy, getPathType);
    REPLAY_REGISTER(MutableFileSystemProxy, getPath);
    REPLAY_REGISTER(MutableFileSystemProxy, clearCache);
    REPLAY_REGISTER(MutableFileSystemProxy, enumeratePathContents);
    REPLAY_REGISTER(MutableFileSystemProxy, getOSPathKind);
    // getSimplifiedPath, getCanonicalPath not implemented in proxy
    REPLAY_REGISTER(MutableFileSystemProxy, saveFile);
    REPLAY_REGISTER(MutableFileSystemProxy, saveFileBlob);
    REPLAY_REGISTER(MutableFileSystemProxy, remove);
    REPLAY_REGISTER(MutableFileSystemProxy, createDirectory);
}

// Static initialization - register handlers when library loads
namespace {
    struct HandlerRegistrar {
        HandlerRegistrar() {
            registerAllHandlers();
        }
    };
    static HandlerRegistrar s_registrar;
}

} // namespace SlangRecord
