#ifndef SLANG_PROXY_COMPONENT_TYPE_H
#define SLANG_PROXY_COMPONENT_TYPE_H

#include "../slang/slang-ast-type.h"
#include "../slang/slang-compiler-api.h"
#include "proxy-base.h"
#include "proxy-macros.h"
#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

/// Unified proxy for all IComponentType-derived interfaces:
///   IComponentType, IModule, IEntryPoint, ITypeConformance
///   plus IComponentType2 and IModulePrecompileService_Experimental.
///
/// Takes an ISlangUnknown* in its constructor and probes for each interface,
/// storing optional typed pointers. Methods for unsupported interfaces return
/// SLANG_E_NOT_IMPLEMENTED, following the same pattern as MutableFileSystemProxy.
class ComponentTypeProxy : public ProxyBase<
                               slang::IModule,
                               slang::IEntryPoint,
                               slang::ITypeConformance,
                               slang::IComponentType2,
                               slang::IModulePrecompileService_Experimental>
{
public:
    SLANG_COM_INTERFACE(
        0xb3c25ea1,
        0x8fa0,
        0xb162,
        {0xf3, 0xe4, 0xdf, 0xa0, 0x91, 0x82, 0x73, 0x04})

    /// Construct from any IComponentType-derived object.
    /// Uses queryInterface to discover which interfaces the underlying object supports.
    explicit ComponentTypeProxy(ISlangUnknown* actual)
        : ProxyBase(actual)
        , m_componentType(nullptr)
        , m_module(nullptr)
        , m_entryPoint(nullptr)
        , m_typeConformance(nullptr)
        , m_hasRegisteredCoreModule(false)
    {

        // IComponentType is always expected to be supported
        actual->queryInterface(slang::IComponentType::getTypeGuid(), (void**)&m_componentType);
        if (m_componentType)
            m_componentType->release(); // undo addRef - m_actual holds the ref

        // Probe for derived interfaces
        actual->queryInterface(slang::IModule::getTypeGuid(), (void**)&m_module);
        if (m_module)
            m_module->release();

        actual->queryInterface(slang::IEntryPoint::getTypeGuid(), (void**)&m_entryPoint);
        if (m_entryPoint)
            m_entryPoint->release();

        actual->queryInterface(slang::ITypeConformance::getTypeGuid(), (void**)&m_typeConformance);
        if (m_typeConformance)
            m_typeConformance->release();
    }

    ~ComponentTypeProxy()
    {
        // Clear returned entry points to release references before the component is released
        SuppressRefCountRecording guard;
        m_returnedEntryPoints.clear();
    }

    void tryRegisterCoreModule()
    {
        if (!m_module || m_hasRegisteredCoreModule)
            return;
        auto layout = m_module->getLayout(0, nullptr);
        if (layout)
        {
            slang::TypeReflection* coreType = layout->findTypeByName("int");
            Slang::DeclRefType* declRefType =
                Slang::as<Slang::DeclRefType>(Slang::asInternal(coreType));
            IModule* owningModule = Slang::getModule(declRefType->getDeclRef().getDecl());
            wrapObject(owningModule);
            m_hasRegisteredCoreModule = true;
        }
    }

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(ComponentTypeProxy)

    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject)
        SLANG_OVERRIDE
    {
        if (!outObject)
            return SLANG_E_INVALID_ARG;

        // Proxy's own GUID
        if (uuid == ComponentTypeProxy::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<slang::IComponentType*>(static_cast<slang::IModule*>(this));
            return SLANG_OK;
        }

        // IModule (only if underlying supports it)
        if (uuid == slang::IModule::getTypeGuid() && m_module)
        {
            addRef();
            *outObject = static_cast<slang::IModule*>(this);
            return SLANG_OK;
        }

        // IEntryPoint (only if underlying supports it)
        if (uuid == slang::IEntryPoint::getTypeGuid() && m_entryPoint)
        {
            addRef();
            *outObject = static_cast<slang::IEntryPoint*>(this);
            return SLANG_OK;
        }

        // ITypeConformance (only if underlying supports it)
        if (uuid == slang::ITypeConformance::getTypeGuid() && m_typeConformance)
        {
            addRef();
            *outObject = static_cast<slang::ITypeConformance*>(this);
            return SLANG_OK;
        }

        // IComponentType - always supported
        if (uuid == slang::IComponentType::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<slang::IComponentType*>(static_cast<slang::IModule*>(this));
            return SLANG_OK;
        }

        // IComponentType2
        if (uuid == slang::IComponentType2::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<slang::IComponentType2*>(this);
            return SLANG_OK;
        }

        // IModulePrecompileService_Experimental
        if (uuid == slang::IModulePrecompileService_Experimental::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<slang::IModulePrecompileService_Experimental*>(this);
            return SLANG_OK;
        }

        // ISlangUnknown
        if (uuid == ISlangUnknown::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangUnknown*>(static_cast<slang::IModule*>(this));
            return SLANG_OK;
        }

        return m_actual->queryInterface(uuid, outObject);
    }

    // =========================================================================
    // IComponentType
    // =========================================================================

    virtual SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() override
    {
        RECORD_CALL();
        slang::ISession* result = m_componentType->getSession();
        return RECORD_COM_RESULT(result);
    }

    virtual SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL
    getLayout(SlangInt targetIndex, ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(targetIndex);
        PREPARE_POINTER_OUTPUT(outDiagnostics);
        slang::ProgramLayout* result = m_componentType->getLayout(targetIndex, outDiagnostics);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangInt SLANG_MCALL getSpecializationParamCount() override
    {
        RECORD_CALL();
        SlangInt result = m_componentType->getSpecializationParamCount();
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
        auto result = m_componentType->getEntryPointCode(
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
        m_componentType->getEntryPointHash(entryPointIndex, targetIndex, outHash);
        RECORD_COM_OUTPUT(outHash);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL specialize(
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        slang::IComponentType** outSpecializedComponentType,
        ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT_ARRAY(specializationArgs, specializationArgCount);
        PREPARE_POINTER_OUTPUT(outSpecializedComponentType);
        PREPARE_POINTER_OUTPUT(outDiagnostics);
        auto result = m_componentType->specialize(
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
        auto result = m_componentType->link(outLinkedComponentType, outDiagnostics);
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
        auto result = m_componentType->getEntryPointHostCallable(
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
        RECORD_CALL();
        RECORD_INPUT_ARRAY(compilerOptionEntries, compilerOptionEntryCount);
        PREPARE_POINTER_OUTPUT(outLinkedComponentType);
        PREPARE_POINTER_OUTPUT(outDiagnostics);
        auto result = m_componentType->linkWithOptions(
            outLinkedComponentType,
            compilerOptionEntryCount,
            compilerOptionEntries,
            outDiagnostics);
        RECORD_COM_OUTPUT(outLinkedComponentType);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetCode(SlangInt targetIndex, ISlangBlob** outCode, ISlangBlob** outDiagnostics) override
    {
        RECORD_CALL();
        RECORD_INPUT(targetIndex);
        PREPARE_POINTER_OUTPUT(outCode);
        PREPARE_POINTER_OUTPUT(outDiagnostics);
        auto result = m_componentType->getTargetCode(targetIndex, outCode, outDiagnostics);
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
        auto result = m_componentType->getTargetMetadata(targetIndex, outMetadata, outDiagnostics);
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

    // =========================================================================
    // IModule (only valid when m_module != nullptr)
    // =========================================================================

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    findEntryPointByName(char const* name, slang::IEntryPoint** outEntryPoint) override
    {
        if (!m_module)
            return SLANG_E_NOT_IMPLEMENTED;
        RECORD_CALL();
        RECORD_INPUT(name);
        PREPARE_POINTER_OUTPUT(outEntryPoint);
        SlangResult result = m_module->findEntryPointByName(name, outEntryPoint);
        RECORD_ENTRYPOINT_OUTPUT(outEntryPoint);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangInt32 SLANG_MCALL getDefinedEntryPointCount() override
    {
        if (!m_module)
            return 0;
        RECORD_CALL();
        auto result = m_module->getDefinedEntryPointCount();
        RECORD_INFO(result);
        return result;
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getDefinedEntryPoint(SlangInt32 index, slang::IEntryPoint** outEntryPoint) override
    {
        if (!m_module)
            return SLANG_E_NOT_IMPLEMENTED;
        RECORD_CALL();
        RECORD_INPUT(index);
        PREPARE_POINTER_OUTPUT(outEntryPoint);
        SlangResult result = m_module->getDefinedEntryPoint(index, outEntryPoint);
        RECORD_ENTRYPOINT_OUTPUT(outEntryPoint);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    serialize(ISlangBlob** outSerializedBlob) override
    {
        if (!m_module)
            return SLANG_E_NOT_IMPLEMENTED;
        RECORD_CALL();
        PREPARE_POINTER_OUTPUT(outSerializedBlob);
        auto result = m_module->serialize(outSerializedBlob);
        RECORD_COM_OUTPUT(outSerializedBlob);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL writeToFile(char const* fileName) override
    {
        if (!m_module)
            return SLANG_E_NOT_IMPLEMENTED;
        RECORD_CALL();
        RECORD_INPUT(fileName);
        auto result = m_module->writeToFile(fileName);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW const char* SLANG_MCALL getName() override
    {
        if (!m_module)
            return nullptr;
        RECORD_CALL();
        return m_module->getName();
    }

    virtual SLANG_NO_THROW const char* SLANG_MCALL getFilePath() override
    {
        if (!m_module)
            return nullptr;
        RECORD_CALL();
        return m_module->getFilePath();
    }

    virtual SLANG_NO_THROW const char* SLANG_MCALL getUniqueIdentity() override
    {
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::getUniqueIdentity");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL findAndCheckEntryPoint(
        char const* name,
        SlangStage stage,
        slang::IEntryPoint** outEntryPoint,
        ISlangBlob** outDiagnostics) override
    {
        if (!m_module)
            return SLANG_E_NOT_IMPLEMENTED;
        RECORD_CALL();
        RECORD_INPUT(name);
        RECORD_INPUT(stage);
        PREPARE_POINTER_OUTPUT(outEntryPoint);
        PREPARE_POINTER_OUTPUT(outDiagnostics);
        SlangResult result =
            m_module->findAndCheckEntryPoint(name, stage, outEntryPoint, outDiagnostics);
        RECORD_ENTRYPOINT_OUTPUT(outEntryPoint);
        RECORD_COM_OUTPUT(outDiagnostics);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangInt32 SLANG_MCALL getDependencyFileCount() override
    {
         if (!m_module)
            return 0;
        RECORD_CALL();
        auto result = m_module->getDependencyFileCount();
        RECORD_INFO(result);
        return result;
    }

    virtual SLANG_NO_THROW char const* SLANG_MCALL getDependencyFilePath(SlangInt32 index) override
    {
        if (!m_module)
            return nullptr;
        RECORD_CALL();
        RECORD_INPUT(index);
        auto result = m_module->getDependencyFilePath(index);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW slang::DeclReflection* SLANG_MCALL getModuleReflection() override
    {
        if (!m_module)
            return nullptr;
        RECORD_CALL();
        return m_module->getModuleReflection();
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    disassemble(slang::IBlob** outDisassembledBlob) override
    {
        SLANG_UNUSED(outDisassembledBlob);
        REPLAY_UNIMPLEMENTED_X("ComponentTypeProxy::disassemble");
    }

    // =========================================================================
    // IEntryPoint (only valid when m_entryPoint != nullptr)
    // =========================================================================

    virtual SLANG_NO_THROW slang::FunctionReflection* SLANG_MCALL getFunctionReflection() override
    {
        if (!m_entryPoint)
            return nullptr;
        RECORD_CALL();
        return m_entryPoint->getFunctionReflection();
    }

    // ITypeConformance has no additional methods beyond IComponentType

    // =========================================================================
    // IComponentType2
    // =========================================================================

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
            entryPointIndex,
            targetIndex,
            outCompileResult,
            outDiagnostics);
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

    // =========================================================================
    // IModulePrecompileService_Experimental
    // =========================================================================

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

private:
    slang::IComponentType* m_componentType;     // Always valid
    slang::IModule* m_module;                   // May be null
    slang::IEntryPoint* m_entryPoint;           // May be null
    slang::ITypeConformance* m_typeConformance; // May be null
    bool m_hasRegisteredCoreModule;
    List<ComPtr<slang::IEntryPoint>> m_returnedEntryPoints;
};

} // namespace SlangRecord

#endif // SLANG_PROXY_COMPONENT_TYPE_H
