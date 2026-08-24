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
