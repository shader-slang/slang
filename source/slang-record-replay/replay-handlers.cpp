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

namespace SlangRecord {

/// Register all replay handlers.
/// This function is called during static initialization.
static void registerAllHandlers()
{
    // =========================================================================
    // GlobalSessionProxy handlers
    // =========================================================================
    
    // Methods with full recording implementation:
    REPLAY_REGISTER(GlobalSessionProxy, createSession);
    REPLAY_REGISTER(GlobalSessionProxy, findProfile);
    
    // TODO: Add more methods as they are implemented with RECORD_CALL():
    // REPLAY_REGISTER(GlobalSessionProxy, setDownstreamCompilerPath);
    // REPLAY_REGISTER(GlobalSessionProxy, setDownstreamCompilerPrelude);
    // REPLAY_REGISTER(GlobalSessionProxy, getDownstreamCompilerPrelude);
    // REPLAY_REGISTER(GlobalSessionProxy, getBuildTagString);
    // REPLAY_REGISTER(GlobalSessionProxy, setDefaultDownstreamCompiler);
    // REPLAY_REGISTER(GlobalSessionProxy, getDefaultDownstreamCompiler);
    // REPLAY_REGISTER(GlobalSessionProxy, setLanguagePrelude);
    // REPLAY_REGISTER(GlobalSessionProxy, getLanguagePrelude);
    // REPLAY_REGISTER(GlobalSessionProxy, createCompileRequest);
    // REPLAY_REGISTER(GlobalSessionProxy, addBuiltins);
    // REPLAY_REGISTER(GlobalSessionProxy, setSharedLibraryLoader);
    // REPLAY_REGISTER(GlobalSessionProxy, getSharedLibraryLoader);
    // REPLAY_REGISTER(GlobalSessionProxy, checkCompileTargetSupport);
    // REPLAY_REGISTER(GlobalSessionProxy, checkPassThroughSupport);
    // REPLAY_REGISTER(GlobalSessionProxy, compileCoreModule);
    // REPLAY_REGISTER(GlobalSessionProxy, loadCoreModule);
    // REPLAY_REGISTER(GlobalSessionProxy, saveCoreModule);
    // REPLAY_REGISTER(GlobalSessionProxy, findCapability);
    // REPLAY_REGISTER(GlobalSessionProxy, setDownstreamCompilerForTransition);
    // REPLAY_REGISTER(GlobalSessionProxy, getDownstreamCompilerForTransition);
    // REPLAY_REGISTER(GlobalSessionProxy, getCompilerElapsedTime);
    // REPLAY_REGISTER(GlobalSessionProxy, setSPIRVCoreGrammar);
    // REPLAY_REGISTER(GlobalSessionProxy, parseCommandLineArguments);
    // REPLAY_REGISTER(GlobalSessionProxy, getSessionDescDigest);
    // REPLAY_REGISTER(GlobalSessionProxy, compileBuiltinModule);
    // REPLAY_REGISTER(GlobalSessionProxy, loadBuiltinModule);
    // REPLAY_REGISTER(GlobalSessionProxy, saveBuiltinModule);
    
    // =========================================================================
    // SessionProxy handlers
    // =========================================================================
    
    // TODO: Add SessionProxy methods as they are implemented:
    // REPLAY_REGISTER(SessionProxy, getGlobalSession);
    // REPLAY_REGISTER(SessionProxy, loadModule);
    // REPLAY_REGISTER(SessionProxy, loadModuleFromSource);
    // REPLAY_REGISTER(SessionProxy, createCompositeComponentType);
    // ... etc.
    
    // =========================================================================
    // ModuleProxy handlers  
    // =========================================================================
    
    // TODO: Add ModuleProxy methods as they are implemented
    
    // =========================================================================
    // ComponentTypeProxy handlers
    // =========================================================================
    
    // TODO: Add ComponentTypeProxy methods as they are implemented
    
    // =========================================================================
    // EntryPointProxy handlers
    // =========================================================================
    
    // TODO: Add EntryPointProxy methods as they are implemented
    
    // =========================================================================
    // TypeConformanceProxy handlers
    // =========================================================================
    
    // TODO: Add TypeConformanceProxy methods as they are implemented
    
    // =========================================================================
    // CompileRequestProxy handlers
    // =========================================================================
    
    // TODO: Add CompileRequestProxy methods as they are implemented
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
