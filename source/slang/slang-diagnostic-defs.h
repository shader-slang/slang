//

// The file is meant to be included multiple times, to produce different
// pieces of declaration/definition code related to diagnostic messages
//
// Each diagnostic is declared here with:
//
//     DIAGNOSTIC(id, severity, name, messageFormat)
//
// Where `id` is the unique diagnostic ID, `severity` is the default
// severity (from the `Severity` enum), `name` is a name used to refer
// to this diagnostic from code, and `messageFormat` is the default
// (non-localized) message for the diagnostic, with placeholders
// for any arguments.

#ifndef DIAGNOSTIC
#error Need to #define DIAGNOSTIC(...) before including "DiagnosticDefs.h"
#define DIAGNOSTIC(id, severity, name, messageFormat) /* */
#endif

//
// -1 - Notes that decorate another diagnostic.
//

DIAGNOSTIC(-1, Note, seeDefinitionOf, "see definition of '$0'")
DIAGNOSTIC(-1, Note, seeDefinitionOfStruct, "see definition of struct '$0'")
DIAGNOSTIC(-1, Note, seeConstantBufferDefinition, "see constant buffer definition.")
DIAGNOSTIC(-1, Note, seeUsingOf, "see using of '$0'")
DIAGNOSTIC(-1, Note, seeCallOfFunc, "see call to '$0'")
DIAGNOSTIC(-1, Note, seePreviousDefinition, "see previous definition")
DIAGNOSTIC(-1, Note, seePreviousDefinitionOf, "see previous definition of '$0'")

DIAGNOSTIC(-1, Note, seeDeclarationOf, "see declaration of '$0'")
DIAGNOSTIC(
    -1,
    Note,
    seeDeclarationOfInterfaceRequirement,
    "see interface requirement declaration of '$0'")

DIAGNOSTIC(-1, Note, seeOverloadConsidered, "see overloads considered: '$0'.")

// An alternate wording of the above note, emphasing the position rather than content of the
// declaration.
DIAGNOSTIC(-1, Note, seePreviousDeclarationOf, "see previous declaration of '$0'")

//
// 0xxxx - Command line and interaction with host platform APIs.
//

//
// 15xxx - Preprocessing
//

// 150xx - conditionals
// (definitions moved to slang-diagnostics.lua)

// 151xx - directive parsing
// (definitions moved to slang-diagnostics.lua)

// 152xx - preprocessor expressions
// (definitions moved to slang-diagnostics.lua)

// 153xx - #include
// (definitions moved to slang-diagnostics.lua)

// 154xx - macro definition
// (definitions moved to slang-diagnostics.lua)

// 155xx - macro expansion
// (definitions moved to slang-diagnostics.lua)

// 156xx - pragmas
// (definitions moved to slang-diagnostics.lua)

// 159xx - user-defined error/warning
// (definitions moved to slang-diagnostics.lua)

//
// 2xxxx - Parsing
// (definitions moved to slang-diagnostics.lua)

// 29xxx - Snippet parsing and inline asm
// (definitions moved to slang-diagnostics.lua)

//
// 3xxxx - Semantic analysis
//

// 300xx - 303xx: definitions moved to slang-diagnostics-semantic-checking-1.lua

// Some diagnostics that were already converted earlier (kept here until migration complete):
DIAGNOSTIC(30013, Error, subscriptNonArray, "no subscript operation found for type '$0'")
DIAGNOSTIC(30019, Error, typeMismatch, "expected an expression of type '$0', got '$1'")
DIAGNOSTIC(
    30024,
    Error,
    cannotConvertArrayOfSmallerToLargerSize,
    "Cannot convert array of size $0 to array of size $1 as this would truncate data")
DIAGNOSTIC(30052, Error, invalidSwizzleExpr, "invalid swizzle pattern '$0' on type '$1'")
DIAGNOSTIC(30201, Error, functionRedefinition, "function '$0' already has a body")

// 3xx7x - 3011x: definitions moved to slang-diagnostics-semantic-checking-2.lua

// Note: noteExplicitConversionPossible is kept here because it's used with
// diagnoseWithoutSourceView which doesn't support rich diagnostics
DIAGNOSTIC(
    -1,
    Note,
    noteExplicitConversionPossible,
    "explicit conversion from '$0' to '$1' is possible")

// Include
// (definitions moved to slang-diagnostics-semantic-checking-3.lua)

// Visibility
// (definitions moved to slang-diagnostics-semantic-checking-3.lua)

// Capability
// (definitions moved to slang-diagnostics-semantic-checking-3.lua)

// Attributes
// (definitions moved to slang-diagnostics-semantic-checking-4.lua)

// COM Interface, DerivativeMember, Extern Decl, Custom Derivative
// (definitions moved to slang-diagnostics-semantic-checking-5.lua)

// Differentiation, Modifiers, GLSL/HLSL specifics, Interfaces, Control flow, Enums, Generics
// (definitions moved to slang-diagnostics-semantic-checking-6.lua)

// Link Time, Cyclic Refs, Generics, Initializers, Variables, Parameters, Inheritance, Extensions, Subscripts
// (definitions moved to slang-diagnostics-semantic-checking-7.lua)

// 310xx: properties

// 311xx: accessors - converted to slang-diagnostics-semantic-checking-8.lua
// 313xx: bit fields - converted to slang-diagnostics-semantic-checking-8.lua
// 39999: converted to slang-diagnostics-semantic-checking-8.lua

// Diagnostics 229-333 moved to slang-diagnostics-semantic-checking-9.lua:
// expectedPrefixOperator, expectedPostfixOperator, notEnoughArguments, tooManyArguments,
// invalidIntegerLiteralSuffix, invalidFloatingPointLiteralSuffix, integerLiteralTooLarge,
// integerLiteralTruncated, floatLiteralUnrepresentable, floatLiteralTooSmall,
// matrixColumnOrRowCountIsOne, entryPointFunctionNotFound, expectedTypeForSpecializationArg,
// specifiedStageDoesntMatchAttribute, entryPointHasNoStage, specializationParameterOfNameNotSpecialized,
// specializationParameterNotSpecialized, expectedValueOfTypeForSpecializationArg,
// unhandledModOnEntryPointParameter, entryPointCannotReturnResourceType

// 381xx, 380xx, 382xx diagnostics have been moved to slang-diagnostics-semantic-checking-10.lua

// 4xxxx IL code generation diagnostics have been moved to slang-diagnostics-semantic-checking-12.lua

// Diagnostics 41011-41403 have been converted to rich diagnostics in
// slang-diagnostics-semantic-checking-13.lua

// 5xxxx - Target code generation diagnostics (50010-56003) have been converted to rich diagnostics in
// slang-diagnostics-semantic-checking-14.lua

// Metal
DIAGNOSTIC(
    56101,
    Error,
    resourceTypesInConstantBufferInParameterBlockNotAllowedOnMetal,
    "nesting a 'ConstantBuffer' containing resource types inside a 'ParameterBlock' is not "
    "supported on Metal, please use 'ParameterBlock' instead.")
DIAGNOSTIC(
    56102,
    Error,
    divisionByMatrixNotSupported,
    "division by matrix is not supported for Metal and WGSL targets.")
DIAGNOSTIC(
    56103,
    Error,
    int16NotSupportedInWGSL,
    "16-bit integer type '$0' is not supported by the WGSL backend.")
DIAGNOSTIC(
    56104,
    Error,
    assignToRefNotSupported,
    "whole struct must be assiged to mesh output at once for Metal target.")

DIAGNOSTIC(57001, Warning, spirvOptFailed, "spirv-opt failed. $0")
DIAGNOSTIC(57002, Error, unknownPatchConstantParameter, "unknown patch constant parameter '$0'.")
DIAGNOSTIC(57003, Error, unknownTessPartitioning, "unknown tessellation partitioning '$0'.")
DIAGNOSTIC(
    57004,
    Error,
    outputSpvIsEmpty,
    "output SPIR-V contains no exported symbols. Please make sure to specify at least one "
    "entrypoint.")

// GLSL Compatibility
DIAGNOSTIC(
    58001,
    Error,
    entryPointMustReturnVoidWhenGlobalOutputPresent,
    "entry point must return 'void' when global output variables are present.")
DIAGNOSTIC(
    58002,
    Error,
    unhandledGLSLSSBOType,
    "Unhandled GLSL Shader Storage Buffer Object contents, unsized arrays as a final parameter "
    "must be the only parameter")

DIAGNOSTIC(
    58003,
    Error,
    inconsistentPointerAddressSpace,
    "'$0': use of pointer with inconsistent address space.")

// Autodiff checkpoint reporting
DIAGNOSTIC(
    -1,
    Note,
    reportCheckpointIntermediates,
    "checkpointing context of $1 bytes associated with function: '$0'")
DIAGNOSTIC(
    -1,
    Note,
    reportCheckpointVariable,
    "$0 bytes ($1) used to checkpoint the following item:")
DIAGNOSTIC(-1, Note, reportCheckpointCounter, "$0 bytes ($1) used for a loop counter here:")
DIAGNOSTIC(-1, Note, reportCheckpointNone, "no checkpoint contexts to report")

// 9xxxx - Documentation generation
DIAGNOSTIC(
    90001,
    Warning,
    ignoredDocumentationOnOverloadCandidate,
    "documentation comment on overload candidate '$0' is ignored")

//
// 8xxxx - Issues specific to a particular library/technology/platform/etc.
//

// 811xx - NVAPI

DIAGNOSTIC(
    81110,
    Error,
    nvapiMacroMismatch,
    "conflicting definitions for NVAPI macro '$0': '$1' and '$2'")

DIAGNOSTIC(
    81111,
    Error,
    opaqueReferenceMustResolveToGlobal,
    "could not determine register/space for a resource or sampler used with NVAPI")

// 99999 - Internal compiler errors, and not-yet-classified diagnostics.

DIAGNOSTIC(
    99999,
    Internal,
    unimplemented,
    "unimplemented feature in Slang compiler: $0\nFor assistance, file an issue on GitHub "
    "(https://github.com/shader-slang/slang/issues) or join the Slang Discord "
    "(https://khr.io/slangdiscord)")
DIAGNOSTIC(
    99999,
    Internal,
    unexpected,
    "unexpected condition encountered in Slang compiler: $0\nFor assistance, file an issue on "
    "GitHub "
    "(https://github.com/shader-slang/slang/issues) or join the Slang Discord "
    "(https://khr.io/slangdiscord)")
DIAGNOSTIC(
    99999,
    Internal,
    internalCompilerError,
    "Slang internal compiler error\nFor assistance, file an issue on GitHub "
    "(https://github.com/shader-slang/slang/issues) or join the Slang Discord "
    "(https://khr.io/slangdiscord)")
DIAGNOSTIC(
    99999,
    Error,
    compilationAborted,
    "Slang compilation aborted due to internal error\nFor assistance, file an issue on GitHub "
    "(https://github.com/shader-slang/slang/issues) or join the Slang Discord "
    "(https://khr.io/slangdiscord)")
DIAGNOSTIC(
    99999,
    Error,
    compilationAbortedDueToException,
    "Slang compilation aborted due to an exception of $0: $1\nFor assistance, file an issue on "
    "GitHub "
    "(https://github.com/shader-slang/slang/issues) or join the Slang Discord "
    "(https://khr.io/slangdiscord)")
DIAGNOSTIC(
    99999,
    Internal,
    serialDebugVerificationFailed,
    "Verification of serial debug information failed.")
DIAGNOSTIC(99999, Internal, spirvValidationFailed, "Validation of generated SPIR-V failed.")

DIAGNOSTIC(
    99999,
    Internal,
    noBlocksOrIntrinsic,
    "no blocks found for function definition, is there a '$0' intrinsic missing?")

DIAGNOSTIC(
    40100,
    Warning,
    mainEntryPointRenamed,
    "entry point '$0' is not allowed, and has been renamed to '$1'")

//
// Ray tracing
//

DIAGNOSTIC(
    40000,
    Error,
    rayPayloadFieldMissingAccessQualifiers,
    "field '$0' in ray payload struct must have either 'read' OR 'write' access qualifiers")
DIAGNOSTIC(
    40001,
    Error,
    rayPayloadInvalidStageInAccessQualifier,
    "invalid stage name '$0' in ray payload access qualifier; valid stages are 'anyhit', "
    "'closesthit', 'miss', and 'caller'")

//
// Cooperative Matrix
//
DIAGNOSTIC(
    50000,
    Error,
    cooperativeMatrixUnsupportedElementType,
    "Element type '$0' is not supported for matrix'$1'.")

DIAGNOSTIC(
    50000,
    Error,
    cooperativeMatrixInvalidShape,
    "Invalid shape ['$0', '$1'] for cooperative matrix'$2'.")

DIAGNOSTIC(
    51701,
    Fatal,
    cooperativeMatrixUnsupportedCapture,
    "'CoopMat.MapElement' per-element function cannot capture buffers, resources or any opaque "
    "type values. Consider pre-loading the content of any referenced buffers into a local variable "
    "before calling 'CoopMat.MapElement', or moving any referenced resources to global scope.")

#undef DIAGNOSTIC
