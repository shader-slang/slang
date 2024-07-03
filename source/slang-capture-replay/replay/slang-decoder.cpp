#include "slang-decoder.h"
#include "parameter-decoder.h"
#include "../capture-utility.h"

namespace SlangCapture
{

    bool SlangDecoder::processMethodCall(FunctionHeader const& header, ParameterBlock const& parameterBlock)
    {
        ApiClassId classId = static_cast<ApiClassId>(getClassId(header.callId));
        switch(classId)
        {
        default:
            slangCaptureLog(LogLevel::Error, "Unhandled Slang Class Id: %d\n", classId);
            return false;
        case ApiClassId::Class_IGlobalSession:
            return processIGlobalSessionMethods(header.callId, parameterBlock);
            break;
        case ApiClassId::Class_ISession:
            return processISessionMethods(header.callId, parameterBlock);
            break;
        case ApiClassId::Class_IModule:
            return processIModuleMethods(header.callId, parameterBlock);
            break;
        case ApiClassId::Class_IEntryPoint:
            return processIEntryPointMethods(header.callId, parameterBlock);
            break;
        case ApiClassId::Class_ICompositeComponentType:
            return processICompositeComponentTypeMethods(header.callId, parameterBlock);
            break;
        case ApiClassId::Class_ITypeConformance:
            return processITypeConformanceMethods(header.callId, parameterBlock);
            break;
        }
    }

    bool SlangDecoder::processIGlobalSessionMethods(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
        switch(callId)
        {
        default:
            slangCaptureLog(LogLevel::Error, "Unhandled Slang API call: %d\n", callId);
            break;
        case ApiCallId::ICreateGlobalSession:
            ICreateGlobalSession(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_createSession:
            IGlobalSession_createSession(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_findProfile:
            IGlobalSession_findProfile(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_setDownstreamCompilerPath:
            IGlobalSession_setDownstreamCompilerPath(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_setDownstreamCompilerPrelude:
            IGlobalSession_setDownstreamCompilerPrelude(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_getDownstreamCompilerPrelude:
            IGlobalSession_getDownstreamCompilerPrelude(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_getBuildTagString:
            IGlobalSession_getBuildTagString(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_setDefaultDownstreamCompiler:
            IGlobalSession_setDefaultDownstreamCompiler(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_getDefaultDownstreamCompiler:
            IGlobalSession_getDefaultDownstreamCompiler(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_setLanguagePrelude:
            IGlobalSession_setLanguagePrelude(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_getLanguagePrelude:
            IGlobalSession_getLanguagePrelude(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_createCompileRequest:
            IGlobalSession_createCompileRequest(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_addBuiltins:
            IGlobalSession_addBuiltins(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_setSharedLibraryLoader:
            IGlobalSession_setSharedLibraryLoader(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_getSharedLibraryLoader:
            IGlobalSession_getSharedLibraryLoader(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_checkCompileTargetSupport:
            IGlobalSession_checkCompileTargetSupport(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_checkPassThroughSupport:
            IGlobalSession_checkPassThroughSupport(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_compileStdLib:
            IGlobalSession_compileStdLib(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_loadStdLib:
            IGlobalSession_loadStdLib(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_saveStdLib:
            IGlobalSession_saveStdLib(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_findCapability:
            IGlobalSession_findCapability(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_setDownstreamCompilerForTransition:
            IGlobalSession_setDownstreamCompilerForTransition(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_getDownstreamCompilerForTransition:
            IGlobalSession_getDownstreamCompilerForTransition(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_getCompilerElapsedTime:
            IGlobalSession_getCompilerElapsedTime(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_setSPIRVCoreGrammar:
            IGlobalSession_setSPIRVCoreGrammar(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_parseCommandLineArguments:
            IGlobalSession_parseCommandLineArguments(callId, parameterBlock);
            break;
        case ApiCallId::IGlobalSession_getSessionDescDigest:
            IGlobalSession_getSessionDescDigest(callId, parameterBlock);
            break;
        }
        return true;
    }


    bool SlangDecoder::processISessionMethods(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
        switch(callId)
        {
        default:
            slangCaptureLog(LogLevel::Error, "Unhandled Slang API call: %d\n", callId);
            return false;
        case ApiCallId::ISession_getGlobalSession:
            ISession_getGlobalSession(callId, parameterBlock);
            break;
        case ApiCallId::ISession_loadModule:
            ISession_loadModule(callId, parameterBlock);
            break;
        case ApiCallId::ISession_loadModuleFromBlob:
            ISession_loadModuleFromBlob(callId, parameterBlock);
            break;
        case ApiCallId::ISession_loadModuleFromIRBlob:
            ISession_loadModuleFromIRBlob(callId, parameterBlock);
            break;
        case ApiCallId::ISession_loadModuleFromSource:
            ISession_loadModuleFromSource(callId, parameterBlock);
            break;
        case ApiCallId::ISession_loadModuleFromSourceString:
            ISession_loadModuleFromSourceString(callId, parameterBlock);
            break;
        case ApiCallId::ISession_createCompositeComponentType:
            ISession_createCompositeComponentType(callId, parameterBlock);
            break;
        case ApiCallId::ISession_specializeType:
            ISession_specializeType(callId, parameterBlock);
            break;
        case ApiCallId::ISession_getTypeLayout:
            ISession_getTypeLayout(callId, parameterBlock);
            break;
        case ApiCallId::ISession_getContainerType:
            ISession_getContainerType(callId, parameterBlock);
            break;
        case ApiCallId::ISession_getDynamicType:
            ISession_getDynamicType(callId, parameterBlock);
            break;
        case ApiCallId::ISession_getTypeRTTIMangledName:
            ISession_getTypeRTTIMangledName(callId, parameterBlock);
            break;
        case ApiCallId::ISession_getTypeConformanceWitnessMangledName:
            ISession_getTypeConformanceWitnessMangledName(callId, parameterBlock);
            break;
        case ApiCallId::ISession_getTypeConformanceWitnessSequentialID:
            ISession_getTypeConformanceWitnessSequentialID(callId, parameterBlock);
            break;
        case ApiCallId::ISession_createTypeConformanceComponentType:
            ISession_createTypeConformanceComponentType(callId, parameterBlock);
            break;
        case ApiCallId::ISession_createCompileRequest:
            ISession_createCompileRequest(callId, parameterBlock);
            break;
        case ApiCallId::ISession_getLoadedModuleCount:
            ISession_getLoadedModuleCount(callId, parameterBlock);
            break;
        case ApiCallId::ISession_getLoadedModule:
            ISession_getLoadedModule(callId, parameterBlock);
            break;
        case ApiCallId::ISession_isBinaryModuleUpToDate:
            ISession_isBinaryModuleUpToDate(callId, parameterBlock);
            break;
        }
        return true;
    }

    bool SlangDecoder::processIModuleMethods(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
        switch(callId)
        {
        default:
            slangCaptureLog(LogLevel::Error, "Unhandled Slang API call: %d\n", callId);
            return false;
        case ApiCallId::IModule_findEntryPointByName:
            IModule_findEntryPointByName(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getDefinedEntryPointCount:
            IModule_getDefinedEntryPointCount(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getDefinedEntryPoint:
            IModule_getDefinedEntryPoint(callId, parameterBlock);
            break;
        case ApiCallId::IModule_serialize:
            IModule_serialize(callId, parameterBlock);
            break;
        case ApiCallId::IModule_writeToFile:
            IModule_writeToFile(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getName:
            IModule_getName(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getFilePath:
            IModule_getFilePath(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getUniqueIdentity:
            IModule_getUniqueIdentity(callId, parameterBlock);
            break;
        case ApiCallId::IModule_findAndCheckEntryPoint:
            IModule_findAndCheckEntryPoint(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getSession:
            IModule_getSession(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getLayout:
            IModule_getLayout(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getSpecializationParamCount:
            IModule_getSpecializationParamCount(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getEntryPointCode:
            IModule_getEntryPointCode(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getTargetCode:
            IModule_getTargetCode(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getResultAsFileSystem:
            IModule_getResultAsFileSystem(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getEntryPointHash:
            IModule_getEntryPointHash(callId, parameterBlock);
            break;
        case ApiCallId::IModule_specialize:
            IModule_specialize(callId, parameterBlock);
            break;
        case ApiCallId::IModule_link:
            IModule_link(callId, parameterBlock);
            break;
        case ApiCallId::IModule_getEntryPointHostCallable:
            IModule_getEntryPointHostCallable(callId, parameterBlock);
            break;
        case ApiCallId::IModule_renameEntryPoint:
            IModule_renameEntryPoint(callId, parameterBlock);
            break;
        case ApiCallId::IModule_linkWithOptions:
            IModule_linkWithOptions(callId, parameterBlock);
            break;
        }
        return true;
    }

    bool SlangDecoder::processIEntryPointMethods(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
        switch(callId)
        {
        default:
            slangCaptureLog(LogLevel::Error, "Unhandled Slang API call: %d\n", callId);
            return false;
        case ApiCallId::IEntryPoint_getSession:
            IEntryPoint_getSession(callId, parameterBlock);
            break;
        case ApiCallId::IEntryPoint_getLayout:
            IEntryPoint_getLayout(callId, parameterBlock);
            break;
        case ApiCallId::IEntryPoint_getSpecializationParamCount:
            IEntryPoint_getSpecializationParamCount(callId, parameterBlock);
            break;
        case ApiCallId::IEntryPoint_getEntryPointCode:
            IEntryPoint_getEntryPointCode(callId, parameterBlock);
            break;
        case ApiCallId::IEntryPoint_getTargetCode:
            IEntryPoint_getTargetCode(callId, parameterBlock);
            break;
        case ApiCallId::IEntryPoint_getResultAsFileSystem:
            IEntryPoint_getResultAsFileSystem(callId, parameterBlock);
            break;
        case ApiCallId::IEntryPoint_getEntryPointHash:
            IEntryPoint_getEntryPointHash(callId, parameterBlock);
            break;
        case ApiCallId::IEntryPoint_specialize:
            IEntryPoint_specialize(callId, parameterBlock);
            break;
        case ApiCallId::IEntryPoint_link:
            IEntryPoint_link(callId, parameterBlock);
            break;
        case ApiCallId::IEntryPoint_getEntryPointHostCallable:
            IEntryPoint_getEntryPointHostCallable(callId, parameterBlock);
            break;
        case ApiCallId::IEntryPoint_renameEntryPoint:
            IEntryPoint_renameEntryPoint(callId, parameterBlock);
            break;
        case ApiCallId::IEntryPoint_linkWithOptions:
            IEntryPoint_linkWithOptions(callId, parameterBlock);
            break;
        }
        return true;
    }

    bool SlangDecoder::processICompositeComponentTypeMethods(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
        switch(callId)
        {
        default:
            slangCaptureLog(LogLevel::Error, "Unhandled Slang API call: %d\n", callId);
            break;
        case ApiCallId::ICompositeComponentType_getSession:
            ICompositeComponentType_getSession(callId, parameterBlock);
            break;
        case ApiCallId::ICompositeComponentType_getLayout:
            ICompositeComponentType_getLayout(callId, parameterBlock);
            break;
        case ApiCallId::ICompositeComponentType_getSpecializationParamCount:
            ICompositeComponentType_getSpecializationParamCount(callId, parameterBlock);
            break;
        case ApiCallId::ICompositeComponentType_getEntryPointCode:
            ICompositeComponentType_getEntryPointCode(callId, parameterBlock);
            break;
        case ApiCallId::ICompositeComponentType_getTargetCode:
            ICompositeComponentType_getTargetCode(callId, parameterBlock);
            break;
        case ApiCallId::ICompositeComponentType_getResultAsFileSystem:
            ICompositeComponentType_getResultAsFileSystem(callId, parameterBlock);
            break;
        case ApiCallId::ICompositeComponentType_getEntryPointHash:
            ICompositeComponentType_getEntryPointHash(callId, parameterBlock);
            break;
        case ApiCallId::ICompositeComponentType_specialize:
            ICompositeComponentType_specialize(callId, parameterBlock);
            break;
        case ApiCallId::ICompositeComponentType_link:
            ICompositeComponentType_link(callId, parameterBlock);
            break;
        case ApiCallId::ICompositeComponentType_getEntryPointHostCallable:
            ICompositeComponentType_getEntryPointHostCallable(callId, parameterBlock);
            break;
        case ApiCallId::ICompositeComponentType_renameEntryPoint:
            ICompositeComponentType_renameEntryPoint(callId, parameterBlock);
            break;
        case ApiCallId::ICompositeComponentType_linkWithOptions:
            ICompositeComponentType_linkWithOptions(callId, parameterBlock);
            break;
        }
        return true;
    }

    bool SlangDecoder::processITypeConformanceMethods(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
        switch(callId)
        {
        default:
            slangCaptureLog(LogLevel::Error, "Unhandled Slang API call: %d\n", callId);
            return false;
        case ApiCallId::ITypeConformance_getSession:
            ITypeConformance_getSession(callId, parameterBlock);
            break;
        case ApiCallId::ITypeConformance_getLayout:
            ITypeConformance_getLayout(callId, parameterBlock);
            break;
        case ApiCallId::ITypeConformance_getSpecializationParamCount:
            ITypeConformance_getSpecializationParamCount(callId, parameterBlock);
            break;
        case ApiCallId::ITypeConformance_getEntryPointCode:
            ITypeConformance_getEntryPointCode(callId, parameterBlock);
            break;
        case ApiCallId::ITypeConformance_getTargetCode:
            ITypeConformance_getTargetCode(callId, parameterBlock);
            break;
        case ApiCallId::ITypeConformance_getResultAsFileSystem:
            ITypeConformance_getResultAsFileSystem(callId, parameterBlock);
            break;
        case ApiCallId::ITypeConformance_getEntryPointHash:
            ITypeConformance_getEntryPointHash(callId, parameterBlock);
            break;
        case ApiCallId::ITypeConformance_specialize:
            ITypeConformance_specialize(callId, parameterBlock);
            break;
        case ApiCallId::ITypeConformance_link:
            ITypeConformance_link(callId, parameterBlock);
            break;
        case ApiCallId::ITypeConformance_getEntryPointHostCallable:
            ITypeConformance_getEntryPointHostCallable(callId, parameterBlock);
            break;
        case ApiCallId::ITypeConformance_renameEntryPoint:
            ITypeConformance_renameEntryPoint(callId, parameterBlock);
            break;
        case ApiCallId::ITypeConformance_linkWithOptions:
            ITypeConformance_linkWithOptions(callId, parameterBlock);
            break;
        }
        return true;
    }

    bool SlangDecoder::processFunctionCall(FunctionHeader const& header, ParameterBlock const& parameterBlock)
    {
    }


    void SlangDecoder::ICreateGlobalSession(ApiCallId callId, ParameterBlock const& parameterBlock)
    {

    }

    void SlangDecoder::IGlobalSession_createSession(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
        TypeDecoder<slang::SessionDesc> sessionDesc;
        ParameterDecoder::decodeStruct(parameterBlock.parameterBuffer, parameterBlock.parameterBufferSize, sessionDesc);

        for (auto consumer: m_consumers)
        {
            // consume the session desc
            // consumer->IGlobalSession_createSession(object, sessionDesc);
        }
    }

    void SlangDecoder::IGlobalSession_findProfile(ApiCallId callId, ParameterBlock const& parameterBlock)
    {

    }

    void SlangDecoder::IGlobalSession_setDownstreamCompilerPath(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_setDownstreamCompilerPrelude(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_getDownstreamCompilerPrelude(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_getBuildTagString(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_setDefaultDownstreamCompiler(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_getDefaultDownstreamCompiler(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_setLanguagePrelude(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_getLanguagePrelude(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_createCompileRequest(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_addBuiltins(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_setSharedLibraryLoader(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_getSharedLibraryLoader(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_checkCompileTargetSupport(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_checkPassThroughSupport(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_compileStdLib(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_loadStdLib(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_saveStdLib(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_findCapability(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_setDownstreamCompilerForTransition(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_getDownstreamCompilerForTransition(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_getCompilerElapsedTime(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_setSPIRVCoreGrammar(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_parseCommandLineArguments(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IGlobalSession_getSessionDescDigest(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }


    void SlangDecoder::ISession_getGlobalSession(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_loadModule(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_loadModuleFromBlob(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_loadModuleFromIRBlob(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_loadModuleFromSource(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_loadModuleFromSourceString(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_createCompositeComponentType(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_specializeType(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_getTypeLayout(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_getContainerType(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_getDynamicType(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_getTypeRTTIMangledName(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_getTypeConformanceWitnessMangledName(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_getTypeConformanceWitnessSequentialID(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_createTypeConformanceComponentType(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_createCompileRequest(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_getLoadedModuleCount(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_getLoadedModule(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ISession_isBinaryModuleUpToDate(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }


    void SlangDecoder::IModule_findEntryPointByName(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getDefinedEntryPointCount(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getDefinedEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_serialize(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_writeToFile(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getName(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getFilePath(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getUniqueIdentity(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_findAndCheckEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getSession(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getLayout(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getSpecializationParamCount(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getEntryPointCode(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getTargetCode(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getResultAsFileSystem(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getEntryPointHash(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_specialize(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_link(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_getEntryPointHostCallable(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_renameEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IModule_linkWithOptions(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }


    void SlangDecoder::IEntryPoint_getSession(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IEntryPoint_getLayout(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IEntryPoint_getSpecializationParamCount(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IEntryPoint_getEntryPointCode(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IEntryPoint_getTargetCode(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IEntryPoint_getResultAsFileSystem(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IEntryPoint_getEntryPointHash(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IEntryPoint_specialize(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IEntryPoint_link(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IEntryPoint_getEntryPointHostCallable(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IEntryPoint_renameEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::IEntryPoint_linkWithOptions(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }


    void SlangDecoder::ICompositeComponentType_getSession(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ICompositeComponentType_getLayout(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ICompositeComponentType_getSpecializationParamCount(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ICompositeComponentType_getEntryPointCode(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ICompositeComponentType_getTargetCode(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ICompositeComponentType_getResultAsFileSystem(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ICompositeComponentType_getEntryPointHash(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ICompositeComponentType_specialize(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ICompositeComponentType_link(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ICompositeComponentType_getEntryPointHostCallable(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ICompositeComponentType_renameEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ICompositeComponentType_linkWithOptions(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }


    void SlangDecoder::ITypeConformance_getSession(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ITypeConformance_getLayout(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ITypeConformance_getSpecializationParamCount(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ITypeConformance_getEntryPointCode(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ITypeConformance_getTargetCode(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ITypeConformance_getResultAsFileSystem(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ITypeConformance_getEntryPointHash(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ITypeConformance_specialize(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ITypeConformance_link(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ITypeConformance_getEntryPointHostCallable(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ITypeConformance_renameEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

    void SlangDecoder::ITypeConformance_linkWithOptions(ApiCallId callId, ParameterBlock const& parameterBlock)
    {
    }

}
