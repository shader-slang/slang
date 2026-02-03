#ifndef SLANG_PROXY_SESSION_H
#define SLANG_PROXY_SESSION_H

#include "proxy-base.h"

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

    // ISession
    virtual SLANG_NO_THROW slang::IGlobalSession* SLANG_MCALL getGlobalSession() override
    {
        SLANG_UNIMPLEMENTED_X("SessionProxy::getGlobalSession");
    }

    virtual SLANG_NO_THROW slang::IModule* SLANG_MCALL
    loadModule(const char* moduleName, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(moduleName);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("SessionProxy::loadModule");
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
        SLANG_UNIMPLEMENTED_X("SessionProxy::loadModuleFromSource");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL createCompositeComponentType(
        slang::IComponentType* const* componentTypes,
        SlangInt componentTypeCount,
        slang::IComponentType** outCompositeComponentType,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(componentTypes);
        SLANG_UNUSED(componentTypeCount);
        SLANG_UNUSED(outCompositeComponentType);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("SessionProxy::createCompositeComponentType");
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
        SLANG_UNIMPLEMENTED_X("SessionProxy::specializeType");
    }

    virtual SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getTypeLayout(
        slang::TypeReflection* type,
        SlangInt targetIndex,
        slang::LayoutRules rules,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(type);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(rules);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("SessionProxy::getTypeLayout");
    }

    virtual SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL getContainerType(
        slang::TypeReflection* elementType,
        slang::ContainerType containerType,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(elementType);
        SLANG_UNUSED(containerType);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("SessionProxy::getContainerType");
    }

    virtual SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL getDynamicType() override
    {
        SLANG_UNIMPLEMENTED_X("SessionProxy::getDynamicType");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTypeRTTIMangledName(slang::TypeReflection* type, ISlangBlob** outNameBlob) override
    {
        SLANG_UNUSED(type);
        SLANG_UNUSED(outNameBlob);
        SLANG_UNIMPLEMENTED_X("SessionProxy::getTypeRTTIMangledName");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTypeConformanceWitnessMangledName(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        ISlangBlob** outNameBlob) override
    {
        SLANG_UNUSED(type);
        SLANG_UNUSED(interfaceType);
        SLANG_UNUSED(outNameBlob);
        SLANG_UNIMPLEMENTED_X("SessionProxy::getTypeConformanceWitnessMangledName");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTypeConformanceWitnessSequentialID(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        uint32_t* outId) override
    {
        SLANG_UNUSED(type);
        SLANG_UNUSED(interfaceType);
        SLANG_UNUSED(outId);
        SLANG_UNIMPLEMENTED_X("SessionProxy::getTypeConformanceWitnessSequentialID");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    createCompileRequest(SlangCompileRequest** outCompileRequest) override
    {
        SLANG_UNUSED(outCompileRequest);
        SLANG_UNIMPLEMENTED_X("SessionProxy::createCompileRequest");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL createTypeConformanceComponentType(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        slang::ITypeConformance** outConformance,
        SlangInt conformanceIdOverride,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(type);
        SLANG_UNUSED(interfaceType);
        SLANG_UNUSED(outConformance);
        SLANG_UNUSED(conformanceIdOverride);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("SessionProxy::createTypeConformanceComponentType");
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
        SLANG_UNIMPLEMENTED_X("SessionProxy::loadModuleFromIRBlob");
    }

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getLoadedModuleCount() override
    {
        SLANG_UNIMPLEMENTED_X("SessionProxy::getLoadedModuleCount");
    }

    virtual SLANG_NO_THROW slang::IModule* SLANG_MCALL getLoadedModule(SlangInt index) override
    {
        SLANG_UNUSED(index);
        SLANG_UNIMPLEMENTED_X("SessionProxy::getLoadedModule");
    }

    virtual SLANG_NO_THROW bool SLANG_MCALL
    isBinaryModuleUpToDate(const char* modulePath, slang::IBlob* binaryModuleBlob) override
    {
        SLANG_UNUSED(modulePath);
        SLANG_UNUSED(binaryModuleBlob);
        SLANG_UNIMPLEMENTED_X("SessionProxy::isBinaryModuleUpToDate");
    }

    virtual SLANG_NO_THROW slang::IModule* SLANG_MCALL loadModuleFromSourceString(
        const char* moduleName,
        const char* path,
        const char* string,
        slang::IBlob** outDiagnostics) override
    {
        SLANG_UNUSED(moduleName);
        SLANG_UNUSED(path);
        SLANG_UNUSED(string);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("SessionProxy::loadModuleFromSourceString");
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
        SLANG_UNIMPLEMENTED_X("SessionProxy::getDynamicObjectRTTIBytes");
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
        SLANG_UNIMPLEMENTED_X("SessionProxy::loadModuleInfoFromIRBlob");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_SESSION_H
