#ifndef SLANG_PROXY_ENTRY_POINT_H
#define SLANG_PROXY_ENTRY_POINT_H

#include "../../core/slang-smart-pointer.h"
#include "slang-com-helper.h"
#include "slang.h"

namespace SlangProxy
{
using namespace Slang;

class EntryPointProxy : public slang::IEntryPoint,
                        public slang::IComponentType2,
                        public slang::IModulePrecompileService_Experimental,
                        public RefObject
{
public:
    SLANG_COM_INTERFACE(
        0xd5e47fc3,
        0xafc2,
        0xd384,
        {0x15, 0x06, 0xf1, 0xc2, 0xb3, 0xa4, 0x95, 0x26})

    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ISlangUnknown* getInterface(const Guid& guid);

    // IComponentType
    virtual SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() override
    {
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getSession");
    }

    virtual SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL
    getLayout(SlangInt targetIndex, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getLayout");
    }

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getSpecializationParamCount() override
    {
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getSpecializationParamCount");
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
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getEntryPointCode");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getResultAsFileSystem(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ISlangMutableFileSystem** outFileSystem) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outFileSystem);
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getResultAsFileSystem");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    getEntryPointHash(SlangInt entryPointIndex, SlangInt targetIndex, ISlangBlob** outHash) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outHash);
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getEntryPointHash");
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
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::specialize");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    link(slang::IComponentType** outLinkedComponentType, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(outLinkedComponentType);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::link");
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
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getEntryPointHostCallable");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    renameEntryPoint(const char* newName, slang::IComponentType** outEntryPoint) override
    {
        SLANG_UNUSED(newName);
        SLANG_UNUSED(outEntryPoint);
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::renameEntryPoint");
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
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::linkWithOptions");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetCode(SlangInt targetIndex, ISlangBlob** outCode, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outCode);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getTargetCode");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetMetadata(
        SlangInt targetIndex,
        slang::IMetadata** outMetadata,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outMetadata);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getTargetMetadata");
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
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getEntryPointMetadata");
    }

    // IEntryPoint
    virtual SLANG_NO_THROW slang::FunctionReflection* SLANG_MCALL getFunctionReflection() override
    {
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getFunctionReflection");
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
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getTargetCompileResult");
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
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getEntryPointCompileResult");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetHostCallable(
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary,
        slang::IBlob** outDiagnostics) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outSharedLibrary);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getTargetHostCallable");
    }

    // IModulePrecompileService_Experimental
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    precompileForTarget(SlangCompileTarget target, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(target);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::precompileForTarget");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getPrecompiledTargetCode(
        SlangCompileTarget target,
        ISlangBlob** outCode,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(target);
        SLANG_UNUSED(outCode);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getPrecompiledTargetCode");
    }

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getModuleDependencyCount() override
    {
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getModuleDependencyCount");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getModuleDependency(
        SlangInt dependencyIndex,
        slang::IModule** outModule,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(dependencyIndex);
        SLANG_UNUSED(outModule);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("EntryPointProxy::getModuleDependency");
    }
};

} // namespace SlangProxy

#endif // SLANG_PROXY_ENTRY_POINT_H
