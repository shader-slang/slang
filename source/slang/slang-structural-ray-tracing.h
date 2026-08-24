#pragma once

#include "core/slang-dictionary.h"
#include "slang-compiler-fwd.h"

namespace Slang
{

class InterfaceDecl;
class FunctionDeclBase;
class AggTypeDecl;
class AssocTypeDecl;
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
    HitAttributes,
    TriangleBarycentricCoord,
    TriangleFrontFacing,
    RayTCurrent,
    HitKind,
    WorldRayOrigin,
    WorldRayDirection,
    ObjectSpaceRay,
    PrimitiveIndex,
    GeometryIndex,
    IgnoreHit,
    AcceptHitAndEndSearch,
    ReportHit,
    ReportHitWithKind,
    Count,
};

enum class StructuralRayTracingAssociatedTypeKind
{
    TracePayload,
    HitTraceContext,
    HitPrimitive,
    PrimitiveAttributes,
    MissTraceContext,
    CallableData,
    ProgramTraceContext,
    ProgramHitGroups,
    ProgramMissGroups,
    ProgramCallableGroups,
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

struct StructuralRayTracingEntryPointInfo
{
    FuncDecl* invokeMethod = nullptr;
    Type* contextType = nullptr;
    Type* payloadType = nullptr;
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

    InterfaceDecl* getStageInterface(StructuralRayTracingStageKind kind) const;
    StructuralRayTracingStageKind getStageKind(InterfaceDecl* interfaceDecl) const;
    AggTypeDecl* getStageInputType(StructuralRayTracingStageKind kind) const;
    StructuralRayTracingStageKind getStageInputKind(AggTypeDecl* typeDecl) const;
    StructuralRayTracingMetadataKind getMetadataKind(InterfaceDecl* interfaceDecl) const;
    StructuralRayTracingStageInputOperationKind getStageInputOperationKind(
        FunctionDeclBase* functionDecl) const;
    bool isTraceMethod(FunctionDeclBase* functionDecl) const;
    AssocTypeDecl* getAssociatedTypeRequirement(StructuralRayTracingAssociatedTypeKind kind) const;
    Type* resolveAssociatedType(
        ASTBuilder* astBuilder,
        SubtypeWitness* witness,
        StructuralRayTracingAssociatedTypeKind kind) const;
    StructuralRayTracingHitAttributesKind getHitAttributesKind(Type* primitiveType) const;

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

private:
    InterfaceDecl* m_stageInterfaces[int(StructuralRayTracingStageKind::Count)] = {};
    AggTypeDecl* m_stageInputTypes[int(StructuralRayTracingStageKind::Count)] = {};
    FunctionDeclBase* m_stageInvokeRequirements[int(StructuralRayTracingStageKind::Count)] = {};
    InterfaceDecl* m_metadataInterfaces[int(StructuralRayTracingMetadataKind::Count)] = {};
    AssocTypeDecl*
        m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::Count)] = {};
    Dictionary<FunctionDeclBase*, StructuralRayTracingStageInputOperationKind>
        m_stageInputOperations;
    ModuleDecl* m_trustedModuleDecl = nullptr;
    AggTypeDecl* m_rayTracerType = nullptr;
    AggTypeDecl* m_trianglePrimitiveType = nullptr;
    AggTypeDecl* m_curvePrimitiveType = nullptr;
    Dictionary<FunctionDeclBase*, StructuralRayTracingStageKind> m_stageImplementations;
    Dictionary<Module*, RayTracingAPIUsage> m_apiUsage;
};

const char* getStructuralRayTracingStageInterfaceName(StructuralRayTracingStageKind kind);

} // namespace Slang
