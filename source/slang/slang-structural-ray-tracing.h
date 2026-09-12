#pragma once

#include "compiler-core/slang-source-loc.h"
#include "core/slang-dictionary.h"
#include "core/slang-list.h"
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
class ConcreteTypePack;
class TypePackSubtypeWitness;
class GenericDecl;
class ExtensionDecl;
class GenericTypeParamDecl;

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
    HitGroup,
    HitGroupList,
    MissShaderList,
    CallableShaderList,
    TraceProgramSchema,
    Count,
};

/// Identifies one independently openable entry section of a trace program schema.
///
/// This enum is serialized into compiler-owned IR metadata. It deliberately describes semantic
/// entry roles rather than the physical order of associated types or generic arguments.
enum class StructuralRayTracingSectionKind
{
    HitGroups,
    MissShaders,
    CallableShaders,
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
    TraceAccelerationStructure,
    TraceMotion,
    StageTraceContext,
    StageRecord,
    PayloadContextPayload,
    HitPrimitive,
    PrimitiveAttributes,
    CallableData,
    ProgramTraceContext,
    ProgramHitGroups,
    ProgramMissShaders,
    ProgramCallableShaders,
    HitGroupContext,
    HitGroupClosestHit,
    HitGroupAnyHit,
    HitGroupIntersection,
    ClosestHitShaderContext,
    AnyHitShaderContext,
    IntersectionStageContext,
    MissShaderContext,
    CallableShaderContext,
    Count,
};

enum class RayTracingAPIFamily
{
    Structural,
    Legacy,
};

enum class StructuralRayTracingTraceMethodKind
{
    None,
    ExplicitPayload,
    ImplicitEmptyPayload,
};

/// Identifies where one generic argument for the payload-taking trace overload comes from.
///
/// The implicit-empty-payload overload is lowered by specializing and calling its paired
/// payload-taking overload. That specialization must follow the semantic roles recorded from the
/// trusted standard module instead of assuming that `Payload` is the first generic parameter.
enum class StructuralRayTracingPairedTraceArgumentSourceKind
{
    PayloadType,
    ImplicitEmptyPayloadMethodArgument,
};

struct StructuralRayTracingPairedTraceArgumentSource
{
    StructuralRayTracingPairedTraceArgumentSourceKind kind =
        StructuralRayTracingPairedTraceArgumentSourceKind::PayloadType;
    Index argumentIndex = -1;
};

/// Records how a trusted `RayTracer` extension obtains its schema specialization.
///
/// Generic applications serialize ordinary arguments and constraint witnesses into one operand
/// list. Recording the declaration-derived indices here keeps later lowering independent of that
/// physical ordering.
struct StructuralRayTracingRayTracerMethodInfo
{
    ExtensionDecl* extensionDecl = nullptr;
    GenericDecl* schemaGenericDecl = nullptr;
    GenericTypeParamDecl* schemaTypeParameter = nullptr;
    GenericTypeConstraintDecl* schemaConstraint = nullptr;
    Index schemaTypeArgumentIndex = -1;
    Index schemaWitnessArgumentIndex = -1;
};

/// Records the complete compiler contract for one trusted `RayTracer.trace` overload.
///
/// Parameter indices name source parameters and therefore exclude the implicit receiver. The
/// registry validates these roles once while loading `slang.raytracing`; semantic checking and IR
/// lowering only consume this record and never rediscover a role from parameter counts.
struct StructuralRayTracingTraceMethodInfo
{
    StructuralRayTracingTraceMethodKind kind = StructuralRayTracingTraceMethodKind::None;
    FunctionDeclBase* pairedPayloadMethod = nullptr;
    GenericDecl* methodGenericDecl = nullptr;
    Index traversalDescParameterIndex = -1;
    Index accelerationStructureParameterIndex = -1;
    Index descriptorParameterIndex = -1;
    Index payloadParameterIndex = -1;
    Index payloadGenericArgumentIndex = -1;
    List<StructuralRayTracingPairedTraceArgumentSource> pairedPayloadGenericArguments;
};

/// Records the semantic roles in the trusted `RayTracer.callShader` method.
///
/// The callable-context conformance witness is not necessarily the first hidden generic argument:
/// source constraint order is semantically irrelevant. Recording both declaration-derived indices
/// keeps schema materialization and target lowering independent of that serialization detail.
struct StructuralRayTracingCallShaderMethodInfo
{
    GenericDecl* methodGenericDecl = nullptr;
    GenericTypeParamDecl* callableContextTypeParameter = nullptr;
    GenericTypeConstraintDecl* callableContextConstraint = nullptr;
    Index callableContextTypeArgumentIndex = -1;
    Index callableContextWitnessArgumentIndex = -1;
    Index callableIndexParameterIndex = -1;
    Index descriptorParameterIndex = -1;
    Index dataParameterIndex = -1;
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

/// Identifies one field in the compiler-synthesized Metal program-descriptor resource struct.
enum class StructuralRayTracingDescriptorResourceKind
{
    IntersectionFunctionTable,
    MissVisibleFunctionTable,
    ClosestHitVisibleFunctionTable,
    CallableVisibleFunctionTable,
    Records,
    Count,
};

/// Identifies the fixed intersection-function-table slot used by one Metal geometry kind.
///
/// Metal selects an intersection function from acceleration-structure geometry metadata, before
/// the generated function can inspect the logical SBT record. Keeping these indices independent
/// of schemas and payload partitions lets one acceleration structure select the same geometry
/// kind in every compiler-synthesized table.
enum class StructuralRayTracingMetalCandidateKind : UInt
{
    Triangle = 0,
    BoundingBox = 1,
    Curve = 2,
    Count,
};

/// Size and alignment, in bytes, of the compiler-owned Metal SBT record header.
///
/// The first word contains the reflected function index. The remaining bytes are reserved so the
/// application record that follows begins at the same 16-byte boundary for every section.
static constexpr UInt kStructuralRayTracingMetalRecordHeaderSize = 16;
static constexpr UInt kStructuralRayTracingMetalRecordAlignment = 16;

/// Returns the fixed Metal SBT section stride for an application record of `dataSize` bytes.
///
/// Reflection and Metal lowering deliberately share this calculation. The native host must use
/// the reflected result when packing records; reproducing this formula outside the compiler would
/// create a second ABI source of truth.
inline UInt64 getStructuralRayTracingMetalRecordStride(UInt64 dataSize)
{
    const UInt64 unalignedSize = kStructuralRayTracingMetalRecordHeaderSize + dataSize;
    return (unalignedSize + kStructuralRayTracingMetalRecordAlignment - 1) &
           ~UInt64(kStructuralRayTracingMetalRecordAlignment - 1);
}

/// Returns the logical field name for one synthesized Metal descriptor resource.
///
/// Per-payload fields include their partition index only when a schema has multiple payloads,
/// matching the IR producer. Reflection exposes this logical name while physical binding follows
/// the separately reflected field order.
String getStructuralRayTracingMetalDescriptorResourceName(
    StructuralRayTracingDescriptorResourceKind kind,
    Index payloadIndex,
    Index payloadCount);

struct StructuralRayTracingEntryPointInfo
{
    StructuralRayTracingStageKind stageKind = StructuralRayTracingStageKind::Count;
    FuncDecl* invokeMethod = nullptr;
    Type* stageType = nullptr;
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

struct StructuralRayTracingEntryPack
{
    ConcreteTypePack* types = nullptr;
    TypePackSubtypeWitness* witnesses = nullptr;
};

struct StructuralRayTracingOpenSectionInfo
{
    Type* tagType = nullptr;
    StructuralRayTracingEntryPack listedEntries;
};

StructuralRayTracingEntryPack getStructuralRayTracingEntryPack(
    ASTBuilder* astBuilder,
    Type* entryListType);

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
    AggTypeDecl* getOpenSectionType(StructuralRayTracingSectionKind kind) const;
    InterfaceDecl* getSectionEntryInterface(StructuralRayTracingSectionKind kind) const;
    bool tryGetOpenSectionInfo(
        ASTBuilder* astBuilder,
        Type* sectionType,
        StructuralRayTracingSectionKind expectedKind,
        StructuralRayTracingOpenSectionInfo& outInfo) const;
    bool isValidOpenSectionTag(
        ASTBuilder* astBuilder,
        Type* tagType,
        StructuralRayTracingSectionKind kind) const;
    void collectOpenSectionTags(
        ASTBuilder* astBuilder,
        Type* declaredTagType,
        StructuralRayTracingSectionKind kind,
        List<Type*>& outTagTypes) const;
    SubtypeWitness* projectOpenSectionEntryWitness(
        ASTBuilder* astBuilder,
        SubtypeWitness* tagWitness,
        StructuralRayTracingSectionKind kind) const;
    StructuralRayTracingStageInputOperationKind getStageInputOperationKind(
        FunctionDeclBase* functionDecl) const;
    StructuralRayTracingTraceMethodKind getTraceMethodKind(FunctionDeclBase* functionDecl) const;
    const StructuralRayTracingTraceMethodInfo* getTraceMethodInfo(
        FunctionDeclBase* functionDecl) const;
    const StructuralRayTracingRayTracerMethodInfo* getRayTracerMethodInfo(
        FunctionDeclBase* functionDecl) const;
    const StructuralRayTracingCallShaderMethodInfo* getCallShaderMethodInfo(
        FunctionDeclBase* functionDecl) const;
    bool isTraceMethod(FunctionDeclBase* functionDecl) const
    {
        return getTraceMethodKind(functionDecl) != StructuralRayTracingTraceMethodKind::None;
    }
    bool isCallShaderMethod(FunctionDeclBase* functionDecl) const
    {
        return getCallShaderMethodInfo(functionDecl) != nullptr;
    }
    AssocTypeDecl* getAssociatedTypeRequirement(StructuralRayTracingAssociatedTypeKind kind) const;
    Type* resolveAssociatedType(
        ASTBuilder* astBuilder,
        SubtypeWitness* witness,
        StructuralRayTracingAssociatedTypeKind kind) const;
    SubtypeWitness* resolveAssociatedTypeConstraint(
        ASTBuilder* astBuilder,
        SubtypeWitness* witness,
        StructuralRayTracingAssociatedTypeKind kind) const;
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
    InterfaceDecl* m_intersectionStageInterface = nullptr;
    AggTypeDecl* m_stageInputTypes[int(StructuralRayTracingStageKind::Count)] = {};
    FunctionDeclBase* m_stageInvokeRequirements[int(StructuralRayTracingStageKind::Count)] = {};
    InterfaceDecl* m_metadataInterfaces[int(StructuralRayTracingMetadataKind::Count)] = {};
    AggTypeDecl* m_openSectionTypes[int(StructuralRayTracingSectionKind::Count)] = {};
    GenericTypeParamDecl* m_openSectionTagParameters[int(StructuralRayTracingSectionKind::Count)] =
        {};
    AssocTypeDecl*
        m_associatedTypeRequirements[int(StructuralRayTracingAssociatedTypeKind::Count)] = {};
    GenericTypeConstraintDecl* m_associatedTypeConstraintRequirements[int(
        StructuralRayTracingAssociatedTypeKind::Count)] = {};
    Dictionary<FunctionDeclBase*, StructuralRayTracingStageInputOperationKind>
        m_stageInputOperations;
    Dictionary<FunctionDeclBase*, StructuralRayTracingTraceMethodInfo> m_traceMethods;
    Dictionary<FunctionDeclBase*, StructuralRayTracingRayTracerMethodInfo> m_rayTracerMethods;
    Dictionary<FunctionDeclBase*, StructuralRayTracingCallShaderMethodInfo> m_callShaderMethods;
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

/// Returns the source declaration path used as the name hint for a structural ray-tracing type.
///
/// The path includes namespaces and enclosing types, but excludes module names. A closed generic
/// specialization gains a suffix derived from its canonical semantic type, so two specializations
/// cannot silently claim the same structural entry-point name.
String getStructuralRayTracingSourceTypeName(ASTBuilder* astBuilder, Type* type);

/// Returns whether `type` is a resolved user struct with no instance storage.
///
/// Empty payloads use an implicit representation in the structural ray-tracing API. This query is
/// shared by semantic checking, which rejects explicit values, and IR lowering, which selects the
/// one eligible empty payload in a concrete schema.
bool isSemanticallyEmptyStructuralRayTracingPayload(ASTBuilder* astBuilder, Type* type);

/// Returns the deterministic target symbol used for a portable structural ray-tracing stage type.
///
/// Ordinary source identifiers other than target-reserved names are preserved. Qualified,
/// reserved, or otherwise target-unsafe names are encoded injectively so reflection and
/// synthesized entry points agree on one physical name.
String getStructuralRayTracingEntryPointName(UnownedStringSlice sourceTypeName);

/// Returns the exact exported Metal name for one miss visible-function-table entry.
///
/// Unlike a portable stage entry point, a Metal adapter's ABI also depends on its schema and
/// payload partition. The reflected function indices are part of the name so repeated uses of one
/// source stage still name distinct physical table entries deterministically.
String getStructuralRayTracingMetalMissFunctionName(
    UnownedStringSlice schemaSourceTypeName,
    Index payloadIndex,
    Index functionIndex,
    UnownedStringSlice stageSourceTypeName);

/// Returns the exact exported Metal name for one closest-hit visible-function-table entry.
///
/// The group identity is included because two hit groups can reuse one closest-hit stage while
/// requiring different record or custom-attribute lowering.
String getStructuralRayTracingMetalClosestHitFunctionName(
    UnownedStringSlice schemaSourceTypeName,
    Index payloadIndex,
    Index functionIndex,
    UnownedStringSlice groupSourceTypeName,
    UnownedStringSlice stageSourceTypeName);

/// Returns the exact exported Metal name for a synthesized no-op closest-hit table entry.
///
/// A payload partition uses one dense visible-function table. If any group has a real closest-hit
/// stage, groups using `NoClosestHit` still need a signature-compatible function in their slots;
/// this helper gives lowering and reflection one shared identity for that physical placeholder.
String getStructuralRayTracingMetalNoOpClosestHitFunctionName(
    UnownedStringSlice schemaSourceTypeName,
    Index payloadIndex,
    Index functionIndex,
    UnownedStringSlice groupSourceTypeName);

/// Returns the exact exported Metal name for one schema-wide callable table entry.
String getStructuralRayTracingMetalCallableFunctionName(
    UnownedStringSlice schemaSourceTypeName,
    Index functionIndex,
    UnownedStringSlice stageSourceTypeName);

/// Returns the exact exported Metal name for a payload partition's candidate dispatcher.
///
/// Any-hit and intersection source stages are implementation arms of this combined function on
/// Metal, so this schema/payload/geometry name is the symbol that host reflection must expose.
String getStructuralRayTracingMetalCandidateDispatcherName(
    UnownedStringSlice schemaSourceTypeName,
    Index payloadIndex,
    StructuralRayTracingMetalCandidateKind candidateKind);

} // namespace Slang
