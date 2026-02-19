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

DIAGNOSTIC(
    31158,
    Error,
    primalSubstituteTargetMustHaveHigherDifferentiabilityLevel,
    "primal substitute function for differentiable method must also be differentiable. Use "
    "[Differentiable] or [TreatAsDifferentiable] (for empty derivatives)")
DIAGNOSTIC(
    31159,
    Warning,
    noDerivativeOnNonDifferentiableThisType,
    "There is no derivative calculated for member '$0' because the parent struct is not "
    "differentiable. "
    "If this is intended, consider using [NoDiffThis] on the function '$1' to suppress this "
    "warning. Alternatively, users can mark the parent struct as [Differentiable] to propagate "
    "derivatives.")
DIAGNOSTIC(
    31160,
    Error,
    invalidAddressOf,
    "'__getAddress' only supports groupshared variables and members of groupshared/device memory.")
DIAGNOSTIC(31200, Warning, deprecatedUsage, "$0 has been deprecated: $1")
DIAGNOSTIC(31201, Error, modifierNotAllowed, "modifier '$0' is not allowed here.")
DIAGNOSTIC(
    31202,
    Error,
    duplicateModifier,
    "modifier '$0' is redundant or conflicting with existing modifier '$1'")
DIAGNOSTIC(31203, Error, cannotExportIncompleteType, "cannot export incomplete type '$0'")
DIAGNOSTIC(
    31206,
    Error,
    memoryQualifierNotAllowedOnANonImageTypeParameter,
    "modifier $0 is not allowed on a non image type parameter.")
DIAGNOSTIC(
    31208,
    Error,
    requireInputDecoratedVarForParameter,
    "$0 expects for argument $1 a type which is a shader input (`in`) variable.")
DIAGNOSTIC(
    31210,
    Error,
    derivativeGroupQuadMustBeMultiple2ForXYThreads,
    "compute derivative group quad requires thread dispatch count of X and Y to each be at a "
    "multiple of 2")
DIAGNOSTIC(
    31211,
    Error,
    derivativeGroupLinearMustBeMultiple4ForTotalThreadCount,
    "compute derivative group linear requires total thread dispatch count to be at a multiple of 4")
DIAGNOSTIC(
    31212,
    Error,
    onlyOneOfDerivativeGroupLinearOrQuadCanBeSet,
    "cannot set compute derivative group linear and compute derivative group quad at the same time")
DIAGNOSTIC(
    31213,
    Error,
    cudaKernelMustReturnVoid,
    "return type of a CUDA kernel function cannot be non-void.")
DIAGNOSTIC(
    31214,
    Error,
    differentiableKernelEntryPointCannotHaveDifferentiableParams,
    "differentiable kernel entry point cannot have differentiable parameters. Consider using "
    "DiffTensorView to pass differentiable data, or marking this parameter with 'no_diff'")
DIAGNOSTIC(
    31215,
    Error,
    cannotUseUnsizedTypeInConstantBuffer,
    "cannot use unsized type '$0' in a constant buffer.")
DIAGNOSTIC(31216, Error, unrecognizedGLSLLayoutQualifier, "GLSL layout qualifier is unrecognized")
DIAGNOSTIC(
    31217,
    Error,
    unrecognizedGLSLLayoutQualifierOrRequiresAssignment,
    "GLSL layout qualifier is unrecognized or requires assignment")
DIAGNOSTIC(
    31218,
    Error,
    specializationConstantMustBeScalar,
    "specialization constant must be a scalar.")
DIAGNOSTIC(
    31219,
    Error,
    pushOrSpecializationConstantCannotBeStatic,
    "push or specialization constants cannot be 'static'.")
DIAGNOSTIC(
    31220,
    Error,
    variableCannotBePushAndSpecializationConstant,
    "'$0' cannot be a push constant and a specialization constant at the same time")
DIAGNOSTIC(31221, Error, invalidHLSLRegisterName, "invalid HLSL register name '$0'.")
DIAGNOSTIC(
    31222,
    Error,
    invalidHLSLRegisterNameForType,
    "invalid HLSL register name '$0' for type '$1'.")
DIAGNOSTIC(
    31223,
    Error,
    ExternAndExportVarDeclMustBeConst,
    "extern and export variables must be static const: '$0'")

DIAGNOSTIC(
    31224,
    Error,
    constGlobalVarWithInitRequiresStatic,
    "global const variable with initializer must be declared static: '$0'")

DIAGNOSTIC(
    31225,
    Error,
    staticConstVariableRequiresInitializer,
    "static const variable '$0' must have an initializer")

// Enums

DIAGNOSTIC(32000, Error, invalidEnumTagType, "invalid tag type for 'enum': '$0'")
DIAGNOSTIC(32003, Error, unexpectedEnumTagExpr, "unexpected form for 'enum' tag value expression")

// 303xx: interfaces and associated types
DIAGNOSTIC(
    30300,
    Error,
    assocTypeInInterfaceOnly,
    "'associatedtype' can only be defined in an 'interface'.")
DIAGNOSTIC(
    30301,
    Error,
    globalGenParamInGlobalScopeOnly,
    "'type_param' can only be defined global scope.")
DIAGNOSTIC(
    30302,
    Error,
    staticConstRequirementMustBeIntOrBool,
    "'static const' requirement can only have int or bool type.")
DIAGNOSTIC(
    30303,
    Error,
    valueRequirementMustBeCompileTimeConst,
    "requirement in the form of a simple value must be declared as 'static const'.")
DIAGNOSTIC(30310, Error, typeIsNotDifferentiable, "type '$0' is not differentiable.")

DIAGNOSTIC(
    30311,
    Error,
    nonMethodInterfaceRequirementCannotHaveBody,
    "non-method interface requirement cannot have a body.")
DIAGNOSTIC(
    30312,
    Error,
    interfaceRequirementCannotBeOverride,
    "interface requirement cannot override a base declaration.")

// Interop
DIAGNOSTIC(
    30400,
    Error,
    cannotDefinePtrTypeToManagedResource,
    "pointer to a managed resource is invalid, use `NativeRef<T>` instead")

// Control flow
DIAGNOSTIC(
    30500,
    Warning,
    forLoopSideEffectChangingDifferentVar,
    "the for loop initializes and checks variable '$0' but the side effect expression is modifying "
    "'$1'.")
DIAGNOSTIC(
    30501,
    Warning,
    forLoopPredicateCheckingDifferentVar,
    "the for loop initializes and modifies variable '$0' but the predicate expression is checking "
    "'$1'.")
DIAGNOSTIC(
    30502,
    Warning,
    forLoopChangingIterationVariableInOppsoiteDirection,
    "the for loop is modifiying variable '$0' in the opposite direction from loop exit condition.")
DIAGNOSTIC(
    30503,
    Warning,
    forLoopNotModifyingIterationVariable,
    "the for loop is not modifiying variable '$0' because the step size evaluates to 0.")
DIAGNOSTIC(
    30504,
    Warning,
    forLoopTerminatesInFewerIterationsThanMaxIters,
    "the for loop is statically determined to terminate within $0 iterations, which is less than "
    "what [MaxIters] specifies.")
DIAGNOSTIC(
    30505,
    Warning,
    loopRunsForZeroIterations,
    "the loop runs for 0 iterations and will be removed.")
DIAGNOSTIC(
    30510,
    Error,
    loopInDiffFuncRequireUnrollOrMaxIters,
    "loops inside a differentiable function need to provide either '[MaxIters(n)]' or "
    "'[ForceUnroll]' attribute.")

// Switch
DIAGNOSTIC(
    30600,
    Error,
    switchMultipleDefault,
    "multiple 'default' cases not allowed within a 'switch' statement")
DIAGNOSTIC(
    30601,
    Error,
    switchDuplicateCases,
    "duplicate cases not allowed within a 'switch' statement")

// 310xx: properties

// 311xx: accessors

DIAGNOSTIC(
    31100,
    Error,
    accessorMustBeInsideSubscriptOrProperty,
    "an accessor declaration is only allowed inside a subscript or property declaration")

DIAGNOSTIC(
    31101,
    Error,
    nonSetAccessorMustNotHaveParams,
    "accessors other than 'set' must not have parameters")
DIAGNOSTIC(
    31102,
    Error,
    setAccessorMayNotHaveMoreThanOneParam,
    "a 'set' accessor may not have more than one parameter")
DIAGNOSTIC(
    31102,
    Error,
    setAccessorParamWrongType,
    "'set' parameter '$0' has type '$1' which does not match the expected type '$2'")

// 313xx: bit fields
DIAGNOSTIC(
    31300,
    Error,
    bitFieldTooWide,
    "bit-field size ($0) exceeds the width of its type $1 ($2)")
DIAGNOSTIC(31301, Error, bitFieldNonIntegral, "bit-field type ($0) must be an integral type")

// 39999 waiting to be placed in the right range

DIAGNOSTIC(
    39999,
    Error,
    expectedIntegerConstantNotConstant,
    "expression does not evaluate to a compile-time constant")
DIAGNOSTIC(
    39999,
    Error,
    expectedIntegerConstantNotLiteral,
    "could not extract value from integer constant")

DIAGNOSTIC(
    39999,
    Error,
    expectedRayTracingPayloadObjectAtLocationButMissing,
    "raytracing payload expected at location $0 but it is missing")

DIAGNOSTIC(
    39999,
    Error,
    noApplicableOverloadForNameWithArgs,
    "no overload for '$0' applicable to arguments of type $1")
DIAGNOSTIC(39999, Error, noApplicableWithArgs, "no overload applicable to arguments of type $0")

DIAGNOSTIC(
    39999,
    Error,
    ambiguousOverloadForNameWithArgs,
    "ambiguous call to '$0' with arguments of type $1")
DIAGNOSTIC(
    39999,
    Error,
    ambiguousOverloadWithArgs,
    "ambiguous call to overloaded operation with arguments of type $0")

DIAGNOSTIC(39999, Note, overloadCandidate, "candidate: $0")
DIAGNOSTIC(39999, Note, invisibleOverloadCandidate, "candidate (invisible): $0")

DIAGNOSTIC(39999, Note, moreOverloadCandidates, "$0 more overload candidates")

DIAGNOSTIC(39999, Error, caseOutsideSwitch, "'case' not allowed outside of a 'switch' statement")
DIAGNOSTIC(
    39999,
    Error,
    defaultOutsideSwitch,
    "'default' not allowed outside of a 'switch' statement")

DIAGNOSTIC(39999, Error, expectedAGeneric, "expected a generic when using '<...>' (found: '$0')")

DIAGNOSTIC(
    39999,
    Error,
    genericArgumentInferenceFailed,
    "could not specialize generic for arguments of type $0")

DIAGNOSTIC(39999, Error, ambiguousReference, "ambiguous reference to '$0'")
DIAGNOSTIC(39999, Error, ambiguousExpression, "ambiguous reference")

DIAGNOSTIC(39999, Error, declarationDidntDeclareAnything, "declaration does not declare anything")

DIAGNOSTIC(
    39999,
    Error,
    expectedPrefixOperator,
    "function called as prefix operator was not declared `__prefix`")
DIAGNOSTIC(
    39999,
    Error,
    expectedPostfixOperator,
    "function called as postfix operator was not declared `__postfix`")

DIAGNOSTIC(39999, Error, notEnoughArguments, "not enough arguments to call (got $0, expected $1)")
DIAGNOSTIC(39999, Error, tooManyArguments, "too many arguments to call (got $0, expected $1)")

DIAGNOSTIC(39999, Error, invalidIntegerLiteralSuffix, "invalid suffix '$0' on integer literal")
DIAGNOSTIC(
    39999,
    Error,
    invalidFloatingPointLiteralSuffix,
    "invalid suffix '$0' on floating-point literal")
DIAGNOSTIC(
    39999,
    Warning,
    integerLiteralTooLarge,
    "integer literal is too large to be represented in a signed integer type, interpreting as "
    "unsigned")

DIAGNOSTIC(
    39999,
    Warning,
    integerLiteralTruncated,
    "integer literal '$0' too large for type '$1' truncated to '$2'")
DIAGNOSTIC(
    39999,
    Warning,
    floatLiteralUnrepresentable,
    "$0 literal '$1' unrepresentable, converted to '$2'")
DIAGNOSTIC(
    39999,
    Warning,
    floatLiteralTooSmall,
    "'$1' is smaller than the smallest representable value for type $0, converted to '$2'")

DIAGNOSTIC(
    39999,
    Error,
    matrixColumnOrRowCountIsOne,
    "matrices with 1 column or row are not supported by the current code generation target")

// 38xxx

DIAGNOSTIC(
    38000,
    Error,
    entryPointFunctionNotFound,
    "no function found matching entry point name '$0'")

DIAGNOSTIC(
    38005,
    Error,
    expectedTypeForSpecializationArg,
    "expected a type as argument for specialization parameter '$0'")

DIAGNOSTIC(
    38006,
    Warning,
    specifiedStageDoesntMatchAttribute,
    "entry point '$0' being compiled for the '$1' stage has a '[shader(...)]' attribute that "
    "specifies the '$2' stage")
DIAGNOSTIC(
    38007,
    Error,
    entryPointHasNoStage,
    "no stage specified for entry point '$0'; use either a '[shader(\"name\")]' function attribute "
    "or the '-stage <name>' command-line option to specify a stage")

DIAGNOSTIC(
    38008,
    Error,
    specializationParameterOfNameNotSpecialized,
    "no specialization argument was provided for specialization parameter '$0'")
DIAGNOSTIC(
    38008,
    Error,
    specializationParameterNotSpecialized,
    "no specialization argument was provided for specialization parameter")

DIAGNOSTIC(
    38009,
    Error,
    expectedValueOfTypeForSpecializationArg,
    "expected a constant value of type '$0' as argument for specialization parameter '$1'")

DIAGNOSTIC(
    38010,
    Warning,
    unhandledModOnEntryPointParameter,
    "$0 on parameter '$1' is unsupported on entry point parameters and will be ignored")

DIAGNOSTIC(
    38011,
    Error,
    entryPointCannotReturnResourceType,
    "entry point '$0' cannot return type '$1' that contains resource types")

DIAGNOSTIC(
    38100,
    Error,
    typeDoesntImplementInterfaceRequirement,
    "type '$0' does not provide required interface member '$1'")
DIAGNOSTIC(
    38105,
    Error,
    memberDoesNotMatchRequirementSignature,
    "member '$0' does not match interface requirement.")
DIAGNOSTIC(
    38106,
    Error,
    memberReturnTypeMismatch,
    "member '$0' return type '$1' does not match interface requirement return type '$2'.")
DIAGNOSTIC(
    38107,
    Error,
    genericSignatureDoesNotMatchRequirement,
    "generic signature of '$0' does not match interface requirement.")
DIAGNOSTIC(
    38108,
    Error,
    parameterDirectionDoesNotMatchRequirement,
    "parameter '$0' direction '$1' does not match interface requirement '$2'.")

DIAGNOSTIC(
    38101,
    Error,
    thisExpressionOutsideOfTypeDecl,
    "'this' expression can only be used in members of an aggregate type")
DIAGNOSTIC(
    38102,
    Error,
    initializerNotInsideType,
    "an 'init' declaration is only allowed inside a type or 'extension' declaration")
DIAGNOSTIC(
    38103,
    Error,
    thisTypeOutsideOfTypeDecl,
    "'This' type can only be used inside of an aggregate type")
DIAGNOSTIC(
    38104,
    Error,
    returnValNotAvailable,
    "cannot use '__return_val' here. '__return_val' is defined only in functions that return a "
    "non-copyable value.")
DIAGNOSTIC(
    38021,
    Error,
    typeArgumentForGenericParameterDoesNotConformToInterface,
    "type argument `$0` for generic parameter `$1` does not conform to interface `$2`.")

DIAGNOSTIC(
    38022,
    Error,
    cannotSpecializeGlobalGenericToItself,
    "the global type parameter '$0' cannot be specialized to itself")
DIAGNOSTIC(
    38023,
    Error,
    cannotSpecializeGlobalGenericToAnotherGenericParam,
    "the global type parameter '$0' cannot be specialized using another global type parameter "
    "('$1')")

DIAGNOSTIC(
    38024,
    Error,
    invalidDispatchThreadIDType,
    "parameter with SV_DispatchThreadID must be either scalar or vector (1 to 3) of uint/int but "
    "is $0")

DIAGNOSTIC(
    38025,
    Error,
    mismatchSpecializationArguments,
    "expected $0 specialization arguments ($1 provided)")

DIAGNOSTIC(
    38028,
    Error,
    invalidFormOfSpecializationArg,
    "global specialization argument $0 has an invalid form.")

DIAGNOSTIC(
    38029,
    Error,
    typeArgumentDoesNotConformToInterface,
    "type argument '$0' does not conform to the required interface '$1'")

DIAGNOSTIC(
    38031,
    Error,
    invalidUseOfNoDiff,
    "'no_diff' can only be used to decorate a call or a subscript operation")
DIAGNOSTIC(
    38032,
    Error,
    useOfNoDiffOnDifferentiableFunc,
    "use 'no_diff' on a call to a differentiable function has no meaning.")
DIAGNOSTIC(
    38033,
    Error,
    cannotUseNoDiffInNonDifferentiableFunc,
    "cannot use 'no_diff' in a non-differentiable function.")
DIAGNOSTIC(
    38034,
    Error,
    cannotUseBorrowInOnDifferentiableParameter,
    "cannot use 'borrow in' on a differentiable parameter.")
DIAGNOSTIC(
    38034,
    Error,
    cannotUseConstRefOnDifferentiableMemberMethod,
    "cannot use '[constref]' on a differentiable member method of a differentiable type.")

DIAGNOSTIC(
    38040,
    Warning,
    nonUniformEntryPointParameterTreatedAsUniform,
    "parameter '$0' is treated as 'uniform' because it does not have a system-value semantic.")

DIAGNOSTIC(
    38041,
    Error,
    intValFromNonIntSpecConstEncountered,
    "cannot cast non-integer specialization constant to compile-time integer")

DIAGNOSTIC(38200, Error, recursiveModuleImport, "module `$0` recursively imports itself")
DIAGNOSTIC(
    39999,
    Error,
    errorInImportedModule,
    "import of module '$0' failed because of a compilation error")

DIAGNOSTIC(
    38201,
    Error,
    glslModuleNotAvailable,
    "'glsl' module is not available from the current global session. To enable GLSL compatibility "
    "mode, specify 'SlangGlobalSessionDesc::enableGLSL' when creating the global session.")
DIAGNOSTIC(39999, Fatal, compilationCeased, "compilation ceased")

DIAGNOSTIC(
    38203,
    Error,
    vectorWithDisallowedElementTypeEncountered,
    "vector with disallowed element type '$0' encountered")

DIAGNOSTIC(
    38203,
    Error,
    vectorWithInvalidElementCountEncountered,
    "vector has invalid element count '$0', valid values are between '$1' and '$2' inclusive")

DIAGNOSTIC(
    38204,
    Error,
    cannotUseResourceTypeInStructuredBuffer,
    "StructuredBuffer element type '$0' cannot contain resource or opaque handle types")

DIAGNOSTIC(
    38205,
    Error,
    recursiveTypesFoundInStructuredBuffer,
    "structured buffer element type '$0' contains recursive type references")

// 39xxx - Type layout and parameter binding.

DIAGNOSTIC(
    39000,
    Error,
    conflictingExplicitBindingsForParameter,
    "conflicting explicit bindings for parameter '$0'")
DIAGNOSTIC(
    39001,
    Warning,
    parameterBindingsOverlap,
    "explicit binding for parameter '$0' overlaps with parameter '$1'")

DIAGNOSTIC(39007, Error, unknownRegisterClass, "unknown register class: '$0'")
DIAGNOSTIC(39008, Error, expectedARegisterIndex, "expected a register index after '$0'")
DIAGNOSTIC(39009, Error, expectedSpace, "expected 'space', got '$0'")
DIAGNOSTIC(39010, Error, expectedSpaceIndex, "expected a register space index after 'space'")
DIAGNOSTIC(39011, Error, invalidComponentMask, "invalid register component mask '$0'.")

DIAGNOSTIC(
    39012,
    Warning,
    requestedBindlessSpaceIndexUnavailable,
    "requested bindless space index '$0' is unavailable, using the next available index '$1'.")
DIAGNOSTIC(
    39029,
    Error,
    targetDoesNotSupportDescriptorHandle,
    "the current compilation target does not support 'DescriptorHandle' types.")
DIAGNOSTIC(
    39013,
    Warning,
    registerModifierButNoVulkanLayout,
    "shader parameter '$0' has a 'register' specified for D3D, but no '[[vk::binding(...)]]` "
    "specified for Vulkan")
DIAGNOSTIC(
    39014,
    Error,
    unexpectedSpecifierAfterSpace,
    "unexpected specifier after register space: '$0'")
DIAGNOSTIC(
    39015,
    Error,
    wholeSpaceParameterRequiresZeroBinding,
    "shader parameter '$0' consumes whole descriptor sets, so the binding must be in the form "
    "'[[vk::binding(0, ...)]]'; the non-zero binding '$1' is not allowed")

DIAGNOSTIC(
    39017,
    Error,
    dontExpectOutParametersForStage,
    "the '$0' stage does not support `out` or `inout` entry point parameters")
DIAGNOSTIC(
    39018,
    Error,
    dontExpectInParametersForStage,
    "the '$0' stage does not support `in` entry point parameters")

DIAGNOSTIC(
    39019,
    Warning,
    globalUniformNotExpected,
    "'$0' is implicitly a global shader parameter, not a global variable. If a global variable is "
    "intended, add the 'static' modifier. If a uniform shader parameter is intended, add the "
    "'uniform' modifier to silence this warning.")

DIAGNOSTIC(
    39020,
    Error,
    tooManyShaderRecordConstantBuffers,
    "can have at most one 'shader record' attributed constant buffer; found $0.")

DIAGNOSTIC(
    39022,
    Warning,
    vkIndexWithoutVkLocation,
    "ignoring '[[vk::index(...)]]` attribute without a corresponding '[[vk::location(...)]]' "
    "attribute")
DIAGNOSTIC(
    39023,
    Error,
    mixingImplicitAndExplicitBindingForVaryingParams,
    "mixing explicit and implicit bindings for varying parameters is not supported (see '$0' and "
    "'$1')")

DIAGNOSTIC(
    39025,
    Error,
    conflictingVulkanInferredBindingForParameter,
    "conflicting vulkan inferred binding for parameter '$0' overlap is $1 and $2")

DIAGNOSTIC(
    39026,
    Error,
    matrixLayoutModifierOnNonMatrixType,
    "matrix layout modifier cannot be used on non-matrix type '$0'.")

DIAGNOSTIC(
    39027,
    Error,
    getAttributeAtVertexMustReferToPerVertexInput,
    "'GetAttributeAtVertex' must reference a vertex input directly, and the vertex input must be "
    "decorated with 'pervertex' or 'nointerpolation'.")

DIAGNOSTIC(
    39028,
    Error,
    notValidVaryingParameter,
    "parameter '$0' is not a valid varying parameter.")

DIAGNOSTIC(
    39029,
    Warning,
    registerModifierButNoVkBindingNorShift,
    "shader parameter '$0' has a 'register' specified for D3D, but no '[[vk::binding(...)]]` "
    "specified for Vulkan, nor is `-fvk-$1-shift` used.")

DIAGNOSTIC(
    39071,
    Warning,
    bindingAttributeIgnoredOnUniform,
    "binding attribute on uniform '$0' will be ignored since it will be packed into the default "
    "constant buffer at descriptor set 0 binding 0. To use explicit bindings, declare the uniform "
    "inside a constant buffer.")

//

// 4xxxx - IL code generation.
//
DIAGNOSTIC(40006, Error, unimplementedSystemValueSemantic, "unknown system-value semantic '$0'")

DIAGNOSTIC(49999, Error, unknownSystemValueSemantic, "unknown system-value semantic '$0'")

DIAGNOSTIC(40007, Internal, irValidationFailed, "IR validation failed: $0")

DIAGNOSTIC(
    40008,
    Error,
    invalidLValueForRefParameter,
    "the form of this l-value argument is not valid for a `ref` parameter")

DIAGNOSTIC(40012, Error, needCompileTimeConstant, "expected a compile-time constant")

DIAGNOSTIC(40013, Error, argIsNotConstexpr, "arg $0 in '$1' is not a compile-time constant")

DIAGNOSTIC(
    40020,
    Error,
    cannotUnrollLoop,
    "loop does not terminate within the limited number of iterations, unrolling is aborted.")

DIAGNOSTIC(
    40030,
    Fatal,
    functionNeverReturnsFatal,
    "function '$0' never returns, compilation ceased.")

// 41000 - IR-level validation issues

DIAGNOSTIC(41000, Warning, unreachableCode, "unreachable code detected")
DIAGNOSTIC(41001, Error, recursiveType, "type '$0' contains cyclic reference to itself.")

DIAGNOSTIC(
    41009,
    Error,
    missingReturnError,
    "non-void function must return in all cases for target '$0'")
DIAGNOSTIC(41010, Warning, missingReturn, "non-void function does not return in all cases")
DIAGNOSTIC(
    41011,
    Error,
    profileIncompatibleWithTargetSwitch,
    "__target_switch has no compatable target with current profile '$0'")
DIAGNOSTIC(
    41012,
    Warning,
    profileImplicitlyUpgraded,
    "entry point '$0' uses additional capabilities that are not part of the specified profile "
    "'$1'. The profile setting is automatically updated to include these capabilities: '$2'")
DIAGNOSTIC(
    41012,
    Error,
    profileImplicitlyUpgradedRestrictive,
    "entry point '$0' uses capabilities that are not part of the specified profile '$1'. Missing "
    "capabilities are: '$2'")
DIAGNOSTIC(41015, Warning, usingUninitializedOut, "use of uninitialized out parameter '$0'")
DIAGNOSTIC(41016, Warning, usingUninitializedVariable, "use of uninitialized variable '$0'")
DIAGNOSTIC(41016, Warning, usingUninitializedValue, "use of uninitialized value of type '$0'")
DIAGNOSTIC(
    41017,
    Warning,
    usingUninitializedGlobalVariable,
    "use of uninitialized global variable '$0'")
DIAGNOSTIC(
    41018,
    Warning,
    returningWithUninitializedOut,
    "returning without initializing out parameter '$0'")
DIAGNOSTIC(
    41020,
    Warning,
    constructorUninitializedField,
    "exiting constructor without initializing field '$0'")
DIAGNOSTIC(
    41021,
    Warning,
    fieldNotDefaultInitialized,
    "default initializer for '$0' will not initialize field '$1'")
DIAGNOSTIC(
    41024,
    Warning,
    commaOperatorUsedInExpression,
    "comma operator used in expression (may be unintended)")

DIAGNOSTIC(
    41026,
    Warning,
    switchFallthroughRestructured,
    "switch fall-through is not supported by this target and will be restructured; "
    "this may affect wave/subgroup convergence if the duplicated code contains wave operations")

DIAGNOSTIC(
    41024,
    Error,
    cannotDefaultInitializeResource,
    "cannot default-initialize $0 with '{}'. Resource types must be explicitly initialized")

DIAGNOSTIC(
    41024,
    Error,
    cannotDefaultInitializeStructWithUninitializedResource,
    "cannot default-initialize struct '$0' with '{}' because it contains an uninitialized $1 field")

DIAGNOSTIC(
    41024,
    Error,
    cannotDefaultInitializeStructContainingResources,
    "cannot default-initialize struct '$0' with '{}' because it contains resource fields")

DIAGNOSTIC(
    41011,
    Error,
    typeDoesNotFitAnyValueSize,
    "type '$0' does not fit in the size required by its conforming interface.")
DIAGNOSTIC(-1, Note, typeAndLimit, "sizeof($0) is $1, limit is $2")
DIAGNOSTIC(
    41014,
    Error,
    typeCannotBePackedIntoAnyValue,
    "type '$0' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.")
DIAGNOSTIC(
    41020,
    Error,
    lossOfDerivativeDueToCallOfNonDifferentiableFunction,
    "derivative cannot be propagated through call to non-$1-differentiable function `$0`, use "
    "'no_diff' to clarify intention.")
DIAGNOSTIC(
    41024,
    Error,
    lossOfDerivativeAssigningToNonDifferentiableLocation,
    "derivative is lost during assignment to non-differentiable location, use 'detach()' to "
    "clarify intention.")
DIAGNOSTIC(
    41025,
    Error,
    lossOfDerivativeUsingNonDifferentiableLocationAsOutArg,
    "derivative is lost when passing a non-differentiable location to an `out` or `inout` "
    "parameter, consider passing a temporary variable instead.")
DIAGNOSTIC(
    41023,
    Error,
    getStringHashMustBeOnStringLiteral,
    "getStringHash can only be called when argument is statically resolvable to a string literal")

DIAGNOSTIC(
    41030,
    Warning,
    operatorShiftLeftOverflow,
    "left shift amount exceeds the number of bits and the result will be always zero, (`$0` << "
    "`$1`).")

DIAGNOSTIC(
    41901,
    Error,
    unsupportedUseOfLValueForAutoDiff,
    "unsupported use of L-value for auto differentiation.")

DIAGNOSTIC(
    42001,
    Error,
    invalidUseOfTorchTensorTypeInDeviceFunc,
    "invalid use of TorchTensor type in device/kernel functions. use `TensorView` instead.")

DIAGNOSTIC(
    42050,
    Warning,
    potentialIssuesWithPreferRecomputeOnSideEffectMethod,
    "$0 has [PreferRecompute] and may have side effects. side effects may execute multiple times. "
    "use [PreferRecompute(SideEffectBehavior.Allow)], or mark function with [__NoSideEffect]")

DIAGNOSTIC(45001, Error, unresolvedSymbol, "unresolved external symbol '$0'.")

DIAGNOSTIC(
    41201,
    Warning,
    expectDynamicUniformArgument,
    "argument for '$0' might not be a dynamic uniform, use `asDynamicUniform()` to silence this "
    "warning.")
DIAGNOSTIC(
    41201,
    Warning,
    expectDynamicUniformValue,
    "value stored at this location must be dynamic uniform, use `asDynamicUniform()` to silence "
    "this warning.")

DIAGNOSTIC(
    41202,
    Error,
    notEqualBitCastSize,
    "invalid to bit_cast differently sized types: '$0' with size '$1' casted into '$2' with size "
    "'$3'")

DIAGNOSTIC(
    41300,
    Error,
    byteAddressBufferUnaligned,
    "invalid alignment `$0` specified for the byte address buffer resource with the element size "
    "of `$1`")

DIAGNOSTIC(41400, Error, staticAssertionFailure, "static assertion failed, $0")
DIAGNOSTIC(41401, Error, staticAssertionFailureWithoutMessage, "static assertion failed.")
DIAGNOSTIC(
    41402,
    Error,
    staticAssertionConditionNotConstant,
    "condition for static assertion cannot be evaluated at compile time.")

DIAGNOSTIC(
    41402,
    Error,
    multiSampledTextureDoesNotAllowWrites,
    "cannot write to a multisampled texture with target '$0'.")

DIAGNOSTIC(
    41403,
    Error,
    invalidAtomicDestinationPointer,
    "cannot perform atomic operation because destination is neither groupshared nor from a device "
    "buffer.")

//
// 5xxxx - Target code generation.
//

DIAGNOSTIC(
    50010,
    Internal,
    missingExistentialBindingsForParameter,
    "missing argument for existential parameter slot")
DIAGNOSTIC(
    50011,
    Warning,
    spirvVersionNotSupported,
    "Slang's SPIR-V backend only supports SPIR-V version 1.3 and later."
    " Use `-emit-spirv-via-glsl` option to produce SPIR-V 1.0 through 1.2.")

DIAGNOSTIC(
    50060,
    Error,
    invalidMeshStageOutputTopology,
    "Invalid mesh stage output topology '$0' for target '$1', must be one of: $2")

DIAGNOSTIC(
    50100,
    Error,
    noTypeConformancesFoundForInterface,
    "No type conformances are found for interface '$0'. Code generation for current target "
    "requires at least one implementation type present in the linkage.")
DIAGNOSTIC(
    50101,
    Error,
    dynamicDispatchOnPotentiallyUninitializedExistential,
    "Cannot dynamically dispatch on potentially uninitialized interface object '$0'.")
DIAGNOSTIC(
    50102,
    Note,
    dynamicDispatchCodeGeneratedHere,
    "generated dynamic dispatch code for this site. $0 possible types: '$1'")
DIAGNOSTIC(
    50103,
    Note,
    specializedDynamicDispatchCodeGeneratedHere,
    "generated specialized dynamic dispatch code for this site. $0 possible types: '$1'. "
    "specialization arguments: '$2'.")

DIAGNOSTIC(
    52000,
    Error,
    multiLevelBreakUnsupported,
    "control flow appears to require multi-level `break`, which Slang does not yet support")

DIAGNOSTIC(
    52001,
    Warning,
    dxilNotFound,
    "dxil shared library not found, so 'dxc' output cannot be signed! Shader code will not be "
    "runnable in non-development environments.")

DIAGNOSTIC(
    52002,
    Error,
    passThroughCompilerNotFound,
    "could not find a suitable pass-through compiler for '$0'.")
DIAGNOSTIC(52003, Error, cannotDisassemble, "cannot disassemble '$0'.")

DIAGNOSTIC(52004, Error, unableToWriteFile, "unable to write file '$0'")
DIAGNOSTIC(52005, Error, unableToReadFile, "unable to read file '$0'")

DIAGNOSTIC(
    52006,
    Error,
    compilerNotDefinedForTransition,
    "compiler not defined for transition '$0' to '$1'.")

DIAGNOSTIC(
    52009,
    Error,
    cannotEmitReflectionWithoutTarget,
    "cannot emit reflection JSON; no compilation target available")

DIAGNOSTIC(54001, Warning, meshOutputMustBeOut, "Mesh shader outputs must be declared with 'out'.")
DIAGNOSTIC(54002, Error, meshOutputMustBeArray, "HLSL style mesh shader outputs must be arrays")
DIAGNOSTIC(
    54003,
    Error,
    meshOutputArrayMustHaveSize,
    "HLSL style mesh shader output arrays must have a length specified")
DIAGNOSTIC(
    54004,
    Warning,
    unnecessaryHLSLMeshOutputModifier,
    "Unnecessary HLSL style mesh shader output modifier")

DIAGNOSTIC(
    55101,
    Error,
    invalidTorchKernelReturnType,
    "'$0' is not a valid return type for a pytorch kernel function.")
DIAGNOSTIC(
    55102,
    Error,
    invalidTorchKernelParamType,
    "'$0' is not a valid parameter type for a pytorch kernel function.")

DIAGNOSTIC(
    55200,
    Error,
    unsupportedBuiltinType,
    "'$0' is not a supported builtin type for the target.")
DIAGNOSTIC(
    55201,
    Error,
    unsupportedRecursion,
    "recursion detected in call to '$0', but the current code generation target does not allow "
    "recursion.")
DIAGNOSTIC(
    55202,
    Error,
    systemValueAttributeNotSupported,
    "system value semantic '$0' is not supported for the current target.")
DIAGNOSTIC(
    55203,
    Error,
    systemValueTypeIncompatible,
    "system value semantic '$0' should have type '$1' or be convertible to type '$1'.")
DIAGNOSTIC(
    55204,
    Error,
    unsupportedTargetIntrinsic,
    "intrinsic operation '$0' is not supported for the current target.")
DIAGNOSTIC(
    55205,
    Error,
    unsupportedSpecializationConstantForNumThreads,
    "Specialization constants are not supported in the 'numthreads' attribute for the current "
    "target.")
DIAGNOSTIC(
    56001,
    Error,
    unableToAutoMapCUDATypeToHostType,
    "Could not automatically map '$0' to a host type. Automatic binding generation failed for '$1'")
DIAGNOSTIC(
    56002,
    Error,
    attemptToQuerySizeOfUnsizedArray,
    "cannot obtain the size of an unsized array.")

DIAGNOSTIC(56003, Fatal, useOfUninitializedOpaqueHandle, "use of uninitialized opaque handle '$0'.")

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
