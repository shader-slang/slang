#ifndef SLANG_PROXY_SESSION_H
#define SLANG_PROXY_SESSION_H

#include "proxy-base.h"
#include "proxy-macros.h"

namespace SlangRecord
{

class SessionProxy : public ProxyBase<slang::ISession>
{
public:
    SLANG_COM_INTERFACE(
        0xa2b14d90,
        0x7e8f,
        0xa051,
        {0xe2, 0xd3, 0xce, 0x9f, 0x80, 0x71, 0x62, 0xf3})

    explicit SessionProxy(slang::ISession* actual)
        : ProxyBase(actual)
    {
    }

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(SessionProxy)

    // ISession
    virtual SLANG_NO_THROW slang::IGlobalSession* SLANG_MCALL getGlobalSession() override
    {
        RECORD_CALL();
        slang::IGlobalSession* result = getActual<slang::ISession>()->getGlobalSession();
        return RECORD_COM_RESULT(result);
    }

    virtual SLANG_NO_THROW slang::IModule* SLANG_MCALL
    loadModule(const char* moduleName, ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(moduleName);
        
        PREPARE_POINTER_OUTPUT(outDiagnostics);
            
        slang::IModule* result = getActual<slang::ISession>()->loadModule(moduleName, outDiagnostics);
        
        RECORD_COM_OUTPUT(outDiagnostics);
        return RECORD_COM_RESULT(result);
    }

    virtual SLANG_NO_THROW slang::IModule* SLANG_MCALL loadModuleFromSource(
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        slang::IBlob** outDiagnostics) override
    {
        SLANG_UNUSED(moduleName);
        SLANG_UNUSED(path);
        SLANG_UNUSED(source);
        SLANG_UNUSED(outDiagnostics);
        REPLAY_UNIMPLEMENTED_X("SessionProxy::loadModuleFromSource");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL createCompositeComponentType(
        slang::IComponentType* const* componentTypes,
        SlangInt componentTypeCount,
        slang::IComponentType** outCompositeComponentType,
        ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT_ARRAY(componentTypes, componentTypeCount);

        // Call create session
        PREPARE_POINTER_OUTPUT(outCompositeComponentType);
        PREPARE_POINTER_OUTPUT(outDiagnostics);
        auto result = getActual<slang::ISession>()->createCompositeComponentType(componentTypes, componentTypeCount, outCompositeComponentType, outDiagnostics);

        RECORD_COM_OUTPUT(outCompositeComponentType);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL specializeType(
        slang::TypeReflection* type,
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(type);
        SLANG_UNUSED(specializationArgs);
        SLANG_UNUSED(specializationArgCount);
        SLANG_UNUSED(outDiagnostics);
        REPLAY_UNIMPLEMENTED_X("SessionProxy::specializeType");
    }

    virtual SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getTypeLayout(
        slang::TypeReflection* type,
        SlangInt targetIndex,
        slang::LayoutRules rules,
        ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(type);
        RECORD_INPUT(targetIndex);
        RECORD_INPUT(rules);
        
        PREPARE_POINTER_OUTPUT(outDiagnostics);
            
        slang::TypeLayoutReflection* result = getActual<slang::ISession>()->getTypeLayout(type, targetIndex, rules, outDiagnostics);
        
        RECORD_COM_OUTPUT(outDiagnostics);
        return result;        
    }

    virtual SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL getContainerType(
        slang::TypeReflection* elementType,
        slang::ContainerType containerType,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(elementType);
        SLANG_UNUSED(containerType);
        SLANG_UNUSED(outDiagnostics);
        REPLAY_UNIMPLEMENTED_X("SessionProxy::getContainerType");
    }

    virtual SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL getDynamicType() override
    {
        REPLAY_UNIMPLEMENTED_X("SessionProxy::getDynamicType");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTypeRTTIMangledName(slang::TypeReflection* type, ISlangBlob** outNameBlob) override
    {
        SLANG_UNUSED(type);
        SLANG_UNUSED(outNameBlob);
        REPLAY_UNIMPLEMENTED_X("SessionProxy::getTypeRTTIMangledName");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTypeConformanceWitnessMangledName(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        ISlangBlob** outNameBlob) override
    {
        SLANG_UNUSED(type);
        SLANG_UNUSED(interfaceType);
        SLANG_UNUSED(outNameBlob);
        REPLAY_UNIMPLEMENTED_X("SessionProxy::getTypeConformanceWitnessMangledName");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTypeConformanceWitnessSequentialID(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        uint32_t* outId) override
    {
        RECORD_CALL();
        RECORD_INPUT(type);
        RECORD_INPUT(interfaceType);
        PREPARE_POINTER_OUTPUT(outId);
        auto result = getActual<slang::ISession>()->getTypeConformanceWitnessSequentialID(type, interfaceType, outId);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    createCompileRequest(SlangCompileRequest** outCompileRequest) override
    {
        SLANG_UNUSED(outCompileRequest);
        REPLAY_UNIMPLEMENTED_X("SessionProxy::createCompileRequest");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL createTypeConformanceComponentType(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        slang::ITypeConformance** outConformance,
        SlangInt conformanceIdOverride,
        ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(type);
        RECORD_INPUT(interfaceType);
        RECORD_INPUT(conformanceIdOverride);

        PREPARE_POINTER_OUTPUT(outConformance);
        PREPARE_POINTER_OUTPUT(outDiagnostics);
        auto result = getActual<slang::ISession>()->createTypeConformanceComponentType(
            type,
            interfaceType,
            outConformance,
            conformanceIdOverride,
            outDiagnostics);

        RECORD_COM_OUTPUT(outConformance);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW slang::IModule* SLANG_MCALL loadModuleFromIRBlob(
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        slang::IBlob** outDiagnostics) override
    {
        SLANG_UNUSED(moduleName);
        SLANG_UNUSED(path);
        SLANG_UNUSED(source);
        SLANG_UNUSED(outDiagnostics);
        REPLAY_UNIMPLEMENTED_X("SessionProxy::loadModuleFromIRBlob");
    }

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getLoadedModuleCount() override
    {
        REPLAY_UNIMPLEMENTED_X("SessionProxy::getLoadedModuleCount");
    }

    virtual SLANG_NO_THROW slang::IModule* SLANG_MCALL getLoadedModule(SlangInt index) override
    {
        SLANG_UNUSED(index);
        REPLAY_UNIMPLEMENTED_X("SessionProxy::getLoadedModule");
    }

    virtual SLANG_NO_THROW bool SLANG_MCALL
    isBinaryModuleUpToDate(const char* modulePath, slang::IBlob* binaryModuleBlob) override
    {
        SLANG_UNUSED(modulePath);
        SLANG_UNUSED(binaryModuleBlob);
        REPLAY_UNIMPLEMENTED_X("SessionProxy::isBinaryModuleUpToDate");
    }

    virtual SLANG_NO_THROW slang::IModule* SLANG_MCALL loadModuleFromSourceString(
        const char* moduleName,
        const char* path,
        const char* string,
        slang::IBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(moduleName);
        RECORD_INPUT(path);
        RECORD_INPUT(string);
        
        PREPARE_POINTER_OUTPUT(outDiagnostics);
            
        slang::IModule* result = getActual<slang::ISession>()->loadModuleFromSourceString(moduleName, path, string, outDiagnostics);
        
        RECORD_COM_OUTPUT(outDiagnostics);
        return RECORD_COM_RESULT(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getDynamicObjectRTTIBytes(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        uint32_t* outRTTIDataBuffer,
        uint32_t bufferSizeInBytes) override
    {
        SLANG_UNUSED(type);
        SLANG_UNUSED(interfaceType);
        SLANG_UNUSED(outRTTIDataBuffer);
        SLANG_UNUSED(bufferSizeInBytes);
        REPLAY_UNIMPLEMENTED_X("SessionProxy::getDynamicObjectRTTIBytes");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadModuleInfoFromIRBlob(
        slang::IBlob* source,
        SlangInt& outModuleVersion,
        const char*& outModuleCompilerVersion,
        const char*& outModuleName) override
    {
        SLANG_UNUSED(source);
        SLANG_UNUSED(outModuleVersion);
        SLANG_UNUSED(outModuleCompilerVersion);
        SLANG_UNUSED(outModuleName);
        REPLAY_UNIMPLEMENTED_X("SessionProxy::loadModuleInfoFromIRBlob");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_SESSION_H
