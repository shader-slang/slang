#include "slang-check-impl.h"
#include "slang-session.h"

namespace Slang
{

static FunctionDeclBase* _getStageImplementation(
    const StructuralRayTracingDeclRegistry& registry,
    StructuralRayTracingStageKind stageKind,
    WitnessTable* witnessTable)
{
    auto invokeRequirement = registry.getStageInvokeRequirement(stageKind);
    RequirementWitness invokeWitness;
    if (!invokeRequirement || !witnessTable ||
        !witnessTable->getRequirementDictionary().tryGetValue(invokeRequirement, invokeWitness) ||
        invokeWitness.getFlavor() != RequirementWitness::Flavor::declRef)
    {
        return nullptr;
    }

    Decl* implementation = invokeWitness.getDeclRef().getDecl();
    while (auto genericDecl = as<GenericDecl>(implementation))
        implementation = genericDecl->inner;
    return as<FunctionDeclBase>(implementation);
}

static void _registerRayTracingAPIUse(
    Linkage* linkage,
    Module* module,
    RayTracingAPIFamily family,
    Decl* decl,
    DiagnosticSink* sink)
{
    auto& registry = linkage->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return;

    Decl* otherDecl = nullptr;
    if (!registry.registerAPIUse(module, family, decl, &otherDecl))
        return;

    auto currentAPI = family == RayTracingAPIFamily::Structural ? "structural" : "legacy";
    auto otherAPI = family == RayTracingAPIFamily::Structural ? "legacy" : "structural";
    sink->diagnose(Diagnostics::MixedRayTracingApis{
        .currentAPI = currentAPI,
        .otherAPI = otherAPI,
        .currentDecl = decl,
        .otherDecl = otherDecl});
}

void SemanticsVisitor::registerStructuralRayTracingStageConformance(
    DeclRef<InterfaceDecl> superInterfaceDeclRef,
    WitnessTable* witnessTable)
{
    auto& registry = getLinkage()->getStructuralRayTracingDeclRegistry();
    auto stageKind = registry.getStageKind(superInterfaceDeclRef.getDecl());
    auto metadataKind = registry.getMetadataKind(superInterfaceDeclRef.getDecl());
    if ((stageKind == StructuralRayTracingStageKind::Count &&
         metadataKind == StructuralRayTracingMetadataKind::Count) ||
        !witnessTable)
        return;

    auto witnessedType = as<DeclRefType>(witnessTable->witnessedType);
    auto witnessedDecl = witnessedType ? witnessedType->getDeclRef().getDecl() : nullptr;
    if (witnessedDecl)
    {
        _registerRayTracingAPIUse(
            getLinkage(),
            getModule(witnessedDecl),
            RayTracingAPIFamily::Structural,
            witnessedDecl,
            getSink());
    }

    if (stageKind == StructuralRayTracingStageKind::Count)
        return;

    registry.registerStageImplementation(
        _getStageImplementation(registry, stageKind, witnessTable),
        stageKind);
}

static bool _isLegacyRayTracingStage(Stage stage)
{
    switch (stage)
    {
    case Stage::ClosestHit:
    case Stage::AnyHit:
    case Stage::Intersection:
    case Stage::Miss:
    case Stage::Callable:
        return true;
    default:
        return false;
    }
}

void diagnoseMixedRayTracingAPIUse(EntryPoint* entryPoint, DiagnosticSink* sink)
{
    if (!_isLegacyRayTracingStage(entryPoint->getStage()))
        return;

    auto entryPointDecl = entryPoint->getFuncDecl();
    auto family = entryPoint->getStructuralRayTracingInvokeMethod()
                      ? RayTracingAPIFamily::Structural
                      : RayTracingAPIFamily::Legacy;
    _registerRayTracingAPIUse(
        entryPoint->getLinkage(),
        getModule(entryPointDecl),
        family,
        entryPointDecl,
        sink);
}

static void _registerAttributedLegacyEntryPoints(
    Linkage* linkage,
    Module* module,
    ContainerDecl* containerDecl,
    DiagnosticSink* sink)
{
    for (auto member : containerDecl->getDirectMemberDecls())
    {
        auto innerMember = member;
        if (auto genericDecl = as<GenericDecl>(innerMember))
            innerMember = genericDecl->inner;

        if (auto functionDecl = as<FuncDecl>(innerMember))
        {
            if (auto entryPointAttr = functionDecl->findModifier<EntryPointAttribute>())
            {
                auto stage =
                    getStageFromAtom(CapabilitySet{entryPointAttr->capabilitySet}.getTargetStage());
                if (_isLegacyRayTracingStage(stage))
                {
                    _registerRayTracingAPIUse(
                        linkage,
                        module,
                        RayTracingAPIFamily::Legacy,
                        functionDecl,
                        sink);
                }
            }
        }

        if (auto childContainer = as<ContainerDecl>(innerMember))
            _registerAttributedLegacyEntryPoints(linkage, module, childContainer, sink);
    }
}

void diagnoseMixedRayTracingAPIsInModule(Linkage* linkage, Module* module, DiagnosticSink* sink)
{
    auto& registry = linkage->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return;
    _registerAttributedLegacyEntryPoints(linkage, module, module->getModuleDecl(), sink);
}

static StructuralRayTracingStageKind _findStageImplementationFromParentConformance(
    StructuralRayTracingDeclRegistry& registry,
    FunctionDeclBase* functionDecl)
{
    Decl* parent = functionDecl->parentDecl;
    while (auto genericDecl = as<GenericDecl>(parent))
        parent = genericDecl->parentDecl;
    auto container = as<ContainerDecl>(parent);
    if (!container)
        return StructuralRayTracingStageKind::Count;

    for (auto inheritanceDecl : container->getDirectMemberDeclsOfType<InheritanceDecl>())
    {
        auto interfaceType = as<DeclRefType>(inheritanceDecl->base.type);
        auto interfaceDeclRef = interfaceType ? interfaceType->getDeclRef().as<InterfaceDecl>()
                                              : DeclRef<InterfaceDecl>();
        auto stageKind = registry.getStageKind(interfaceDeclRef.getDecl());
        if (stageKind == StructuralRayTracingStageKind::Count)
            continue;

        auto implementation =
            _getStageImplementation(registry, stageKind, inheritanceDecl->witnessTable);
        if (implementation == functionDecl)
        {
            registry.registerStageImplementation(functionDecl, stageKind);
            return stageKind;
        }
    }
    return StructuralRayTracingStageKind::Count;
}

static FunctionDeclBase* _getStageImplementationFromSubtypeWitness(
    const StructuralRayTracingDeclRegistry& registry,
    StructuralRayTracingStageKind stageKind,
    SubtypeWitness* witness)
{
    witness = witness ? as<SubtypeWitness>(witness->resolve()) : nullptr;
    if (auto declaredWitness = as<DeclaredSubtypeWitness>(witness))
    {
        auto inheritanceDecl = declaredWitness->getDeclRef().as<InheritanceDecl>().getDecl();
        if (inheritanceDecl)
        {
            if (auto implementation =
                    _getStageImplementation(registry, stageKind, inheritanceDecl->witnessTable))
                return implementation;
        }
    }
    else if (auto transitiveWitness = as<TransitiveSubtypeWitness>(witness))
    {
        if (auto implementation = _getStageImplementationFromSubtypeWitness(
                registry,
                stageKind,
                transitiveWitness->getSubToMid()))
            return implementation;
        return _getStageImplementationFromSubtypeWitness(
            registry,
            stageKind,
            transitiveWitness->getMidToSup());
    }
    return nullptr;
}

static Stage _getNativeStage(StructuralRayTracingStageKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        return Stage::ClosestHit;
    case StructuralRayTracingStageKind::AnyHit:
        return Stage::AnyHit;
    case StructuralRayTracingStageKind::Intersection:
        return Stage::Intersection;
    case StructuralRayTracingStageKind::Miss:
        return Stage::Miss;
    case StructuralRayTracingStageKind::Callable:
        return Stage::Callable;
    default:
        return Stage::Unknown;
    }
}

static StructuralRayTracingStageKind _getStructuralStage(Stage stage)
{
    switch (stage)
    {
    case Stage::ClosestHit:
        return StructuralRayTracingStageKind::ClosestHit;
    case Stage::AnyHit:
        return StructuralRayTracingStageKind::AnyHit;
    case Stage::Intersection:
        return StructuralRayTracingStageKind::Intersection;
    case Stage::Miss:
        return StructuralRayTracingStageKind::Miss;
    case Stage::Callable:
        return StructuralRayTracingStageKind::Callable;
    default:
        return StructuralRayTracingStageKind::Count;
    }
}

static FuncDecl* _createStructuralEntryPointDecl(
    Linkage* linkage,
    Module* module,
    AggTypeDecl* stageType)
{
    auto astBuilder = linkage->getASTBuilder();
    auto moduleDecl = module->getModuleDecl();

    auto funcDecl = astBuilder->create<FuncDecl>();
    funcDecl->nameAndLoc = stageType->nameAndLoc;
    funcDecl->loc = stageType->loc;
    funcDecl->closingSourceLoc = stageType->closingSourceLoc;
    funcDecl->parentDecl = moduleDecl;
    funcDecl->returnType.type = astBuilder->getVoidType();
    funcDecl->ownedScope = astBuilder->create<Scope>();
    funcDecl->ownedScope->containerDecl = funcDecl;
    funcDecl->ownedScope->parent = moduleDecl->ownedScope;

    auto body = astBuilder->create<BlockStmt>();
    body->scopeDecl = astBuilder->create<ScopeDecl>();
    body->scopeDecl->ownedScope = astBuilder->create<Scope>();
    body->scopeDecl->ownedScope->parent = funcDecl->ownedScope;
    body->scopeDecl->parentDecl = funcDecl;
    body->body = astBuilder->create<SeqStmt>();
    body->loc = stageType->loc;
    body->closingSourceLoc = stageType->closingSourceLoc;
    funcDecl->body = body;
    funcDecl->setCheckState(DeclCheckState::CapabilityChecked);

    return funcDecl;
}

DeclRef<FuncDecl> findStructuralRayTracingEntryPointByName(
    Linkage* linkage,
    Module* module,
    Name* name,
    Profile& ioProfile,
    DiagnosticSink* sink,
    bool* outFoundStruct,
    FuncDecl** outInvokeMethod)
{
    *outFoundStruct = false;
    *outInvokeMethod = nullptr;
    auto& registry = linkage->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return DeclRef<FuncDecl>();

    auto expr = module->findDeclFromString(getText(name), sink);
    auto declRefExpr = as<DeclRefExpr>(expr);
    auto stageTypeDeclRef =
        declRefExpr ? declRefExpr->declRef.as<AggTypeDecl>() : DeclRef<AggTypeDecl>();
    if (!stageTypeDeclRef || getModule(stageTypeDeclRef.getDecl()) != module)
        return DeclRef<FuncDecl>();

    *outFoundStruct = true;
    SharedSemanticsContext sharedContext(linkage, module, sink);
    for (auto dependency : module->getModuleDependencies())
    {
        auto moduleDecl = dependency->getModuleDecl();
        if (sharedContext.importedModulesSet.add(moduleDecl))
            sharedContext.importedModulesList.add(moduleDecl);
    }
    SemanticsVisitor visitor(&sharedContext);
    visitor.ensureDecl(stageTypeDeclRef, DeclCheckState::ReadyForConformances);

    FunctionDeclBase* stageImplementations[int(StructuralRayTracingStageKind::Count)] = {};
    auto stageType = DeclRefType::create(linkage->getASTBuilder(), stageTypeDeclRef);
    for (auto facet : visitor.getShared()->getInheritanceInfo(stageType).facets)
    {
        auto interfaceDeclRef = facet->origin.declRef.as<InterfaceDecl>();
        auto kind = registry.getStageKind(interfaceDeclRef.getDecl());
        if (kind != StructuralRayTracingStageKind::Count)
        {
            stageImplementations[int(kind)] =
                _getStageImplementationFromSubtypeWitness(registry, kind, facet->subtypeWitness);
        }
    }

    Count implementedStageCount = 0;
    StructuralRayTracingStageKind onlyImplementedStage = StructuralRayTracingStageKind::Count;
    for (int i = 0; i < int(StructuralRayTracingStageKind::Count); ++i)
    {
        if (stageImplementations[i])
        {
            ++implementedStageCount;
            onlyImplementedStage = StructuralRayTracingStageKind(i);
        }
    }

    if (implementedStageCount == 0)
    {
        sink->diagnose(Diagnostics::StructuralRayTracingEntryPointNotStage{
            .stageType = stageTypeDeclRef.getDecl()});
        return DeclRef<FuncDecl>();
    }

    auto requestedStage = ioProfile.getStage();
    auto selectedStage = _getStructuralStage(requestedStage);
    if (requestedStage != Stage::Unknown)
    {
        if (selectedStage == StructuralRayTracingStageKind::Count ||
            !stageImplementations[int(selectedStage)])
        {
            sink->diagnose(Diagnostics::StructuralRayTracingEntryPointStageMismatch{
                .stage = getStageName(requestedStage),
                .stageType = stageTypeDeclRef.getDecl()});
            return DeclRef<FuncDecl>();
        }
    }
    else if (implementedStageCount == 1)
    {
        selectedStage = onlyImplementedStage;
        ioProfile = Profile(_getNativeStage(selectedStage));
    }
    else
    {
        sink->diagnose(Diagnostics::StructuralRayTracingEntryPointAmbiguousStage{
            .stageType = stageTypeDeclRef.getDecl()});
        return DeclRef<FuncDecl>();
    }

    bool hasInstanceField = false;
    for (auto field : stageTypeDeclRef.getDecl()->getFields())
    {
        if (!isEffectivelyStatic(field))
        {
            sink->diagnose(Diagnostics::StructuralRayTracingStageInstanceField{.field = field});
            hasInstanceField = true;
        }
    }
    if (hasInstanceField)
        return DeclRef<FuncDecl>();

    auto invokeMethod = as<FuncDecl>(stageImplementations[int(selectedStage)]);
    if (!invokeMethod)
    {
        sink->diagnose(Diagnostics::InternalCompilerError{.location = stageTypeDeclRef.getLoc()});
        return DeclRef<FuncDecl>();
    }

    *outInvokeMethod = invokeMethod;
    auto funcDecl = _createStructuralEntryPointDecl(linkage, module, stageTypeDeclRef.getDecl());
    return makeDeclRef(funcDecl);
}

enum class StructuralRayTracingRuntimeTypeKind
{
    None,
    Stage,
    StageInput,
    Metadata,
};

static StructuralRayTracingStageKind _getDirectStageInputKind(
    const StructuralRayTracingDeclRegistry& registry,
    Type* type)
{
    while (auto modifiedType = as<ModifiedType>(type))
        type = modifiedType->getBase();
    auto declRefType = as<DeclRefType>(type);
    auto typeDecl = declRefType ? declRefType->getDeclRef().as<AggTypeDecl>().getDecl() : nullptr;
    return registry.getStageInputKind(typeDecl);
}

static StructuralRayTracingRuntimeTypeKind _getInterfaceRuntimeTypeKind(
    const StructuralRayTracingDeclRegistry& registry,
    InterfaceDecl* interfaceDecl)
{
    if (registry.getStageKind(interfaceDecl) != StructuralRayTracingStageKind::Count)
        return StructuralRayTracingRuntimeTypeKind::Stage;
    if (registry.getMetadataKind(interfaceDecl) != StructuralRayTracingMetadataKind::Count)
        return StructuralRayTracingRuntimeTypeKind::Metadata;
    return StructuralRayTracingRuntimeTypeKind::None;
}

static StructuralRayTracingRuntimeTypeKind _getDirectStructuralRuntimeTypeKind(
    SemanticsVisitor* visitor,
    const StructuralRayTracingDeclRegistry& registry,
    Type* type)
{
    if (auto declRefType = as<DeclRefType>(type))
    {
        if (auto typeDecl = declRefType->getDeclRef().as<AggTypeDecl>())
        {
            visitor->ensureDecl(typeDecl, DeclCheckState::ReadyForConformances);
            auto kind =
                _getInterfaceRuntimeTypeKind(registry, as<InterfaceDecl>(typeDecl.getDecl()));
            if (kind != StructuralRayTracingRuntimeTypeKind::None)
                return kind;

            // A parameter can be checked before inheritance facets have been cached for its
            // concrete type. Inspect the declared bases as well so that the restriction does not
            // depend on declaration-checking order.
            for (auto inheritanceDecl :
                 typeDecl.getDecl()->getDirectMemberDeclsOfType<InheritanceDecl>())
            {
                visitor->ensureDecl(inheritanceDecl, DeclCheckState::CanUseBaseOfInheritanceDecl);
                auto baseType = as<DeclRefType>(inheritanceDecl->base.type);
                auto baseInterface = baseType ? baseType->getDeclRef().as<InterfaceDecl>()
                                              : DeclRef<InterfaceDecl>();
                kind = _getInterfaceRuntimeTypeKind(registry, baseInterface.getDecl());
                if (kind != StructuralRayTracingRuntimeTypeKind::None)
                    return kind;
            }
        }
    }

    for (auto facet : visitor->getShared()->getInheritanceInfo(type).facets)
    {
        auto interfaceDeclRef = facet->origin.declRef.as<InterfaceDecl>();
        auto kind = _getInterfaceRuntimeTypeKind(registry, interfaceDeclRef.getDecl());
        if (kind != StructuralRayTracingRuntimeTypeKind::None)
            return kind;
    }
    return StructuralRayTracingRuntimeTypeKind::None;
}

static StructuralRayTracingRuntimeTypeKind _findStructuralRuntimeType(
    SemanticsVisitor* visitor,
    Type* type,
    HashSet<Decl*>& seenDecls)
{
    if (!type || as<ErrorType>(type))
        return StructuralRayTracingRuntimeTypeKind::None;

    while (auto modifiedType = as<ModifiedType>(type))
        type = modifiedType->getBase();

    auto& registry = visitor->getLinkage()->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return StructuralRayTracingRuntimeTypeKind::None;
    if (_getDirectStageInputKind(registry, type) != StructuralRayTracingStageKind::Count)
        return StructuralRayTracingRuntimeTypeKind::StageInput;
    auto directKind = _getDirectStructuralRuntimeTypeKind(visitor, registry, type);
    if (directKind != StructuralRayTracingRuntimeTypeKind::None)
        return directKind;

    if (auto structType = as<DeclRefType>(type))
    {
        if (auto structDecl = structType->getDeclRef().as<StructDecl>().getDecl())
        {
            if (!seenDecls.add(structDecl))
                return StructuralRayTracingRuntimeTypeKind::None;
            for (auto field : structDecl->getFields())
            {
                visitor->ensureDecl(field, DeclCheckState::CanUseTypeOfValueDecl);
                auto fieldDeclRef = visitor->getASTBuilder()
                                        ->getMemberDeclRef(structType->getDeclRef(), field)
                                        .as<VarDeclBase>();
                auto fieldType = fieldDeclRef ? getType(visitor->getASTBuilder(), fieldDeclRef)
                                              : field->type.type;
                auto kind = _findStructuralRuntimeType(visitor, fieldType, seenDecls);
                if (kind != StructuralRayTracingRuntimeTypeKind::None)
                    return kind;
            }
            seenDecls.remove(structDecl);
        }
    }

    if (auto arrayType = as<ArrayExpressionType>(type))
        return _findStructuralRuntimeType(visitor, arrayType->getElementType(), seenDecls);
    if (auto optionalType = as<OptionalType>(type))
        return _findStructuralRuntimeType(visitor, optionalType->getValueType(), seenDecls);
    if (auto pointerType = as<PtrTypeBase>(type))
        return _findStructuralRuntimeType(visitor, pointerType->getValueType(), seenDecls);
    if (auto tupleType = as<TupleType>(type))
    {
        for (Index i = 0; i < tupleType->getMemberCount(); ++i)
        {
            auto kind = _findStructuralRuntimeType(visitor, tupleType->getMember(i), seenDecls);
            if (kind != StructuralRayTracingRuntimeTypeKind::None)
                return kind;
        }
    }
    return StructuralRayTracingRuntimeTypeKind::None;
}

static StructuralRayTracingRuntimeTypeKind _findStructuralRuntimeType(
    SemanticsVisitor* visitor,
    Type* type)
{
    if (!type || as<ErrorType>(type))
        return StructuralRayTracingRuntimeTypeKind::None;
    HashSet<Decl*> seenDecls;
    return _findStructuralRuntimeType(visitor, type, seenDecls);
}

static void _diagnoseInvalidStructuralRayTracingRuntimeType(
    SemanticsVisitor* visitor,
    StructuralRayTracingRuntimeTypeKind kind,
    Type* type,
    SourceLoc location)
{
    if (kind == StructuralRayTracingRuntimeTypeKind::Stage)
    {
        visitor->getSink()->diagnose(
            Diagnostics::StructuralRayTracingStageRuntimeValue{.type = type, .location = location});
    }
    else if (kind == StructuralRayTracingRuntimeTypeKind::StageInput)
    {
        visitor->getSink()->diagnose(
            Diagnostics::StructuralRayTracingInputStorage{.type = type, .location = location});
    }
    else if (kind == StructuralRayTracingRuntimeTypeKind::Metadata)
    {
        visitor->getSink()->diagnose(Diagnostics::StructuralRayTracingMetadataRuntimeValue{
            .type = type,
            .location = location});
    }
}

void SemanticsVisitor::diagnoseInvalidStructuralRayTracingVariableType(VarDeclBase* varDecl)
{
    auto type = varDecl->type.type;
    auto kind = _findStructuralRuntimeType(this, type);
    if (kind == StructuralRayTracingRuntimeTypeKind::None)
        return;

    if (kind == StructuralRayTracingRuntimeTypeKind::StageInput && as<ParamDecl>(varDecl) &&
        _getDirectStageInputKind(getLinkage()->getStructuralRayTracingDeclRegistry(), type) !=
            StructuralRayTracingStageKind::Count)
    {
        return;
    }

    _diagnoseInvalidStructuralRayTracingRuntimeType(this, kind, type, varDecl->loc);
}

void SemanticsVisitor::diagnoseInvalidStructuralRayTracingCallableResult(CallableDecl* callableDecl)
{
    if (as<ConstructorDecl>(callableDecl))
        return;
    auto type = callableDecl->returnType.type;
    auto kind = _findStructuralRuntimeType(this, type);
    if (kind == StructuralRayTracingRuntimeTypeKind::None)
        return;

    auto location =
        callableDecl->returnType.exp ? callableDecl->returnType.exp->loc : callableDecl->loc;
    _diagnoseInvalidStructuralRayTracingRuntimeType(this, kind, type, location);
}

void SemanticsVisitor::diagnoseInvalidStructuralRayTracingPropertyType(PropertyDecl* propertyDecl)
{
    auto type = propertyDecl->type.type;
    _diagnoseInvalidStructuralRayTracingRuntimeType(
        this,
        _findStructuralRuntimeType(this, type),
        type,
        propertyDecl->type.exp ? propertyDecl->type.exp->loc : propertyDecl->loc);
}

bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingConstruction(InvokeExpr* invoke)
{
    auto typeType = as<TypeType>(invoke->functionExpr->type);
    if (!typeType)
        return false;
    auto type = typeType->getType();
    if (_findStructuralRuntimeType(this, type) == StructuralRayTracingRuntimeTypeKind::None)
        return false;

    getSink()->diagnose(Diagnostics::StructuralRayTracingTypeConstruction{
        .type = type,
        .location = invoke->functionExpr->loc});
    return true;
}

bool SemanticsVisitor::diagnoseDirectStructuralRayTracingStageInvoke(
    InvokeExpr* invoke,
    FunctionDeclBase* functionDecl)
{
    auto& registry = getLinkage()->getStructuralRayTracingDeclRegistry();
    auto stageKind = registry.getStageKind(functionDecl);
    if (stageKind == StructuralRayTracingStageKind::Count)
        stageKind = _findStageImplementationFromParentConformance(registry, functionDecl);
    if (stageKind == StructuralRayTracingStageKind::Count)
        return false;

    getSink()->diagnose(
        Diagnostics::DirectStructuralRayTracingStageInvoke{.location = invoke->functionExpr->loc});
    return true;
}

} // namespace Slang
