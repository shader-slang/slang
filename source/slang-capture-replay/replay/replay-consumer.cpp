#include "replay-consumer.h"

namespace SlangCapture
{
    void ReplayConsumer::ICreateGlobalSession(ObjectID objectId, const uint8_t* parameterBuffer, int64_t bufferSize)
    {
    }

    void ReplayConsumer::IGlobalSession_createSession(ObjectID objectId, slang::SessionDesc const&  desc, ObjectID outSessionId)
    {
    }


    void ReplayConsumer::IGlobalSession_findProfile(ObjectID objectId, char const* name)
    {
    }


    void ReplayConsumer::IGlobalSession_setDownstreamCompilerPath(ObjectID objectId, SlangPassThrough passThrough, char const* path)
    {
    }


    void ReplayConsumer::IGlobalSession_setDownstreamCompilerPrelude(ObjectID objectId, SlangPassThrough inPassThrough, char const* prelude)
    {
    }


    void ReplayConsumer::IGlobalSession_getDownstreamCompilerPrelude(ObjectID objectId, SlangPassThrough inPassThrough, ObjectID outPreludeId)
    {
    }


    void ReplayConsumer::IGlobalSession_setDefaultDownstreamCompiler(ObjectID objectId, SlangSourceLanguage sourceLanguage, SlangPassThrough defaultCompiler)
    {
    }


    void ReplayConsumer::IGlobalSession_getDefaultDownstreamCompiler(ObjectID objectId, SlangSourceLanguage sourceLanguage)
    {
    }


    void ReplayConsumer::IGlobalSession_setLanguagePrelude(ObjectID objectId, SlangSourceLanguage inSourceLanguage, char const* prelude)
    {
    }


    void ReplayConsumer::IGlobalSession_getLanguagePrelude(ObjectID objectId, SlangSourceLanguage inSourceLanguage, ObjectID outPreludeId)
    {
    }


    void ReplayConsumer::IGlobalSession_createCompileRequest(ObjectID objectId, ObjectID outCompileRequest)
    {
    }


    void ReplayConsumer::IGlobalSession_addBuiltins(ObjectID objectId, char const* sourcePath, char const* sourceString)
    {
    }


    void ReplayConsumer::IGlobalSession_setSharedLibraryLoader(ObjectID objectId, ObjectID loaderId)
    {
    }


    void ReplayConsumer::IGlobalSession_getSharedLibraryLoader(ObjectID objectId, ObjectID outLoaderId)
    {
    }


    void ReplayConsumer::IGlobalSession_checkCompileTargetSupport(ObjectID objectId, SlangCompileTarget target)
    {
    }


    void ReplayConsumer::IGlobalSession_checkPassThroughSupport(ObjectID objectId, SlangPassThrough passThrough)
    {
    }


    void ReplayConsumer::IGlobalSession_compileStdLib(ObjectID objectId, slang::CompileStdLibFlags flags)
    {
    }


    void ReplayConsumer::IGlobalSession_loadStdLib(ObjectID objectId, const void* stdLib, size_t stdLibSizeInBytes)
    {
    }


    void ReplayConsumer::IGlobalSession_saveStdLib(ObjectID objectId, SlangArchiveType archiveType, ObjectID outBlobId)
    {
    }


    void ReplayConsumer::IGlobalSession_findCapability(ObjectID objectId, char const* name)
    {
    }


    void ReplayConsumer::IGlobalSession_setDownstreamCompilerForTransition(ObjectID objectId, SlangCompileTarget source, SlangCompileTarget target, SlangPassThrough compiler)
    {
    }


    void ReplayConsumer::IGlobalSession_getDownstreamCompilerForTransition(ObjectID objectId, SlangCompileTarget source, SlangCompileTarget target)
    {
    }


    void ReplayConsumer::IGlobalSession_setSPIRVCoreGrammar(ObjectID objectId, char const* jsonPath)
    {
    }


    void ReplayConsumer::IGlobalSession_parseCommandLineArguments(ObjectID objectId, int argc, const char* const* argv, ObjectID outSessionDescId, ObjectID outAllocationId)
    {
    }


    void ReplayConsumer::IGlobalSession_getSessionDescDigest(ObjectID objectId, slang::SessionDesc* sessionDesc, ObjectID outBlobId)
    {
    }



    // ISession
    void ReplayConsumer::ISession_getGlobalSession(ObjectID objectId, ObjectID outGlobalSessionId)
    {
    }


    void ReplayConsumer::ISession_loadModule(ObjectID objectId, const char* moduleName, ObjectID outDiagnostics, ObjectID outModuleId)
    {
    }


    void ReplayConsumer::ISession_loadModuleFromIRBlob(ObjectID objectId, const char* moduleName,
            const char* path, slang::IBlob* source, ObjectID outDiagnosticsId, ObjectID outModuleId)
    {
    }


    void ReplayConsumer::ISession_loadModuleFromSource(ObjectID objectId, const char* moduleName,
            const char* path, slang::IBlob* source, ObjectID outDiagnosticsId, ObjectID outModuleId)
    {
    }


    void ReplayConsumer::ISession_loadModuleFromSourceString(ObjectID objectId, const char* moduleName,
            const char* path, const char* string, ObjectID outDiagnosticsId, ObjectID outModuleId)
    {
    }


    void ReplayConsumer::ISession_createCompositeComponentType(ObjectID objectId, ObjectID* componentTypeIds,
        SlangInt componentTypeCount, ObjectID outCompositeComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void ReplayConsumer::ISession_specializeType(ObjectID objectId, ObjectID typeId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outDiagnosticsId, ObjectID outTypeReflectionId)
    {
    }



    void ReplayConsumer::ISession_getTypeLayout(ObjectID objectId, ObjectID typeId, SlangInt targetIndex,
            slang::LayoutRules rules,  ObjectID outDiagnosticsId, ObjectID outTypeLayoutReflection)
    {
    }

    void ReplayConsumer::ISession_getContainerType(ObjectID objectId, ObjectID elementType,
            slang::ContainerType containerType, ObjectID outDiagnosticsId, ObjectID outTypeReflectionId)
    {
    }

    void ReplayConsumer::ISession_getDynamicType(ObjectID objectId, ObjectID outTypeReflectionId)
    {
    }

    void ReplayConsumer::ISession_getTypeRTTIMangledName(ObjectID objectId, ObjectID typeId, ObjectID outNameBlobId)
    {
    }

    void ReplayConsumer::ISession_getTypeConformanceWitnessMangledName(ObjectID objectId, ObjectID typeId,
			ObjectID interfaceTypeId, ObjectID outNameBlobId)
    {
    }


    void ReplayConsumer::ISession_getTypeConformanceWitnessSequentialID(ObjectID objectId, ObjectID typeId,
            ObjectID interfaceTypeId, uint32_t outId)
    {
    }

    void ReplayConsumer::ISession_createTypeConformanceComponentType(ObjectID objectId, ObjectID typeId,
            ObjectID interfaceTypeId, ObjectID outConformanceId,
            SlangInt conformanceIdOverride, ObjectID outDiagnosticsId)
    {
    }



    void ReplayConsumer::ISession_createCompileRequest(ObjectID objectId, ObjectID outCompileRequestId)
    {
    }


    void ReplayConsumer::ISession_getLoadedModule(ObjectID objectId, SlangInt index, ObjectID outModuleId)
    {
    }



    // IModule
    void ReplayConsumer::IModule_findEntryPointByName(ObjectID objectId, char const* name, ObjectID outEntryPointId)
    {
    }


    void ReplayConsumer::IModule_getDefinedEntryPoint(ObjectID objectId, SlangInt32 index, ObjectID outEntryPointId)
    {
    }


    void ReplayConsumer::IModule_serialize(ObjectID objectId, ObjectID outSerializedBlobId)
    {
    }


    void ReplayConsumer::IModule_writeToFile(ObjectID objectId, char const* fileName)
    {
    }


    void ReplayConsumer::IModule_findAndCheckEntryPoint(ObjectID objectId, char const* name, SlangStage stage, ObjectID outEntryPointId, ObjectID outDiagnostics)
    {
    }



    void ReplayConsumer::IModule_getSession(ObjectID objectId, ObjectID outSessionId)
    {
    }


    void ReplayConsumer::IModule_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
    }


    void ReplayConsumer::IModule_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::IModule_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::IModule_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystem)
    {
    }


    void ReplayConsumer::IModule_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
    }


    void ReplayConsumer::IModule_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void ReplayConsumer::IModule_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void ReplayConsumer::IModule_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibrary, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::IModule_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
    }


    void ReplayConsumer::IModule_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
    }



    // IEntryPoint
    void ReplayConsumer::IEntryPoint_getSession(ObjectID objectId, ObjectID outSessionId)
    {
    }


    void ReplayConsumer::IEntryPoint_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
    }


    void ReplayConsumer::IEntryPoint_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::IEntryPoint_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::IEntryPoint_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystem)
    {
    }


    void ReplayConsumer::IEntryPoint_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
    }


    void ReplayConsumer::IEntryPoint_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void ReplayConsumer::IEntryPoint_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void ReplayConsumer::IEntryPoint_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibrary, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::IEntryPoint_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
    }


    void ReplayConsumer::IEntryPoint_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
    }



    // ICompositeComponentType
    void ReplayConsumer::ICompositeComponentType_getSession(ObjectID objectId, ObjectID outSessionId)
    {
    }


    void ReplayConsumer::ICompositeComponentType_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
    }


    void ReplayConsumer::ICompositeComponentType_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::ICompositeComponentType_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::ICompositeComponentType_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystem)
    {
    }


    void ReplayConsumer::ICompositeComponentType_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
    }


    void ReplayConsumer::ICompositeComponentType_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void ReplayConsumer::ICompositeComponentType_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void ReplayConsumer::ICompositeComponentType_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibrary, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::ICompositeComponentType_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
    }


    void ReplayConsumer::ICompositeComponentType_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
    }



    // ITypeConformance
    void ReplayConsumer::ITypeConformance_getSession(ObjectID objectId, ObjectID outSessionId)
    {
    }


    void ReplayConsumer::ITypeConformance_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
    }


    void ReplayConsumer::ITypeConformance_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::ITypeConformance_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::ITypeConformance_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystem)
    {
    }


    void ReplayConsumer::ITypeConformance_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
    }


    void ReplayConsumer::ITypeConformance_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void ReplayConsumer::ITypeConformance_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void ReplayConsumer::ITypeConformance_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibrary, ObjectID outDiagnostics)
    {
    }


    void ReplayConsumer::ITypeConformance_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
    }


    void ReplayConsumer::ITypeConformance_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
    }


}; // namespace SlangCapture
