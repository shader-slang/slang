#include "slang-ir-structural-ray-tracing.h"

#include "slang-diagnostics.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
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
    case kIROp_StructuralRayTracingTrace:
    case kIROp_StructuralRayTracingCallShader:
    case kIROp_MetalStructuralRayTracingTrace:
    case kIROp_MetalStructuralRayTracingCallShader:
    case kIROp_MetalStructuralRayTracingDispatchRaysIndex:
    case kIROp_MetalStructuralRayTracingDispatchRaysDimensions:
    case kIROp_StructuralRayTracingEntryPointInfoDecoration:
    case kIROp_StructuralRayTracingProgramPayloadLocationDecoration:
    case kIROp_StructuralRayTracingSemanticallyEmptyPayloadDecoration:
    case kIROp_StructuralRayTracingMetalPayloadMetadataDecoration:
    case kIROp_StructuralRayTracingOpenSectionDecoration:
    case kIROp_StructuralRayTracingTaggedConformanceDecoration:
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

struct _StructuralRayTracingLinkedEntryCandidate
{
    IRDecoration* entryInfo = nullptr;
    IRStringLit* sourceTypeName = nullptr;
    IRStringLit* typeIdentity = nullptr;
};

static const char* _getStructuralRayTracingSectionDiagnosticName(
    StructuralRayTracingSectionKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        return "hit-group";
    case StructuralRayTracingSectionKind::MissShaders:
        return "miss-shader";
    case StructuralRayTracingSectionKind::CallableShaders:
        return "callable-shader";
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
}

static const char* _getStructuralRayTracingSectionEntryInterfaceName(
    StructuralRayTracingSectionKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        return "IHitGroup";
    case StructuralRayTracingSectionKind::MissShaders:
        return "IMissShader";
    case StructuralRayTracingSectionKind::CallableShaders:
        return "ICallableShader";
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
}

static void _collectStructuralRayTracingOpenSections(
    IRInst* root,
    List<IRStructuralRayTracingOpenSectionDecoration*>& outSections)
{
    for (auto inst = root->getFirstDecorationOrChild(); inst; inst = inst->getNextInst())
    {
        if (auto section = as<IRStructuralRayTracingOpenSectionDecoration>(inst))
            outSections.add(section);
        _collectStructuralRayTracingOpenSections(inst, outSections);
    }
}

static IRDecoration* _findStructuralRayTracingEntryInfo(
    IRWitnessTable* witnessTable,
    StructuralRayTracingSectionKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        return witnessTable->findDecoration<IRStructuralRayTracingHitGroupInfoDecoration>();
    case StructuralRayTracingSectionKind::MissShaders:
        return witnessTable->findDecoration<IRStructuralRayTracingMissShaderInfoDecoration>();
    case StructuralRayTracingSectionKind::CallableShaders:
        return witnessTable->findDecoration<IRStructuralRayTracingCallableShaderInfoDecoration>();
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
}

static _StructuralRayTracingLinkedEntryCandidate _getStructuralRayTracingLinkedEntryCandidate(
    IRWitnessTable* witnessTable,
    StructuralRayTracingSectionKind kind)
{
    _StructuralRayTracingLinkedEntryCandidate result;
    result.entryInfo = _findStructuralRayTracingEntryInfo(witnessTable, kind);
    SLANG_RELEASE_ASSERT(result.entryInfo);
    IRType* entryType = nullptr;

    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        {
            auto info = cast<IRStructuralRayTracingHitGroupInfoDecoration>(result.entryInfo);
            entryType = info->getGroupType();
            result.sourceTypeName = info->getGroupSourceTypeName();
            result.typeIdentity = info->getGroupTypeIdentity();
            SLANG_RELEASE_ASSERT(info->getIsLinked()->getValue());
            break;
        }
    case StructuralRayTracingSectionKind::MissShaders:
        {
            auto info = cast<IRStructuralRayTracingMissShaderInfoDecoration>(result.entryInfo);
            entryType = info->getMissType();
            result.sourceTypeName = info->getMissSourceTypeName();
            result.typeIdentity = info->getMissTypeIdentity();
            SLANG_RELEASE_ASSERT(info->getIsLinked()->getValue());
            break;
        }
    case StructuralRayTracingSectionKind::CallableShaders:
        {
            auto info = cast<IRStructuralRayTracingCallableShaderInfoDecoration>(result.entryInfo);
            entryType = info->getCallableType();
            result.sourceTypeName = info->getCallableSourceTypeName();
            result.typeIdentity = info->getCallableTypeIdentity();
            SLANG_RELEASE_ASSERT(info->getIsLinked()->getValue());
            break;
        }
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
    SLANG_RELEASE_ASSERT(
        entryType && result.sourceTypeName && result.typeIdentity &&
        result.sourceTypeName->getStringSlice().getLength() != 0 &&
        result.typeIdentity->getStringSlice().getLength() != 0);
    return result;
}

static IRStructuralRayTracingHitGroupInfoDecoration* _appendStructuralRayTracingLinkedHitGroup(
    IRBuilder& builder,
    IRInst* operation,
    IRStructuralRayTracingHitGroupInfoDecoration* source,
    Index functionIndex)
{
    // Copy by semantic field, not physical operand position. The only rewritten facts are the
    // program-local dense index and linked origin; payload location is assigned in the following
    // whole-program pass together with listed entries.
    IRInst* operands[] = {
        source->getGroupType(),
        source->getGroupSourceTypeName(),
        source->getGroupTypeIdentity(),
        builder.getIntValue(builder.getIntType(), functionIndex),
        source->getContextType(),
        source->getTraceContextType(),
        source->getPrimitiveType(),
        source->getPayloadType(),
        source->getPayloadSemanticType(),
        source->getRecordType(),
        source->getHitAttributesType(),
        source->getHitAttributesKind(),
        source->getClosestHitType(),
        source->getClosestHitSourceTypeName(),
        source->getClosestHitTypeIdentity(),
        source->getHasClosestHit(),
        source->getClosestHit(),
        source->getAnyHitType(),
        source->getAnyHitSourceTypeName(),
        source->getAnyHitTypeIdentity(),
        source->getHasAnyHit(),
        source->getAnyHit(),
        source->getIntersectionType(),
        source->getIntersectionSourceTypeName(),
        source->getIntersectionTypeIdentity(),
        source->getHasIntersection(),
        source->getIntersection(),
        builder.getBoolValue(true),
        source->getPayloadLocation(),
    };
    auto result = cast<IRStructuralRayTracingHitGroupInfoDecoration>(builder.addDecoration(
        operation,
        kIROp_StructuralRayTracingHitGroupInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands)));
    result->sourceLoc = source->sourceLoc;
    return result;
}

static IRStructuralRayTracingMissShaderInfoDecoration* _appendStructuralRayTracingLinkedMissShader(
    IRBuilder& builder,
    IRInst* operation,
    IRStructuralRayTracingMissShaderInfoDecoration* source,
    Index functionIndex)
{
    IRInst* operands[] = {
        builder.getIntValue(builder.getIntType(), functionIndex),
        source->getContextType(),
        source->getTraceContextType(),
        source->getPayloadType(),
        source->getPayloadSemanticType(),
        source->getRecordType(),
        source->getMissType(),
        source->getMissSourceTypeName(),
        source->getMissTypeIdentity(),
        source->getMiss(),
        builder.getBoolValue(true),
        source->getPayloadLocation(),
    };
    auto result = cast<IRStructuralRayTracingMissShaderInfoDecoration>(builder.addDecoration(
        operation,
        kIROp_StructuralRayTracingMissShaderInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands)));
    result->sourceLoc = source->sourceLoc;
    return result;
}

static IRStructuralRayTracingCallableShaderInfoDecoration*
_appendStructuralRayTracingLinkedCallableShader(
    IRBuilder& builder,
    IRInst* operation,
    IRStructuralRayTracingCallableShaderInfoDecoration* source,
    Index functionIndex)
{
    IRInst* operands[] = {
        builder.getIntValue(builder.getIntType(), functionIndex),
        source->getContextType(),
        source->getTraceContextType(),
        source->getCallableDataType(),
        source->getRecordType(),
        source->getCallableType(),
        source->getCallableSourceTypeName(),
        source->getCallableTypeIdentity(),
        source->getCallable(),
        builder.getBoolValue(true),
    };
    auto result = cast<IRStructuralRayTracingCallableShaderInfoDecoration>(builder.addDecoration(
        operation,
        kIROp_StructuralRayTracingCallableShaderInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands)));
    result->sourceLoc = source->sourceLoc;
    return result;
}

static void _collectStructuralRayTracingListedEntryState(
    IRInst* operation,
    StructuralRayTracingSectionKind kind,
    HashSet<UnownedStringSlice>& typeIdentities,
    Dictionary<IRType*, Index>& nextIndicesByPayload,
    Index& nextCallableIndex)
{
    for (auto decoration : operation->getDecorations())
    {
        switch (kind)
        {
        case StructuralRayTracingSectionKind::HitGroups:
            {
                auto info = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
                if (!info)
                    break;
                typeIdentities.add(info->getGroupTypeIdentity()->getStringSlice());
                auto& nextIndex =
                    nextIndicesByPayload.getOrAddValue(info->getPayloadSemanticType(), 0);
                nextIndex = Math::Max(nextIndex, Index(info->getFunctionIndex()->getValue() + 1));
                break;
            }
        case StructuralRayTracingSectionKind::MissShaders:
            {
                auto info = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration);
                if (!info)
                    break;
                typeIdentities.add(info->getMissTypeIdentity()->getStringSlice());
                auto& nextIndex =
                    nextIndicesByPayload.getOrAddValue(info->getPayloadSemanticType(), 0);
                nextIndex = Math::Max(nextIndex, Index(info->getFunctionIndex()->getValue() + 1));
                break;
            }
        case StructuralRayTracingSectionKind::CallableShaders:
            {
                auto info = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration);
                if (!info)
                    break;
                typeIdentities.add(info->getCallableTypeIdentity()->getStringSlice());
                nextCallableIndex =
                    Math::Max(nextCallableIndex, Index(info->getFunctionIndex()->getValue() + 1));
                break;
            }
        default:
            SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
        }
    }
}

bool completeOpenStructuralRayTracingSchemas(IRModule* module, DiagnosticSink* sink)
{
    List<IRStructuralRayTracingOpenSectionDecoration*> openSections;
    _collectStructuralRayTracingOpenSections(module->getModuleInst(), openSections);
    if (openSections.getCount() == 0)
        return true;

    // Selected tables are ordinary global witness tables. The linker has already filtered the
    // input-module conformance index by exact requested tag, so this lexical global enumeration is
    // the canonical linked set; it neither follows arbitrary operand graphs nor scans names.
    List<IRWitnessTable*> taggedWitnessTables;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto table = as<IRWitnessTable>(inst))
        {
            if (table->findDecoration<IRStructuralRayTracingTaggedConformanceDecoration>())
                taggedWitnessTables.add(table);
        }
    }

    Dictionary<IRType*, UInt> diagnosedInvalidTagKinds;
    bool isValid = true;
    IRBuilder builder(module);
    for (auto request : openSections)
    {
        auto operation = request->getParent();
        SLANG_RELEASE_ASSERT(
            as<IRStructuralRayTracingTrace>(operation) ||
            as<IRStructuralRayTracingCallShader>(operation));
        auto kindValue = request->getSectionKind()->getValue();
        SLANG_RELEASE_ASSERT(
            kindValue >= 0 && kindValue < IRIntegerValue(StructuralRayTracingSectionKind::Count));
        auto kind = StructuralRayTracingSectionKind(kindValue);

        if (!request->getIsValidTag()->getValue())
        {
            auto kindBit = UInt(1) << UInt(kind);
            auto& diagnosedKinds = diagnosedInvalidTagKinds.getOrAddValue(request->getTagType(), 0);
            if ((diagnosedKinds & kindBit) == 0)
            {
                diagnosedKinds |= kindBit;
                sink->diagnose(Diagnostics::StructuralRayTracingOpenTagNotEntryInterface{
                    .section = _getStructuralRayTracingSectionDiagnosticName(kind),
                    .tag = request->getTagType(),
                    .entryInterface = _getStructuralRayTracingSectionEntryInterfaceName(kind),
                    .location = operation->sourceLoc});
            }
            isValid = false;
            request->removeAndDeallocate();
            continue;
        }

        List<_StructuralRayTracingLinkedEntryCandidate> candidates;
        for (auto table : taggedWitnessTables)
        {
            for (auto decoration : table->getDecorations())
            {
                auto tagged = as<IRStructuralRayTracingTaggedConformanceDecoration>(decoration);
                if (!tagged || tagged->getSectionKind()->getValue() != kindValue ||
                    tagged->getTagType() != request->getTagType())
                {
                    continue;
                }
                candidates.add(_getStructuralRayTracingLinkedEntryCandidate(table, kind));
            }
        }
        candidates.sort(
            [](const _StructuralRayTracingLinkedEntryCandidate& left,
               const _StructuralRayTracingLinkedEntryCandidate& right)
            {
                auto sourceNameOrder = compare(
                    left.sourceTypeName->getStringSlice(),
                    right.sourceTypeName->getStringSlice());
                if (sourceNameOrder != 0)
                    return sourceNameOrder < 0;
                // Qualified names are the specified public order. The opaque canonical identity
                // is only an injective tie-breaker for same-spelled declarations from distinct
                // modules or generic specializations; it is never parsed to recover semantics.
                return compare(
                           left.typeIdentity->getStringSlice(),
                           right.typeIdentity->getStringSlice()) < 0;
            });

        HashSet<UnownedStringSlice> typeIdentities;
        Dictionary<IRType*, Index> nextIndicesByPayload;
        Index nextCallableIndex = 0;
        _collectStructuralRayTracingListedEntryState(
            operation,
            kind,
            typeIdentities,
            nextIndicesByPayload,
            nextCallableIndex);

        builder.setInsertInto(operation);
        for (auto candidate : candidates)
        {
            if (!typeIdentities.add(candidate.typeIdentity->getStringSlice()))
                continue;

            switch (kind)
            {
            case StructuralRayTracingSectionKind::HitGroups:
                {
                    auto source =
                        cast<IRStructuralRayTracingHitGroupInfoDecoration>(candidate.entryInfo);
                    auto& nextIndex =
                        nextIndicesByPayload.getOrAddValue(source->getPayloadSemanticType(), 0);
                    _appendStructuralRayTracingLinkedHitGroup(
                        builder,
                        operation,
                        source,
                        nextIndex++);
                    break;
                }
            case StructuralRayTracingSectionKind::MissShaders:
                {
                    auto source =
                        cast<IRStructuralRayTracingMissShaderInfoDecoration>(candidate.entryInfo);
                    auto& nextIndex =
                        nextIndicesByPayload.getOrAddValue(source->getPayloadSemanticType(), 0);
                    _appendStructuralRayTracingLinkedMissShader(
                        builder,
                        operation,
                        source,
                        nextIndex++);
                    break;
                }
            case StructuralRayTracingSectionKind::CallableShaders:
                _appendStructuralRayTracingLinkedCallableShader(
                    builder,
                    operation,
                    cast<IRStructuralRayTracingCallableShaderInfoDecoration>(candidate.entryInfo),
                    nextCallableIndex++);
                break;
            default:
                SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
            }
        }

        // The request marker has served its only liveness purpose. The complete operation metadata
        // now roots every selected stage directly, so later per-entry linking and DCE need not
        // retain or rediscover the contributing witness tables.
        request->removeAndDeallocate();
    }
    return isValid;
}

} // namespace Slang
