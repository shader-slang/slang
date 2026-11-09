// slang-linkable-impls.cpp
#include "slang-linkable-impls.h"

#include "slang-lower-to-ir.h" // for `generateIRForTypeConformance`
#include "slang-mangle.h"

namespace Slang
{

//
// CompositeComponentType
//

RefPtr<ComponentType> CompositeComponentType::create(
    Linkage* linkage,
    List<RefPtr<ComponentType>> const& childComponents)
{
    // TODO: We should ideally be caching the results of
    // composition on the `linkage`, so that if we get
    // asked for the same composite again later we re-use
    // it rather than re-create it.
    //
    // Similarly, we might want to do some amount of
    // work to "canonicalize" the input for composition.
    // E.g., if the user does:
    //
    //    X = compose(A,B);
    //    Y = compose(C,D);
    //    Z = compose(X,Y);
    //
    //    W = compose(A, B, C, D);
    //
    // Then there is no observable difference between
    // Z and W, so we might prefer to have them be identical.

    // If there is only a single child, then we should
    // just return that child rather than create a dummy composite.
    //
    if (childComponents.getCount() == 1)
    {
        return childComponents[0];
    }

    return new CompositeComponentType(linkage, childComponents);
}


CompositeComponentType::CompositeComponentType(
    Linkage* linkage,
    List<RefPtr<ComponentType>> const& childComponents)
    : ComponentType(linkage), m_childComponents(childComponents)
{
    HashSet<ComponentType*> requirementsSet;
    for (auto child : childComponents)
    {
        child->enumerateModules([&](Module* module) { requirementsSet.add(module); });
    }

    for (auto child : childComponents)
    {
        auto childEntryPointCount = child->getEntryPointCount();
        for (Index cc = 0; cc < childEntryPointCount; ++cc)
        {
            m_entryPoints.add(child->getEntryPoint(cc));
            m_entryPointMangledNames.add(child->getEntryPointMangledName(cc));
            m_entryPointNameOverrides.add(child->getEntryPointNameOverride(cc));
        }

        auto childShaderParamCount = child->getShaderParamCount();
        for (Index pp = 0; pp < childShaderParamCount; ++pp)
        {
            m_shaderParams.add(child->getShaderParam(pp));
        }

        auto childSpecializationParamCount = child->getSpecializationParamCount();
        for (Index pp = 0; pp < childSpecializationParamCount; ++pp)
        {
            m_specializationParams.add(child->getSpecializationParam(pp));
        }

        for (auto module : child->getModuleDependencies())
        {
            m_moduleDependencyList.addDependency(module);
        }
        for (auto sourceFile : child->getFileDependencies())
        {
            m_fileDependencyList.addDependency(sourceFile);
        }

        auto childRequirementCount = child->getRequirementCount();
        for (Index rr = 0; rr < childRequirementCount; ++rr)
        {
            auto childRequirement = child->getRequirement(rr);
            if (!requirementsSet.contains(childRequirement))
            {
                requirementsSet.add(childRequirement);
                m_requirements.add(childRequirement);
            }
        }
    }
}

void CompositeComponentType::buildHash(DigestBuilder<SHA1>& builder)
{
    auto componentCount = getChildComponentCount();

    for (Index i = 0; i < componentCount; ++i)
    {
        getChildComponent(i)->buildHash(builder);
    }
}

Index CompositeComponentType::getEntryPointCount()
{
    return m_entryPoints.getCount();
}

RefPtr<EntryPoint> CompositeComponentType::getEntryPoint(Index index)
{
    return m_entryPoints[index];
}

String CompositeComponentType::getEntryPointMangledName(Index index)
{
    return m_entryPointMangledNames[index];
}

String CompositeComponentType::getEntryPointNameOverride(Index index)
{
    return m_entryPointNameOverrides[index];
}

Index CompositeComponentType::getShaderParamCount()
{
    return m_shaderParams.getCount();
}

ShaderParamInfo CompositeComponentType::getShaderParam(Index index)
{
    return m_shaderParams[index];
}

Index CompositeComponentType::getSpecializationParamCount()
{
    return m_specializationParams.getCount();
}

SpecializationParam const& CompositeComponentType::getSpecializationParam(Index index)
{
    return m_specializationParams[index];
}

Index CompositeComponentType::getRequirementCount()
{
    return m_requirements.getCount();
}

RefPtr<ComponentType> CompositeComponentType::getRequirement(Index index)
{
    return m_requirements[index];
}

List<Module*> const& CompositeComponentType::getModuleDependencies()
{
    return m_moduleDependencyList.getModuleList();
}

List<SourceFile*> const& CompositeComponentType::getFileDependencies()
{
    return m_fileDependencyList.getFileList();
}

void CompositeComponentType::acceptVisitor(
    ComponentTypeVisitor* visitor,
    SpecializationInfo* specializationInfo)
{
    visitor->visitComposite(this, as<CompositeSpecializationInfo>(specializationInfo));
}

RefPtr<ComponentType::SpecializationInfo> CompositeComponentType::_validateSpecializationArgsImpl(
    SpecializationArg const* args,
    Index argCount,
    Index& outConsumedArgCount,
    DiagnosticSink* sink)
{
    SLANG_UNUSED(argCount);

    RefPtr<CompositeSpecializationInfo> specializationInfo = new CompositeSpecializationInfo();

    Index offset = 0;
    for (auto child : m_childComponents)
    {
        Index consumedArgCount = 0;
        auto childInfo = child->_validateSpecializationArgs(
            args + offset,
            argCount - offset,
            consumedArgCount,
            sink);
        specializationInfo->childInfos.add(childInfo);
        offset += consumedArgCount;
    }
    outConsumedArgCount = offset;
    return specializationInfo;
}

//
// SpecializedComponentType
//

/// Order groups by their entry-point indices, independent of declaration order.
static bool precedesUniformLayoutGroup(List<Index> const& left, List<Index> const& right)
{
    auto count = Math::Min(left.getCount(), right.getCount());
    for (Index i = 0; i < count; ++i)
    {
        if (left[i] != right[i])
            return left[i] < right[i];
    }
    return left.getCount() < right.getCount();
}

/// Canonicalize sets of entry-point indices without changing entry-point enumeration.
static void normalizeUniformLayoutGroups(List<List<Index>>& groups)
{
    for (auto& group : groups)
    {
        group.sort();
        for (Index i = group.getCount() - 1; i > 0; --i)
        {
            if (group[i] == group[i - 1])
                group.removeAt(i);
        }
    }
    groups.sort(precedesUniformLayoutGroup);
    for (Index i = groups.getCount() - 1; i > 0; --i)
    {
        if (!precedesUniformLayoutGroup(groups[i - 1], groups[i]))
            groups.removeAt(i);
    }
}

SlangResult ComponentType::createEntryPointUniformLayoutConstraints(
    slang::EntryPointUniformLayoutGroup const* groups,
    SlangInt groupCount,
    slang::IComponentType** outConstraints,
    slang::IBlob** outDiagnostics)
{
    if (!outConstraints)
        return SLANG_E_INVALID_ARG;
    *outConstraints = nullptr;
    if (outDiagnostics)
        *outDiagnostics = nullptr;
    if (groupCount < 0 || (groupCount && !groups))
        return SLANG_E_INVALID_ARG;

    std::lock_guard<std::recursive_mutex> lock(getLinkage()->getComponentTypeOperationMutex());
    List<List<Index>> copiedGroups;
    for (Index g = 0; g < groupCount; ++g)
    {
        auto const& group = groups[g];
        if (group.entryPointCount <= 0 || !group.entryPointIndices)
            return SLANG_E_INVALID_ARG;
        List<Index> indices;
        for (Index i = 0; i < group.entryPointCount; ++i)
        {
            auto index = group.entryPointIndices[i];
            if (index < 0 || index >= getEntryPointCount())
                return SLANG_E_INVALID_ARG;
            indices.add(index);
        }
        copiedGroups.add(_Move(indices));
    }
    normalizeUniformLayoutGroups(copiedGroups);
    RefPtr<EntryPointUniformLayoutConstraints> constraints =
        new EntryPointUniformLayoutConstraints(this, copiedGroups);
    *outConstraints = asExternal(constraints.detach());
    return SLANG_OK;
}

void EntryPointUniformLayoutConstraints::buildHash(DigestBuilder<SHA1>& builder)
{
    builder.append(UnownedStringSlice("entry-point-uniform-layout-constraints-v1"));
    m_source->buildHash(builder);
    builder.append(m_groups.getCount());
    for (auto const& group : m_groups)
    {
        builder.append(group.getCount());
        for (auto index : group)
            builder.append(index);
    }
}

void EntryPointUniformLayoutConstraints::acceptVisitor(
    ComponentTypeVisitor* visitor,
    SpecializationInfo* specializationInfo)
{
    SLANG_RELEASE_ASSERT(!specializationInfo);
    visitor->visitEntryPointUniformLayoutConstraints(this);
}

/// Resolve membership using the component tree that defines entry-point enumeration.
/// Consider compose({module, V, F}) with a constraint on indices {0, 1}. When that program
/// is embedded after another entry point, its namespace starts at index 1, so the resolved
/// group becomes {1, 2}. Specialization preserves these occurrences. Function declarations
/// and mangled names cannot identify them: the same function can have several specializations.
struct CollectUniformLayoutGroups : ComponentTypeVisitor
{
    Dictionary<ComponentType*, List<Index>> namespaces;
    List<EntryPointUniformLayoutConstraints*> constraints;
    Index nextEntryPoint = 0;

    void visitComponent(ComponentType* component, ComponentType::SpecializationInfo* info = nullptr)
    {
        auto& offsets = namespaces.getOrAddValue(component, List<Index>());
        offsets.add(nextEntryPoint);
        component->acceptVisitor(this, info);
    }

    void visitEntryPoint(EntryPoint*, EntryPoint::EntryPointSpecializationInfo*) SLANG_OVERRIDE
    {
        ++nextEntryPoint;
    }

    void visitModule(Module*, Module::ModuleSpecializationInfo*) SLANG_OVERRIDE {}
    void visitTypeConformance(TypeConformance*) SLANG_OVERRIDE {}

    void visitComposite(
        CompositeComponentType* composite,
        CompositeComponentType::CompositeSpecializationInfo* info) SLANG_OVERRIDE
    {
        for (Index i = 0; i < composite->getChildComponentCount(); ++i)
            visitComponent(composite->getChildComponent(i), info ? info->childInfos[i] : nullptr);
    }

    void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
    {
        visitComponent(specialized->getBaseComponentType(), specialized->getSpecializationInfo());
    }

    void visitRenamedEntryPoint(
        RenamedEntryPointComponentType* renamed,
        EntryPoint::EntryPointSpecializationInfo* info) SLANG_OVERRIDE
    {
        visitComponent(renamed->getBase(), info);
    }

    void visitEntryPointUniformLayoutConstraints(EntryPointUniformLayoutConstraints* component)
        SLANG_OVERRIDE
    {
        constraints.add(component);
    }
};

SlangResult ComponentType::getEntryPointUniformLayoutGroups(
    List<List<Index>>& outGroups,
    DiagnosticSink* sink)
{
    CollectUniformLayoutGroups collector;
    collector.visitComponent(this);
    SLANG_RELEASE_ASSERT(collector.nextEntryPoint == getEntryPointCount());
    outGroups.clear();
    for (auto constraints : collector.constraints)
    {
        auto offsets = collector.namespaces.tryGetValue(constraints->getSource());
        if (!offsets || offsets->getCount() != 1)
        {
            sink->diagnose(Diagnostics::InvalidEntryPointUniformLayoutConstraints{
                .reason = "the referenced component must occur exactly once in the program"});
            return SLANG_E_INVALID_ARG;
        }
        for (auto const& group : constraints->getGroups())
        {
            List<Index> indices;
            for (auto index : group)
                indices.add((*offsets)[0] + index);
            outGroups.add(_Move(indices));
        }
    }
    normalizeUniformLayoutGroups(outGroups);
    return SLANG_OK;
}

/// Utility type for collecting modules references by types/declarations
struct SpecializationArgModuleCollector : ComponentTypeVisitor
{
    HashSet<Module*> m_modulesSet;
    List<Module*> m_modulesList;

    void addModule(Module* module)
    {
        m_modulesList.add(module);
        m_modulesSet.add(module);
    }

    void maybeAddModule(Module* module)
    {
        if (!module)
            return;
        if (m_modulesSet.contains(module))
            return;

        addModule(module);
    }

    void collectReferencedModules(Decl* decl)
    {
        auto module = getModule(decl);
        maybeAddModule(module);
    }

    void collectReferencedModules(SubstitutionSet substitutions)
    {
        substitutions.forEachGenericSubstitution(
            [this](GenericDecl*, Val::OperandView<Val> args)
            {
                for (auto arg : args)
                {
                    collectReferencedModules(arg);
                }
            });
    }

    void collectReferencedModules(DeclRefBase* declRef)
    {
        collectReferencedModules(declRef->getDecl());
        collectReferencedModules(SubstitutionSet(declRef));
    }

    void collectReferencedModules(Type* type)
    {
        if (auto declRefType = as<DeclRefType>(type))
        {
            collectReferencedModules(declRefType->getDeclRef());
        }

        // TODO: Handle non-decl-ref composite type cases
        // (e.g., function types).
    }

    void collectReferencedModules(Val* val)
    {
        if (auto type = as<Type>(val))
        {
            collectReferencedModules(type);
        }
        else if (auto declRefVal = as<DeclRefIntVal>(val))
        {
            collectReferencedModules(declRefVal->getDeclRef());
        }

        // TODO: other cases of values that could reference
        // a declaration.
    }

    void collectReferencedModules(List<ExpandedSpecializationArg> const& args)
    {
        for (auto arg : args)
        {
            collectReferencedModules(arg.val);
            collectReferencedModules(arg.witness);
        }
    }

    //
    // ComponentTypeVisitor methods
    //

    void visitEntryPoint(
        EntryPoint* entryPoint,
        EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        SLANG_UNUSED(entryPoint);

        if (!specializationInfo)
            return;

        collectReferencedModules(specializationInfo->specializedFuncDeclRef);
        collectReferencedModules(specializationInfo->existentialSpecializationArgs);
    }

    void visitRenamedEntryPoint(
        RenamedEntryPointComponentType* entryPoint,
        EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        entryPoint->getBase()->acceptVisitor(this, specializationInfo);
    }

    void visitModule(Module* module, Module::ModuleSpecializationInfo* specializationInfo)
        SLANG_OVERRIDE
    {
        SLANG_UNUSED(module);

        if (!specializationInfo)
            return;

        for (auto arg : specializationInfo->genericArgs)
        {
            collectReferencedModules(arg.argVal);
        }
        collectReferencedModules(specializationInfo->existentialArgs);
    }

    void visitComposite(
        CompositeComponentType* composite,
        CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
    {
        visitChildren(composite, specializationInfo);
    }

    void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
    {
        visitChildren(specialized);
    }

    void visitTypeConformance(TypeConformance* conformance) SLANG_OVERRIDE
    {
        SLANG_UNUSED(conformance);
    }
};

SpecializedComponentType::SpecializedComponentType(
    ComponentType* base,
    ComponentType::SpecializationInfo* specializationInfo,
    List<SpecializationArg> const& specializationArgs,
    DiagnosticSink* sink)
    : ComponentType(base->getLinkage())
    , m_base(base)
    , m_specializationInfo(specializationInfo)
    , m_specializationArgs(specializationArgs)
{
    m_optionSet.overrideWith(base->getOptionSet());

    m_irModule = generateIRForSpecializedComponentType(this, sink);

    // We need to account for the fact that a specialized
    // entity like `myShader<SomeType>` needs to not only
    // depend on the module(s) that `myShader` depends on,
    // but also on any modules that `SomeType` depends on.
    //
    // We will set up a "collector" type that will be
    // used to build a list of these additional modules.
    //
    SpecializationArgModuleCollector moduleCollector;

    // We don't want to go adding additional requirements for
    // modules that the base component type already includes,
    // so we will add those to the set of modules in
    // the collector before we starting trying to add others.
    //
    base->enumerateModules([&](Module* module) { moduleCollector.m_modulesSet.add(module); });

    // In order to collect the additional modules, we need
    // to inspect the specialization arguments and see what
    // they depend on.
    //
    // Naively, it seems like we'd just want to iterate
    // over `specializationArgs`, which gives the specialization
    // arguments as the user supplied them. However, such
    // an approach would have a subtle problem.
    //
    // If we have a generic entry point like:
    //
    //      // In module A
    //      myShader<T : IThing>
    //
    //
    // And the type `SomeType` that is being used as an argument doesn't
    // directly conform to `IThing`:
    //
    //      // In module B
    //      struct SomeType { ... }
    //
    // and the conformance of `SomeType` to `IThing` is
    // coming from yet another module:
    //
    //      // In module C
    //      import B;
    //      extension SomeType : IThing { ... }
    //
    // In this case, the specialized component for `myShader<SomeType>`
    // needs to depend on all of:
    //
    // * Module A, because it defines `myShader`
    // * Module B, because it defines `SomeType`
    // * Module C, because it defines the conformance `SomeType : IThing`
    //
    // We thus need to iterate over a form of the specialization
    // arguments that includes the "expanded" arguments like
    // interface conformance witnesses that got added during
    // semantic checking.
    //
    // The expanded arguments are being stored in the `specializationInfo`
    // today (for use by downstream code generation), and the easiest
    // way to walk that information and get to the leaf nodes where
    // the expanded arguments are stored is to apply a visitor to
    // the specialized component type we are in the middle of constructing.
    //
    moduleCollector.visitSpecialized(this);

    // Now that we've collected our additional information, we can
    // start to build up the final lists for the specialized component type.
    //
    // The starting point for our lists comes from the base component type.
    //
    m_moduleDependencies = base->getModuleDependencies();
    m_fileDependencies = base->getFileDependencies();

    Index baseRequirementCount = base->getRequirementCount();
    for (Index r = 0; r < baseRequirementCount; r++)
    {
        m_requirements.add(base->getRequirement(r));
    }

    // The specialized component type will need to have additional
    // dependencies and requirements based on the modules that
    // were collected when looking at the specialization arguments.

    // We want to avoid adding the same file dependency more than once.
    //
    HashSet<SourceFile*> fileDependencySet;
    for (SourceFile* sourceFile : m_fileDependencies)
        fileDependencySet.add(sourceFile);

    for (auto module : moduleCollector.m_modulesList)
    {
        // The specialized component type will have an open (unsatisfied)
        // requirement for each of the modules that its specialization
        // arguments need.
        //
        // Note: what this means in practice is that the component type
        // records that the given module(s) will need to be linked in
        // before final code can be generated, but it importantly
        // does not dictate the final placement of the parameters from
        // those modules in the layout.
        //
        m_requirements.add(module);

        // The speciialized component type will also have a dependency
        // on all the files that any of the modules involved in
        // it depend on (including those that are required but not
        // yet linked in).
        //
        // The file path information is what a client would need to
        // use to decide if kernel code is out of date compared to
        // source files, so we want to include anything that could
        // affect the validity of generated code.
        //
        for (SourceFile* sourceFile : module->getFileDependencies())
        {
            if (fileDependencySet.contains(sourceFile))
                continue;
            fileDependencySet.add(sourceFile);
            m_fileDependencies.add(sourceFile);
        }

        // Finalyl we also add the module for the specialization arguments
        // to the list of modules that would be used for legacy lookup
        // operations where we need an implicit/default scope to use
        // and want it to be expansive.
        //
        // TODO: This stuff really isn't worth keeping around long
        // term, and we should ditch the entire "legacy lookup" idea.
        //
        m_moduleDependencies.add(module);
    }

    // Because we are specializing shader code, the mangled entry
    // point names for this component type may be different than
    // for the base component type (e.g., the mangled name for `f<int>`
    // is different than that that of the generic `f` function
    // itself).
    //
    // We will compute the mangled names of all the entry points and
    // store them here, so that we don't have to do it on the fly.
    // Because the `ComponentType` structure is hierarchical, we
    // need to use a recursive visitor to compute the names,
    // and we will define that visitor locally:
    //
    struct EntryPointMangledNameCollector : ComponentTypeVisitor
    {
        List<String>* mangledEntryPointNames;
        List<String>* entryPointNameOverrides;

        void visitEntryPoint(
            EntryPoint* entryPoint,
            EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
        {
            auto funcDeclRef = entryPoint->getFuncDeclRef();
            if (specializationInfo)
                funcDeclRef = specializationInfo->specializedFuncDeclRef;

            (*mangledEntryPointNames).add(getMangledName(m_astBuilder, funcDeclRef));
            (*entryPointNameOverrides).add(entryPoint->getEntryPointNameOverride(0));
        }

        void visitRenamedEntryPoint(
            RenamedEntryPointComponentType* entryPoint,
            EntryPoint::EntryPointSpecializationInfo* specializationInfo) SLANG_OVERRIDE
        {
            entryPoint->getBase()->acceptVisitor(this, specializationInfo);
            (*entryPointNameOverrides).getLast() = entryPoint->getEntryPointNameOverride(0);
        }

        void visitModule(Module*, Module::ModuleSpecializationInfo*) SLANG_OVERRIDE {}
        void visitComposite(
            CompositeComponentType* composite,
            CompositeComponentType::CompositeSpecializationInfo* specializationInfo) SLANG_OVERRIDE
        {
            visitChildren(composite, specializationInfo);
        }
        void visitSpecialized(SpecializedComponentType* specialized) SLANG_OVERRIDE
        {
            visitChildren(specialized);
        }
        void visitTypeConformance(TypeConformance* conformance) SLANG_OVERRIDE
        {
            SLANG_UNUSED(conformance);
        }
        EntryPointMangledNameCollector(ASTBuilder* astBuilder)
            : m_astBuilder(astBuilder)
        {
        }
        ASTBuilder* m_astBuilder;
    };

    // With the visitor defined, we apply it to ourself to compute
    // and collect the mangled entry point names.
    //
    EntryPointMangledNameCollector collector(getLinkage()->getASTBuilder());
    collector.mangledEntryPointNames = &m_entryPointMangledNames;
    collector.entryPointNameOverrides = &m_entryPointNameOverrides;
    collector.visitSpecialized(this);
}

void SpecializedComponentType::buildHash(DigestBuilder<SHA1>& builder)
{
    SLANG_AST_BUILDER_RAII(getLinkage()->getASTBuilder());

    auto specializationArgCount = getSpecializationArgCount();
    for (Index i = 0; i < specializationArgCount; ++i)
    {
        auto specializationArg = getSpecializationArg(i);
        auto argString = specializationArg.toString();
        builder.append(argString);
    }

    getBaseComponentType()->buildHash(builder);
}

void SpecializedComponentType::acceptVisitor(
    ComponentTypeVisitor* visitor,
    SpecializationInfo* specializationInfo)
{
    SLANG_ASSERT(specializationInfo == nullptr);
    SLANG_UNUSED(specializationInfo);
    visitor->visitSpecialized(this);
}

Index SpecializedComponentType::getRequirementCount()
{
    return m_requirements.getCount();
}

RefPtr<ComponentType> SpecializedComponentType::getRequirement(Index index)
{
    return m_requirements[index];
}

String SpecializedComponentType::getEntryPointMangledName(Index index)
{
    if (auto specializationInfo =
            as<EntryPoint::EntryPointSpecializationInfo>(m_specializationInfo))
    {
        auto specializedFuncDeclRef = specializationInfo->specializedFuncDeclRef;
        if (specializedFuncDeclRef)
            return getMangledName(getLinkage()->getASTBuilder(), specializedFuncDeclRef);
    }

    return m_entryPointMangledNames[index];
}

String SpecializedComponentType::getEntryPointNameOverride(Index index)
{
    return m_entryPointNameOverrides[index];
}

//
// RenamedEntryPointComponentType
//

RenamedEntryPointComponentType::RenamedEntryPointComponentType(ComponentType* base, String newName)
    : ComponentType(base->getLinkage()), m_base(base), m_entryPointNameOverride(newName)
{
}

void RenamedEntryPointComponentType::acceptVisitor(
    ComponentTypeVisitor* visitor,
    SpecializationInfo* specializationInfo)
{
    visitor->visitRenamedEntryPoint(
        this,
        as<EntryPoint::EntryPointSpecializationInfo>(specializationInfo));
}

void RenamedEntryPointComponentType::buildHash(DigestBuilder<SHA1>& builder)
{
    SLANG_UNUSED(builder);
}

//
// TypeConformance
//

TypeConformance::TypeConformance(
    Linkage* linkage,
    SubtypeWitness* witness,
    Int confomrmanceIdOverride,
    DiagnosticSink* sink)
    : ComponentType(linkage)
    , m_subtypeWitness(witness)
    , m_conformanceIdOverride(confomrmanceIdOverride)
{
    addDepedencyFromWitness(witness);
    m_irModule = generateIRForTypeConformance(this, m_conformanceIdOverride, sink);
}

void TypeConformance::addDepedencyFromWitness(SubtypeWitness* witness)
{
    if (auto declaredWitness = as<DeclaredSubtypeWitness>(witness))
    {
        auto declModule = getModule(declaredWitness->getDeclRef().getDecl());
        m_moduleDependencyList.addDependency(declModule);
        m_fileDependencyList.addDependency(declModule);
        if (m_requirementSet.add(declModule))
        {
            m_requirements.add(declModule);
        }
        // TODO: handle the specialization arguments in declaredWitness->declRef.substitutions.
    }
    else if (auto transitiveWitness = as<TransitiveSubtypeWitness>(witness))
    {
        addDepedencyFromWitness(transitiveWitness->getMidToSup());
        addDepedencyFromWitness(transitiveWitness->getSubToMid());
    }
}

ISlangUnknown* TypeConformance::getInterface(const Guid& guid)
{
    if (guid == slang::ITypeConformance::getTypeGuid())
        return static_cast<slang::ITypeConformance*>(this);

    return Super::getInterface(guid);
}

void TypeConformance::buildHash(DigestBuilder<SHA1>& builder)
{
    // TODO: Implement some kind of hashInto for Val then replace this
    auto subtypeWitness = m_subtypeWitness->toString();

    builder.append(subtypeWitness);
    builder.append(m_conformanceIdOverride);
}

List<Module*> const& TypeConformance::getModuleDependencies()
{
    return m_moduleDependencyList.getModuleList();
}

List<SourceFile*> const& TypeConformance::getFileDependencies()
{
    return m_fileDependencyList.getFileList();
}

Index TypeConformance::getRequirementCount()
{
    return m_requirements.getCount();
}

RefPtr<ComponentType> TypeConformance::getRequirement(Index index)
{
    return m_requirements[index];
}

void TypeConformance::acceptVisitor(
    ComponentTypeVisitor* visitor,
    ComponentType::SpecializationInfo* specializationInfo)
{
    SLANG_UNUSED(specializationInfo);
    visitor->visitTypeConformance(this);
}

RefPtr<ComponentType::SpecializationInfo> TypeConformance::_validateSpecializationArgsImpl(
    SpecializationArg const* args,
    Index argCount,
    Index& outConsumedArgCount,
    DiagnosticSink* sink)
{
    SLANG_UNUSED(args);
    SLANG_UNUSED(argCount);
    SLANG_UNUSED(sink);
    outConsumedArgCount = 0;
    return nullptr;
}

//
// ComponentTypeVisitor
//

void ComponentTypeVisitor::visitChildren(
    CompositeComponentType* composite,
    CompositeComponentType::CompositeSpecializationInfo* specializationInfo)
{
    auto childCount = composite->getChildComponentCount();
    for (Index ii = 0; ii < childCount; ++ii)
    {
        auto child = composite->getChildComponent(ii);
        auto childSpecializationInfo =
            specializationInfo ? specializationInfo->childInfos[ii] : nullptr;

        child->acceptVisitor(this, childSpecializationInfo);
    }
}

void ComponentTypeVisitor::visitChildren(SpecializedComponentType* specialized)
{
    specialized->getBaseComponentType()->acceptVisitor(this, specialized->getSpecializationInfo());
}

} // namespace Slang
