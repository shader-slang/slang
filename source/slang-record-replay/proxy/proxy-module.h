#ifndef SLANG_PROXY_MODULE_H
#define SLANG_PROXY_MODULE_H

#include "proxy-base.h"
#include "proxy-macros.h"

#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class ModuleProxy : public ProxyBase<slang::IModule, slang::IComponentType2, slang::IModulePrecompileService_Experimental>
{
public:
    SLANG_COM_INTERFACE(
        0xc4d36fb2,
        0x9fb1,
        0xc273,
        {0x04, 0xf5, 0xe0, 0xb1, 0xa2, 0x93, 0x84, 0x15})

    explicit ModuleProxy(slang::IModule* actual)
        : ProxyBase(actual)
    {
    }

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(ModuleProxy)

    // IComponentType
    virtual SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() override
    {
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getSession");
    }

    virtual SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL
    getLayout(SlangInt targetIndex, ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(targetIndex);
        
        ISlangBlob* diagnosticsPtr = nullptr;
        if (!outDiagnostics)
            outDiagnostics = &diagnosticsPtr;
            
        slang::ProgramLayout* result = getActual<slang::IModule>()->getLayout(targetIndex, outDiagnostics);
        
        RECORD_COM_OUTPUT(outDiagnostics);
        return result; // don't capture pointer
    }

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getSpecializationParamCount() override
    {
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getSpecializationParamCount");
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
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getEntryPointCode");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getResultAsFileSystem(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ISlangMutableFileSystem** outFileSystem) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outFileSystem);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getResultAsFileSystem");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    getEntryPointHash(SlangInt entryPointIndex, SlangInt targetIndex, ISlangBlob** outHash) override
    {
        SLANG_UNUSED(entryPointIndex);
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outHash);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getEntryPointHash");
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
        SLANG_UNIMPLEMENTED_X("ModuleProxy::specialize");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    link(slang::IComponentType** outLinkedComponentType, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(outLinkedComponentType);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::link");
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
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getEntryPointHostCallable");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    renameEntryPoint(const char* newName, slang::IComponentType** outEntryPoint) override
    {
        SLANG_UNUSED(newName);
        SLANG_UNUSED(outEntryPoint);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::renameEntryPoint");
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
        SLANG_UNIMPLEMENTED_X("ModuleProxy::linkWithOptions");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetCode(SlangInt targetIndex, ISlangBlob** outCode, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outCode);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getTargetCode");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetMetadata(
        SlangInt targetIndex,
        slang::IMetadata** outMetadata,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outMetadata);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getTargetMetadata");
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
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getEntryPointMetadata");
    }

    // IModule
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    findEntryPointByName(char const* name, slang::IEntryPoint** outEntryPoint) override
    {
        RECORD_CALL();
        RECORD_INPUT(name);
        
        slang::IEntryPoint* entryPointPtr = nullptr;
        if (!outEntryPoint)
            outEntryPoint = &entryPointPtr;
            
        SlangResult result = getActual<slang::IModule>()->findEntryPointByName(name, outEntryPoint);
        
        RECORD_COM_OUTPUT(outEntryPoint);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangInt32 SLANG_MCALL getDefinedEntryPointCount() override
    {
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getDefinedEntryPointCount");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getDefinedEntryPoint(SlangInt32 index, slang::IEntryPoint** outEntryPoint) override
    {
        SLANG_UNUSED(index);
        SLANG_UNUSED(outEntryPoint);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getDefinedEntryPoint");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL serialize(ISlangBlob** outSerializedBlob) override
    {
        SLANG_UNUSED(outSerializedBlob);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::serialize");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL writeToFile(char const* fileName) override
    {
        SLANG_UNUSED(fileName);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::writeToFile");
    }

    virtual SLANG_NO_THROW const char* SLANG_MCALL getName() override
    {
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getName");
    }

    virtual SLANG_NO_THROW const char* SLANG_MCALL getFilePath() override
    {
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getFilePath");
    }

    virtual SLANG_NO_THROW const char* SLANG_MCALL getUniqueIdentity() override
    {
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getUniqueIdentity");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL findAndCheckEntryPoint(
        char const* name,
        SlangStage stage,
        slang::IEntryPoint** outEntryPoint,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(name);
        SLANG_UNUSED(stage);
        SLANG_UNUSED(outEntryPoint);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::findAndCheckEntryPoint");
    }

    virtual SLANG_NO_THROW SlangInt32 SLANG_MCALL getDependencyFileCount() override
    {
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getDependencyFileCount");
    }

    virtual SLANG_NO_THROW char const* SLANG_MCALL getDependencyFilePath(SlangInt32 index) override
    {
        SLANG_UNUSED(index);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getDependencyFilePath");
    }

    virtual SLANG_NO_THROW slang::DeclReflection* SLANG_MCALL getModuleReflection() override
    {
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getModuleReflection");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    disassemble(slang::IBlob** outDisassembledBlob) override
    {
        SLANG_UNUSED(outDisassembledBlob);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::disassemble");
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
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getTargetCompileResult");
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
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getEntryPointCompileResult");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetHostCallable(
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary,
        slang::IBlob** outDiagnostics = 0) override
    {
        SLANG_UNUSED(targetIndex);
        SLANG_UNUSED(outSharedLibrary);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getTargetHostCallable");
    }

    // IModulePrecompileService_Experimental
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    precompileForTarget(SlangCompileTarget target, ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(target);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::precompileForTarget");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getPrecompiledTargetCode(
        SlangCompileTarget target,
        ISlangBlob** outCode,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(target);
        SLANG_UNUSED(outCode);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getPrecompiledTargetCode");
    }

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getModuleDependencyCount() override
    {
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getModuleDependencyCount");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getModuleDependency(
        SlangInt dependencyIndex,
        slang::IModule** outModule,
        ISlangBlob** outDiagnostics) override
    {
        SLANG_UNUSED(dependencyIndex);
        SLANG_UNUSED(outModule);
        SLANG_UNUSED(outDiagnostics);
        SLANG_UNIMPLEMENTED_X("ModuleProxy::getModuleDependency");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_MODULE_H
