// slang-lower-to-ir.h
#ifndef SLANG_LOWER_TO_IR_H_INCLUDED
#define SLANG_LOWER_TO_IR_H_INCLUDED

// The lowering step translates from a (type-checked) AST into
// our intermediate representation, to facilitate further
// optimization and transformation.

#include "../core/slang-basic.h"
#include "slang-compiler.h"
#include "slang-ir.h"

namespace Slang
{
class EntryPoint;
class ProgramLayout;
class SpecializedComponentType;
class TranslationUnitRequest;

/// Generate an IR module to represent the code in the given `translationUnit`.
///
/// The generated module will include IR definitions for any functions/types
/// in `translationUnit`, but it is *not* guaranteed to contain any definitions
/// from modules that are `import`ed into `translationUnit`. The resulting IR
/// module must be linked against other IR modules that define any symbols
/// that are imported before code generation can be performed.
///
RefPtr<IRModule> generateIRForTranslationUnit(
    ASTBuilder* astBuilder,
    TranslationUnitRequest* translationUnit);

/// Generate an IR module to represent the specializations applied by `componentType`.
///
/// The generated IR will encode how `componentType` specializes global or
/// entry-point specialization parameters to concrete arguments (e.g., types).
///
/// The generated IR module is *not* guaranteed to contain anything more, such
/// as the actual definitions of functions or types being specialized. The
/// resulting IR module must be linked against other IR modules that define
/// those symbols before code generation can be performed.
///
RefPtr<IRModule> generateIRForSpecializedComponentType(
    SpecializedComponentType* componentType,
    DiagnosticSink* sink);

/// Generate an IR module to represent a user specified `TypeConformance` component type.
/// The generated IR will include an extern symbol representing the type conformance
/// (typically a `IRWitnessTable` or a `specialize(IRWitnessTable)` inst), with a `public`
/// decoration to keep the referenced witness table alive during linking.
RefPtr<IRModule> generateIRForTypeConformance(
    TypeConformance* typeConformance,
    Int conformanceIdOverride,
    DiagnosticSink* sink);

/// Generate IR for an entry point wrapper function and add it to an existing IR module.
///
/// This function creates an entry point wrapper with the appropriate signature
/// (replacing `varying in` parameters with `borrow in` parameters) that calls
/// through to the ordinary function. This allows the same function to be used
/// both as an entry point and as a callable function.
///
/// This is typically called during initial IR generation for entry points discovered
/// via [shader(...)] attributes, but can also be called after module loading to add
/// entry points discovered via API (e.g., findAndCheckEntryPoint).
///
void addEntryPointToExistingIR(
    IRModule* irModule,
    EntryPoint* entryPoint,
    String moduleName,
    DiagnosticSink* sink);
} // namespace Slang
#endif
