#ifndef DECODER_CONSUMER_H
#define DECODER_CONSUMER_H

namespace SlangCapture
{
    class DecoderConsumerBase
    {
    public:
        virtual ~DecoderConsumerBase() {}

        virtual void ICreateGlobalSession(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_createSession(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_findProfile(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_setDownstreamCompilerPath(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_setDownstreamCompilerPrelude(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_getDownstreamCompilerPrelude(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_getBuildTagString(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_setDefaultDownstreamCompiler(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_getDefaultDownstreamCompiler(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_setLanguagePrelude(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_getLanguagePrelude(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_createCompileRequest(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_addBuiltins(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_setSharedLibraryLoader(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_getSharedLibraryLoader(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_checkCompileTargetSupport(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_checkPassThroughSupport(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_compileStdLib(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_loadStdLib(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_saveStdLib(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_findCapability(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_setDownstreamCompilerForTransition(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_getDownstreamCompilerForTransition(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_getCompilerElapsedTime(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_setSPIRVCoreGrammar(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_parseCommandLineArguments(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IGlobalSession_getSessionDescDigest(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;

		virtual void ISession_getGlobalSession(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_loadModule(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_loadModuleFromBlob(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_loadModuleFromIRBlob(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_loadModuleFromSource(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_loadModuleFromSourceString(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_createCompositeComponentType(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_specializeType(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_getTypeLayout(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_getContainerType(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_getDynamicType(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_getTypeRTTIMangledName(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_getTypeConformanceWitnessMangledName(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_getTypeConformanceWitnessSequentialID(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_createTypeConformanceComponentType(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_createCompileRequest(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_getLoadedModuleCount(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_getLoadedModule(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ISession_isBinaryModuleUpToDate(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;

		virtual void IModule_findEntryPointByName(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getDefinedEntryPointCount(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getDefinedEntryPoint(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_serialize(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_writeToFile(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getName(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getFilePath(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getUniqueIdentity(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_findAndCheckEntryPoint(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getSession(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getLayout(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getSpecializationParamCount(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getEntryPointCode(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getTargetCode(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getResultAsFileSystem(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getEntryPointHash(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_specialize(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_link(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_getEntryPointHostCallable(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_renameEntryPoint(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IModule_linkWithOptions(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;

		virtual void IEntryPoint_getSession(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IEntryPoint_getLayout(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IEntryPoint_getSpecializationParamCount(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IEntryPoint_getEntryPointCode(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IEntryPoint_getTargetCode(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IEntryPoint_getResultAsFileSystem(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IEntryPoint_getEntryPointHash(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IEntryPoint_specialize(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IEntryPoint_link(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IEntryPoint_getEntryPointHostCallable(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IEntryPoint_renameEntryPoint(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void IEntryPoint_linkWithOptions(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;

		virtual void ICompositeComponentType_getSession(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ICompositeComponentType_getLayout(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ICompositeComponentType_getSpecializationParamCount(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ICompositeComponentType_getEntryPointCode(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ICompositeComponentType_getTargetCode(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ICompositeComponentType_getResultAsFileSystem(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ICompositeComponentType_getEntryPointHash(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ICompositeComponentType_specialize(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ICompositeComponentType_link(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ICompositeComponentType_getEntryPointHostCallable(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ICompositeComponentType_renameEntryPoint(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ICompositeComponentType_linkWithOptions(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;

		virtual void ITypeConformance_getSession(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ITypeConformance_getLayout(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ITypeConformance_getSpecializationParamCount(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ITypeConformance_getEntryPointCode(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ITypeConformance_getTargetCode(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ITypeConformance_getResultAsFileSystem(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ITypeConformance_getEntryPointHash(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ITypeConformance_specialize(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ITypeConformance_link(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ITypeConformance_getEntryPointHostCallable(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ITypeConformance_renameEntryPoint(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
		virtual void ITypeConformance_linkWithOptions(ApiCallId callId, const uint8_t* parameterBuffer, int64_t bufferSize) = 0;
    };
}


#endif // DECODER_CONSUMER_H
