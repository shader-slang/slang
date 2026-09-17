#pragma once

#include "compiler-core/slang-source-loc.h"
#include "core/slang-dictionary.h"
#include "core/slang-list.h"
#include "slang-ast-support-types.h"
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
class GenericTypeParamDecl;

/// Returns whether `functionDecl` is one of the top-level legacy pipeline intrinsics in `core`.
///
/// The check deliberately excludes methods such as `RayQuery.TraceRayInline` and
/// `HitObject.TraceRay`: those are separate APIs that may coexist with a structural pipeline.
bool isCoreLegacyRayTracingPipelineMethod(FunctionDeclBase* functionDecl);

/// Identifies one logical stage contract exported by the trusted ray-tracing module.
///
/// The value keys the matching source interface, stage-input type, `invoke` requirement, and IR
/// interface opcode. `Count` is the invalid/sentinel value as well as the array bound.
enum class StructuralRayTracingStageKind
{
    ClosestHit,
    AnyHit,
    Intersection,
    Miss,
    Callable,
    Count,
};

/// Identifies a compile-time metadata interface whose conformers have no runtime representation.
///
/// These roles let semantic checking recognize hit groups, section lists, and schemas by trusted
/// declaration identity instead of by user-visible names.
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
/// The values describe semantic entry roles rather than the physical order of associated types or
/// generic arguments.
enum class StructuralRayTracingSectionKind
{
    HitGroups,
    MissShaders,
    CallableShaders,
    Count,
};

/// Gives each associated-type requirement in the structural API a stable semantic role.
///
/// The registry resolves these roles to trusted declarations once. Consumers can then project a
/// context or schema witness without depending on declaration order or source spelling.
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

/// Distinguishes the structural pipeline API from the legacy pipeline intrinsics.
///
/// The frontend records the first use of each family per source module so it can diagnose a mixed
/// pipeline model once without treating ray queries and hit-object APIs as legacy pipeline use.
enum class RayTracingAPIFamily
{
    Structural,
    Legacy,
};

/// Distinguishes the two trusted `RayTracer.trace` source contracts.
///
/// The explicit form owns an `inout` payload parameter. The implicit form represents the schema's
/// unique empty payload without exposing a payload value in source.
enum class StructuralRayTracingTraceMethodKind
{
    None,
    ExplicitPayload,
    ImplicitEmptyPayload,
};

/// Identifies a trusted structural operation that still requires target-independent lowering.
///
/// These values are serialized on the corresponding standard-module IR functions. They describe
/// source semantics only; they are not target opcodes and must be consumed before entry-point ABI
/// legalization begins.
enum class StructuralRayTracingSourceOperationKind
{
    TraceExplicitPayload,
    TraceImplicitEmptyPayload,
    CallShader,
    Count,
};

/// Associates one trusted standard-module method with the source operation it represents.
struct StructuralRayTracingSourceOperation
{
    FunctionDeclBase* functionDecl = nullptr;
    StructuralRayTracingSourceOperationKind kind = StructuralRayTracingSourceOperationKind::Count;
};

/// Records the frontend facts consumed for one trusted `RayTracer.trace` overload.
///
/// The method kind distinguishes the explicit and implicit empty-payload contracts. Only the
/// explicit form has a payload parameter; its source-parameter index lets semantic checking reject
/// an explicitly passed empty payload without depending on a parameter name or position.
struct StructuralRayTracingTraceMethodInfo
{
    StructuralRayTracingTraceMethodKind kind = StructuralRayTracingTraceMethodKind::None;
    Index payloadParameterIndex = -1;
};

/// Classifies how a selected hit stage obtains its intersection attributes.
///
/// Triangle and curve attributes are compiler-defined property views; custom attributes come from
/// a procedural primitive. `None` is the invalid/non-hit-stage sentinel.
enum class StructuralRayTracingHitAttributesKind
{
    None,
    Triangle,
    Curve,
    Custom,
};

/// Holds the fully checked source contract for one selected structural stage entry point.
///
/// All pointers are AST objects owned by the entry point's linkage. AST-to-IR lowering copies the
/// semantic roles into non-operational metadata before those AST identities become unavailable.
struct StructuralRayTracingEntryPointInfo
{
    StructuralRayTracingStageKind stageKind = StructuralRayTracingStageKind::Count;
    Type* stageType = nullptr;
    Type* contextType = nullptr;
    /// The payload is present only for hit and miss stages.
    Type* payloadType = nullptr;
    /// Every executable stage has a record type, including `void` when no data is required.
    Type* recordType = nullptr;
    /// Hit stages retain the resolved primitive for frontend target-capability validation.
    Type* primitiveType = nullptr;
    /// Hit stages carry attributes; miss and callable stages leave this null.
    Type* hitAttributesType = nullptr;
    /// Callable stages carry callable data; all other stages leave this null.
    Type* callableDataType = nullptr;
    StructuralRayTracingHitAttributesKind hitAttributesKind =
        StructuralRayTracingHitAttributesKind::None;
};

/// Records the first structural and legacy pipeline uses seen in one source module.
///
/// `diagnosed` prevents later calls or entry-point discovery from repeating the same mixed-API
/// diagnostic for that module.
struct RayTracingAPIUsage
{
    Decl* structuralDecl = nullptr;
    Decl* legacyDecl = nullptr;
    bool diagnosed = false;
};

/// Names the concrete entries and matching subtype witnesses encoded by a section-list type.
///
/// Empty section types have an empty `types` pack and no witness pack. Non-empty lists keep the two
/// packs index-aligned, but callers discover each pack by semantic value kind rather than operand
/// position.
struct StructuralRayTracingEntryPack
{
    ConcreteTypePack* types = nullptr;
    TypePackSubtypeWitness* witnesses = nullptr;
};

/// Describes the source-local part of one open schema section.
///
/// `tagType` is the interface used for link-time discovery; `listedEntries` are the explicitly
/// named entries that participate even before linked conformances are collected.
struct StructuralRayTracingOpenSectionInfo
{
    Type* tagType = nullptr;
    StructuralRayTracingEntryPack listedEntries;
};

/// Decodes the entry and witness packs from a trusted closed or open section-list specialization.
StructuralRayTracingEntryPack getStructuralRayTracingEntryPack(
    ASTBuilder* astBuilder,
    Type* entryListType);

/// Caches the compiler-owned declaration identities from the packaged ray-tracing module.
///
/// One registry belongs to a `Linkage`, and every stored AST pointer is owned by that linkage's
/// imported module graph. Registration authenticates the complete private source contract once;
/// later semantic and IR code asks for roles through this registry instead of matching names or
/// generic-argument positions. The registry also keeps per-linkage validation state used to avoid
/// duplicate diagnostics and to inspect calls reachable from structural stage implementations.
class StructuralRayTracingDeclRegistry
{
public:
    /// Authenticates and records the declarations in the packaged `slang.raytracing` module.
    ///
    /// Returns false when a required stage interface, input, or `invoke` requirement is absent.
    /// `outMissingStage`, when supplied, identifies that incomplete stage contract. Other malformed
    /// private contracts are compiler/package mismatches and are asserted at registration.
    bool registerTrustedModule(
        Module* module,
        StructuralRayTracingStageKind* outMissingStage = nullptr);

    /// Returns whether the trusted stage declarations have been installed for this linkage.
    bool isInitialized() const { return m_stageInterfaces[0] != nullptr; }

    /// Returns whether `module` is the exact packaged module authenticated by this registry.
    bool isTrustedModule(Module* module) const;

    /// Returns the trusted source interface for `kind`, or null for an invalid kind.
    InterfaceDecl* getStageInterface(StructuralRayTracingStageKind kind) const;

    /// Returns the role of a trusted stage interface, or `Count` for any other interface.
    StructuralRayTracingStageKind getStageKind(InterfaceDecl* interfaceDecl) const;

    /// Returns the trusted zero-storage input type for `kind`, or null for an invalid kind.
    AggTypeDecl* getStageInputType(StructuralRayTracingStageKind kind) const;

    /// Returns the role of a trusted stage-input declaration, or `Count` for any other type.
    StructuralRayTracingStageKind getStageInputKind(AggTypeDecl* typeDecl) const;

    /// Returns the role of a trusted compile-time metadata interface, or `Count` otherwise.
    StructuralRayTracingMetadataKind getMetadataKind(InterfaceDecl* interfaceDecl) const;

    /// Returns the trusted metadata interface for `kind`, or null for an invalid kind.
    InterfaceDecl* getMetadataInterface(StructuralRayTracingMetadataKind kind) const;

    /// Decodes `sectionType` when it is the trusted open form for `expectedKind`.
    ///
    /// On success, `outInfo` receives the tag argument and explicitly listed entry pack. On
    /// failure it is cleared, so callers never observe information from an earlier query.
    bool tryGetOpenSectionInfo(
        ASTBuilder* astBuilder,
        Type* sectionType,
        StructuralRayTracingSectionKind expectedKind,
        StructuralRayTracingOpenSectionInfo& outInfo) const;

    /// Returns whether `functionDecl` is a trusted accessor for a stage input's payload property.
    bool isPayloadStageInputAccessor(FunctionDeclBase* functionDecl) const;

    /// Returns the source contract implemented by a trusted trace overload.
    StructuralRayTracingTraceMethodKind getTraceMethodKind(FunctionDeclBase* functionDecl) const;

    /// Returns the checked trace-overload metadata, or null for any untrusted method.
    const StructuralRayTracingTraceMethodInfo* getTraceMethodInfo(
        FunctionDeclBase* functionDecl) const;

    /// Returns whether `functionDecl` implements either trusted trace source contract.
    bool isTraceMethod(FunctionDeclBase* functionDecl) const
    {
        return getTraceMethodKind(functionDecl) != StructuralRayTracingTraceMethodKind::None;
    }

    /// Returns whether `functionDecl` is a trusted `RayTracer.callShader` overload.
    bool isCallShaderMethod(FunctionDeclBase* functionDecl) const;

    /// Appends every trusted trace and callable operation together with its semantic role.
    ///
    /// Consumers use this list to transfer the source contract onto serialized IR without
    /// rediscovering operations from method names or overload positions.
    void getSourceOperations(List<StructuralRayTracingSourceOperation>& outOperations) const;
    /// Returns the trusted associated-type declaration assigned to `kind`.
    AssocTypeDecl* getAssociatedTypeRequirement(StructuralRayTracingAssociatedTypeKind kind) const;

    /// Resolves the associated type identified by `kind` through the exact supplied witness path.
    ///
    /// Returns null when the witness cannot be projected to the interface that owns the
    /// requirement or the witness does not provide a type value.
    Type* resolveAssociatedType(
        ASTBuilder* astBuilder,
        SubtypeWitness* witness,
        StructuralRayTracingAssociatedTypeKind kind) const;

    /// Resolves the subtype witness attached to the associated type identified by `kind`.
    ///
    /// This operation preserves the caller's interface-conformance path instead of starting an
    /// unrelated subtype lookup for the concrete type.
    SubtypeWitness* resolveAssociatedTypeConstraint(
        ASTBuilder* astBuilder,
        SubtypeWitness* witness,
        StructuralRayTracingAssociatedTypeKind kind) const;

    /// Classifies the attribute model associated with a trusted primitive type.
    StructuralRayTracingHitAttributesKind getHitAttributesKind(Type* primitiveType) const;

    /// Returns the trusted `invoke` requirement for `kind`, or null for an invalid kind.
    FunctionDeclBase* getStageInvokeRequirement(StructuralRayTracingStageKind kind) const;

    /// Records the concrete method selected to satisfy one trusted stage `invoke` requirement.
    void registerStageImplementation(
        FunctionDeclBase* implementation,
        StructuralRayTracingStageKind kind);

    /// Returns the role of a trusted requirement or registered implementation method.
    StructuralRayTracingStageKind getStageKind(FunctionDeclBase* implementation) const;

    /// Returns true the first time semantic checking validates `declaration`'s stage
    /// representation.
    ///
    /// Interface conformance checking can report the same completed witness more than once, while
    /// several concrete stage types can share a base struct. Representation diagnostics belong to
    /// the declaration that introduces the invalid kind or field, so callers emit them once there.
    bool beginStageRepresentationDeclarationCheck(AggTypeDecl* declaration);

    /// Returns true the first time semantic checking validates a non-aggregate stage `type`.
    bool beginStageRepresentationTypeCheck(Type* type);

    /// Records one pipeline-API use and reports whether it completes a new mixed-family pair.
    ///
    /// A true result means the caller should diagnose once at `decl`; `outOtherDecl` then names the
    /// first use of the other family. False means the module is not mixed or was already diagnosed.
    bool registerAPIUse(
        Module* module,
        RayTracingAPIFamily family,
        Decl* decl,
        Decl** outOtherDecl);

    /// Records one checked call edge and remembers direct calls to trusted `callShader` methods.
    void registerFunctionCall(
        FunctionDeclBase* caller,
        FunctionDeclBase* callee,
        SourceLoc callLoc);

    /// Finds a `callShader` invocation reachable from `function` in the checked source call graph.
    ///
    /// On success, `outCallLoc` identifies the direct call site that made the operation reachable.
    bool findReachableCallShader(FunctionDeclBase* function, SourceLoc& outCallLoc) const;

private:
    // These declarations give every compiler-owned source role one authenticated AST identity.
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

    // These members classify trusted accessors and dispatch methods without relying on spelling at
    // each use site.
    HashSet<FunctionDeclBase*> m_payloadStageInputAccessors;
    Dictionary<FunctionDeclBase*, StructuralRayTracingTraceMethodInfo> m_traceMethods;
    HashSet<FunctionDeclBase*> m_callShaderMethods;
    ModuleDecl* m_trustedModuleDecl = nullptr;
    AggTypeDecl* m_rayTracerType = nullptr;
    AggTypeDecl* m_trianglePrimitiveType = nullptr;
    AggTypeDecl* m_curvePrimitiveType = nullptr;
    AggTypeDecl* m_motionTypes[4] = {};

    // These sets suppress duplicate stage diagnostics and support same-module API validation.
    Dictionary<FunctionDeclBase*, StructuralRayTracingStageKind> m_stageImplementations;
    HashSet<AggTypeDecl*> m_stageDeclarationsWithCheckedRepresentation;
    HashSet<Type*> m_stageTypesWithCheckedRepresentation;
    Dictionary<Module*, RayTracingAPIUsage> m_apiUsage;

    // The checked source call graph is retained only for stage restrictions that must see through
    // ordinary helper functions, such as the prohibition on callable dispatch from any-hit.
    Dictionary<FunctionDeclBase*, HashSet<FunctionDeclBase*>> m_functionCallees;
    Dictionary<FunctionDeclBase*, SourceLoc> m_callShaderCallers;
};

/// Returns the public source name of the trusted stage interface for diagnostics and registration.
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
/// shared by the semantic checks that reject both explicit trace arguments and explicit access to
/// the corresponding stage-input property.
bool isSemanticallyEmptyStructuralRayTracingPayload(ASTBuilder* astBuilder, Type* type);

/// Returns the deterministic target symbol used for a portable structural ray-tracing stage type.
///
/// Ordinary source identifiers other than target-reserved names are preserved. Qualified,
/// reserved, or otherwise target-unsafe names are encoded injectively so reflection and
/// synthesized entry points agree on one physical name.
String getStructuralRayTracingEntryPointName(UnownedStringSlice sourceTypeName);

} // namespace Slang
