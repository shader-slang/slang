#pragma once

#include "slang-ast-support-types.h"
#include "slang-compiler-fwd.h"
#include "slang-profile.h"

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

/// Allows the ray-tracing stage-interface and descriptor AST types in imported library
/// declarations.
bool isRayTracingBuiltinTypeModifier(MagicTypeModifier* modifier, Decl* decl);

/// Distinguishes structural shader declarations from legacy shader entry points.
///
/// The frontend records the first shader declaration or stage-input use of each family per module
/// so it can diagnose mixing the two shader models once. Ordinary dispatch calls do not select
/// either family.
enum class RayTracingAPIFamily
{
    Structural,
    Legacy,
};

/// Identifies a structural operation that still requires target-independent lowering.
///
/// These values are serialized on the corresponding standard-module IR functions. They describe
/// source semantics only; they are not target opcodes and must be consumed before entry-point ABI
/// legalization begins.
enum class StructuralRayTracingSourceOperationKind
{
    // Values 0 and 1 previously marked trace functions. Metal tracing now has an explicit IR
    // instruction; leave those values unused rather than changing CallShader's saved value.
    CallShader = 2,
};

/// Names the concrete entries and matching subtype witnesses encoded by a section-list type.
///
/// Concrete lists keep the two packs index-aligned, including two zero-length packs for an empty
/// list. A list that is still a type parameter or unresolved associated type returns an empty
/// `types` pack to skip the current duplicate check; this does not prove that the list is empty
/// or arrange another check after specialization. Callers discover each pack by semantic value
/// kind rather than operand position.
struct StructuralRayTracingEntryPack
{
    ConcreteTypePack* types = nullptr;
    TypePackSubtypeWitness* witnesses = nullptr;
};

/// Decodes the entry and witness packs from a trusted section-list specialization.
StructuralRayTracingEntryPack getStructuralRayTracingEntryPack(
    ASTBuilder* astBuilder,
    Type* entryListType);

/// Returns the executable stage represented by the interface's AST type, or Stage::Unknown.
Stage getStructuralRayTracingStageKind(InterfaceDecl* interfaceDecl);

/// Returns whether the interface represents an executable stage with an `invoke` requirement.
bool isExecutableStructuralRayTracingStageInterface(InterfaceDecl* interfaceDecl);

/// Returns the stage named by the input struct's KnownBuiltin annotation, or Stage::Unknown.
Stage getStructuralRayTracingStageInputKind(AggTypeDecl* typeDecl);

/// Returns the stage identified by a KnownBuiltin role, or Stage::Unknown for other roles.
Stage getStructuralRayTracingStageInputKind(KnownBuiltinDeclName builtin);

/// Returns whether the function is an annotated structural callable operation.
bool isStructuralRayTracingCallShaderMethod(FunctionDeclBase* functionDecl);

/// Returns this executable interface's `invoke` requirement, or null for other interfaces.
FunctionDeclBase* getStructuralRayTracingStageInvokeRequirement(InterfaceDecl* interfaceDecl);

/// Returns the source declaration path used as the name hint for a structural ray-tracing type.
///
/// The path includes namespaces and enclosing types, but excludes module names. A closed generic
/// specialization gains a suffix derived from its canonical semantic type, so two specializations
/// cannot silently claim the same structural entry-point name.
String getStructuralRayTracingSourceTypeName(ASTBuilder* astBuilder, Type* type);

/// Returns the deterministic target symbol for a structural ray-tracing type and selected stage.
///
/// Dots in the source type name become underscores. The canonical stage name is appended so a
/// struct implementing multiple stages gets distinct target symbols while retaining its
/// source-level lookup name.
String getStructuralRayTracingEntryPointName(UnownedStringSlice sourceTypeName, Stage stage);

class EntryPoint;
struct IRModule;
struct IRFunc;

/// Sets the public and target names for an entry point selected by its source stage type.
void initializeStructuralRayTracingEntryPoint(
    EntryPoint* entryPoint,
    Name* entryPointName,
    Type* stageType);

/// Rejects structural source representations that remain before native ABI legalization.
SlangResult diagnoseUnloweredStructuralRayTracing(
    IRModule* module,
    const List<IRFunc*>& entryPoints,
    DiagnosticSink* sink);

} // namespace Slang
