#pragma once

#include "slang-ast-support-types.h"
#include "slang-compiler-fwd.h"
#include "slang-ir-insts-enum.h"

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
class MagicTypeModifier;

/// Allows only the executable-stage magic type family on ordinary interface declarations.
bool isRayTracingStageInterfaceModifier(MagicTypeModifier* modifier, Decl* decl);

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

/// Gives each associated-type requirement in the structural API a stable semantic role.
///
/// A KnownBuiltin annotation identifies each requirement on an ordinary interface. For example,
/// the Record requirement is found through the selected context conformance even when another
/// interface also declares a member named Record. Neither interface needs a special AST type.
enum class StructuralRayTracingAssociatedTypeKind
{
    StageContext,
    StageRecord,
    PayloadContextPayload,
    HitPrimitive,
    PrimitiveAttributes,
    CallableData,
    ProgramHitGroups,
    ProgramMissShaders,
    ProgramCallableShaders,
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

/// Returns the executable stage represented by the interface's AST type.
StructuralRayTracingStageKind getStructuralRayTracingStageKind(InterfaceDecl* interfaceDecl);

/// Returns whether the interface represents an executable stage with an `invoke` requirement.
bool isExecutableStructuralRayTracingStageInterface(InterfaceDecl* interfaceDecl);

/// Returns the stage named by the input struct's KnownBuiltin annotation, or Count otherwise.
StructuralRayTracingStageKind getStructuralRayTracingStageInputKind(AggTypeDecl* typeDecl);

/// Returns the source contract of an annotated trace overload, or `None` for other functions.
StructuralRayTracingTraceMethodKind getStructuralRayTracingTraceMethodKind(
    FunctionDeclBase* functionDecl);

/// Returns whether the function is an annotated structural trace operation.
bool isStructuralRayTracingTraceMethod(FunctionDeclBase* functionDecl);

/// Returns whether the function is an annotated structural callable operation.
bool isStructuralRayTracingCallShaderMethod(FunctionDeclBase* functionDecl);

/// Returns the semantic role attached to an ordinary associated-type requirement.
StructuralRayTracingAssociatedTypeKind getStructuralRayTracingAssociatedTypeKind(
    AssocTypeDecl* requirement);

/// Resolves an associated type through the exact supplied interface-conformance path.
///
/// Returns null if the interface and its inherited interfaces do not declare the requested
/// requirement, or the selected witness does not provide a type value.
Type* resolveStructuralRayTracingAssociatedType(
    ASTBuilder* astBuilder,
    SubtypeWitness* witness,
    StructuralRayTracingAssociatedTypeKind kind);

/// Resolves the constraint on an associated type that supplies the required next member.
/// For example, Context may have several bounds; resolving its Payload selects the bound
/// whose checked interface facets declare the annotated Payload requirement.
SubtypeWitness* resolveStructuralRayTracingAssociatedTypeConstraint(
    ASTBuilder* astBuilder,
    SubtypeWitness* witness,
    StructuralRayTracingAssociatedTypeKind kind,
    StructuralRayTracingAssociatedTypeKind requiredMember);

/// Classifies built-in primitive attribute ABIs from the ordinary struct's semantic annotation.
/// For example, TrianglePrimitive selects triangle attributes; BoundingBoxPrimitive<A> selects A.
StructuralRayTracingHitAttributesKind getStructuralRayTracingHitAttributesKind(Type* primitiveType);

/// Returns this executable interface's `invoke` requirement, or null for other interfaces.
FunctionDeclBase* getStructuralRayTracingStageInvokeRequirement(InterfaceDecl* interfaceDecl);

/// Returns the source declaration path used as the name hint for a structural ray-tracing type.
///
/// The path includes namespaces and enclosing types, but excludes module names. A closed generic
/// specialization gains a suffix derived from its canonical semantic type, so two specializations
/// cannot silently claim the same structural entry-point name.
String getStructuralRayTracingSourceTypeName(ASTBuilder* astBuilder, Type* type);

/// Returns the deterministic target symbol used for a portable structural ray-tracing stage type.
///
/// Ordinary source identifiers other than target-reserved names are preserved. Qualified,
/// reserved, or otherwise target-unsafe names are encoded injectively so reflection and
/// synthesized entry points agree on one physical name.
String getStructuralRayTracingEntryPointName(UnownedStringSlice sourceTypeName);

struct FrontEndEntryPointRequest;
struct CapabilitySet;
class EntryPoint;
struct IRModule;
struct IRFunc;

/// Returns the source-stage capabilities, including synthesized structural stages.
CapabilitySet getEntryPointStageCapabilities(EntryPoint* entryPoint);

/// Returns the public entry-point declaration used by capability diagnostics.
Decl* getEntryPointCapabilityDiagnosticDecl(EntryPoint* entryPoint);

/// Creates a structural entry point if the request selects a stage struct.
RefPtr<EntryPoint> tryCreateStructuralRayTracingEntryPoint(
    FrontEndEntryPointRequest* entryPointReq,
    bool* outFoundStructuralStage,
    CapabilitySet* outCapabilities);

/// Returns whether a type represents the opaque trace-program resource handle.
bool isStructuralRayTracingOpaqueHandleType(Type* type);

/// Diagnoses source use of compiler-owned structural IR identities and metadata.
bool diagnoseInvalidStructuralRayTracingIntrinsicOp(
    IROp op,
    bool isCoreModule,
    UnownedStringSlice operationName,
    SourceLoc loc,
    DiagnosticSink* sink);

/// Rejects structural source representations that remain before native ABI legalization.
SlangResult diagnoseUnloweredStructuralRayTracing(
    IRModule* module,
    const List<IRFunc*>& entryPoints,
    DiagnosticSink* sink);

} // namespace Slang
