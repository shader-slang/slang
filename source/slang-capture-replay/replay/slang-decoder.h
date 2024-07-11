#ifndef SLANG_DECODER_H
#define SLANG_DECODER_H

#include <vector>
#include <unordered_map>
#include "../util/capture-format.h"
#include "decoder-consumer.h"

namespace SlangCapture
{
    class SlangDecoder {
    public:
		struct ParameterBlock
		{
			const uint8_t* parameterBuffer = nullptr;
			int64_t parameterBufferSize = 0;

			const uint8_t* outputBuffer = nullptr;
			int64_t outputBufferSize = 0;
		};

        SlangDecoder() {};
        ~SlangDecoder() {};

		bool processMethodCall(FunctionHeader const& header, ParameterBlock const& parameterBlock);
		bool processFunctionCall(FunctionHeader const& header, ParameterBlock const& parameterBlock);

		bool processIGlobalSessionMethods(ApiCallId callId, ParameterBlock const& parameterBlock);
		bool processISessionMethods(ApiCallId callId, ParameterBlock const& parameterBlock);
		bool processIModuleMethods(ApiCallId callId, ParameterBlock const& parameterBlock);
		bool processIEntryPointMethods(ApiCallId callId, ParameterBlock const& parameterBlock);
		bool processICompositeComponentTypeMethods(ApiCallId callId, ParameterBlock const& parameterBlock);
		bool processITypeConformanceMethods(ApiCallId callId, ParameterBlock const& parameterBlock);

		void ICreateGlobalSession(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_createSession(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_findProfile(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_setDownstreamCompilerPath(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_setDownstreamCompilerPrelude(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_getDownstreamCompilerPrelude(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_getBuildTagString(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_setDefaultDownstreamCompiler(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_getDefaultDownstreamCompiler(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_setLanguagePrelude(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_getLanguagePrelude(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_createCompileRequest(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_addBuiltins(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_setSharedLibraryLoader(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_getSharedLibraryLoader(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_checkCompileTargetSupport(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_checkPassThroughSupport(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_compileStdLib(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_loadStdLib(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_saveStdLib(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_findCapability(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_setDownstreamCompilerForTransition(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_getDownstreamCompilerForTransition(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_getCompilerElapsedTime(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_setSPIRVCoreGrammar(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_parseCommandLineArguments(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IGlobalSession_getSessionDescDigest(ApiCallId callId, ParameterBlock const& parameterBlock);

		void ISession_getGlobalSession(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_loadModule(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_loadModuleFromBlob(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_loadModuleFromIRBlob(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_loadModuleFromSource(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_loadModuleFromSourceString(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_createCompositeComponentType(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_specializeType(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_getTypeLayout(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_getContainerType(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_getDynamicType(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_getTypeRTTIMangledName(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_getTypeConformanceWitnessMangledName(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_getTypeConformanceWitnessSequentialID(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_createTypeConformanceComponentType(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_createCompileRequest(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_getLoadedModuleCount(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_getLoadedModule(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ISession_isBinaryModuleUpToDate(ApiCallId callId, ParameterBlock const& parameterBlock);

		void IModule_findEntryPointByName(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getDefinedEntryPointCount(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getDefinedEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_serialize(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_writeToFile(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getName(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getFilePath(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getUniqueIdentity(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_findAndCheckEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getSession(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getLayout(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getSpecializationParamCount(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getEntryPointCode(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getTargetCode(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getResultAsFileSystem(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getEntryPointHash(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_specialize(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_link(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_getEntryPointHostCallable(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_renameEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IModule_linkWithOptions(ApiCallId callId, ParameterBlock const& parameterBlock);

		void IEntryPoint_getSession(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IEntryPoint_getLayout(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IEntryPoint_getSpecializationParamCount(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IEntryPoint_getEntryPointCode(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IEntryPoint_getTargetCode(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IEntryPoint_getResultAsFileSystem(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IEntryPoint_getEntryPointHash(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IEntryPoint_specialize(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IEntryPoint_link(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IEntryPoint_getEntryPointHostCallable(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IEntryPoint_renameEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock);
		void IEntryPoint_linkWithOptions(ApiCallId callId, ParameterBlock const& parameterBlock);

		void ICompositeComponentType_getSession(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ICompositeComponentType_getLayout(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ICompositeComponentType_getSpecializationParamCount(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ICompositeComponentType_getEntryPointCode(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ICompositeComponentType_getTargetCode(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ICompositeComponentType_getResultAsFileSystem(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ICompositeComponentType_getEntryPointHash(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ICompositeComponentType_specialize(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ICompositeComponentType_link(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ICompositeComponentType_getEntryPointHostCallable(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ICompositeComponentType_renameEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ICompositeComponentType_linkWithOptions(ApiCallId callId, ParameterBlock const& parameterBlock);

		void ITypeConformance_getSession(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ITypeConformance_getLayout(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ITypeConformance_getSpecializationParamCount(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ITypeConformance_getEntryPointCode(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ITypeConformance_getTargetCode(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ITypeConformance_getResultAsFileSystem(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ITypeConformance_getEntryPointHash(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ITypeConformance_specialize(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ITypeConformance_link(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ITypeConformance_getEntryPointHostCallable(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ITypeConformance_renameEntryPoint(ApiCallId callId, ParameterBlock const& parameterBlock);
		void ITypeConformance_linkWithOptions(ApiCallId callId, ParameterBlock const& parameterBlock);

	private:
		std::vector<DecoderConsumerBase*> m_consumers;

		// Map of the address of the object allocated by slang during capture to
		// the address of the object allocated by the replay.
		// We need to have this map because we never save the content of the object
		// allocated by slang, because those are just opaque objects or handles, we
		// only need to provide them to the corresponding replay function or call the
		// methods on the correct object.
		std::unordered_map<AddressFormat, void*> m_addressMap;
    };
}
#endif // SLANG_DECODER_H
