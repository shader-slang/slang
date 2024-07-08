#include "json-consumer.h"

namespace SlangCapture
{
    void JsonConsumer::ICreateGlobalSession(ObjectID objectId, const uint8_t* parameterBuffer, int64_t bufferSize)
    {
    }

    void JsonConsumer::IGlobalSession_createSession(ObjectID objectId, slang::SessionDesc const&  desc, ObjectID outSessionId)
    {
    }


    void JsonConsumer::IGlobalSession_findProfile(ObjectID objectId, char const* name)
    {
    }


    void JsonConsumer::IGlobalSession_setDownstreamCompilerPath(ObjectID objectId, SlangPassThrough passThrough, char const* path)
    {
    }


    void JsonConsumer::IGlobalSession_setDownstreamCompilerPrelude(ObjectID objectId, SlangPassThrough inPassThrough, char const* prelude)
    {
    }


    void JsonConsumer::IGlobalSession_getDownstreamCompilerPrelude(ObjectID objectId, SlangPassThrough inPassThrough, ObjectID outPreludeId)
    {
    }


    void JsonConsumer::IGlobalSession_setDefaultDownstreamCompiler(ObjectID objectId, SlangSourceLanguage sourceLanguage, SlangPassThrough defaultCompiler)
    {
    }


    void JsonConsumer::IGlobalSession_getDefaultDownstreamCompiler(ObjectID objectId, SlangSourceLanguage sourceLanguage)
    {
    }


    void JsonConsumer::IGlobalSession_setLanguagePrelude(ObjectID objectId, SlangSourceLanguage inSourceLanguage, char const* prelude)
    {
    }


    void JsonConsumer::IGlobalSession_getLanguagePrelude(ObjectID objectId, SlangSourceLanguage inSourceLanguage, ObjectID outPreludeId)
    {
    }


    void JsonConsumer::IGlobalSession_createCompileRequest(ObjectID objectId, ObjectID outCompileRequest)
    {
    }


    void JsonConsumer::IGlobalSession_addBuiltins(ObjectID objectId, char const* sourcePath, char const* sourceString)
    {
    }


    void JsonConsumer::IGlobalSession_setSharedLibraryLoader(ObjectID objectId, ObjectID loaderId)
    {
    }


    void JsonConsumer::IGlobalSession_getSharedLibraryLoader(ObjectID objectId, ObjectID outLoaderId)
    {
    }


    void JsonConsumer::IGlobalSession_checkCompileTargetSupport(ObjectID objectId, SlangCompileTarget target)
    {
    }


    void JsonConsumer::IGlobalSession_checkPassThroughSupport(ObjectID objectId, SlangPassThrough passThrough)
    {
    }


    void JsonConsumer::IGlobalSession_compileStdLib(ObjectID objectId, slang::CompileStdLibFlags flags)
    {
    }


    void JsonConsumer::IGlobalSession_loadStdLib(ObjectID objectId, const void* stdLib, size_t stdLibSizeInBytes)
    {
    }


    void JsonConsumer::IGlobalSession_saveStdLib(ObjectID objectId, SlangArchiveType archiveType, ObjectID outBlobId)
    {
    }


    void JsonConsumer::IGlobalSession_findCapability(ObjectID objectId, char const* name)
    {
    }


    void JsonConsumer::IGlobalSession_setDownstreamCompilerForTransition(ObjectID objectId, SlangCompileTarget source, SlangCompileTarget target, SlangPassThrough compiler)
    {
    }


    void JsonConsumer::IGlobalSession_getDownstreamCompilerForTransition(ObjectID objectId, SlangCompileTarget source, SlangCompileTarget target)
    {
    }


    void JsonConsumer::IGlobalSession_setSPIRVCoreGrammar(ObjectID objectId, char const* jsonPath)
    {
    }


    void JsonConsumer::IGlobalSession_parseCommandLineArguments(ObjectID objectId, int argc, const char* const* argv, ObjectID outSessionDescId, ObjectID outAllocationId)
    {
    }


    void JsonConsumer::IGlobalSession_getSessionDescDigest(ObjectID objectId, slang::SessionDesc* sessionDesc, ObjectID outBlobId)
    {
    }



    // ISession
    void JsonConsumer::ISession_getGlobalSession(ObjectID objectId, ObjectID outGlobalSessionId)
    {
    }


    void JsonConsumer::ISession_loadModule(ObjectID objectId, const char* moduleName, ObjectID outDiagnostics, ObjectID outModuleId)
    {
    }

    void JsonConsumer::ISession_loadModuleFromIRBlob(ObjectID objectId, const char* moduleName,
            const char* path, slang::IBlob* source, ObjectID outDiagnosticsId, ObjectID outModuleId)
    {
    }


    void JsonConsumer::ISession_loadModuleFromSource(ObjectID objectId, const char* moduleName,
            const char* path, slang::IBlob* source, ObjectID outDiagnosticsId, ObjectID outModuleId)
    {
    }


    void JsonConsumer::ISession_loadModuleFromSourceString(ObjectID objectId, const char* moduleName,
            const char* path, const char* string, ObjectID outDiagnosticsId, ObjectID outModuleId)
    {
    }


    void JsonConsumer::ISession_createCompositeComponentType(ObjectID objectId, ObjectID* componentTypeIds,
        SlangInt componentTypeCount, ObjectID outCompositeComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void JsonConsumer::ISession_specializeType(ObjectID objectId, ObjectID typeId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outDiagnosticsId, ObjectID outTypeReflectionId)
    {
    }

    void JsonConsumer::ISession_getTypeLayout(ObjectID objectId, ObjectID typeId, SlangInt targetIndex,
            slang::LayoutRules rules,  ObjectID outDiagnosticsId, ObjectID outTypeLayoutReflection)
    {
    }

    void JsonConsumer::ISession_getContainerType(ObjectID objectId, ObjectID elementType,
            slang::ContainerType containerType, ObjectID outDiagnosticsId, ObjectID outTypeReflectionId)
    {
    }

    void JsonConsumer::ISession_getDynamicType(ObjectID objectId, ObjectID outTypeReflectionId)
    {
    }

    void JsonConsumer::ISession_getTypeRTTIMangledName(ObjectID objectId, ObjectID typeId, ObjectID outNameBlobId)
    {
    }

    void JsonConsumer::ISession_getTypeConformanceWitnessMangledName(ObjectID objectId, ObjectID typeId,
			ObjectID interfaceTypeId, ObjectID outNameBlobId)
    {
    }


    void JsonConsumer::ISession_getTypeConformanceWitnessSequentialID(ObjectID objectId, ObjectID typeId,
            ObjectID interfaceTypeId, uint32_t outId)
    {
    }

    void JsonConsumer::ISession_createTypeConformanceComponentType(ObjectID objectId, ObjectID typeId,
            ObjectID interfaceTypeId, ObjectID outConformanceId,
            SlangInt conformanceIdOverride, ObjectID outDiagnosticsId)
    {
    }


    void JsonConsumer::ISession_createCompileRequest(ObjectID objectId, ObjectID outCompileRequestId)
    {
    }


    void JsonConsumer::ISession_getLoadedModule(ObjectID objectId, SlangInt index, ObjectID outModuleId)
    {
    }



    // IModule
    void JsonConsumer::IModule_findEntryPointByName(ObjectID objectId, char const* name, ObjectID outEntryPointId)
    {
    }


    void JsonConsumer::IModule_getDefinedEntryPoint(ObjectID objectId, SlangInt32 index, ObjectID outEntryPointId)
    {
    }


    void JsonConsumer::IModule_serialize(ObjectID objectId, ObjectID outSerializedBlobId)
    {
    }


    void JsonConsumer::IModule_writeToFile(ObjectID objectId, char const* fileName)
    {
    }


    void JsonConsumer::IModule_findAndCheckEntryPoint(ObjectID objectId, char const* name, SlangStage stage, ObjectID outEntryPointId, ObjectID outDiagnostics)
    {
    }



    void JsonConsumer::IModule_getSession(ObjectID objectId, ObjectID outSessionId)
    {
    }


    void JsonConsumer::IModule_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
    }


    void JsonConsumer::IModule_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::IModule_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::IModule_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystem)
    {
    }


    void JsonConsumer::IModule_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
    }


    void JsonConsumer::IModule_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void JsonConsumer::IModule_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void JsonConsumer::IModule_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibrary, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::IModule_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
    }


    void JsonConsumer::IModule_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
    }



    // IEntryPoint
    void JsonConsumer::IEntryPoint_getSession(ObjectID objectId, ObjectID outSessionId)
    {
    }


    void JsonConsumer::IEntryPoint_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
    }


    void JsonConsumer::IEntryPoint_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::IEntryPoint_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::IEntryPoint_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystem)
    {
    }


    void JsonConsumer::IEntryPoint_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
    }


    void JsonConsumer::IEntryPoint_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void JsonConsumer::IEntryPoint_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void JsonConsumer::IEntryPoint_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibrary, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::IEntryPoint_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
    }


    void JsonConsumer::IEntryPoint_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
    }



    // ICompositeComponentType
    void JsonConsumer::ICompositeComponentType_getSession(ObjectID objectId, ObjectID outSessionId)
    {
    }


    void JsonConsumer::ICompositeComponentType_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
    }


    void JsonConsumer::ICompositeComponentType_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::ICompositeComponentType_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::ICompositeComponentType_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystem)
    {
    }


    void JsonConsumer::ICompositeComponentType_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
    }


    void JsonConsumer::ICompositeComponentType_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void JsonConsumer::ICompositeComponentType_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void JsonConsumer::ICompositeComponentType_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibrary, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::ICompositeComponentType_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
    }


    void JsonConsumer::ICompositeComponentType_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
    }



    // ITypeConformance
    void JsonConsumer::ITypeConformance_getSession(ObjectID objectId, ObjectID outSessionId)
    {
    }


    void JsonConsumer::ITypeConformance_getLayout(ObjectID objectId, SlangInt targetIndex, ObjectID outDiagnosticsId, ObjectID retProgramLayoutId)
    {
    }


    void JsonConsumer::ITypeConformance_getEntryPointCode(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::ITypeConformance_getTargetCode(ObjectID objectId, SlangInt targetIndex, ObjectID outCode, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::ITypeConformance_getResultAsFileSystem(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outFileSystem)
    {
    }


    void JsonConsumer::ITypeConformance_getEntryPointHash(ObjectID objectId, SlangInt entryPointIndex, SlangInt targetIndex, ObjectID outHashId)
    {
    }


    void JsonConsumer::ITypeConformance_specialize(ObjectID objectId, slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount, ObjectID outSpecializedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void JsonConsumer::ITypeConformance_link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId)
    {
    }


    void JsonConsumer::ITypeConformance_getEntryPointHostCallable(ObjectID objectId, int entryPointIndex, int targetIndex, ObjectID outSharedLibrary, ObjectID outDiagnostics)
    {
    }


    void JsonConsumer::ITypeConformance_renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId)
    {
    }


    void JsonConsumer::ITypeConformance_linkWithOptions(ObjectID objectId, ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount, slang::CompilerOptionEntry* compilerOptionEntries, ObjectID outDiagnosticsId)
    {
    }


}; // namespace SlangCapture
