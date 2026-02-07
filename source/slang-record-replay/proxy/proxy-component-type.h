#ifndef SLANG_PROXY_COMPONENT_TYPE_H
#define SLANG_PROXY_COMPONENT_TYPE_H

#include "proxy-base.h"
#include "proxy-macros.h"

#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class ComponentTypeProxy :  public ProxyBase<slang::IComponentType, slang::IComponentType2, slang::IModulePrecompileService_Experimental>
{
public:
    SLANG_COM_INTERFACE(
        0xb3c25ea1,
        0x8fa0,
        0xb162,
        {0xf3, 0xe4, 0xdf, 0xa0, 0x91, 0x82, 0x73, 0x04})

    explicit ComponentTypeProxy(slang::IComponentType* actual)
        : ProxyBase(actual)
    {
    }

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(ComponentTypeProxy)

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE
    {
        if (!outObject) return SLANG_E_INVALID_ARG;

        if (uuid == ComponentTypeProxy::getTypeGuid() ||
            uuid == slang::IComponentType::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<slang::IComponentType*>(this);
            return SLANG_OK;
        }
        if (uuid == slang::IComponentType2::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<slang::IComponentType2*>(this);
            return SLANG_OK;
        }
        if (uuid == slang::IModulePrecompileService_Experimental::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<slang::IModulePrecompileService_Experimental*>(this);
            return SLANG_OK;
        }
        if (uuid == ISlangUnknown::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangUnknown*>(static_cast<slang::IComponentType*>(this));
            return SLANG_OK;
        }
        return m_actual->queryInterface(uuid, outObject);
    }

    // IComponentType
    virtual SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() override
    {
        RECORD_CALL();
        slang::ISession* result = getActual<slang::IComponentType>()->getSession();
        return RECORD_COM_RESULT(result);
    }

    virtual SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL
    getLayout(SlangInt targetIndex, ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(targetIndex);

        PREPARE_POINTER_OUTPUT(outDiagnostics);

        slang::ProgramLayout* result = getActual<slang::IComponentType>()->getLayout(targetIndex, outDiagnostics);

        RECORD_COM_OUTPUT(outDiagnostics);
        // Note: ProgramLayout* is a raw pointer to reflection data, not a COM object
        return result;
    }

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getSpecializationParamCount() override
    {
        RECORD_CALL();
        SlangInt result = getActual<slang::IComponentType>()->getSpecializationParamCount();
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ISlangBlob** outCode,
        ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(entryPointIndex);
        RECORD_INPUT(targetIndex);

        PREPARE_POINTER_OUTPUT(outCode);
        PREPARE_POINTER_OUTPUT(outDiagnostics);

        auto result = getActual<slang::IComponentType>()->getEntryPointCode(
            entryPointIndex,
            targetIndex,
            outCode,
            outDiagnostics);

        RECORD_COM_OUTPUT(outCode);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getResultAsFileSystem(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ISlangMutableFileSystem** outFileSystem) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outFileSystem);
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::getResultAsFileSystem");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    getEntryPointHash(SlangInt entryPointIndex, SlangInt targetIndex, ISlangBlob** outHash) override
    {
        RECORD_CALL();
        RECORD_INPUT(entryPointIndex);
        RECORD_INPUT(targetIndex);

        PREPARE_POINTER_OUTPUT(outHash);

        getActual<slang::IComponentType>()->getEntryPointHash(entryPointIndex, targetIndex, outHash);

        RECORD_COM_OUTPUT(outHash);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL specialize(
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        slang::IComponentType** outSpecializedComponentType,
        ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();

        // Record the specialization args array
        RECORD_INPUT_ARRAY(specializationArgs, specializationArgCount);

        PREPARE_POINTER_OUTPUT(outSpecializedComponentType);
        PREPARE_POINTER_OUTPUT(outDiagnostics);

        auto result = getActual<slang::IComponentType>()->specialize(
            specializationArgs,
            specializationArgCount,
            outSpecializedComponentType,
            outDiagnostics);

        RECORD_COM_OUTPUT(outSpecializedComponentType);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    link(slang::IComponentType** outLinkedComponentType, ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();

        PREPARE_POINTER_OUTPUT(outLinkedComponentType);
        PREPARE_POINTER_OUTPUT(outDiagnostics);

        auto result = getActual<slang::IComponentType>()->link(
            outLinkedComponentType,
            outDiagnostics);

        RECORD_COM_OUTPUT(outLinkedComponentType);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
        int entryPointIndex,
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary,
        slang::IBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(entryPointIndex);
        RECORD_INPUT(targetIndex);

        PREPARE_POINTER_OUTPUT(outSharedLibrary);
        PREPARE_POINTER_OUTPUT(outDiagnostics);

        auto result = getActual<slang::IComponentType>()->getEntryPointHostCallable(
            entryPointIndex,
            targetIndex,
            outSharedLibrary,
            outDiagnostics);

        RECORD_COM_OUTPUT(outSharedLibrary);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    renameEntryPoint(const char* newName, slang::IComponentType** outEntryPoint) override
    {
        SLANG_UNUSED(newName);
        SLANG_UNUSED(outEntryPoint);
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::renameEntryPoint");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL linkWithOptions(
        slang::IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(outLinkedComponentType);
        SLANG_UNUSED(compilerOptionEntryCount);
        SLANG_UNUSED(compilerOptionEntries);
        SLANG_UNUSED(outDiagnostics);
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::linkWithOptions");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetCode(SlangInt targetIndex, ISlangBlob** outCode, ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(targetIndex);

        PREPARE_POINTER_OUTPUT(outCode);
        PREPARE_POINTER_OUTPUT(outDiagnostics);

        auto result = getActual<slang::IComponentType>()->getTargetCode(targetIndex, outCode, outDiagnostics);

        RECORD_COM_OUTPUT(outCode);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetMetadata(
        SlangInt targetIndex,
        slang::IMetadata** outMetadata,
        ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(targetIndex);
        PREPARE_POINTER_OUTPUT(outMetadata);
        PREPARE_POINTER_OUTPUT(outDiagnostics);
        auto result = getActual<slang::IComponentType>()->getTargetMetadata(targetIndex, outMetadata, outDiagnostics);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointMetadata(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IMetadata** outMetadata,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outMetadata);
        SLANG_UNUSED(outDiagnostics);
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::getEntryPointMetadata");
    }

    // IComponentType2
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCompileResult(
        SlangInt targetIndex,
        slang::ICompileResult** outCompileResult,
        slang::IBlob** outDiagnostics) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outCompileResult);
        SLANG_UNUSED(outDiagnostics);
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::getTargetCompileResult");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCompileResult(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::ICompileResult** outCompileResult,
        slang::IBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(entryPointIndex);
        RECORD_INPUT(targetIndex);
        PREPARE_POINTER_OUTPUT(outCompileResult);
        PREPARE_POINTER_OUTPUT(outDiagnostics);
        auto result = getActual<slang::IComponentType2>()->getEntryPointCompileResult(
            entryPointIndex, targetIndex, outCompileResult, outDiagnostics);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetHostCallable(
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary,
        slang::IBlob** outDiagnostics = 0) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outSharedLibrary);
        SLANG_UNUSED(outDiagnostics);
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::getTargetHostCallable");
    }

    // IModulePrecompileService_Experimental
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    precompileForTarget(SlangCompileTarget target, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(target);
        SLANG_UNUSED(outDiagnostics);
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::precompileForTarget");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getPrecompiledTargetCode(
        SlangCompileTarget target,
        ISlangBlob** outCode,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(target);
        SLANG_UNUSED(outCode);
        SLANG_UNUSED(outDiagnostics);
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::getPrecompiledTargetCode");
    }

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getModuleDependencyCount() override
    {
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::getModuleDependencyCount");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getModuleDependency(
        SlangInt dependencyIndex,
        slang::IModule** outModule,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(dependencyIndex);
        SLANG_UNUSED(outModule);
        SLANG_UNUSED(outDiagnostics);
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::getModuleDependency");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_COMPONENT_TYPE_H
