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

// Per-compilation state for the post-specialization stage-input checks: the set of generic
// instantiations whose bodies were already validated after substitution (specialized decl-refs
// are interned Vals, so pointer identity distinguishes instantiations), and a cached scan of
// the loaded modules for the stage-input declarations.
class PortableRTSpecializationCheckState : public RefObject
{
public:
    HashSet<Val*> checkedSpecializationDeclRefs;
    Index loadedModuleCountAtScan = -1;
    bool stageInputsLoaded = false;
};

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

/// Checks whether the variable's storage, parameter direction, and initializer allow an RT
/// input. The visitor resolves substituted types whose conformances the use site may precede.
void diagnoseInvalidVariableType(
    SemanticsVisitor* visitor,
    DiagnosticSink* sink,
    VarDeclBase* varDecl);

/// Diagnoses restricted RT types used as function results.
void diagnoseInvalidReturnType(SemanticsVisitor* visitor, DiagnosticSink* sink, CallableDecl* decl);

/// Diagnoses restricted RT types used as property results.
void diagnoseInvalidPropertyType(
    SemanticsVisitor* visitor,
    DiagnosticSink* sink,
    PropertyDecl* decl);

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

/// Checks a generic callable's body for stage-input uses that its declaration-time checking could
/// not see: `specializedCallableDeclRef` is the callable's checked declaration reference with the
/// concrete generic arguments bound (a `GenericAppDeclRef` in its substitution chain). A null or
/// unsubstituted reference exits without diagnosing. `instantiationLoc` names the call or
/// specialization that bound the arguments, and is reported beside any diagnostic found inside the
/// generic body so the reader can see which concrete call made the type known.
void checkSpecializedCallableBody(
    SemanticsVisitor* visitor,
    DeclRefBase* specializedCallableDeclRef,
    SourceLoc instantiationLoc);

/// Sets the public and target names for an entry point selected by its source stage type.
void initializeEntryPoint(EntryPoint* entryPoint, Name* entryPointName, Type* stageType);

} // namespace PortableRT
} // namespace Slang
