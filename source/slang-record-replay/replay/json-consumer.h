#ifndef JSON_CONSUMER_H
#define JSON_CONSUMER_H

#include "../../core/slang-stream.h"
#include "../util/record-format.h"
#include "../util/record-utility.h"
#include "decoder-consumer.h"

namespace SlangRecord
{
class CommonInterfaceWriter
{
public:
    CommonInterfaceWriter(ApiClassId classId, Slang::FileStream& fileStream)
        : m_fileStream(fileStream)
    {
        switch (classId)
        {
        case ApiClassId::Class_IModule:
            m_className = "IModule";
            break;
        case ApiClassId::Class_IEntryPoint:
            m_className = "IEntryPoint";
            break;
        case ApiClassId::Class_ICompositeComponentType:
            m_className = "ICompositeComponentType";
            break;
        case ApiClassId::Class_ITypeConformance:
            m_className = "ITypeConformance";
            break;
        case ApiClassId::Class_IComponentType2:
            m_className = "IComponentType2";
            break;
        default:
            slangRecordLog(LogLevel::Error, "Invalid classNo %u\n", classId);
            break;
        }
    }
    CommonInterfaceWriter() = delete;
    void getSession(ObjectID objectId, ObjectID outSessionId);
    void getLayout(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outDiagnosticsId,
        ObjectID retProgramLayoutId);
    void getEntryPointCode(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outCodeId,
        ObjectID outDiagnosticsId);
    void getTargetCode(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outCodeId,
        ObjectID outDiagnosticsId);
    void getResultAsFileSystem(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outFileSystemId);
    void getEntryPointHash(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outHashId);
    void specialize(
        ObjectID objectId,
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        ObjectID outSpecializedComponentTypeId,
        ObjectID outDiagnosticsId);
    void link(ObjectID objectId, ObjectID outLinkedComponentTypeId, ObjectID outDiagnosticsId);
    void getEntryPointHostCallable(
        ObjectID objectId,
        int entryPointIndex,
        int targetIndex,
        ObjectID outSharedLibraryId,
        ObjectID outDiagnosticsId);
    void renameEntryPoint(ObjectID objectId, const char* newName, ObjectID outEntryPointId);
    void linkWithOptions(
        ObjectID objectId,
        ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ObjectID outDiagnosticsId);

    void queryInterface(ObjectID objectId, const SlangUUID& guid, ObjectID outInterfaceId);

    // IComponentType2 methods.
    void getTargetCompileResult(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outCompileResultId,
        ObjectID outDiagnosticsId);
    void getEntryPointCompileResult(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outCompileResultId,
        ObjectID outDiagnosticsId);

protected:
    Slang::String m_className;
    Slang::FileStream& m_fileStream;
};

class JsonConsumer : public IDecoderConsumer, public Slang::RefObject
{
public:
    JsonConsumer(const Slang::String& filePath);
    virtual ~JsonConsumer() = default;
    virtual void CreateGlobalSession(
        SlangGlobalSessionDesc const& desc,
        ObjectID outGlobalSessionId) override;
    virtual void IGlobalSession_createSession(
        ObjectID objectId,
        slang::SessionDesc const& desc,
        ObjectID outSessionId) override;
    virtual void IGlobalSession_findProfile(ObjectID objectId, char const* name) override;
    virtual void IGlobalSession_setDownstreamCompilerPath(
        ObjectID objectId,
        SlangPassThrough passThrough,
        char const* path) override;
    virtual void IGlobalSession_setDownstreamCompilerPrelude(
        ObjectID objectId,
        SlangPassThrough inPassThrough,
        char const* prelude) override;
    virtual void IGlobalSession_getDownstreamCompilerPrelude(
        ObjectID objectId,
        SlangPassThrough inPassThrough,
        ObjectID outPreludeId) override;

    virtual void IGlobalSession_getBuildTagString(ObjectID objectId) override { (void)objectId; }

    virtual void IGlobalSession_setDefaultDownstreamCompiler(
        ObjectID objectId,
        SlangSourceLanguage sourceLanguage,
        SlangPassThrough defaultCompiler) override;
    virtual void IGlobalSession_getDefaultDownstreamCompiler(
        ObjectID objectId,
        SlangSourceLanguage sourceLanguage) override;
    virtual void IGlobalSession_setLanguagePrelude(
        ObjectID objectId,
        SlangSourceLanguage inSourceLanguage,
        char const* prelude) override;
    virtual void IGlobalSession_getLanguagePrelude(
        ObjectID objectId,
        SlangSourceLanguage inSourceLanguage,
        ObjectID outPreludeId) override;
    virtual void IGlobalSession_createCompileRequest(ObjectID objectId, ObjectID outCompileRequest)
        override;
    virtual void IGlobalSession_addBuiltins(
        ObjectID objectId,
        char const* sourcePath,
        char const* sourceString) override;
    virtual void IGlobalSession_setSharedLibraryLoader(ObjectID objectId, ObjectID loaderId)
        override;
    virtual void IGlobalSession_getSharedLibraryLoader(ObjectID objectId, ObjectID outLoaderId)
        override;
    virtual void IGlobalSession_checkCompileTargetSupport(
        ObjectID objectId,
        SlangCompileTarget target) override;
    virtual void IGlobalSession_checkPassThroughSupport(
        ObjectID objectId,
        SlangPassThrough passThrough) override;
    virtual void IGlobalSession_compileCoreModule(
        ObjectID objectId,
        slang::CompileCoreModuleFlags flags) override;
    virtual void IGlobalSession_loadCoreModule(
        ObjectID objectId,
        const void* coreModule,
        size_t coreModuleSizeInBytes) override;
    virtual void IGlobalSession_saveCoreModule(
        ObjectID objectId,
        SlangArchiveType archiveType,
        ObjectID outBlobId) override;
    virtual void IGlobalSession_findCapability(ObjectID objectId, char const* name) override;
    virtual void IGlobalSession_setDownstreamCompilerForTransition(
        ObjectID objectId,
        SlangCompileTarget source,
        SlangCompileTarget target,
        SlangPassThrough compiler) override;
    virtual void IGlobalSession_getDownstreamCompilerForTransition(
        ObjectID objectId,
        SlangCompileTarget source,
        SlangCompileTarget target) override;

    virtual void IGlobalSession_getCompilerElapsedTime(ObjectID objectId) override
    {
        (void)objectId;
    }

    virtual void IGlobalSession_setSPIRVCoreGrammar(ObjectID objectId, char const* jsonPath)
        override;
    virtual void IGlobalSession_parseCommandLineArguments(
        ObjectID objectId,
        int argc,
        const char* const* argv,
        ObjectID outSessionDescId,
        ObjectID outAllocationId) override;
    virtual void IGlobalSession_getSessionDescDigest(
        ObjectID objectId,
        slang::SessionDesc* sessionDesc,
        ObjectID outBlobId) override;

    // ISession
    virtual void ISession_getGlobalSession(ObjectID objectId, ObjectID outGlobalSessionId) override;
    virtual void ISession_loadModule(
        ObjectID objectId,
        const char* moduleName,
        ObjectID outDiagnostics,
        ObjectID outModuleId) override;

    virtual void ISession_loadModuleFromIRBlob(
        ObjectID objectId,
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        ObjectID outDiagnosticsId,
        ObjectID outModuleId) override;
    virtual void ISession_loadModuleFromSource(
        ObjectID objectId,
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        ObjectID outDiagnosticsId,
        ObjectID outModuleId) override;
    virtual void ISession_loadModuleFromSourceString(
        ObjectID objectId,
        const char* moduleName,
        const char* path,
        const char* string,
        ObjectID outDiagnosticsId,
        ObjectID outModuleId) override;
    virtual void ISession_createCompositeComponentType(
        ObjectID objectId,
        ObjectID* componentTypeIds,
        SlangInt componentTypeCount,
        ObjectID outCompositeComponentTypeIds,
        ObjectID outDiagnosticsId) override;

    virtual void ISession_specializeType(
        ObjectID objectId,
        ObjectID typeId,
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        ObjectID outDiagnosticsId,
        ObjectID outTypeReflectionId) override;

    virtual void ISession_getTypeLayout(
        ObjectID objectId,
        ObjectID typeId,
        SlangInt targetIndex,
        slang::LayoutRules rules,
        ObjectID outDiagnosticsId,
        ObjectID outTypeLayoutReflection) override;

    virtual void ISession_getContainerType(
        ObjectID objectId,
        ObjectID elementType,
        slang::ContainerType containerType,
        ObjectID outDiagnosticsId,
        ObjectID outTypeReflectionId) override;

    virtual void ISession_getDynamicType(ObjectID objectId, ObjectID outTypeReflectionId) override;

    virtual void ISession_getTypeRTTIMangledName(
        ObjectID objectId,
        ObjectID typeId,
        ObjectID outNameBlobId) override;

    virtual void ISession_getTypeConformanceWitnessMangledName(
        ObjectID objectId,
        ObjectID typeId,
        ObjectID interfaceTypeId,
        ObjectID outNameBlobId) override;

    virtual void ISession_getTypeConformanceWitnessSequentialID(
        ObjectID objectId,
        ObjectID typeId,
        ObjectID interfaceTypeId,
        uint32_t outId) override;

    virtual void ISession_createTypeConformanceComponentType(
        ObjectID objectId,
        ObjectID typeId,
        ObjectID interfaceTypeId,
        ObjectID outConformanceId,
        SlangInt conformanceIdOverride,
        ObjectID outDiagnosticsId) override;

    virtual void ISession_createCompileRequest(ObjectID objectId, ObjectID outCompileRequestId)
        override;

    virtual void ISession_getLoadedModuleCount(ObjectID objectId) override { (void)objectId; }

    virtual void ISession_getLoadedModule(ObjectID objectId, SlangInt index, ObjectID outModuleId)
        override;

    virtual void ISession_isBinaryModuleUpToDate(ObjectID objectId) override { (void)objectId; }

    // IModule
    virtual void IModule_findEntryPointByName(
        ObjectID objectId,
        char const* name,
        ObjectID outEntryPointId) override;

    virtual void IModule_getDefinedEntryPointCount(ObjectID objectId) override { (void)objectId; }

    virtual void IModule_getDefinedEntryPoint(
        ObjectID objectId,
        SlangInt32 index,
        ObjectID outEntryPointId) override;
    virtual void IModule_serialize(ObjectID objectId, ObjectID outSerializedBlobId) override;
    virtual void IModule_writeToFile(ObjectID objectId, char const* fileName) override;

    virtual void IModule_getName(ObjectID objectId) override { (void)objectId; }
    virtual void IModule_getFilePath(ObjectID objectId) override { (void)objectId; }
    virtual void IModule_getUniqueIdentity(ObjectID objectId) override { (void)objectId; }

    virtual void IModule_findAndCheckEntryPoint(
        ObjectID objectId,
        char const* name,
        SlangStage stage,
        ObjectID outEntryPointId,
        ObjectID outDiagnostics) override;

    virtual void IModule_getSession(ObjectID objectId, ObjectID outSessionId) override;
    virtual void IModule_getLayout(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outDiagnosticsId,
        ObjectID retProgramLayoutId) override;

    virtual void IModule_getSpecializationParamCount(ObjectID objectId) override { (void)objectId; }

    virtual void IModule_getEntryPointCode(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outCodeId,
        ObjectID outDiagnosticsId) override;
    virtual void IModule_getTargetCode(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outCodeId,
        ObjectID outDiagnosticsId) override;
    virtual void IModule_getResultAsFileSystem(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outFileSystem) override;
    virtual void IModule_getEntryPointHash(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outHashId) override;
    virtual void IModule_specialize(
        ObjectID objectId,
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        ObjectID outSpecializedComponentTypeId,
        ObjectID outDiagnosticsId) override;
    virtual void IModule_link(
        ObjectID objectId,
        ObjectID outLinkedComponentTypeId,
        ObjectID outDiagnosticsId) override;
    virtual void IModule_getEntryPointHostCallable(
        ObjectID objectId,
        int entryPointIndex,
        int targetIndex,
        ObjectID outSharedLibrary,
        ObjectID outDiagnostics) override;
    virtual void IModule_renameEntryPoint(
        ObjectID objectId,
        const char* newName,
        ObjectID outEntryPointId) override;
    virtual void IModule_linkWithOptions(
        ObjectID objectId,
        ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ObjectID outDiagnosticsId) override;

    // IEntryPoint
    virtual void IEntryPoint_getSession(ObjectID objectId, ObjectID outSessionId) override;
    virtual void IEntryPoint_getLayout(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outDiagnosticsId,
        ObjectID retProgramLayoutId) override;

    virtual void IEntryPoint_getSpecializationParamCount(ObjectID objectId) override
    {
        (void)objectId;
    }

    virtual void IEntryPoint_getEntryPointCode(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outCodeId,
        ObjectID outDiagnosticsId) override;
    virtual void IEntryPoint_getTargetCode(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outCodeId,
        ObjectID outDiagnosticsId) override;
    virtual void IEntryPoint_getResultAsFileSystem(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outFileSystem) override;
    virtual void IEntryPoint_getEntryPointHash(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outHashId) override;
    virtual void IEntryPoint_specialize(
        ObjectID objectId,
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        ObjectID outSpecializedComponentTypeId,
        ObjectID outDiagnosticsId) override;
    virtual void IEntryPoint_link(
        ObjectID objectId,
        ObjectID outLinkedComponentTypeId,
        ObjectID outDiagnosticsId) override;
    virtual void IEntryPoint_getEntryPointHostCallable(
        ObjectID objectId,
        int entryPointIndex,
        int targetIndex,
        ObjectID outSharedLibrary,
        ObjectID outDiagnostics) override;
    virtual void IEntryPoint_renameEntryPoint(
        ObjectID objectId,
        const char* newName,
        ObjectID outEntryPointId) override;
    virtual void IEntryPoint_linkWithOptions(
        ObjectID objectId,
        ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ObjectID outDiagnosticsId) override;

    // ICompositeComponentType
    virtual void ICompositeComponentType_getSession(ObjectID objectId, ObjectID outSessionId)
        override;
    virtual void ICompositeComponentType_getLayout(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outDiagnosticsId,
        ObjectID retProgramLayoutId) override;

    virtual void ICompositeComponentType_getSpecializationParamCount(ObjectID objectId) override
    {
        (void)objectId;
    }

    virtual void ICompositeComponentType_getEntryPointCode(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outCodeId,
        ObjectID outDiagnostics) override;
    virtual void ICompositeComponentType_getTargetCode(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outCodeId,
        ObjectID outDiagnostics) override;
    virtual void ICompositeComponentType_getResultAsFileSystem(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outFileSystem) override;
    virtual void ICompositeComponentType_getEntryPointHash(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outHashId) override;
    virtual void ICompositeComponentType_specialize(
        ObjectID objectId,
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        ObjectID outSpecializedComponentTypeId,
        ObjectID outDiagnosticsId) override;
    virtual void ICompositeComponentType_link(
        ObjectID objectId,
        ObjectID outLinkedComponentTypeId,
        ObjectID outDiagnosticsId) override;
    virtual void ICompositeComponentType_getEntryPointHostCallable(
        ObjectID objectId,
        int entryPointIndex,
        int targetIndex,
        ObjectID outSharedLibrary,
        ObjectID outDiagnostics) override;
    virtual void ICompositeComponentType_renameEntryPoint(
        ObjectID objectId,
        const char* newName,
        ObjectID outEntryPointId) override;
    virtual void ICompositeComponentType_linkWithOptions(
        ObjectID objectId,
        ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ObjectID outDiagnosticsId) override;

    virtual void ICompositeComponentType_queryInterface(
        ObjectID objectId,
        const SlangUUID& guid,
        ObjectID outInterfaceId) override;

    // ITypeConformance
    virtual void ITypeConformance_getSession(ObjectID objectId, ObjectID outSessionId) override;
    virtual void ITypeConformance_getLayout(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outDiagnosticsId,
        ObjectID retProgramLayoutId) override;

    virtual void ITypeConformance_getSpecializationParamCount(ObjectID objectId) override
    {
        (void)objectId;
    }

    virtual void ITypeConformance_getEntryPointCode(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outCodeId,
        ObjectID outDiagnosticsId) override;
    virtual void ITypeConformance_getTargetCode(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outCodeId,
        ObjectID outDiagnosticsId) override;
    virtual void ITypeConformance_getResultAsFileSystem(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outFileSystem) override;
    virtual void ITypeConformance_getEntryPointHash(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outHashId) override;
    virtual void ITypeConformance_specialize(
        ObjectID objectId,
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        ObjectID outSpecializedComponentTypeId,
        ObjectID outDiagnosticsId) override;
    virtual void ITypeConformance_link(
        ObjectID objectId,
        ObjectID outLinkedComponentTypeId,
        ObjectID outDiagnosticsId) override;
    virtual void ITypeConformance_getEntryPointHostCallable(
        ObjectID objectId,
        int entryPointIndex,
        int targetIndex,
        ObjectID outSharedLibrary,
        ObjectID outDiagnostics) override;
    virtual void ITypeConformance_renameEntryPoint(
        ObjectID objectId,
        const char* newName,
        ObjectID outEntryPointId) override;
    virtual void ITypeConformance_linkWithOptions(
        ObjectID objectId,
        ObjectID outLinkedComponentTypeId,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ObjectID outDiagnosticsId) override;

    // IComponentType2 methods.
    virtual void IComponentType2_getTargetCompileResult(
        ObjectID objectId,
        SlangInt targetIndex,
        ObjectID outCompileResultId,
        ObjectID outDiagnosticsId) override;
    virtual void IComponentType2_getEntryPointCompileResult(
        ObjectID objectId,
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ObjectID outCompileResultId,
        ObjectID outDiagnosticsId) override;

    static void _writeCompilerOptionEntryHelper(
        Slang::StringBuilder& builder,
        int indent,
        const slang::CompilerOptionEntry* compilerOptionEntries,
        uint32_t compilerOptionEntryCount,
        bool isLast = false);
    static void _writeGlobalSessionDescHelper(
        Slang::StringBuilder& builder,
        int indent,
        SlangGlobalSessionDesc const& globalSessionDesc,
        Slang::String keyName,
        bool isLast = false);
    static void _writeSessionDescHelper(
        Slang::StringBuilder& builder,
        int indent,
        slang::SessionDesc const& sessionDesc,
        Slang::String keyName,
        bool isLast = false);

private:
    Slang::FileStream m_fileStream;
    bool m_isFileValid = false;
    CommonInterfaceWriter m_moduleHelper{ApiClassId::Class_IModule, m_fileStream};
    CommonInterfaceWriter m_entryPointHelper{ApiClassId::Class_IEntryPoint, m_fileStream};
    CommonInterfaceWriter m_compositeComponentTypeHelper{
        ApiClassId::Class_ICompositeComponentType,
        m_fileStream};
    CommonInterfaceWriter m_typeConformanceHelper{ApiClassId::Class_ITypeConformance, m_fileStream};
    CommonInterfaceWriter m_componentType2Helper{ApiClassId::Class_IComponentType2, m_fileStream};
};
} // namespace SlangRecord
#endif // JSON_CONSUMER_H
