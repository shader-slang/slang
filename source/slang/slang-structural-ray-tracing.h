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

/// Returns this executable interface's `invoke` requirement, or null for other interfaces.
FunctionDeclBase* getStructuralRayTracingStageInvokeRequirement(InterfaceDecl* interfaceDecl);

/// Returns the type name used to form a structural ray-tracing entry-point symbol.
///
/// Nongeneric types use the source declaration path, including namespaces and enclosing types
/// but excluding module names. Generic specializations use ordinary type mangling to preserve
/// the selected arguments.
String getStructuralRayTracingSourceTypeName(ASTBuilder* astBuilder, Type* stageType);

/// Returns the deterministic target symbol for a structural ray-tracing type and selected stage.
///
/// Dots in the source type name become underscores. The canonical stage name is appended so a
/// struct implementing multiple stages gets distinct target symbols while retaining its
/// source-level lookup name.
String getStructuralRayTracingEntryPointName(UnownedStringSlice sourceTypeName, Stage stage);

class EntryPoint;

/// Sets the public and target names for an entry point selected by its source stage type.
void initializeStructuralRayTracingEntryPoint(
    EntryPoint* entryPoint,
    Name* entryPointName,
    Type* stageType);

} // namespace Slang
