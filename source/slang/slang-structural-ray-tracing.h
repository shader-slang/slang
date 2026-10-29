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
struct SemanticsVisitor;

// These checks inspect already-checked declarations, types, and expressions. The checks taking
// a visitor use its existing diagnoseOnce state to avoid reporting the same error twice.

/// Checks the additional stage-implementation restrictions after ordinary conformance checking.
void checkStructuralRayTracingStageRestrictions(
    SemanticsVisitor* visitor,
    DeclRef<InterfaceDecl> superInterfaceDeclRef,
    WitnessTable* witnessTable,
    SourceLoc conformanceLoc);

/// Diagnoses repeated program types in a checked schema section.
void diagnoseDuplicateStructuralRayTracingSchemaEntries(
    ASTBuilder* astBuilder,
    DiagnosticSink* sink,
    Type* entryListType,
    AssocTypeDecl* associatedTypeRequirement,
    Type* schemaType,
    SourceLoc satisfyingDeclLoc);

/// Checks whether the variable's storage, parameter direction, and initializer allow an RT input.
void diagnoseInvalidStructuralRayTracingVariableType(
    SemanticsVisitor* visitor,
    VarDeclBase* varDecl);

/// Diagnoses restricted RT types used as function results.
void diagnoseInvalidStructuralRayTracingReturnType(DiagnosticSink* sink, CallableDecl* decl);

/// Diagnoses restricted RT types used as property results.
void diagnoseInvalidStructuralRayTracingPropertyType(DiagnosticSink* sink, PropertyDecl* decl);

/// Diagnoses forbidden RT input construction or a call returning a restricted RT type.
bool diagnoseInvalidStructuralRayTracingExpression(SemanticsVisitor* visitor, Expr* expr);

/// Diagnoses restricted RT types in the selected call's explicit or inferred generic arguments.
bool diagnoseInvalidStructuralRayTracingGenericArguments(DiagnosticSink* sink, InvokeExpr* invoke);

/// Diagnoses restricted RT types in an explicit generic type application.
bool diagnoseInvalidStructuralRayTracingGenericTypeApplication(
    DiagnosticSink* sink,
    GenericAppExpr* genericApplication);

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
