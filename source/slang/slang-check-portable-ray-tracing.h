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
class EntryPoint;

namespace PortableRT
{

// These checks inspect already-checked declarations, types, and expressions. The checks taking
// a visitor use its existing diagnoseOnce state to avoid reporting the same error twice.

/// Checks the additional stage-implementation restrictions after ordinary conformance checking.
void checkStageRestrictions(
    SemanticsVisitor* visitor,
    DeclRef<InterfaceDecl> superInterfaceDeclRef,
    WitnessTable* witnessTable,
    SourceLoc conformanceLoc);

/// Diagnoses repeated program types in a checked schema section.
void diagnoseDuplicateSchemaEntries(
    ASTBuilder* astBuilder,
    DiagnosticSink* sink,
    Type* entryListType,
    AssocTypeDecl* associatedTypeRequirement,
    Type* schemaType,
    SourceLoc satisfyingDeclLoc);

/// Checks whether the variable's storage, parameter direction, and initializer allow an RT input.
void diagnoseInvalidVariableType(SemanticsVisitor* visitor, VarDeclBase* varDecl);

/// Diagnoses restricted RT types used as function results.
void diagnoseInvalidReturnType(DiagnosticSink* sink, CallableDecl* decl);

/// Diagnoses restricted RT types used as property results.
void diagnoseInvalidPropertyType(DiagnosticSink* sink, PropertyDecl* decl);

/// Diagnoses forbidden RT input construction or a call returning a restricted RT type.
bool diagnoseInvalidExpression(SemanticsVisitor* visitor, Expr* expr);

/// Diagnoses restricted RT types in the selected call's explicit or inferred generic arguments.
bool diagnoseInvalidGenericArguments(DiagnosticSink* sink, InvokeExpr* invoke);

/// Diagnoses restricted RT types in an explicit generic type application.
bool diagnoseInvalidGenericTypeApplication(
    DiagnosticSink* sink,
    GenericAppExpr* genericApplication);

/// Allows the ray-tracing stage-interface and descriptor AST types in imported library
/// declarations.
bool isBuiltinTypeModifier(MagicTypeModifier* modifier, Decl* decl);

/// Returns the executable stage represented by the interface's AST type, or Stage::Unknown.
Stage getStageKind(InterfaceDecl* interfaceDecl);

/// Returns whether the interface represents an executable stage with an `invoke` requirement.
bool isExecutableStageInterface(InterfaceDecl* interfaceDecl);

/// Returns the stage named by the input struct's KnownBuiltin annotation, or Stage::Unknown.
Stage getStageInputKind(AggTypeDecl* typeDecl);

/// Returns this executable interface's `invoke` requirement, or null for other interfaces.
FunctionDeclBase* getStageInvokeRequirement(InterfaceDecl* interfaceDecl);

/// Returns the type name used to form a structural ray-tracing entry-point symbol.
///
/// Nongeneric types use the source declaration path, including namespaces and enclosing types
/// but excluding module names. Generic specializations use ordinary type mangling to preserve
/// the selected arguments.
String getSourceTypeName(ASTBuilder* astBuilder, Type* stageType);

/// Returns the deterministic target symbol for a structural ray-tracing type and selected stage.
///
/// Dots in the source type name become underscores. The canonical stage name is appended so a
/// struct implementing multiple stages gets distinct target symbols while retaining its
/// source-level lookup name.
String getEntryPointName(UnownedStringSlice sourceTypeName, Stage stage);


/// Selects the checked stage implementation named by a stage type or its invoke member.
DeclRef<FuncDecl> findEntryPointByName(
    Linkage* linkage,
    Module* module,
    Name* name,
    Profile& ioProfile,
    DiagnosticSink* sink,
    bool* outFoundStruct,
    Type** outStageType);

/// Diagnoses stage inputs exposed by a selected native entry-point signature.
void diagnoseInputInNativeEntryPoint(EntryPoint* entryPoint, DiagnosticSink* sink);

/// Sets the public and target names for an entry point selected by its source stage type.
void initializeEntryPoint(EntryPoint* entryPoint, Name* entryPointName, Type* stageType);

} // namespace PortableRT
} // namespace Slang
