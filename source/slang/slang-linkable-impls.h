// slang-linkable-impl.h
#pragma once

//
// This file declares various implementations of linkable
// objects (subclasses of `ComponentType`).
//
// Note that the base `ComponentType` class is declared
// in `slang-linkable.h`.
//
// Note that the most important two classes of linkable
// objects, `Module`s and `EntryPoint`s, have their own
// headers: `slang-module.h` and `slang-entry-point.h`,
// respectively.
//

#include "slang-entry-point.h"
#include "slang-linkable.h"
#include "slang-module.h"

namespace Slang
{
/// A component type built up from other component types.
class CompositeComponentType : public ComponentType
{
public:
    static RefPtr<ComponentType> create(
        Linkage* linkage,
        List<RefPtr<ComponentType>> const& childComponents);

    virtual void buildHash(DigestBuilder<SHA1>& builder) SLANG_OVERRIDE;

    List<RefPtr<ComponentType>> const& getChildComponents() { return m_childComponents; };
    Index getChildComponentCount() { return m_childComponents.getCount(); }
    RefPtr<ComponentType> getChildComponent(Index index) { return m_childComponents[index]; }

    Index getEntryPointCount() SLANG_OVERRIDE;
    RefPtr<EntryPoint> getEntryPoint(Index index) SLANG_OVERRIDE;
    String getEntryPointMangledName(Index index) SLANG_OVERRIDE;
    String getEntryPointNameOverride(Index index) SLANG_OVERRIDE;

    Index getShaderParamCount() SLANG_OVERRIDE;
    ShaderParamInfo getShaderParam(Index index) SLANG_OVERRIDE;

    SLANG_NO_THROW Index SLANG_MCALL getSpecializationParamCount() SLANG_OVERRIDE;
    SpecializationParam const& getSpecializationParam(Index index) SLANG_OVERRIDE;

    Index getRequirementCount() SLANG_OVERRIDE;
    RefPtr<ComponentType> getRequirement(Index index) SLANG_OVERRIDE;

    List<Module*> const& getModuleDependencies() SLANG_OVERRIDE;
    List<SourceFile*> const& getFileDependencies() SLANG_OVERRIDE;

    class CompositeSpecializationInfo : public SpecializationInfo
    {
    public:
        List<RefPtr<SpecializationInfo>> childInfos;
    };

protected:
    void acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo)
        SLANG_OVERRIDE;


    RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
        SpecializationArg const* args,
        Index argCount,
        Index& outConsumedArgCount,
        DiagnosticSink* sink) SLANG_OVERRIDE;

public:
    CompositeComponentType(Linkage* linkage, List<RefPtr<ComponentType>> const& childComponents);

private:
    List<RefPtr<ComponentType>> m_childComponents;

    // The following arrays hold the concatenated entry points, parameters,
    // etc. from the child components. This approach allows for reasonably
    // fast (constant time) access through operations like `getShaderParam`,
    // but means that the memory usage of a composite is proportional to
    // the sum of the memory usage of the children, rather than being fixed
    // by the number of children (as it would be if we just stored
    // `m_childComponents`).
    //
    // TODO: We could conceivably build some O(numChildren) arrays that
    // support binary-search to provide logarithmic-time access to entry
    // points, parameters, etc. while giving a better overall memory usage.
    //
    List<EntryPoint*> m_entryPoints;
    List<String> m_entryPointMangledNames;
    List<String> m_entryPointNameOverrides;
    List<ShaderParamInfo> m_shaderParams;
    List<SpecializationParam> m_specializationParams;
    List<ComponentType*> m_requirements;

    ModuleDependencyList m_moduleDependencyList;
    FileDependencyList m_fileDependencyList;
};

/// A component type created by specializing another component type.
class SpecializedComponentType : public ComponentType
{
public:
    SpecializedComponentType(
        ComponentType* base,
        SpecializationInfo* specializationInfo,
        List<SpecializationArg> const& specializationArgs,
        DiagnosticSink* sink);

    virtual void buildHash(DigestBuilder<SHA1>& builer) SLANG_OVERRIDE;

    /// Get the base (unspecialized) component type that is being specialized.
    RefPtr<ComponentType> getBaseComponentType() { return m_base; }

    RefPtr<SpecializationInfo> getSpecializationInfo() { return m_specializationInfo; }

    /// Get the number of arguments supplied for existential type parameters.
    ///
    /// Note that the number of arguments may not match the number of parameters.
    /// In particular, an unspecialized entry point may have many parameters, but zero arguments.
    Index getSpecializationArgCount() { return m_specializationArgs.getCount(); }

    /// Get the existential type argument (type and witness table) at `index`.
    SpecializationArg const& getSpecializationArg(Index index)
    {
        return m_specializationArgs[index];
    }

    /// Get an array of all existential type arguments.
    SpecializationArg const* getSpecializationArgs() { return m_specializationArgs.getBuffer(); }

    Index getEntryPointCount() SLANG_OVERRIDE { return m_base->getEntryPointCount(); }
    RefPtr<EntryPoint> getEntryPoint(Index index) SLANG_OVERRIDE
    {
        return m_base->getEntryPoint(index);
    }
    String getEntryPointMangledName(Index index) SLANG_OVERRIDE;
    String getEntryPointNameOverride(Index index) SLANG_OVERRIDE;

    Index getShaderParamCount() SLANG_OVERRIDE { return m_base->getShaderParamCount(); }
    ShaderParamInfo getShaderParam(Index index) SLANG_OVERRIDE
    {
        return m_base->getShaderParam(index);
    }

    SLANG_NO_THROW Index SLANG_MCALL getSpecializationParamCount() SLANG_OVERRIDE { return 0; }
    SpecializationParam const& getSpecializationParam(Index index) SLANG_OVERRIDE
    {
        SLANG_UNUSED(index);
        static SpecializationParam dummy;
        return dummy;
    }

    Index getRequirementCount() SLANG_OVERRIDE;
    RefPtr<ComponentType> getRequirement(Index index) SLANG_OVERRIDE;

    List<Module*> const& getModuleDependencies() SLANG_OVERRIDE { return m_moduleDependencies; }
    List<SourceFile*> const& getFileDependencies() SLANG_OVERRIDE { return m_fileDependencies; }

    RefPtr<IRModule> getIRModule() { return m_irModule; }

    void acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo)
        SLANG_OVERRIDE;

protected:
    RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
        SpecializationArg const* args,
        Index argCount,
        Index& outConsumedArgCount,
        DiagnosticSink* sink) SLANG_OVERRIDE
    {
        SLANG_UNUSED(args);
        SLANG_UNUSED(argCount);
        SLANG_UNUSED(sink);
        outConsumedArgCount = 0;
        return nullptr;
    }

private:
    RefPtr<ComponentType> m_base;
    RefPtr<SpecializationInfo> m_specializationInfo;
    SpecializationArgs m_specializationArgs;
    RefPtr<IRModule> m_irModule;

    List<String> m_entryPointMangledNames;
    List<String> m_entryPointNameOverrides;

    List<Module*> m_moduleDependencies;
    List<SourceFile*> m_fileDependencies;
    List<RefPtr<ComponentType>> m_requirements;
};

class RenamedEntryPointComponentType : public ComponentType
{
public:
    using Super = ComponentType;

    RenamedEntryPointComponentType(ComponentType* base, String newName);

    ComponentType* getBase() { return m_base.Ptr(); }

    // Forward `IComponentType` methods

    SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() SLANG_OVERRIDE
    {
        return Super::getSession();
    }

    SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL
    getLayout(SlangInt targetIndex, slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getLayout(targetIndex, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL specialize(
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        slang::IComponentType** outSpecializedComponentType,
        ISlangBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::specialize(
            specializationArgs,
            specializationArgCount,
            outSpecializedComponentType,
            outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    renameEntryPoint(const char* newName, slang::IComponentType** outEntryPoint) SLANG_OVERRIDE
    {
        return Super::renameEntryPoint(newName, outEntryPoint);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    link(slang::IComponentType** outLinkedComponentType, ISlangBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::link(outLinkedComponentType, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
        int entryPointIndex,
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getEntryPointHostCallable(
            entryPointIndex,
            targetIndex,
            outSharedLibrary,
            outDiagnostics);
    }

    List<Module*> const& getModuleDependencies() SLANG_OVERRIDE
    {
        return m_base->getModuleDependencies();
    }
    List<SourceFile*> const& getFileDependencies() SLANG_OVERRIDE
    {
        return m_base->getFileDependencies();
    }

    SLANG_NO_THROW Index SLANG_MCALL getSpecializationParamCount() SLANG_OVERRIDE
    {
        return m_base->getSpecializationParamCount();
    }

    SpecializationParam const& getSpecializationParam(Index index) SLANG_OVERRIDE
    {
        return m_base->getSpecializationParam(index);
    }

    Index getRequirementCount() SLANG_OVERRIDE { return m_base->getRequirementCount(); }
    RefPtr<ComponentType> getRequirement(Index index) SLANG_OVERRIDE
    {
        return m_base->getRequirement(index);
    }
    Index getEntryPointCount() SLANG_OVERRIDE { return m_base->getEntryPointCount(); }
    RefPtr<EntryPoint> getEntryPoint(Index index) SLANG_OVERRIDE
    {
        return m_base->getEntryPoint(index);
    }
    String getEntryPointMangledName(Index index) SLANG_OVERRIDE
    {
        return m_base->getEntryPointMangledName(index);
    }
    String getEntryPointNameOverride(Index index) SLANG_OVERRIDE
    {
        SLANG_UNUSED(index);
        SLANG_ASSERT(index == 0);
        return m_entryPointNameOverride;
    }

    Index getShaderParamCount() SLANG_OVERRIDE { return m_base->getShaderParamCount(); }
    ShaderParamInfo getShaderParam(Index index) SLANG_OVERRIDE
    {
        return m_base->getShaderParam(index);
    }

    void acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo)
        SLANG_OVERRIDE;

    virtual void buildHash(DigestBuilder<SHA1>& builder) SLANG_OVERRIDE;

private:
    RefPtr<ComponentType> m_base;
    String m_entryPointNameOverride;

protected:
    RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
        SpecializationArg const* args,
        Index argCount,
        Index& outConsumedArgCount,
        DiagnosticSink* sink) SLANG_OVERRIDE
    {
        return m_base->_validateSpecializationArgsImpl(args, argCount, outConsumedArgCount, sink);
    }
};

class TypeConformance : public ComponentType, public slang::ITypeConformance
{
    typedef ComponentType Super;

public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    ISlangUnknown* getInterface(const Guid& guid);

    TypeConformance(
        Linkage* linkage,
        SubtypeWitness* witness,
        Int confomrmanceIdOverride,
        DiagnosticSink* sink);

    // Forward `IComponentType` methods

    SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() SLANG_OVERRIDE
    {
        return Super::getSession();
    }

    SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL
    getLayout(SlangInt targetIndex, slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getLayout(targetIndex, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCode(
        SlangInt targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getTargetCode(targetIndex, outCode, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointMetadata(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IMetadata** outMetadata,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getEntryPointMetadata(
            entryPointIndex,
            targetIndex,
            outMetadata,
            outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetMetadata(
        SlangInt targetIndex,
        slang::IMetadata** outMetadata,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getTargetMetadata(targetIndex, outMetadata, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCompileResult(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::ICompileResult** outCompileResult,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getEntryPointCompileResult(
            entryPointIndex,
            targetIndex,
            outCompileResult,
            outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCompileResult(
        SlangInt targetIndex,
        slang::ICompileResult** outCompileResult,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getTargetCompileResult(targetIndex, outCompileResult, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getResultAsFileSystem(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        ISlangMutableFileSystem** outFileSystem) SLANG_OVERRIDE
    {
        return Super::getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL specialize(
        slang::SpecializationArg const* specializationArgs,
        SlangInt specializationArgCount,
        slang::IComponentType** outSpecializedComponentType,
        ISlangBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::specialize(
            specializationArgs,
            specializationArgCount,
            outSpecializedComponentType,
            outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    renameEntryPoint(const char* newName, slang::IComponentType** outEntryPoint) SLANG_OVERRIDE
    {
        return Super::renameEntryPoint(newName, outEntryPoint);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    link(slang::IComponentType** outLinkedComponentType, ISlangBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::link(outLinkedComponentType, outDiagnostics);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL linkWithOptions(
        slang::IComponentType** outLinkedComponentType,
        uint32_t count,
        slang::CompilerOptionEntry* entries,
        ISlangBlob** outDiagnostics) override
    {
        return Super::linkWithOptions(outLinkedComponentType, count, entries, outDiagnostics);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
        int entryPointIndex,
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary,
        slang::IBlob** outDiagnostics) SLANG_OVERRIDE
    {
        return Super::getEntryPointHostCallable(
            entryPointIndex,
            targetIndex,
            outSharedLibrary,
            outDiagnostics);
    }

    SLANG_NO_THROW void SLANG_MCALL getEntryPointHash(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::IBlob** outHash) SLANG_OVERRIDE
    {
        return Super::getEntryPointHash(entryPointIndex, targetIndex, outHash);
    }

    virtual void buildHash(DigestBuilder<SHA1>& builder) SLANG_OVERRIDE;

    List<Module*> const& getModuleDependencies() SLANG_OVERRIDE;
    List<SourceFile*> const& getFileDependencies() SLANG_OVERRIDE;

    SLANG_NO_THROW Index SLANG_MCALL getSpecializationParamCount() SLANG_OVERRIDE { return 0; }

    /// Get the existential type parameter at `index`.
    SpecializationParam const& getSpecializationParam(Index /*index*/) SLANG_OVERRIDE
    {
        static SpecializationParam emptyParam;
        return emptyParam;
    }

    Index getRequirementCount() SLANG_OVERRIDE;
    RefPtr<ComponentType> getRequirement(Index index) SLANG_OVERRIDE;
    Index getEntryPointCount() SLANG_OVERRIDE { return 0; };
    RefPtr<EntryPoint> getEntryPoint(Index index) SLANG_OVERRIDE
    {
        SLANG_UNUSED(index);
        return nullptr;
    }
    String getEntryPointMangledName(Index /*index*/) SLANG_OVERRIDE { return ""; }
    String getEntryPointNameOverride(Index /*index*/) SLANG_OVERRIDE { return ""; }

    Index getShaderParamCount() SLANG_OVERRIDE { return 0; }
    ShaderParamInfo getShaderParam(Index index) SLANG_OVERRIDE
    {
        SLANG_UNUSED(index);
        return ShaderParamInfo();
    }

    SubtypeWitness* getSubtypeWitness() { return m_subtypeWitness; }
    IRModule* getIRModule() { return m_irModule.Ptr(); }

protected:
    void acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo)
        SLANG_OVERRIDE;

    RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
        SpecializationArg const* args,
        Index argCount,
        Index& outConsumedArgCount,
        DiagnosticSink* sink) SLANG_OVERRIDE;

private:
    SubtypeWitness* m_subtypeWitness;
    ModuleDependencyList m_moduleDependencyList;
    FileDependencyList m_fileDependencyList;
    List<RefPtr<Module>> m_requirements;
    HashSet<Module*> m_requirementSet;
    RefPtr<IRModule> m_irModule;
    Int m_conformanceIdOverride;
    void addDepedencyFromWitness(SubtypeWitness* witness);
};

/// A visitor for use with `ComponentType`s, allowing dispatch over the concrete subclasses.
class ComponentTypeVisitor
{
public:
    // The following methods should be overriden in a concrete subclass
    // to customize how it acts on each of the concrete types of component.
    //
    // In cases where the application wants to simply "recurse" on a
    // composite, specialized, or legacy component type it can use
    // the `visitChildren` methods below.
    //
    virtual void visitEntryPoint(
        EntryPoint* entryPoint,
        EntryPoint::EntryPointSpecializationInfo* specializationInfo) = 0;
    virtual void visitModule(
        Module* module,
        Module::ModuleSpecializationInfo* specializationInfo) = 0;
    virtual void visitComposite(
        CompositeComponentType* composite,
        CompositeComponentType::CompositeSpecializationInfo* specializationInfo) = 0;
    virtual void visitSpecialized(SpecializedComponentType* specialized) = 0;
    virtual void visitTypeConformance(TypeConformance* conformance) = 0;
    virtual void visitRenamedEntryPoint(
        RenamedEntryPointComponentType* renamedEntryPoint,
        EntryPoint::EntryPointSpecializationInfo* specializationInfo) = 0;

protected:
    // These helpers can be used to recurse into the logical children of a
    // component type, and are useful for the common case where a visitor
    // only cares about a few leaf cases.
    //
    void visitChildren(
        CompositeComponentType* composite,
        CompositeComponentType::CompositeSpecializationInfo* specializationInfo);
    void visitChildren(SpecializedComponentType* specialized);
};

} // namespace Slang
