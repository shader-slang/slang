#ifndef SLANG_PROXY_COMPONENT_TYPE_H
#define SLANG_PROXY_COMPONENT_TYPE_H

#include "proxy-base.h"

#include "../../core/slang-smart-pointer.h"
#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class ComponentTypeProxy : public slang::IComponentType,
                           public slang::IComponentType2,
                           public slang::IModulePrecompileService_Experimental,
                           public RefObject,
                           public ProxyBase
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

    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ISlangUnknown* getInterface(const Guid& guid);

    // IComponentType
    virtual SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() override
    {
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getSession");
    }

    virtual SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL
    getLayout(SlangInt targetIndex, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getLayout");
    }

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getSpecializationParamCount() override
    {
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getSpecializationParamCount");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ISlangBlob** outCode,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outCode);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getEntryPointCode");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getResultAsFileSystem(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ISlangMutableFileSystem** outFileSystem) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outFileSystem);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getResultAsFileSystem");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    getEntryPointHash(SlangInt entryPointIndex, SlangInt targetIndex, ISlangBlob** outHash) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outHash);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getEntryPointHash");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL specialize(
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        slang::IComponentType** outSpecializedComponentType,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(specializationArgs);
        SLANG_UNUSED(specializationArgCount);
        SLANG_UNUSED(outSpecializedComponentType);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::specialize");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    link(slang::IComponentType** outLinkedComponentType, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(outLinkedComponentType);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::link");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
        int entryPointIndex,
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary,
        slang::IBlob** outDiagnostics) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outSharedLibrary);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getEntryPointHostCallable");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    renameEntryPoint(const char* newName, slang::IComponentType** outEntryPoint) override
    {
        SLANG_UNUSED(newName);
        SLANG_UNUSED(outEntryPoint);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::renameEntryPoint");
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
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::linkWithOptions");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetCode(SlangInt targetIndex, ISlangBlob** outCode, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outCode);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getTargetCode");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetMetadata(
        SlangInt targetIndex,
        slang::IMetadata** outMetadata,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outMetadata);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getTargetMetadata");
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
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getEntryPointMetadata");
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
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getTargetCompileResult");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCompileResult(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::ICompileResult** outCompileResult,
        slang::IBlob** outDiagnostics) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outCompileResult);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getEntryPointCompileResult");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetHostCallable(
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary,
        slang::IBlob** outDiagnostics = 0) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outSharedLibrary);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getTargetHostCallable");
    }

    // IModulePrecompileService_Experimental
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    precompileForTarget(SlangCompileTarget target, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(target);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::precompileForTarget");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getPrecompiledTargetCode(
        SlangCompileTarget target,
        ISlangBlob** outCode,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(target);
        SLANG_UNUSED(outCode);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getPrecompiledTargetCode");
    }

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getModuleDependencyCount() override
    {
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getModuleDependencyCount");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getModuleDependency(
        SlangInt dependencyIndex,
        slang::IModule** outModule,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(dependencyIndex);
        SLANG_UNUSED(outModule);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ComponentTypeProxy::getModuleDependency");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_COMPONENT_TYPE_H
