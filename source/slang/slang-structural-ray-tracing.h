#pragma once

#include "compiler-core/slang-source-loc.h"
#include "core/slang-dictionary.h"
#include "slang-compiler-fwd.h"

namespace Slang
{

class InterfaceDecl;
class FunctionDeclBase;
class AggTypeDecl;
class AssocTypeDecl;
class GenericTypeConstraintDecl;
class Decl;
class ModuleDecl;
class FuncDecl;
class Type;
class ASTBuilder;
class SubtypeWitness;

enum class StructuralRayTracingStageKind
{
    ClosestHit,
    AnyHit,
    Intersection,
    Miss,
    Callable,
    Count,
};

enum class StructuralRayTracingMetadataKind
{
    ShaderGroupSlot,
    HitGroup,
    MissGroup,
    CallableGroup,
    HitGroupList,
    MissGroupList,
    CallableGroupList,
    TraceProgramLayout,
    Count,
};

enum class StructuralRayTracingStageInputOperationKind
{
    Payload,
    CallableData,
    Record,
    HitAttributes,
    TriangleBarycentricCoord,
    TriangleFrontFacing,
    CurveParameter,
    RayTMin,
    RayTCurrent,
    RayTime,
    RayFlags,
    HitKind,
    WorldRayOrigin,
    WorldRayDirection,
    ObjectSpaceRay,
    PrimitiveIndex,
    GeometryIndex,
    InstanceIndex,
    InstanceID,
    ObjectToWorld,
    WorldToObject,
    DispatchRaysIndex,
    DispatchRaysDimensions,
    IgnoreHit,
    AcceptHitAndEndSearch,
    ReportHit,
    ReportHitWithKind,
    Count,
};

enum class StructuralRayTracingAssociatedTypeKind
{
    TracePayload,
    TraceMotion,
    HitTraceContext,
    HitPrimitive,
    HitRecord,
    PrimitiveAttributes,
    MissTraceContext,
    MissRecord,
    CallableTraceContext,
    CallableData,
    CallableRecord,
    ProgramTraceContext,
    ProgramHitGroups,
    ProgramMissGroups,
    ProgramCallableGroups,
    HitGroupSlot,
    HitGroupContext,
    HitGroupClosestHit,
    HitGroupAnyHit,
    HitGroupIntersection,
    MissGroupSlot,
    MissGroupContext,
    MissGroupMiss,
    CallableGroupSlot,
    CallableGroupContext,
    CallableGroupCallable,
    Count,
};

enum class RayTracingAPIFamily
{
    Structural,
    Legacy,
};

enum class StructuralRayTracingHitAttributesKind
{
    None,
    Triangle,
    Curve,
    Custom,
};

enum class StructuralRayTracingMotionKind : UInt
{
    None = 0,
    Primitive = 1 << 0,
    Instance = 1 << 1,
    Invalid = ~UInt(0),
};

struct StructuralRayTracingEntryPointInfo
{
    StructuralRayTracingStageKind stageKind = StructuralRayTracingStageKind::Count;
    FuncDecl* invokeMethod = nullptr;
    Type* contextType = nullptr;
    Type* payloadType = nullptr;
    Type* recordType = nullptr;
    Type* hitAttributesType = nullptr;
    Type* callableDataType = nullptr;
    StructuralRayTracingHitAttributesKind hitAttributesKind =
        StructuralRayTracingHitAttributesKind::None;
};

struct RayTracingAPIUsage
{
    Decl* structuralDecl = nullptr;
    Decl* legacyDecl = nullptr;
    bool diagnosed = false;
};

class StructuralRayTracingDeclRegistry
{
public:
    bool registerTrustedModule(
        Module* module,
        StructuralRayTracingStageKind* outMissingStage = nullptr);
    bool isInitialized() const { return m_stageInterfaces[0] != nullptr; }
    bool isTrustedModule(Module* module) const;

    InterfaceDecl* getStageInterface(StructuralRayTracingStageKind kind) const;
    StructuralRayTracingStageKind getStageKind(InterfaceDecl* interfaceDecl) const;
    AggTypeDecl* getStageInputType(StructuralRayTracingStageKind kind) const;
    StructuralRayTracingStageKind getStageInputKind(AggTypeDecl* typeDecl) const;
    StructuralRayTracingMetadataKind getMetadataKind(InterfaceDecl* interfaceDecl) const;
    InterfaceDecl* getMetadataInterface(StructuralRayTracingMetadataKind kind) const;
    StructuralRayTracingStageInputOperationKind getStageInputOperationKind(
        FunctionDeclBase* functionDecl) const;
    bool isTraceMethod(FunctionDeclBase* functionDecl) const;
    bool isCallShaderMethod(FunctionDeclBase* functionDecl) const;
    AssocTypeDecl* getAssociatedTypeRequirement(StructuralRayTracingAssociatedTypeKind kind) const;
    Type* resolveAssociatedType(
        ASTBuilder* astBuilder,
        SubtypeWitness* witness,
        StructuralRayTracingAssociatedTypeKind kind) const;
    Type* resolveConcreteAssociatedType(
        ASTBuilder* astBuilder,
        Type* conformingType,
        SubtypeWitness* witness,
        StructuralRayTracingAssociatedTypeKind kind) const;
    SubtypeWitness* resolveAssociatedTypeConstraint(
        ASTBuilder* astBuilder,
        SubtypeWitness* witness,
        StructuralRayTracingAssociatedTypeKind kind) const;
    bool tryGetShaderGroupSlotIndex(ASTBuilder* astBuilder, Type* slotType, int64_t& outIndex)
        const;
    bool isStagePlaceholder(StructuralRayTracingStageKind kind, Type* type) const;
    StructuralRayTracingHitAttributesKind getHitAttributesKind(Type* primitiveType) const;
    StructuralRayTracingMotionKind getMotionKind(Type* motionType) const;

    FunctionDeclBase* getStageInvokeRequirement(StructuralRayTracingStageKind kind) const;
    void registerStageImplementation(
        FunctionDeclBase* implementation,
        StructuralRayTracingStageKind kind);
    StructuralRayTracingStageKind getStageKind(FunctionDeclBase* implementation) const;
    bool registerAPIUse(
        Module* module,
        RayTracingAPIFamily family,
        Decl* decl,
        Decl** outOtherDecl);
    void registerFunctionCall(
        FunctionDeclBase* caller,
        FunctionDeclBase* callee,
        SourceLoc callLoc);
    bool functionReachesStructuralTrace(FunctionDeclBase* function) const;
    bool findReachableCallShader(FunctionDeclBase* function, SourceLoc& outCallLoc) const;

private:
    InterfaceDecl* m_stageInterfaces[int(StructuralRayTracingStageKind::Count)] = {};
    AggTypeDecl* m_stageInputTypes[int(StructuralRayTracingStageKind::Count)] = {};
    FunctionDeclBase* m_stageInvokeRequirements[int(StructuralRayTracingStageKind::Count)] = {};
    InterfaceDecl* m_metadataInterfaces[int(StructuralRayTracingMetadataKind::Count)] = {};
    AssocTypeDecl*
        m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::Count)] = {};
    GenericTypeConstraintDecl* m_associatedTypeConstraintRequirements[int(
        StructuralRayTracingAssociatedTypeKind::Count)] = {};
    Dictionary<FunctionDeclBase*, StructuralRayTracingStageInputOperationKind>
        m_stageInputOperations;
    ModuleDecl* m_trustedModuleDecl = nullptr;
    AggTypeDecl* m_rayTracerType = nullptr;
    AggTypeDecl* m_trianglePrimitiveType = nullptr;
    AggTypeDecl* m_curvePrimitiveType = nullptr;
    AggTypeDecl* m_motionTypes[4] = {};
    AggTypeDecl* m_stagePlaceholderTypes[int(StructuralRayTracingStageKind::Count)] = {};
    Dictionary<FunctionDeclBase*, StructuralRayTracingStageKind> m_stageImplementations;
    Dictionary<Module*, RayTracingAPIUsage> m_apiUsage;
    Dictionary<FunctionDeclBase*, HashSet<FunctionDeclBase*>> m_functionCallees;
    HashSet<FunctionDeclBase*> m_structuralProgramCallers;
    Dictionary<FunctionDeclBase*, SourceLoc> m_callShaderCallers;
};

const char* getStructuralRayTracingStageInterfaceName(StructuralRayTracingStageKind kind);

} // namespace Slang
