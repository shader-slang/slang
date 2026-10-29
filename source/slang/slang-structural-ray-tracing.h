#pragma once

#include "slang-ast-support-types.h"
#include "slang-compiler-fwd.h"
#include "slang-profile.h"

namespace Slang
{

class InterfaceDecl;
class FunctionDeclBase;
class AggTypeDecl;
class Decl;
class Type;
class ASTBuilder;
class MagicTypeModifier;

/// Allows the ray-tracing stage-interface and descriptor AST types in imported library
/// declarations.
bool isRayTracingBuiltinTypeModifier(MagicTypeModifier* modifier, Decl* decl);

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
