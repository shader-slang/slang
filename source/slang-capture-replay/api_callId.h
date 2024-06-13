#ifndef API_CALL_ID_H
#define API_CALL_ID_H

#include <cstdint>

namespace SlangCapture
{
    constexpr uint32_t makeApiCallId(uint16_t classId, uint16_t memberFunctionId)
    {
        return ((static_cast<uint32_t>(classId) << 16) & 0xffff0000) | (static_cast<uint32_t>(memberFunctionId) & 0x0000ffff);
    }

    constexpr uint16_t getClassId(uint32_t callId)
    {
        return static_cast<uint16_t>((callId >> 16) & 0x0000ffff);
    }

    constexpr uint16_t getMemberFunctionId(uint32_t callId)
    {
        return static_cast<uint16_t>(callId & 0x0000ffff);
    }

    enum ApiClassId : uint16_t
    {
        GlobalFunction                   = 1,
        Class_IGlobalSession             = 2,
        Class_ISession                   = 3,
        Class_IModule                    = 4,
        Class_IEntryPoint                = 5,
        Class_ICompositeComponentType    = 6,
        Class_ITypeConformance           = 7,
    };

    typedef uint64_t AddressFormat;

    constexpr uint64_t g_globalFunctionHandle = 0;

    enum ApiCallId : uint32_t
    {
        ICreateGlobalSession                                 = makeApiCallId(GlobalFunction, 0x0000),
        IGlobalSession_createSession                         = makeApiCallId(Class_IGlobalSession, 0x0001),
        IGlobalSession_findProfile                           = makeApiCallId(Class_IGlobalSession, 0x0002),
        IGlobalSession_setDownstreamCompilerPath             = makeApiCallId(Class_IGlobalSession, 0x0003),
        IGlobalSession_setDownstreamCompilerPrelude          = makeApiCallId(Class_IGlobalSession, 0x0004),
        IGlobalSession_getDownstreamCompilerPrelude          = makeApiCallId(Class_IGlobalSession, 0x0005),
        IGlobalSession_getBuildTagString                     = makeApiCallId(Class_IGlobalSession, 0x0006),
        IGlobalSession_setDefaultDownstreamCompiler          = makeApiCallId(Class_IGlobalSession, 0x0007),
        IGlobalSession_getDefaultDownstreamCompiler          = makeApiCallId(Class_IGlobalSession, 0x0008),
        IGlobalSession_setLanguagePrelude                    = makeApiCallId(Class_IGlobalSession, 0x0009),
        IGlobalSession_getLanguagePrelude                    = makeApiCallId(Class_IGlobalSession, 0x000A),
        IGlobalSession_createCompileRequest                  = makeApiCallId(Class_IGlobalSession, 0x000B),
        IGlobalSession_addBuiltins                           = makeApiCallId(Class_IGlobalSession, 0x000C),
        IGlobalSession_setSharedLibraryLoader                = makeApiCallId(Class_IGlobalSession, 0x000D),
        IGlobalSession_getSharedLibraryLoader                = makeApiCallId(Class_IGlobalSession, 0x000E),
        IGlobalSession_checkCompileTargetSupport             = makeApiCallId(Class_IGlobalSession, 0x000F),
        IGlobalSession_checkPassThroughSupport               = makeApiCallId(Class_IGlobalSession, 0x0010),
        IGlobalSession_compileStdLib                         = makeApiCallId(Class_IGlobalSession, 0x0011),
        IGlobalSession_loadStdLib                            = makeApiCallId(Class_IGlobalSession, 0x0012),
        IGlobalSession_saveStdLib                            = makeApiCallId(Class_IGlobalSession, 0x0013),
        IGlobalSession_findCapability                        = makeApiCallId(Class_IGlobalSession, 0x0014),
        IGlobalSession_setDownstreamCompilerForTransition    = makeApiCallId(Class_IGlobalSession, 0x0015),
        IGlobalSession_getDownstreamCompilerForTransition    = makeApiCallId(Class_IGlobalSession, 0x0016),
        IGlobalSession_getCompilerElapsedTime                = makeApiCallId(Class_IGlobalSession, 0x0017),
        IGlobalSession_setSPIRVCoreGrammar                   = makeApiCallId(Class_IGlobalSession, 0x0018),
        IGlobalSession_parseCommandLineArguments             = makeApiCallId(Class_IGlobalSession, 0x0019),
        IGlobalSession_getSessionDescDigest                  = makeApiCallId(Class_IGlobalSession, 0x001A),

        ISession_getGlobalSession                            = makeApiCallId(Class_ISession, 0x0001),
        ISession_loadModule                                  = makeApiCallId(Class_ISession, 0x0002),
        ISession_loadModuleFromBlob                          = makeApiCallId(Class_ISession, 0x0003),
        ISession_loadModuleFromIRBlob                        = makeApiCallId(Class_ISession, 0x0004),
        ISession_loadModuleFromSource                        = makeApiCallId(Class_ISession, 0x0005),
        ISession_loadModuleFromSourceString                  = makeApiCallId(Class_ISession, 0x0006),
        ISession_createCompositeComponentType                = makeApiCallId(Class_ISession, 0x0007),
        ISession_specializeType                              = makeApiCallId(Class_ISession, 0x0008),
        ISession_getTypeLayout                               = makeApiCallId(Class_ISession, 0x0009),
        ISession_getContainerType                            = makeApiCallId(Class_ISession, 0x000A),
        ISession_getDynamicType                              = makeApiCallId(Class_ISession, 0x000B),
        ISession_getTypeRTTIMangledName                      = makeApiCallId(Class_ISession, 0x000C),
        ISession_getTypeConformanceWitnessMangledName        = makeApiCallId(Class_ISession, 0x000D),
        ISession_getTypeConformanceWitnessSequentialID       = makeApiCallId(Class_ISession, 0x000E),
        ISession_createTypeConformanceComponentType          = makeApiCallId(Class_ISession, 0x000F),
        ISession_createCompileRequest                        = makeApiCallId(Class_ISession, 0x0010),
        ISession_getLoadedModuleCount                        = makeApiCallId(Class_ISession, 0x0011),
        ISession_getLoadedModule                             = makeApiCallId(Class_ISession, 0x0012),
        ISession_isBinaryModuleUpToDate                      = makeApiCallId(Class_ISession, 0x0013),

        IModule_findEntryPointByName                         = makeApiCallId(Class_IModule, 0x0001),
        IModule_getDefinedEntryPointCount                    = makeApiCallId(Class_IModule, 0x0002),
        IModule_getDefinedEntryPoint                         = makeApiCallId(Class_IModule, 0x0003),
        IModule_serialize                                    = makeApiCallId(Class_IModule, 0x0004),
        IModule_writeToFile                                  = makeApiCallId(Class_IModule, 0x0005),
        IModule_getName                                      = makeApiCallId(Class_IModule, 0x0006),
        IModule_getFilePath                                  = makeApiCallId(Class_IModule, 0x0007),
        IModule_getUniqueIdentity                            = makeApiCallId(Class_IModule, 0x0008),
        IModule_findAndCheckEntryPoint                       = makeApiCallId(Class_IModule, 0x0009),
        IModule_getSession                                   = makeApiCallId(Class_IModule, 0x000A),
        IModule_getLayout                                    = makeApiCallId(Class_IModule, 0x000B),
        IModule_getSpecializationParamCount                  = makeApiCallId(Class_IModule, 0x000C),
        IModule_getEntryPointCode                            = makeApiCallId(Class_IModule, 0x000D),
        IModule_getResultAsFileSystem                        = makeApiCallId(Class_IModule, 0x000E),
        IModule_getEntryPointHash                            = makeApiCallId(Class_IModule, 0x000F),
        IModule_specialize                                   = makeApiCallId(Class_IModule, 0x0010),
        IModule_link                                         = makeApiCallId(Class_IModule, 0x0011),
        IModule_getEntryPointHostCallable                    = makeApiCallId(Class_IModule, 0x0012),
        IModule_renameEntryPoint                             = makeApiCallId(Class_IModule, 0x0013),
        IModule_linkWithOptions                              = makeApiCallId(Class_IModule, 0x0014),

        IEntryPoint_getSession                               = makeApiCallId(Class_IEntryPoint, 0x0001),
        IEntryPoint_getLayout                                = makeApiCallId(Class_IEntryPoint, 0x0002),
        IEntryPoint_getSpecializationParamCount              = makeApiCallId(Class_IEntryPoint, 0x0003),
        IEntryPoint_getEntryPointCode                        = makeApiCallId(Class_IEntryPoint, 0x0004),
        IEntryPoint_getResultAsFileSystem                    = makeApiCallId(Class_IEntryPoint, 0x0005),
        IEntryPoint_getEntryPointHash                        = makeApiCallId(Class_IEntryPoint, 0x0006),
        IEntryPoint_specialize                               = makeApiCallId(Class_IEntryPoint, 0x0007),
        IEntryPoint_link                                     = makeApiCallId(Class_IEntryPoint, 0x0008),
        IEntryPoint_getEntryPointHostCallable                = makeApiCallId(Class_IEntryPoint, 0x0009),
        IEntryPoint_renameEntryPoint                         = makeApiCallId(Class_IEntryPoint, 0x000A),
        IEntryPoint_linkWithOptions                          = makeApiCallId(Class_IEntryPoint, 0x000B),

        ICompositeComponentType_getSession                   = makeApiCallId(Class_ICompositeComponentType, 0x0001),
        ICompositeComponentType_getLayout                    = makeApiCallId(Class_ICompositeComponentType, 0x0002),
        ICompositeComponentType_getSpecializationParamCount  = makeApiCallId(Class_ICompositeComponentType, 0x0003),
        ICompositeComponentType_getEntryPointCode            = makeApiCallId(Class_ICompositeComponentType, 0x0004),
        ICompositeComponentType_getResultAsFileSystem        = makeApiCallId(Class_ICompositeComponentType, 0x0005),
        ICompositeComponentType_getEntryPointHash            = makeApiCallId(Class_ICompositeComponentType, 0x0006),
        ICompositeComponentType_specialize                   = makeApiCallId(Class_ICompositeComponentType, 0x0007),
        ICompositeComponentType_link                         = makeApiCallId(Class_ICompositeComponentType, 0x0008),
        ICompositeComponentType_getEntryPointHostCallable    = makeApiCallId(Class_ICompositeComponentType, 0x0009),
        ICompositeComponentType_renameEntryPoint             = makeApiCallId(Class_ICompositeComponentType, 0x000A),
        ICompositeComponentType_linkWithOptions              = makeApiCallId(Class_ICompositeComponentType, 0x000B),

        ITypeConformance_getSession                          = makeApiCallId(Class_ITypeConformance, 0x0001),
        ITypeConformance_getLayout                           = makeApiCallId(Class_ITypeConformance, 0x0002),
        ITypeConformance_getSpecializationParamCount         = makeApiCallId(Class_ITypeConformance, 0x0003),
        ITypeConformance_getEntryPointCode                   = makeApiCallId(Class_ITypeConformance, 0x0004),
        ITypeConformance_getResultAsFileSystem               = makeApiCallId(Class_ITypeConformance, 0x0005),
        ITypeConformance_getEntryPointHash                   = makeApiCallId(Class_ITypeConformance, 0x0006),
        ITypeConformance_specialize                          = makeApiCallId(Class_ITypeConformance, 0x0007),
        ITypeConformance_link                                = makeApiCallId(Class_ITypeConformance, 0x0008),
        ITypeConformance_getEntryPointHostCallable           = makeApiCallId(Class_ITypeConformance, 0x0009),
        ITypeConformance_renameEntryPoint                    = makeApiCallId(Class_ITypeConformance, 0x000A),
        ITypeConformance_linkWithOptions                     = makeApiCallId(Class_ITypeConformance, 0x000B)
    };
}
#endif
