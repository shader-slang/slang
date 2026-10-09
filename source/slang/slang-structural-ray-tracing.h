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

/// Identifies one logical stage contract represented by a dedicated AST type.
///
/// The value selects the stage profile and IR interface opcode. `Count` is the invalid value.
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
/// Dedicated AST types identify these roles independently of module or declaration names.
enum class StructuralRayTracingMetadataKind
{
    HitGroup,
    HitGroupList,
    MissShaderList,
    CallableShaderList,
    TraceProgramSchema,
    Count,
};

/// Gives each associated-type requirement in the structural API a stable semantic role.
///
/// Each role belongs to a named member of a compiler-recognized interface. Consumers find that
/// requirement through the supplied conformance witness, preserving its inheritance path.
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

/// Distinguishes the two annotated `RayTracer.trace` source contracts.
///
/// The explicit form owns an `inout` payload parameter. The implicit form represents the schema's
/// unique empty payload without exposing a payload value in source.
enum class StructuralRayTracingTraceMethodKind
{
    None,
    ExplicitPayload,
    ImplicitEmptyPayload,
};

/// Identifies a structural operation that still requires target-independent lowering.
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

/// Describes the annotated payload contract of one `RayTracer.trace` overload.
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
    /// The exact executable interface selected through the stage implementation's witness.
    InterfaceDecl* stageInterface = nullptr;
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
/// Concrete lists keep the two packs index-aligned, including two zero-length packs for an empty
/// list. Dependent lists whose entries are not known yet have an empty `types` pack and no witness
/// pack. Callers discover each pack by semantic value kind rather than operand position.
struct StructuralRayTracingEntryPack
{
    ConcreteTypePack* types = nullptr;
    TypePackSubtypeWitness* witnesses = nullptr;
};

/// Decodes the entry and witness packs from a trusted section-list specialization.
StructuralRayTracingEntryPack getStructuralRayTracingEntryPack(
    ASTBuilder* astBuilder,
    Type* entryListType);

/// Returns whether a declaration belongs to an explicitly annotated structural API type or
/// operation.
///
/// Generic declarations and extensions retain the identity of their inner declaration or target.
/// Module membership alone does not grant compiler-recognized behavior.
bool isStructuralRayTracingDeclaration(Decl* declaration);

/// Retains semantic checking state for structural ray tracing within one linkage.
///
/// Compiler-recognized declaration roles come from their AST type or operation annotation. Only
/// completed implementation selections and diagnostic bookkeeping are cached here; module import
/// does not populate this state or modify declaration identity.
class StructuralRayTracingCheckingState
{
public:
    /// Returns the stage represented by the interface's AST type, including the intersection
    /// marker.
    StructuralRayTracingStageKind getStageKind(InterfaceDecl* interfaceDecl) const;

    /// Returns whether the interface represents an executable stage with an `invoke` requirement.
    bool isExecutableStageInterface(InterfaceDecl* interfaceDecl) const;

    /// Returns the stage represented by the input's AST type, or `Count` for an ordinary type.
    StructuralRayTracingStageKind getStageInputKind(AggTypeDecl* typeDecl) const;

    /// Returns the metadata role represented by the interface's AST type, or `Count` otherwise.
    StructuralRayTracingMetadataKind getMetadataKind(InterfaceDecl* interfaceDecl) const;

    /// Returns whether the accessor belongs to a property annotated as a stage payload view.
    bool isPayloadStageInputAccessor(FunctionDeclBase* functionDecl) const;

    /// Returns the source contract of an annotated trace overload, or `None` for other functions.
    StructuralRayTracingTraceMethodKind getTraceMethodKind(FunctionDeclBase* functionDecl) const;

    /// Reads an annotated trace overload and the index of its annotated payload parameter.
    StructuralRayTracingTraceMethodInfo getTraceMethodInfo(FunctionDeclBase* functionDecl) const;

    /// Returns whether the function is an annotated structural trace operation.
    bool isTraceMethod(FunctionDeclBase* functionDecl) const
    {
        return getTraceMethodKind(functionDecl) != StructuralRayTracingTraceMethodKind::None;
    }

    /// Returns whether the function is an annotated structural callable operation.
    bool isCallShaderMethod(FunctionDeclBase* functionDecl) const;

    /// Returns the requirement's role from its name and its declaring interface's AST type.
    StructuralRayTracingAssociatedTypeKind getAssociatedTypeKind(AssocTypeDecl* requirement) const;

    /// Resolves an associated type through the exact supplied interface-conformance path.
    ///
    /// Returns null if the interface and its inherited interfaces do not declare the requested
    /// requirement, or the selected witness does not provide a type value.
    Type* resolveAssociatedType(
        ASTBuilder* astBuilder,
        SubtypeWitness* witness,
        StructuralRayTracingAssociatedTypeKind kind) const;

    /// Resolves the constraint on an associated type through the supplied conformance path.
    SubtypeWitness* resolveAssociatedTypeConstraint(
        ASTBuilder* astBuilder,
        SubtypeWitness* witness,
        StructuralRayTracingAssociatedTypeKind kind) const;

    /// Classifies the attribute model from the primitive's AST type.
    StructuralRayTracingHitAttributesKind getHitAttributesKind(Type* primitiveType) const;

    /// Returns this executable interface's `invoke` requirement, or null for other interfaces.
    FunctionDeclBase* getStageInvokeRequirement(InterfaceDecl* interfaceDecl) const;

    /// Records the concrete method selected by an executable stage's conformance witness.
    void registerStageImplementation(
        FunctionDeclBase* implementation,
        StructuralRayTracingStageKind kind);

    /// Returns the role of a stage requirement or previously selected implementation method.
    StructuralRayTracingStageKind getStageKind(FunctionDeclBase* implementation) const;

    /// Returns true the first time checking validates an aggregate declaration's stage storage.
    bool beginStageRepresentationDeclarationCheck(AggTypeDecl* declaration);

    /// Returns true the first time checking validates a non-aggregate stage's representation.
    bool beginStageRepresentationTypeCheck(Type* type);

    /// Records one pipeline use and reports whether it completes a new mixed-API pair.
    ///
    /// A true result requests one diagnostic at `decl`; `outOtherDecl` identifies the earlier
    /// use of the other API. Later uses in the same module do not repeat the diagnostic.
    bool registerAPIUse(
        Module* module,
        RayTracingAPIFamily family,
        Decl* decl,
        Decl** outOtherDecl);

private:
    Dictionary<FunctionDeclBase*, StructuralRayTracingStageKind> m_stageImplementations;
    HashSet<AggTypeDecl*> m_stageDeclarationsWithCheckedRepresentation;
    HashSet<Type*> m_stageTypesWithCheckedRepresentation;
    Dictionary<Module*, RayTracingAPIUsage> m_apiUsage;
};

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
