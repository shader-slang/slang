#include "slang-reflection-structural-ray-tracing.h"

#include "slang-check-impl.h"
#include "slang-linkable.h"
#include "slang-type-layout.h"

namespace Slang
{

static String _getStageEntryPointName(Type* stageType)
{
    auto declRefType = as<DeclRefType>(stageType ? stageType->resolve() : nullptr);
    auto decl = declRefType ? declRefType->getDeclRef().getDecl() : nullptr;
    auto name = decl ? decl->getName() : nullptr;
    return name ? String(name->text) : String();
}

static RefPtr<StructuralRayTracingStageReflection> _createStageReflection(
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    SubtypeWitness* groupWitness,
    StructuralRayTracingAssociatedTypeKind associatedTypeKind,
    StructuralRayTracingStageKind stageKind)
{
    auto stageType = registry.resolveAssociatedType(astBuilder, groupWitness, associatedTypeKind);
    if (!stageType || registry.isStagePlaceholder(stageKind, stageType))
        return nullptr;

    RefPtr<StructuralRayTracingStageReflection> result = new StructuralRayTracingStageReflection();
    result->stageKind = stageKind;
    result->type = stageType;
    result->entryPointName = _getStageEntryPointName(stageType);
    return result;
}

static bool _addHitGroups(
    StructuralRayTracingProgramLayoutReflection* result,
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* groupListType)
{
    auto pack = getStructuralRayTracingGroupPack(astBuilder, groupListType);
    if (pack.types->getTypeCount() != 0 && !pack.witnesses)
        return false;

    for (Index i = 0; i < pack.types->getTypeCount(); ++i)
    {
        auto groupType = pack.types->getElementType(i);
        auto groupWitness = pack.witnesses->getWitness(i);
        auto slotType = registry.resolveAssociatedType(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::HitGroupSlot);
        auto slotWitness = registry.resolveAssociatedTypeConstraint(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::HitGroupSlot);
        auto contextType = registry.resolveAssociatedType(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::HitGroupContext);
        auto contextWitness = registry.resolveAssociatedTypeConstraint(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::HitGroupContext);
        int64_t slot = 0;
        if (!slotType || !slotWitness || !contextType || !contextWitness ||
            !registry.tryGetShaderGroupSlotIndex(astBuilder, slotWitness, slot))
        {
            return false;
        }

        RefPtr<StructuralRayTracingHitGroupReflection> group =
            new StructuralRayTracingHitGroupReflection();
        group->slot = slot;
        group->groupType = groupType;
        group->contextType = contextType;
        group->recordType = registry.resolveAssociatedType(
            astBuilder,
            contextWitness,
            StructuralRayTracingAssociatedTypeKind::HitRecord);
        group->primitiveType = registry.resolveAssociatedType(
            astBuilder,
            contextWitness,
            StructuralRayTracingAssociatedTypeKind::HitPrimitive);
        auto primitiveWitness = registry.resolveAssociatedTypeConstraint(
            astBuilder,
            contextWitness,
            StructuralRayTracingAssociatedTypeKind::HitPrimitive);
        group->intersectionAttributesType = registry.resolveAssociatedType(
            astBuilder,
            primitiveWitness,
            StructuralRayTracingAssociatedTypeKind::PrimitiveAttributes);
        group->closestHit = _createStageReflection(
            astBuilder,
            registry,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::HitGroupClosestHit,
            StructuralRayTracingStageKind::ClosestHit);
        group->anyHit = _createStageReflection(
            astBuilder,
            registry,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::HitGroupAnyHit,
            StructuralRayTracingStageKind::AnyHit);
        group->intersection = _createStageReflection(
            astBuilder,
            registry,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::HitGroupIntersection,
            StructuralRayTracingStageKind::Intersection);
        if (!group->recordType || !group->primitiveType || !group->intersectionAttributesType)
            return false;
        result->hitGroups.add(group);
    }
    return true;
}

static bool _addMissGroups(
    StructuralRayTracingProgramLayoutReflection* result,
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* groupListType)
{
    auto pack = getStructuralRayTracingGroupPack(astBuilder, groupListType);
    if (pack.types->getTypeCount() != 0 && !pack.witnesses)
        return false;

    for (Index i = 0; i < pack.types->getTypeCount(); ++i)
    {
        auto groupType = pack.types->getElementType(i);
        auto groupWitness = pack.witnesses->getWitness(i);
        auto slotType = registry.resolveAssociatedType(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::MissGroupSlot);
        auto slotWitness = registry.resolveAssociatedTypeConstraint(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::MissGroupSlot);
        auto contextType = registry.resolveAssociatedType(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::MissGroupContext);
        auto contextWitness = registry.resolveAssociatedTypeConstraint(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::MissGroupContext);
        int64_t slot = 0;
        if (!slotType || !slotWitness || !contextType || !contextWitness ||
            !registry.tryGetShaderGroupSlotIndex(astBuilder, slotWitness, slot))
        {
            return false;
        }

        RefPtr<StructuralRayTracingMissGroupReflection> group =
            new StructuralRayTracingMissGroupReflection();
        group->slot = slot;
        group->groupType = groupType;
        group->contextType = contextType;
        group->recordType = registry.resolveAssociatedType(
            astBuilder,
            contextWitness,
            StructuralRayTracingAssociatedTypeKind::MissRecord);
        group->miss = _createStageReflection(
            astBuilder,
            registry,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::MissGroupMiss,
            StructuralRayTracingStageKind::Miss);
        if (!group->recordType || !group->miss)
            return false;
        result->missGroups.add(group);
    }
    return true;
}

static bool _addCallableGroups(
    StructuralRayTracingProgramLayoutReflection* result,
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* groupListType)
{
    auto pack = getStructuralRayTracingGroupPack(astBuilder, groupListType);
    if (pack.types->getTypeCount() != 0 && !pack.witnesses)
        return false;

    for (Index i = 0; i < pack.types->getTypeCount(); ++i)
    {
        auto groupType = pack.types->getElementType(i);
        auto groupWitness = pack.witnesses->getWitness(i);
        auto slotType = registry.resolveAssociatedType(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::CallableGroupSlot);
        auto slotWitness = registry.resolveAssociatedTypeConstraint(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::CallableGroupSlot);
        auto contextType = registry.resolveAssociatedType(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::CallableGroupContext);
        auto contextWitness = registry.resolveAssociatedTypeConstraint(
            astBuilder,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::CallableGroupContext);
        int64_t slot = 0;
        if (!slotType || !slotWitness || !contextType || !contextWitness ||
            !registry.tryGetShaderGroupSlotIndex(astBuilder, slotWitness, slot))
        {
            return false;
        }

        RefPtr<StructuralRayTracingCallableGroupReflection> group =
            new StructuralRayTracingCallableGroupReflection();
        group->slot = slot;
        group->groupType = groupType;
        group->contextType = contextType;
        group->recordType = registry.resolveAssociatedType(
            astBuilder,
            contextWitness,
            StructuralRayTracingAssociatedTypeKind::CallableRecord);
        group->callableDataType = registry.resolveAssociatedType(
            astBuilder,
            contextWitness,
            StructuralRayTracingAssociatedTypeKind::CallableData);
        group->callable = _createStageReflection(
            astBuilder,
            registry,
            groupWitness,
            StructuralRayTracingAssociatedTypeKind::CallableGroupCallable,
            StructuralRayTracingStageKind::Callable);
        if (!group->recordType || !group->callableDataType || !group->callable)
            return false;
        result->callableGroups.add(group);
    }
    return true;
}

StructuralRayTracingProgramLayoutReflection* findStructuralRayTracingProgramLayoutReflection(
    ProgramLayout* programLayout,
    const char* name)
{
    if (!programLayout || !name)
        return nullptr;

    auto program = programLayout->getProgram();
    auto linkage = program->getLinkage();
    auto& registry = linkage->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return nullptr;

    DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
    Type* layoutType = nullptr;
    try
    {
        layoutType = program->getTypeFromString(name, &sink);
    }
    catch (...)
    {
        return nullptr;
    }
    layoutType = layoutType ? as<Type>(layoutType->resolve()) : nullptr;
    if (!layoutType || as<ErrorType>(layoutType))
        return nullptr;

    auto reflectionData = as<StructuralRayTracingReflectionData>(
        programLayout->structuralRayTracingReflectionData.Ptr());
    if (!reflectionData)
    {
        reflectionData = new StructuralRayTracingReflectionData();
        programLayout->structuralRayTracingReflectionData = RefPtr<RefObject>(reflectionData);
    }
    for (auto existing : reflectionData->programLayouts)
    {
        if (existing->layoutType == layoutType)
            return existing;
    }

    auto astBuilder = linkage->getASTBuilder();
    auto layoutInterface =
        registry.getMetadataInterface(StructuralRayTracingMetadataKind::TraceProgramLayout);
    auto layoutInterfaceType =
        layoutInterface ? DeclRefType::create(astBuilder, makeDeclRef(layoutInterface)) : nullptr;
    if (!layoutInterfaceType)
        return nullptr;

    SemanticsContext semanticsContext(linkage->getSemanticsForReflection());
    SemanticsVisitor visitor(semanticsContext);
    auto layoutWitness = visitor.isSubtype(layoutType, layoutInterfaceType, IsSubTypeOptions::None);
    if (!layoutWitness)
        return nullptr;

    auto traceContextType = registry.resolveAssociatedType(
        astBuilder,
        layoutWitness,
        StructuralRayTracingAssociatedTypeKind::ProgramTraceContext);
    auto hitGroupsType = registry.resolveAssociatedType(
        astBuilder,
        layoutWitness,
        StructuralRayTracingAssociatedTypeKind::ProgramHitGroups);
    auto missGroupsType = registry.resolveAssociatedType(
        astBuilder,
        layoutWitness,
        StructuralRayTracingAssociatedTypeKind::ProgramMissGroups);
    auto callableGroupsType = registry.resolveAssociatedType(
        astBuilder,
        layoutWitness,
        StructuralRayTracingAssociatedTypeKind::ProgramCallableGroups);
    if (!traceContextType || !hitGroupsType || !missGroupsType || !callableGroupsType)
        return nullptr;

    RefPtr<StructuralRayTracingProgramLayoutReflection> result =
        new StructuralRayTracingProgramLayoutReflection();
    result->layoutType = layoutType;
    result->traceContextType = traceContextType;
    if (!_addHitGroups(result, astBuilder, registry, hitGroupsType) ||
        !_addMissGroups(result, astBuilder, registry, missGroupsType) ||
        !_addCallableGroups(result, astBuilder, registry, callableGroupsType))
    {
        return nullptr;
    }

    reflectionData->programLayouts.add(result);
    return result;
}

} // namespace Slang
