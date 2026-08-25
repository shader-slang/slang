#include "slang-ir-structural-ray-tracing.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-mangle.h"
#include "slang-module.h"

namespace Slang
{

IROp getStructuralRayTracingStageInterfaceOp(StructuralRayTracingStageKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        return kIROp_ClosestHitStageInterface;
    case StructuralRayTracingStageKind::AnyHit:
        return kIROp_AnyHitStageInterface;
    case StructuralRayTracingStageKind::Intersection:
        return kIROp_IntersectionStageInterface;
    case StructuralRayTracingStageKind::Miss:
        return kIROp_MissStageInterface;
    case StructuralRayTracingStageKind::Callable:
        return kIROp_CallableStageInterface;
    default:
        return kIROp_Invalid;
    }
}

IROp getStructuralRayTracingStageInputOperationOp(StructuralRayTracingStageInputOperationKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingStageInputOperationKind::Payload:
        return kIROp_StructuralRayTracingGetPayload;
    case StructuralRayTracingStageInputOperationKind::CallableData:
        return kIROp_StructuralRayTracingGetCallableData;
    case StructuralRayTracingStageInputOperationKind::HitAttributes:
        return kIROp_StructuralRayTracingGetHitAttributes;
    case StructuralRayTracingStageInputOperationKind::TriangleBarycentricCoord:
        return kIROp_StructuralRayTracingGetTriangleBarycentricCoord;
    case StructuralRayTracingStageInputOperationKind::TriangleFrontFacing:
        return kIROp_StructuralRayTracingGetTriangleFrontFacing;
    case StructuralRayTracingStageInputOperationKind::CurveParameter:
        return kIROp_StructuralRayTracingGetCurveParameter;
    case StructuralRayTracingStageInputOperationKind::RayTCurrent:
        return kIROp_StructuralRayTracingGetRayTCurrent;
    case StructuralRayTracingStageInputOperationKind::HitKind:
        return kIROp_StructuralRayTracingGetHitKind;
    case StructuralRayTracingStageInputOperationKind::WorldRayOrigin:
        return kIROp_StructuralRayTracingGetWorldRayOrigin;
    case StructuralRayTracingStageInputOperationKind::WorldRayDirection:
        return kIROp_StructuralRayTracingGetWorldRayDirection;
    case StructuralRayTracingStageInputOperationKind::ObjectSpaceRay:
        return kIROp_StructuralRayTracingGetObjectSpaceRay;
    case StructuralRayTracingStageInputOperationKind::PrimitiveIndex:
        return kIROp_StructuralRayTracingGetPrimitiveIndex;
    case StructuralRayTracingStageInputOperationKind::GeometryIndex:
        return kIROp_StructuralRayTracingGetGeometryIndex;
    case StructuralRayTracingStageInputOperationKind::IgnoreHit:
        return kIROp_StructuralRayTracingIgnoreHit;
    case StructuralRayTracingStageInputOperationKind::AcceptHitAndEndSearch:
        return kIROp_StructuralRayTracingAcceptHitAndEndSearch;
    case StructuralRayTracingStageInputOperationKind::ReportHit:
        return kIROp_StructuralRayTracingReportHit;
    case StructuralRayTracingStageInputOperationKind::ReportHitWithKind:
        return kIROp_StructuralRayTracingReportHitWithKind;
    default:
        return kIROp_Invalid;
    }
}

static IRInterfaceType* _findInterfaceType(IRInst* inst)
{
    if (auto generic = as<IRGeneric>(inst))
        inst = findInnerMostGenericReturnVal(generic);
    return as<IRInterfaceType>(inst);
}

static IRInterfaceType* _findInterfaceTypeByNameHint(IRInst* inst, UnownedStringSlice expectedName)
{
    if (auto interfaceType = as<IRInterfaceType>(inst))
    {
        if (auto nameHint = interfaceType->findDecoration<IRNameHintDecoration>())
        {
            if (nameHint->getName() == expectedName)
                return interfaceType;
        }
    }
    for (auto child : inst->getDecorationsAndChildren())
    {
        if (auto result = _findInterfaceTypeByNameHint(child, expectedName))
            return result;
    }
    return nullptr;
}

bool identifyStructuralRayTracingStageInterfaces(
    Module* module,
    const StructuralRayTracingDeclRegistry& registry,
    StructuralRayTracingStageKind* outMissingStage)
{
    auto irModule = module->getIRModule();
    auto astBuilder = module->getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);

    for (int i = 0; i < int(StructuralRayTracingStageKind::Count); ++i)
    {
        auto kind = StructuralRayTracingStageKind(i);
        auto interfaceDecl = registry.getStageInterface(kind);
        auto mangledName = getMangledName(astBuilder, interfaceDecl);
        auto symbols = irModule->findSymbolByMangledName(ImmutableHashedString(mangledName));
        auto expectedOp = getStructuralRayTracingStageInterfaceOp(kind);
        bool found = false;

        for (auto symbol : symbols)
        {
            auto interfaceType = _findInterfaceType(symbol);
            if (!interfaceType)
                continue;
            if (interfaceType->getOp() != kIROp_InterfaceType &&
                interfaceType->getOp() != expectedOp)
            {
                continue;
            }

            // All stage-interface ops have the same storage and operand layout as
            // IRInterfaceType. The trusted-module load is the point where the ordinary
            // serialized interface receives its compiler-owned nominal identity.
            interfaceType->m_op = expectedOp;
            found = true;
        }

        if (!found)
        {
            StringBuilder expectedName;
            expectedName << "rt." << getStructuralRayTracingStageInterfaceName(kind);
            if (auto interfaceType = _findInterfaceTypeByNameHint(
                    irModule->getModuleInst(),
                    expectedName.getUnownedSlice()))
            {
                if (interfaceType->getOp() == kIROp_InterfaceType ||
                    interfaceType->getOp() == expectedOp)
                {
                    interfaceType->m_op = expectedOp;
                    found = true;
                }
            }
        }

        if (!found)
        {
            if (outMissingStage)
                *outMissingStage = kind;
            return false;
        }
    }
    return true;
}

} // namespace Slang
