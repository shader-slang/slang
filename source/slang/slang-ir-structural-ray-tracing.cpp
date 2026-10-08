#include "slang-ir-structural-ray-tracing.h"

#include "slang-diagnostics.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-optix-ray-tracing-abi.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-mangle.h"
#include "slang-module.h"
#include "slang-target.h"

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
    case StructuralRayTracingStageInputOperationKind::Record:
        return kIROp_StructuralRayTracingGetRecord;
    case StructuralRayTracingStageInputOperationKind::HitAttributes:
        return kIROp_StructuralRayTracingGetHitAttributes;
    case StructuralRayTracingStageInputOperationKind::TriangleBarycentricCoord:
        return kIROp_StructuralRayTracingGetTriangleBarycentricCoord;
    case StructuralRayTracingStageInputOperationKind::TriangleFrontFacing:
        return kIROp_StructuralRayTracingGetTriangleFrontFacing;
    case StructuralRayTracingStageInputOperationKind::CurveParameter:
        return kIROp_StructuralRayTracingGetCurveParameter;
    case StructuralRayTracingStageInputOperationKind::RayTMin:
        return kIROp_StructuralRayTracingGetRayTMin;
    case StructuralRayTracingStageInputOperationKind::RayTCurrent:
        return kIROp_StructuralRayTracingGetRayTCurrent;
    case StructuralRayTracingStageInputOperationKind::RayTime:
        return kIROp_StructuralRayTracingGetRayTime;
    case StructuralRayTracingStageInputOperationKind::RayFlags:
        return kIROp_StructuralRayTracingGetRayFlags;
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
    case StructuralRayTracingStageInputOperationKind::InstanceIndex:
        return kIROp_StructuralRayTracingGetInstanceIndex;
    case StructuralRayTracingStageInputOperationKind::InstanceID:
        return kIROp_StructuralRayTracingGetInstanceID;
    case StructuralRayTracingStageInputOperationKind::InstanceCount:
        return kIROp_StructuralRayTracingGetInstanceCount;
    case StructuralRayTracingStageInputOperationKind::InstanceIndexAtLevel:
        return kIROp_StructuralRayTracingGetInstanceIndexAtLevel;
    case StructuralRayTracingStageInputOperationKind::InstanceIDAtLevel:
        return kIROp_StructuralRayTracingGetInstanceIDAtLevel;
    case StructuralRayTracingStageInputOperationKind::ObjectToWorld:
        return kIROp_StructuralRayTracingGetObjectToWorld;
    case StructuralRayTracingStageInputOperationKind::WorldToObject:
        return kIROp_StructuralRayTracingGetWorldToObject;
    case StructuralRayTracingStageInputOperationKind::DispatchRaysIndex:
        return kIROp_StructuralRayTracingGetDispatchRaysIndex;
    case StructuralRayTracingStageInputOperationKind::DispatchRaysDimensions:
        return kIROp_StructuralRayTracingGetDispatchRaysDimensions;
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

bool isCompilerOwnedStructuralRayTracingIROp(IROp op)
{
    if ((op >= kIROp_FirstRaytracingStageInterface && op <= kIROp_LastRaytracingStageInterface) ||
        (op >= kIROp_FirstStructuralRayTracingStageInputOperation &&
         op <= kIROp_LastStructuralRayTracingStageInputOperation))
    {
        return true;
    }

    switch (op)
    {
    case kIROp_TraceProgramDescriptorType:
    case kIROp_StructuralRayTracingTrace:
    case kIROp_StructuralRayTracingCallShader:
    case kIROp_StructuralRayTracingProgramSchema:
    case kIROp_StructuralRayTracingLegacyAPIUseDecoration:
    case kIROp_MetalStructuralRayTracingTrace:
    case kIROp_MetalStructuralRayTracingCallShader:
    case kIROp_MetalStructuralRayTracingDispatchRaysIndex:
    case kIROp_MetalStructuralRayTracingDispatchRaysDimensions:
    case kIROp_StructuralRayTracingEntryPointInfoDecoration:
    case kIROp_StructuralRayTracingProgramPayloadLocationDecoration:
    case kIROp_StructuralRayTracingSemanticallyEmptyPayloadDecoration:
    case kIROp_StructuralRayTracingMetalPayloadMetadataDecoration:
    case kIROp_StructuralRayTracingVulkanPayloadStorageDecoration:
    case kIROp_StructuralRayTracingHitGroupInfoDecoration:
    case kIROp_StructuralRayTracingMissShaderInfoDecoration:
    case kIROp_StructuralRayTracingCallableShaderInfoDecoration:
    case kIROp_MetalIntersectionFunctionTable:
    case kIROp_MetalVisibleFunctionTable:
    case kIROp_MetalVisibleFunctionDecoration:
    case kIROp_MetalIntersectionFunctionDecoration:
        return true;
    default:
        return false;
    }
}

static void _collectStructuralRayTracingProgramDescriptorTypes(
    IRInst* root,
    List<IRTraceProgramDescriptorType*>& types)
{
    if (auto type = as<IRTraceProgramDescriptorType>(root))
        types.add(type);
    for (auto child : root->getChildren())
        _collectStructuralRayTracingProgramDescriptorTypes(child, types);
}

void lowerStructuralRayTracingProgramDescriptorTypes(
    IRModule* module,
    const Dictionary<IRType*, IRType*>& targetTypesBySchema,
    IRType* fallbackType)
{
    // Collect first because `replaceUsesWith` updates and deduplicates hoistable users. Walking
    // those users while mutating them can otherwise skip another descriptor nested in a function,
    // tuple, pointer, or specialized user struct type.
    List<IRTraceProgramDescriptorType*> descriptorTypes;
    _collectStructuralRayTracingProgramDescriptorTypes(module->getModuleInst(), descriptorTypes);
    for (auto descriptorType : descriptorTypes)
    {
        IRType* replacement = fallbackType;
        if (auto targetType = targetTypesBySchema.tryGetValue(descriptorType->getSchema()))
            replacement = *targetType;
        if (!replacement)
            continue;
        descriptorType->replaceUsesWith(replacement);
        descriptorType->removeAndDeallocate();
    }

    descriptorTypes.clear();
    _collectStructuralRayTracingProgramDescriptorTypes(module->getModuleInst(), descriptorTypes);
    SLANG_RELEASE_ASSERT(!fallbackType || descriptorTypes.getCount() == 0);
}

struct ReachableRayTracingAPIUses
{
    SourceLoc structuralLocation;
    SourceLoc legacyLocation;
};

// Adds the source stages that a structural dispatch operation may invoke at runtime.
//
// Consider this example:
//
//     struct Schema : rt::ITraceProgramSchema
//     {
//         typealias MissShaders = rt::MissShaderList<ImportedMiss>;
//         ...
//     }
//
//     tracer.trace(desc, scene, program, payload);
//
// The trace instruction has no ordinary IR call to `ImportedMiss::invoke`. Schema lowering records
// that relationship as compiler-owned metadata because the runtime SBT selector decides which
// listed stage runs. Target adapter synthesis later turns the selected metadata into an executable
// call. Mixed-API validation must follow that same producer-owned edge before synthesis; otherwise
// a legacy call hidden behind `ImportedMiss` is absent from the apparent call graph.
//
// A trace can select hit and miss entries only from its payload partition, while a callable
// operation can select every entry in the schema-wide callable table. Keeping those two rules here
// avoids treating unrelated schema entries as reachable merely because their metadata was linked.
static void _addStructuralRayTracingDispatchCallees(IRInst* operation, List<IRFunc*>& outCallees)
{
    if (auto trace = as<IRStructuralRayTracingTrace>(operation))
    {
        auto payloadSemanticType = trace->getPayloadSemanticType();
        SLANG_RELEASE_ASSERT(payloadSemanticType);
        for (auto decoration : trace->getDecorations())
        {
            if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
            {
                if (group->getPayloadSemanticType() != payloadSemanticType)
                    continue;

                static const StructuralRayTracingStageKind kHitGroupStageKinds[] = {
                    StructuralRayTracingStageKind::ClosestHit,
                    StructuralRayTracingStageKind::AnyHit,
                    StructuralRayTracingStageKind::Intersection,
                };
                for (auto stageKind : kHitGroupStageKinds)
                {
                    if (auto invoke = getStructuralRayTracingHitGroupStageInvoke(group, stageKind))
                        outCallees.add(invoke);
                }
            }
            else if (auto miss = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
            {
                if (miss->getPayloadSemanticType() != payloadSemanticType)
                    continue;

                auto invoke = as<IRFunc>(miss->getMiss());
                SLANG_RELEASE_ASSERT(invoke);
                outCallees.add(invoke);
            }
        }
        return;
    }

    auto callShader = as<IRStructuralRayTracingCallShader>(operation);
    SLANG_RELEASE_ASSERT(callShader);
    for (auto decoration : callShader->getDecorations())
    {
        auto callable = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration);
        if (!callable)
            continue;

        auto invoke = as<IRFunc>(callable->getCallable());
        SLANG_RELEASE_ASSERT(invoke);
        outCallees.add(invoke);
    }
}

static void _collectReachableRayTracingAPIUses(
    IRFunc* function,
    HashSet<IRFunc*>& visitedFunctions,
    ReachableRayTracingAPIUses& uses)
{
    if (!function || !visitedFunctions.add(function))
        return;

    if (function->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
        uses.structuralLocation = function->sourceLoc;

    // Consider a separately compiled helper containing `TraceRay`, called by a ray-generation
    // entry point that also calls `RayTracer.trace`. Linking resolves the helper's call target but
    // does not restore its checked AST. Walking direct IR calls and the explicit structural
    // dispatch metadata from the selected entry point lets us observe both compiler-owned markers
    // without treating types or unrelated metadata as call-graph edges.
    List<IRFunc*> callees;
    for (auto block : function->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() == kIROp_StructuralRayTracingTrace ||
                inst->getOp() == kIROp_StructuralRayTracingCallShader)
            {
                uses.structuralLocation = inst->sourceLoc;
                _addStructuralRayTracingDispatchCallees(inst, callees);
            }

            auto call = as<IRCall>(inst);
            if (!call)
                continue;
            if (call->findDecoration<IRStructuralRayTracingLegacyAPIUseDecoration>())
                uses.legacyLocation = call->sourceLoc;
            if (auto callee = as<IRFunc>(getResolvedInstForDecorations(call->getCallee())))
                callees.add(callee);
        }
    }

    for (auto callee : callees)
        _collectReachableRayTracingAPIUses(callee, visitedFunctions, uses);
}

void diagnoseMixedRayTracingAPIsInReachableIR(
    IRModule* module,
    List<IRFunc*> const& entryPoints,
    DiagnosticSink* sink)
{
    SLANG_RELEASE_ASSERT(module && sink);
    for (auto entryPoint : entryPoints)
    {
        HashSet<IRFunc*> visitedFunctions;
        ReachableRayTracingAPIUses uses;
        _collectReachableRayTracingAPIUses(entryPoint, visitedFunctions, uses);
        if (uses.structuralLocation.isValid() && uses.legacyLocation.isValid())
        {
            sink->diagnose(Diagnostics::MixedRayTracingApisInReachableCode{
                .legacyLocation = uses.legacyLocation,
                .structuralLocation = uses.structuralLocation});
            return;
        }
    }
}

IRFunc* getStructuralRayTracingHitGroupStageInvoke(
    IRStructuralRayTracingHitGroupInfoDecoration* group,
    StructuralRayTracingStageKind stageKind)
{
    SLANG_RELEASE_ASSERT(group);

    IRType* stageType = nullptr;
    IRStringLit* sourceTypeName = nullptr;
    IRStringLit* typeIdentity = nullptr;
    IRBoolLit* isPresent = nullptr;
    IRInst* invoke = nullptr;
    switch (stageKind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        stageType = group->getClosestHitType();
        sourceTypeName = group->getClosestHitSourceTypeName();
        typeIdentity = group->getClosestHitTypeIdentity();
        isPresent = group->getHasClosestHit();
        invoke = group->getClosestHit();
        break;
    case StructuralRayTracingStageKind::AnyHit:
        stageType = group->getAnyHitType();
        sourceTypeName = group->getAnyHitSourceTypeName();
        typeIdentity = group->getAnyHitTypeIdentity();
        isPresent = group->getHasAnyHit();
        invoke = group->getAnyHit();
        break;
    case StructuralRayTracingStageKind::Intersection:
        stageType = group->getIntersectionType();
        sourceTypeName = group->getIntersectionSourceTypeName();
        typeIdentity = group->getIntersectionTypeIdentity();
        isPresent = group->getHasIntersection();
        invoke = group->getIntersection();
        break;
    default:
        SLANG_UNEXPECTED("hit group does not contain the requested structural stage");
    }

    SLANG_RELEASE_ASSERT(stageType && sourceTypeName && typeIdentity && isPresent && invoke);
    if (!isPresent->getValue())
    {
        SLANG_RELEASE_ASSERT(
            as<IRVoidType>(stageType) && sourceTypeName->getStringSlice().getLength() == 0 &&
            typeIdentity->getStringSlice().getLength() == 0 && as<IRVoidLit>(invoke));
        return nullptr;
    }

    auto func = as<IRFunc>(invoke);
    SLANG_RELEASE_ASSERT(
        func && !as<IRVoidType>(stageType) && sourceTypeName->getStringSlice().getLength() != 0 &&
        typeIdentity->getStringSlice().getLength() != 0);
    return func;
}

bool isSemanticallyEmptyStructuralRayTracingPayloadType(IRType* type)
{
    auto resolvedType = type ? getResolvedInstForDecorations(unwrapAttributedType(type)) : nullptr;
    return resolvedType &&
           resolvedType->findDecoration<IRStructuralRayTracingSemanticallyEmptyPayloadDecoration>();
}

Result getStructuralRayTracingNativePayloadSize(
    TargetRequest* targetRequest,
    IRBuilder* builder,
    IRType* payloadType,
    IRType* payloadSemanticType,
    IRIntegerValue* outSize)
{
    SLANG_RELEASE_ASSERT(targetRequest && builder && payloadType && payloadSemanticType && outSize);
    *outSize = 0;
    if (isSemanticallyEmptyStructuralRayTracingPayloadType(payloadSemanticType))
        return SLANG_OK;

    if (isCUDATarget(targetRequest))
    {
        OptiXRayTracingPayloadABIInfo abiInfo;
        SLANG_RETURN_ON_FAIL(getOptiXRayTracingPayloadABIInfo(builder, payloadType, &abiInfo));
        *outSize = abiInfo.registerCount * kOptiXRayTracingRegisterSize;
        return SLANG_OK;
    }
    if (isD3DTarget(targetRequest) || isKhronosTarget(targetRequest))
    {
        auto layoutRules = isD3DTarget(targetRequest)
                               ? IRTypeLayoutRules::getD3DRayTracingInterface()
                               : IRTypeLayoutRules::getNatural();
        IRSizeAndAlignment layout;
        SLANG_RETURN_ON_FAIL(getSizeAndAlignment(targetRequest, layoutRules, payloadType, &layout));
        if (layout.size == IRSizeAndAlignment::kIndeterminateSize)
            return SLANG_FAIL;
        // Vulkan defines ray payload and hit-attribute block sizes as if every member used scalar
        // alignment. Slang's natural IR layout is exactly that rule; `getStride()` supplies the
        // final block tail padding. D3D uses the dedicated rule above because DXIL allocation also
        // pads nested aggregates before placing the following field.
        *outSize = layout.getStride();
    }
    return SLANG_OK;
}

Result getStructuralRayTracingNativeHitAttributeSize(
    TargetRequest* targetRequest,
    IRBuilder* builder,
    IRType* attributesType,
    IRIntegerValue* outSize)
{
    SLANG_RELEASE_ASSERT(targetRequest && builder && attributesType && outSize);
    *outSize = 0;
    if (isCUDATarget(targetRequest))
    {
        IRIntegerValue registerCount = 0;
        SLANG_RETURN_ON_FAIL(
            getOptiXRayTracingHitAttributeRegisterCount(builder, attributesType, &registerCount));
        *outSize = registerCount * kOptiXRayTracingRegisterSize;
        return SLANG_OK;
    }
    if (isD3DTarget(targetRequest) || isKhronosTarget(targetRequest))
    {
        auto layoutRules = isD3DTarget(targetRequest)
                               ? IRTypeLayoutRules::getD3DRayTracingInterface()
                               : IRTypeLayoutRules::getNatural();
        IRSizeAndAlignment layout;
        SLANG_RETURN_ON_FAIL(
            getSizeAndAlignment(targetRequest, layoutRules, attributesType, &layout));
        if (layout.size == IRSizeAndAlignment::kIndeterminateSize)
            return SLANG_FAIL;
        *outSize = layout.getStride();
    }
    return SLANG_OK;
}

// Collect every concrete Vulkan ray-payload location in the lexical IR subtree. Payload
// decorations may belong to globals, entry-point parameters, or declarations inside generics, so
// a module-scope-only scan would miss valid legacy locations. The negative value `-1` asks later
// legalization to choose a location and therefore does not reserve one here.
void collectUsedVulkanRayPayloadLocations(IRInst* root, HashSet<IRIntegerValue>& outLocations)
{
    for (auto inst = root->getFirstDecorationOrChild(); inst; inst = inst->getNextInst())
    {
        if (as<IRVulkanRayPayloadDecoration>(inst) || as<IRVulkanRayPayloadInDecoration>(inst))
        {
            auto location = cast<IRIntLit>(inst->getOperand(0))->getValue();
            if (location >= 0)
                outLocations.add(location);
        }
        collectUsedVulkanRayPayloadLocations(inst, outLocations);
    }
}

void addStructuralRayTracingProgramPayloadLocation(
    IRBuilder& builder,
    IRInst* owner,
    IRType* payloadType,
    IRType* payloadSemanticType,
    IRIntegerValue location)
{
    SLANG_RELEASE_ASSERT(owner && payloadType && payloadSemanticType && location >= 0);
    for (auto decoration : owner->getDecorations())
    {
        auto existing = as<IRStructuralRayTracingProgramPayloadLocationDecoration>(decoration);
        if (!existing || existing->getPayloadSemanticType() != payloadSemanticType)
        {
            continue;
        }
        // One owner represents one linked program (or one schema operation in that program), so a
        // semantic payload identity has exactly one realized type and location at this target
        // phase. Distinct identities may intentionally use different locations even when their
        // physical layouts are identical.
        SLANG_RELEASE_ASSERT(existing->getLocation()->getValue() == location);
        SLANG_RELEASE_ASSERT(existing->getPayloadType() == payloadType);
        return;
    }

    IRInst* operands[] = {
        payloadType,
        payloadSemanticType,
        builder.getIntValue(builder.getIntType(), location),
    };
    builder.addDecoration(
        owner,
        kIROp_StructuralRayTracingProgramPayloadLocationDecoration,
        operands,
        SLANG_COUNT_OF(operands));
}

IRIntegerValue findStructuralRayTracingProgramPayloadLocation(
    IRInst* owner,
    IRType* payloadSemanticType)
{
    SLANG_RELEASE_ASSERT(owner && payloadSemanticType);
    IRIntegerValue result = -1;
    for (auto decoration : owner->getDecorations())
    {
        auto assignment = as<IRStructuralRayTracingProgramPayloadLocationDecoration>(decoration);
        if (!assignment || assignment->getPayloadSemanticType() != payloadSemanticType)
        {
            continue;
        }
        auto location = assignment->getLocation()->getValue();
        SLANG_RELEASE_ASSERT(result < 0 || result == location);
        result = location;
    }
    return result;
}

void addStructuralRayTracingEntryPointInfo(
    IRBuilder& builder,
    IRFunc* func,
    const StructuralRayTracingEntryPointIRInfo& info)
{
    SLANG_RELEASE_ASSERT(
        info.invoke && info.stageType && info.stageSourceTypeName && info.stageTypeIdentity);
    auto voidType = builder.getVoidType();
    IRInst* operands[] = {
        builder.getIntValue(builder.getIntType(), IRIntegerValue(info.stageKind)),
        info.invoke,
        info.stageType,
        info.stageSourceTypeName,
        info.stageTypeIdentity,
        info.contextType ? info.contextType : voidType,
        info.payloadType ? info.payloadType : voidType,
        info.payloadSemanticType ? info.payloadSemanticType : builder.getVoidType(),
        info.recordType ? info.recordType : voidType,
        info.hitAttributesType ? info.hitAttributesType : voidType,
        info.callableDataType ? info.callableDataType : voidType,
        builder.getIntValue(builder.getIntType(), IRIntegerValue(info.hitAttributesKind)),
        builder.getIntValue(builder.getIntType(), info.payloadLocation),
    };
    builder.addDecoration(
        func,
        kIROp_StructuralRayTracingEntryPointInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands));
}

static IRInterfaceType* _findInterfaceType(IRInst* inst)
{
    if (auto generic = as<IRGeneric>(inst))
        inst = findInnerMostGenericReturnVal(generic);
    return as<IRInterfaceType>(inst);
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
            if (outMissingStage)
                *outMissingStage = kind;
            return false;
        }
    }
    return true;
}

struct _StructuralRayTracingEmptyPayloadCandidate
{
    IRType* payloadType = nullptr;
    IRType* payloadSemanticType = nullptr;
};

static void _addStructuralRayTracingEmptyPayloadCandidate(
    _StructuralRayTracingEmptyPayloadCandidate candidate,
    _StructuralRayTracingEmptyPayloadCandidate& first,
    _StructuralRayTracingEmptyPayloadCandidate& second)
{
    SLANG_RELEASE_ASSERT(candidate.payloadType && candidate.payloadSemanticType);
    if (!isSemanticallyEmptyStructuralRayTracingPayloadType(candidate.payloadSemanticType))
        return;

    if (!first.payloadSemanticType)
    {
        first = candidate;
        return;
    }
    if (first.payloadSemanticType == candidate.payloadSemanticType)
    {
        // One semantic payload identity has exactly one target representation at this phase.
        // Hit and miss metadata may repeat that identity, but they must not reinterpret it.
        SLANG_RELEASE_ASSERT(first.payloadType == candidate.payloadType);
        return;
    }
    if (!second.payloadSemanticType)
        second = candidate;
}

// Returns the canonical schema type carried by each complete schema metadata owner.
//
// Trace and callable operations own the metadata needed by executable target lowering, while the
// program-schema root owns the same metadata for reflection-only requests. Each owner carries
// all explicitly listed entries, so the semantic schema operand is the only identity needed to
// deduplicate them.
static IRType* _getStructuralRayTracingSchemaOwnerType(IRInst* owner)
{
    if (auto trace = as<IRStructuralRayTracingTrace>(owner))
        return as<IRType>(trace->getProgramLayout());
    if (auto call = as<IRStructuralRayTracingCallShader>(owner))
        return as<IRType>(call->getProgramLayout());
    if (auto schema = as<IRStructuralRayTracingProgramSchema>(owner))
        return schema->getSchemaType();
    return nullptr;
}

static void _collectStructuralRayTracingSchemaOwners(IRInst* root, List<IRInst*>& owners)
{
    // Specialization materializes every executable operation outside its generic template. A
    // template can still contain dependent payload types, so it is not a finalized schema and
    // must not participate in this whole-program invariant.
    if (as<IRGeneric>(root))
        return;

    if (_getStructuralRayTracingSchemaOwnerType(root))
        owners.add(root);
    for (auto child = root->getFirstChild(); child; child = child->getNextInst())
        _collectStructuralRayTracingSchemaOwners(child, owners);
}

// Checks the empty-payload invariant on complete schema metadata rather than on one trace call.
//
// Consider a schema that serves `EmptyA`, `EmptyB`, and `RadiancePayload`. An explicit
// `trace<RadiancePayload>` is locally unambiguous, but reflection and a later payload-less trace
// still observe one schema with two incompatible implicit payload identities. Every schema owner
// contains its complete hit and miss metadata at this boundary, so checking once per canonical
// schema type catches the conflict for executable and reflection-only programs alike.
static bool _validateStructuralRayTracingSchemaEmptyPayloads(
    IRModule* module,
    DiagnosticSink* sink,
    HashSet<IRType*>& outAmbiguousSchemas)
{
    List<IRInst*> owners;
    _collectStructuralRayTracingSchemaOwners(module->getModuleInst(), owners);

    bool isValid = true;
    HashSet<IRType*> validatedSchemas;
    for (auto owner : owners)
    {
        auto schemaType = _getStructuralRayTracingSchemaOwnerType(owner);
        SLANG_RELEASE_ASSERT(schemaType);
        if (!validatedSchemas.add(schemaType))
            continue;

        _StructuralRayTracingEmptyPayloadCandidate first;
        _StructuralRayTracingEmptyPayloadCandidate second;
        for (auto decoration : owner->getDecorations())
        {
            if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
            {
                _addStructuralRayTracingEmptyPayloadCandidate(
                    {group->getPayloadType(), group->getPayloadSemanticType()},
                    first,
                    second);
            }
            else if (auto miss = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
            {
                _addStructuralRayTracingEmptyPayloadCandidate(
                    {miss->getPayloadType(), miss->getPayloadSemanticType()},
                    first,
                    second);
            }
        }

        if (!second.payloadSemanticType)
            continue;

        sink->diagnose(Diagnostics::StructuralRayTracingLinkedAmbiguousEmptyPayload{
            .schemaType = schemaType,
            .firstPayloadType = first.payloadSemanticType,
            .secondPayloadType = second.payloadSemanticType,
            .location = owner->sourceLoc});
        outAmbiguousSchemas.add(schemaType);
        isValid = false;
    }
    return isValid;
}

bool finalizeStructuralRayTracingSchemaPayloads(IRModule* module, DiagnosticSink* sink)
{
    HashSet<IRType*> ambiguousSchemas;
    return _validateStructuralRayTracingSchemaEmptyPayloads(module, sink, ambiguousSchemas);
}

} // namespace Slang
