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

void SemanticsVisitor::registerStructuralRayTracingStageConformance(
    DeclRef<InterfaceDecl> superInterfaceDeclRef,
    WitnessTable* witnessTable)
{
    auto& registry = getLinkage()->getStructuralRayTracingDeclRegistry();
    auto stageKind = registry.getStageKind(superInterfaceDeclRef.getDecl());
    if (stageKind == StructuralRayTracingStageKind::Count || !witnessTable)
        return;

    registry.registerStageImplementation(
        _getStageImplementation(registry, stageKind, witnessTable),
        stageKind);
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
