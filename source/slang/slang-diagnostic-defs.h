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

DIAGNOSTIC(-1, Note, alsoSeePipelineDefinition, "also see pipeline definition")
DIAGNOSTIC(-1, Note, implicitParameterMatchingFailedBecauseNameNotAccessible, "implicit parameter matching failed because the component of the same name is not accessible from '$0'.\ncheck if you have declared necessary requirements and properly used the 'public' qualifier.")
DIAGNOSTIC(-1, Note, implicitParameterMatchingFailedBecauseShaderDoesNotDefineComponent, "implicit parameter matching failed because shader '$0' does not define component '$1'.")
DIAGNOSTIC(-1, Note, implicitParameterMatchingFailedBecauseTypeMismatch, "implicit parameter matching failed because the component of the same name does not match parameter type '$0'.")
DIAGNOSTIC(-1, Note, noteShaderIsTargetingPipeine, "shader '$0' is targeting pipeline '$1'")
DIAGNOSTIC(-1, Note, seeDefinitionOf, "see definition of '$0'")
DIAGNOSTIC(-1, Note, seeInterfaceDefinitionOf, "see interface definition of '$0'")
DIAGNOSTIC(-1, Note, seeUsingOf, "see using of '$0'")
DIAGNOSTIC(-1, Note, seeDefinitionOfShader, "see definition of shader '$0'")
DIAGNOSTIC(-1, Note, seeInclusionOf, "see inclusion of '$0'")
DIAGNOSTIC(-1, Note, seeModuleBeingUsedIn, "see module '$0' being used in '$1'")
DIAGNOSTIC(-1, Note, seePipelineRequirementDefinition, "see pipeline requirement definition")
DIAGNOSTIC(-1, Note, seePotentialDefinitionOfComponent, "see potential definition of component '$0'")
DIAGNOSTIC(-1, Note, seePreviousDefinition, "see previous definition")
DIAGNOSTIC(-1, Note, seePreviousDefinitionOf, "see previous definition of '$0'")
DIAGNOSTIC(-1, Note, seeRequirementDeclaration, "see requirement declaration")
DIAGNOSTIC(-1, Note, doYouForgetToMakeComponentAccessible, "do you forget to make component '$0' acessible from '$1' (missing public qualifier)?")

DIAGNOSTIC(-1, Note, seeDeclarationOf, "see declaration of '$0'")
DIAGNOSTIC(-1, Note, seeOtherDeclarationOf, "see other declaration of '$0'")
DIAGNOSTIC(-1, Note, seePreviousDeclarationOf, "see previous declaration of '$0'")
DIAGNOSTIC(-1, Note, includeOutput, "include $0")

//
// 0xxxx -  Command line and interaction with host platform APIs.
//

DIAGNOSTIC(    1, Error, cannotOpenFile, "cannot open file '$0'.")
DIAGNOSTIC(    2, Error, cannotFindFile, "cannot find file '$0'.")
DIAGNOSTIC(    2, Error, unsupportedCompilerMode, "unsupported compiler mode.")
DIAGNOSTIC(    4, Error, cannotWriteOutputFile, "cannot write output file '$0'.")
DIAGNOSTIC(    5, Error, failedToLoadDynamicLibrary, "failed to load dynamic library '$0'")
DIAGNOSTIC(    6, Error, tooManyOutputPathsSpecified, "$0 output paths specified, but only $1 entry points given")

DIAGNOSTIC(    7, Error, noOutputPathSpecifiedForEntryPoint,
    "no output path specified for entry point '$0' (the '-o' option for an entry point must precede the corresponding '-entry')")

DIAGNOSTIC(    8, Error, outputPathsImplyDifferentFormats,
    "the output paths '$0' and '$1' require different code-generation targets")

DIAGNOSTIC(    10, Error, explicitOutputPathsAndMultipleTargets, "canot use both explicit output paths ('-o') and multiple targets ('-target')")
DIAGNOSTIC(    11, Error, glslIsNotSupported, "the Slang compiler does not support GLSL as a source language")
DIAGNOSTIC(    12, Error, cannotDeduceSourceLanguage, "can't deduce language for input file '$0'")
DIAGNOSTIC(    13, Error, unknownCodeGenerationTarget, "unknown code generation target '$0'")
DIAGNOSTIC(    14, Error, unknownProfile, "unknown profile '$0'")
DIAGNOSTIC(    15, Error, unknownStage, "unknown stage '$0'")
DIAGNOSTIC(    16, Error, unknownPassThroughTarget, "unknown pass-through target '$0'")
DIAGNOSTIC(    17, Error, unknownCommandLineOption, "unknown command-line option '$0'")
DIAGNOSTIC(    18, Error, unknownFileSystemOption, "unknown file-system option '$0'")
DIAGNOSTIC(    19, Error, unknownSourceLanguage, "unknown source language '$0'")

DIAGNOSTIC(    20, Error, entryPointsNeedToBeAssociatedWithTranslationUnits, "when using multiple source files, entry points must be specified after their corresponding source file(s)")
DIAGNOSTIC(    22, Error, unknownDownstreamCompiler, "unknown downstream compiler '$0'")

DIAGNOSTIC(    24, Error, unknownLineDirectiveMode, "unknown '#line' directive mode '$0'")
DIAGNOSTIC(    25, Error, unknownFloatingPointMode, "unknown floating-point mode '$0'")
DIAGNOSTIC(    26, Error, unknownOptimiziationLevel, "unknown optimization level '$0'")
DIAGNOSTIC(    27, Error, unknownDebugInfoLevel, "unknown debug info level '$0'")

DIAGNOSTIC(    28, Error, unableToGenerateCodeForTarget, "unable to generate code for target '$0'")

DIAGNOSTIC(    30, Warning, sameStageSpecifiedMoreThanOnce, "the stage '$0' was specified more than once for entry point '$1'")
DIAGNOSTIC(    31, Error, conflictingStagesForEntryPoint, "conflicting stages have been specified for entry point '$0'")
DIAGNOSTIC(    32, Warning, explicitStageDoesntMatchImpliedStage, "the stage specified for entry point '$0' ('$1') does not match the stage implied by the source file name ('$2')")
DIAGNOSTIC(    33, Error, stageSpecificationIgnoredBecauseNoEntryPoints, "one or more stages were specified, but no entry points were specified with '-entry'")
DIAGNOSTIC(    34, Error, stageSpecificationIgnoredBecauseBeforeAllEntryPoints, "when compiling multiple entry points, any '-stage' options must follow the '-entry' option that they apply to")
DIAGNOSTIC(    35, Error, noStageSpecifiedInPassThroughMode, "no stage was specified for entry point '$0'; when using the '-pass-through' option, stages must be fully specified on the command line")

DIAGNOSTIC(    40, Warning, sameProfileSpecifiedMoreThanOnce, "the '$0' was specified more than once for target '$0'")
DIAGNOSTIC(    41, Error, conflictingProfilesSpecifiedForTarget, "conflicting profiles have been specified for target '$0'")

DIAGNOSTIC(    42, Error, profileSpecificationIgnoredBecauseNoTargets, "a '-profile' option was specified, but no target was specified with '-target'")
DIAGNOSTIC(    43, Error, profileSpecificationIgnoredBecauseBeforeAllTargets, "when using multiple targets, any '-profile' option must follow the '-target' it applies to")

DIAGNOSTIC(    42, Error, targetFlagsIgnoredBecauseNoTargets, "target options were specified, but no target was specified with '-target'")
DIAGNOSTIC(    43, Error, targetFlagsIgnoredBecauseBeforeAllTargets, "when using multiple targets, any target options must follow the '-target' they apply to")

DIAGNOSTIC(    50, Error, duplicateTargets, "the target '$0' has been specified more than once")

DIAGNOSTIC(    60, Error, cannotDeduceOutputFormatFromPath, "cannot infer an output format from the output path '$0'")
DIAGNOSTIC(    61, Error, cannotMatchOutputFileToTarget, "no specified '-target' option matches the output path '$0', which implies the '$1' format")

DIAGNOSTIC(    70, Error, cannotMatchOutputFileToEntryPoint, "the output path '$0' is not associated with any entry point; a '-o' option for a compiled kernel must follow the '-entry' option for its corresponding entry point")

DIAGNOSTIC(    80, Error, duplicateOutputPathsForEntryPointAndTarget, "multiple output paths have been specified entry point '$0' on target '$1'")
DIAGNOSTIC(    81, Error, duplicateOutputPathsForTarget, "multiple output paths have been specified for target '$0'")
DIAGNOSTIC(    82, Error, duplicateDependencyOutputPaths, "the -dep argument can only be specified once")

DIAGNOSTIC(    82, Error, unableToWriteReproFile, "unable to write repro file '%0'")
DIAGNOSTIC(    83, Error, unableToWriteModuleContainer, "unable to write module container '%0'")
DIAGNOSTIC(    84, Error, unableToReadModuleContainer, "unable to read module container '%0'")
DIAGNOSTIC(    85, Error, unableToAddReferenceToModuleContainer, "unable to add a reference to a module container")
DIAGNOSTIC(    86, Error, unableToCreateModuleContainer, "unable to create module container")

DIAGNOSTIC(    87, Error, unableToSetDefaultDownstreamCompiler, "unable to set default downstream compiler for source language '%0' to '%1'")

DIAGNOSTIC(    88, Error, unknownArchiveType, "archive type '%0' is unknown")
DIAGNOSTIC(    89, Error, expectingSlangRiffContainer, "expecting a slang riff container")
DIAGNOSTIC(    90, Error, incompatibleRiffSemanticVersion, "incompatible riff semantic version %0 expecting %1")
DIAGNOSTIC(    91, Error, riffHashMismatch, "riff hash mismatch - incompatible riff")
DIAGNOSTIC(    92, Error, unableToCreateDirectory, "unable to create directory '$0'")
DIAGNOSTIC(    93, Error, unableExtractReproToDirectory, "unable to extract repro to directory '$0'")
DIAGNOSTIC(    94, Error, unableToReadRiff, "unable to read as 'riff'/not a 'riff' file")

DIAGNOSTIC(    95, Error, unknownLibraryKind, "unknown library kind '$0'")
DIAGNOSTIC(    96, Error, kindNotLinkable, "not a known linkable kind '$0'")
DIAGNOSTIC(    97, Error, libraryDoesNotExist, "library '$0' does not exist")

//
// 001xx - Downstream Compilers
//

DIAGNOSTIC(  100, Error, failedToLoadDownstreamCompiler, "failed to load downstream compiler '$0'")
DIAGNOSTIC(  101, Error, downstreamCompilerDoesntSupportWholeProgramCompilation, "downstream compiler '$0' doesn't support whole program compilation")


DIAGNOSTIC(99999, Note, noteFailedToLoadDynamicLibrary, "failed to load dynamic library '$0'")

//
// 15xxx - Preprocessing
//

// 150xx - conditionals
DIAGNOSTIC(15000, Error, endOfFileInPreprocessorConditional, "end of file encountered during preprocessor conditional")
DIAGNOSTIC(15001, Error, directiveWithoutIf, "'$0' directive without '#if'")
DIAGNOSTIC(15002, Error, directiveAfterElse , "'$0' directive without '#if'")

DIAGNOSTIC(-1, Note, seeDirective, "see '$0' directive")

// 151xx - directive parsing
DIAGNOSTIC(15100, Error, expectedPreprocessorDirectiveName, "expected preprocessor directive name")
DIAGNOSTIC(15101, Error, unknownPreprocessorDirective, "unknown preprocessor directive '$0'")
DIAGNOSTIC(15102, Error, expectedTokenInPreprocessorDirective, "expected '$0' in '$1' directive")
DIAGNOSTIC(15102, Error, expected2TokensInPreprocessorDirective, "expected '$0' or '$1' in '$2' directive")
DIAGNOSTIC(15103, Error, unexpectedTokensAfterDirective, "unexpected tokens following '$0' directive")


// 152xx - preprocessor expressions
DIAGNOSTIC(15200, Error, expectedTokenInPreprocessorExpression, "expected '$0' in preprocessor expression")
DIAGNOSTIC(15201, Error, syntaxErrorInPreprocessorExpression, "syntax error in preprocessor expression")
DIAGNOSTIC(15202, Error, divideByZeroInPreprocessorExpression, "division by zero in preprocessor expression")
DIAGNOSTIC(15203, Error, expectedTokenInDefinedExpression, "expected '$0' in 'defined' expression")
DIAGNOSTIC(15204, Warning, directiveExpectsExpression, "'$0' directive requires an expression")
DIAGNOSTIC(15205, Warning, undefinedIdentifierInPreprocessorExpression, "undefined identifier '$0' in preprocessor expression will evaluate to zero")

DIAGNOSTIC(-1, Note, seeOpeningToken, "see opening '$0'")

// 153xx - #include
DIAGNOSTIC(15300, Error, includeFailed, "failed to find include file '$0'")
DIAGNOSTIC(15301, Error, importFailed, "failed to find imported file '$0'")
DIAGNOSTIC(-1, Error, noIncludeHandlerSpecified, "no `#include` handler was specified")
DIAGNOSTIC(15302, Error, noUniqueIdentity, "`#include` handler didn't generate a unique identity for file '$0'")


// 154xx - macro definition
DIAGNOSTIC(15400, Warning, macroRedefinition, "redefinition of macro '$0'")
DIAGNOSTIC(15401, Warning, macroNotDefined, "macro '$0' is not defined")
DIAGNOSTIC(15403, Error, expectedTokenInMacroParameters, "expected '$0' in macro parameters")
DIAGNOSTIC(15404, Warning, builtinMacroRedefinition, "Redefinition of builtin macro '$0'")

DIAGNOSTIC(15405, Error, tokenPasteAtStart, "'##' is not allowed at the start of a macro body")
DIAGNOSTIC(15406, Error, tokenPasteAtEnd, "'##' is not allowed at the end of a macro body")
DIAGNOSTIC(15407, Error, expectedMacroParameterAfterStringize, "'#' in macro body must be followed by the name of a macro parameter")
DIAGNOSTIC(15408, Error, duplicateMacroParameterName, "redefinition of macro parameter '$0'")
DIAGNOSTIC(15409, Error, variadicMacroParameterMustBeLast, "a variadic macro parameter is only allowed at the end of the parameter list")

// 155xx - macro expansion
DIAGNOSTIC(15500, Warning, expectedTokenInMacroArguments, "expected '$0' in macro invocation")
DIAGNOSTIC(15501, Error, wrongNumberOfArgumentsToMacro, "wrong number of arguments to macro (expected $0, got $1)")
DIAGNOSTIC(15502, Error, errorParsingToMacroInvocationArgument, "error parsing macro '$0' invocation argument to '$1'")

DIAGNOSTIC(15503, Warning, invalidTokenPasteResult, "toking pasting with '##' resulted in the invalid token '$0'")

// 156xx - pragmas
DIAGNOSTIC(15600, Error, expectedPragmaDirectiveName, "expected a name after '#pragma'")
DIAGNOSTIC(15601, Warning, unknownPragmaDirectiveIgnored, "ignoring unknown directive '#pragma $0'")
DIAGNOSTIC(15602, Warning, pragmaOnceIgnored, "pragma once was ignored - this is typically because is not placed in an include")

// 159xx - user-defined error/warning
DIAGNOSTIC(15900, Error,    userDefinedError,   "#error: $0")
DIAGNOSTIC(15901, Warning,  userDefinedWarning, "#warning: $0")

//
// 2xxxx - Parsing
//

DIAGNOSTIC(20003, Error, unexpectedToken, "unexpected $0")
DIAGNOSTIC(20001, Error, unexpectedTokenExpectedTokenType, "unexpected $0, expected $1")
DIAGNOSTIC(20001, Error, unexpectedTokenExpectedTokenName, "unexpected $0, expected '$1'")

DIAGNOSTIC(0, Error, tokenNameExpectedButEOF, "\"$0\" expected but end of file encountered.")
DIAGNOSTIC(0, Error, tokenTypeExpectedButEOF, "$0 expected but end of file encountered.")
DIAGNOSTIC(20001, Error, tokenNameExpected, "\"$0\" expected")
DIAGNOSTIC(20001, Error, tokenNameExpectedButEOF2, "\"$0\" expected but end of file encountered.")
DIAGNOSTIC(20001, Error, tokenTypeExpected, "$0 expected")
DIAGNOSTIC(20001, Error, tokenTypeExpectedButEOF2, "$0 expected but end of file encountered.")
DIAGNOSTIC(20001, Error, typeNameExpectedBut, "unexpected $0, expected type name")
DIAGNOSTIC(20001, Error, typeNameExpectedButEOF, "type name expected but end of file encountered.")
DIAGNOSTIC(20001, Error, unexpectedEOF, " Unexpected end of file.")
DIAGNOSTIC(20002, Error, syntaxError, "syntax error.")
DIAGNOSTIC(20004, Error, unexpectedTokenExpectedComponentDefinition, "unexpected token '$0', only component definitions are allowed in a shader scope.")
DIAGNOSTIC(20008, Error, invalidOperator, "invalid operator '$0'.")
DIAGNOSTIC(20011, Error, unexpectedColon, "unexpected ':'.")
DIAGNOSTIC(20012, Error, invalidSPIRVVersion, "Expecting SPIR-V version as either 'major.minor', or quoted if has patch (eg for SPIR-V 1.2, '1.2' or \"1.2\"')")
DIAGNOSTIC(20013, Error, invalidCUDASMVersion, "Expecting CUDA SM version as either 'major.minor', or quoted if has patch (eg for '7.0' or \"7.0\"')")

DIAGNOSTIC(20014, Error, classIsReservedKeyword, "'class' is a reserved keyword in this context; use 'struct' instead.")

//
// 3xxxx - Semantic analysis
//

DIAGNOSTIC(30003, Error, breakOutsideLoop, "'break' must appear inside loop constructs.")
DIAGNOSTIC(30004, Error, continueOutsideLoop, "'continue' must appear inside loop constructs.")
DIAGNOSTIC(30005, Error, whilePredicateTypeError, "'while': expression must evaluate to int.")
DIAGNOSTIC(30006, Error, ifPredicateTypeError,  "'if': expression must evaluate to int.")
DIAGNOSTIC(30006, Error, returnNeedsExpression, "'return' should have an expression.")
DIAGNOSTIC(30007, Error, componentReturnTypeMismatch, "expression type '$0' does not match component's type '$1'")
DIAGNOSTIC(30007, Error, functionReturnTypeMismatch, "expression type '$0' does not match function's return type '$1'")
DIAGNOSTIC(30008, Error, variableNameAlreadyDefined, "variable $0 already defined.")
DIAGNOSTIC(30009, Error, invalidTypeVoid, "invalid type 'void'.")
DIAGNOSTIC(30010, Error, whilePredicateTypeError2, "'while': expression must evaluate to int.")
DIAGNOSTIC(30011, Error, assignNonLValue, "left of '=' is not an l-value.")
DIAGNOSTIC(30012, Error, noApplicationUnaryOperator, "no overload found for operator $0 ($1).")
DIAGNOSTIC(30012, Error, noOverloadFoundForBinOperatorOnTypes, "no overload found for operator $0  ($1, $2).")
DIAGNOSTIC(30013, Error, subscriptNonArray, "no subscript operation found for  type '$0'")
DIAGNOSTIC(30014, Error, subscriptIndexNonInteger, "index expression must evaluate to int.")
DIAGNOSTIC(30015, Error, undefinedIdentifier2, "undefined identifier '$0'.")
DIAGNOSTIC(30017, Error, componentNotAccessibleFromShader, "component '$0' is not accessible from shader '$1'.")
DIAGNOSTIC(30019, Error, typeMismatch, "expected an expression of type '$0', got '$1'")
DIAGNOSTIC(30020, Error, importOperatorReturnTypeMismatch, "import operator should return '$1', but the expression has type '$0''. do you forget 'project'?")
DIAGNOSTIC(30021, Error, noApplicationFunction, "$0: no overload takes arguments ($1)")
DIAGNOSTIC(30022, Error, invalidTypeCast, "invalid type cast between \"$0\" and \"$1\".")
DIAGNOSTIC(30023, Error, typeHasNoPublicMemberOfName, "\"$0\" does not have public member \"$1\".")
DIAGNOSTIC(30025, Error, invalidArraySize, "array size must be larger than zero.")
DIAGNOSTIC(30026, Error, returnInComponentMustComeLast, "'return' can only appear as the last statement in component definition.")
DIAGNOSTIC(30027, Error, noMemberOfNameInType, "'$0' is not a member of '$1'.")
DIAGNOSTIC(30028, Error, forPredicateTypeError, "'for': predicate expression must evaluate to bool.")
DIAGNOSTIC(30030, Error, projectionOutsideImportOperator, "'project': invalid use outside import operator.")
DIAGNOSTIC(30031, Error, projectTypeMismatch, "'project': expression must evaluate to record type '$0'.")
DIAGNOSTIC(30033, Error, invalidTypeForLocalVariable, "cannot declare a local variable of this type.")
DIAGNOSTIC(30035, Error, componentOverloadTypeMismatch, "'$0': type of overloaded component mismatches previous definition.")
DIAGNOSTIC(30041, Error, bitOperationNonIntegral, "bit operation: operand must be integral type.")
DIAGNOSTIC(30047, Error, argumentExpectedLValue, "argument passed to parameter '$0' must be l-value.")
DIAGNOSTIC(30048, Note,  implicitCastUsedAsLValue, "argument was implicitly cast from '$0' to '$1', and Slang does not support using an implicit cast as an l-value")
DIAGNOSTIC(30049, Note,  thisIsImmutableByDefault, "a 'this' parameter is an immutable parameter by default in Slang; apply the `[mutating]` attribute to the function declaration to opt in to a mutable `this`")
DIAGNOSTIC(30050, Error,  mutatingMethodOnImmutableValue, "mutating method '$0' cannot be called on an immutable value")

DIAGNOSTIC(30051, Error, invalidValueForArgument, "invalid value for argument '$0'")
DIAGNOSTIC(30052, Error, invalidSwizzleExpr, "invalid swizzle pattern '$0' on type '$1'")
DIAGNOSTIC(30043, Error, getStringHashRequiresStringLiteral, "getStringHash parameter can only accept a string literal")

DIAGNOSTIC(30060, Error, expectedAType, "expected a type, got a '$0'")
DIAGNOSTIC(30061, Error, expectedANamespace, "expected a namespace, got a '$0'")

DIAGNOSTIC(30065, Error, newCanOnlyBeUsedToInitializeAClass, "`new` can only be used to initialize a class")
DIAGNOSTIC(30066, Error, classCanOnlyBeInitializedWithNew, "a class can only be initialized by a `new` clause")

DIAGNOSTIC(30100, Error, staticRefToNonStaticMember, "type '$0' cannot be used to refer to non-static member '$1'")

DIAGNOSTIC(30200, Error, redeclaration, "declaration of '$0' conflicts with existing declaration")
DIAGNOSTIC(30201, Error, functionRedefinition, "function '$0' already has a body")
DIAGNOSTIC(30202, Error, functionRedeclarationWithDifferentReturnType, "function '$0' declared to return '$1' was previously declared to return '$2'")


DIAGNOSTIC(33070, Error, expectedFunction, "expected a function, got '$0'")
DIAGNOSTIC(33071, Error, expectedAStringLiteral, "expected a string literal")

DIAGNOSTIC(   -1, Note, noteExplicitConversionPossible, "explicit conversion from '$0' to '$1' is possible")
DIAGNOSTIC(30080, Error, ambiguousConversion, "more than one implicit conversion exists from '$0' to '$1'")

DIAGNOSTIC(30090, Error, tryClauseMustApplyToInvokeExpr, "expression in a 'try' clause must be a call to a function or operator overload.")
DIAGNOSTIC(30091, Error, tryInvokeCalleeShouldThrow, "'$0' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'")
DIAGNOSTIC(30092, Error, calleeOfTryCallMustBeFunc, "callee in a 'try' clause must be a function")
DIAGNOSTIC(30093, Error, uncaughtTryCallInNonThrowFunc, "the current function or environment is not declared to throw any errors, but the 'try' clause is not caught")
DIAGNOSTIC(30094, Error, mustUseTryClauseToCallAThrowFunc, "the callee may throw an error, and therefore must be called within a 'try' clause")
DIAGNOSTIC(30095, Error, errorTypeOfCalleeIncompatibleWithCaller, "the error type `$1` of callee `$0` is not compatible with the caller's error type `$2`.")

// Attributes
DIAGNOSTIC(31000, Error, unknownAttributeName, "unknown attribute '$0'")
DIAGNOSTIC(31001, Error, attributeArgumentCountMismatch, "attribute '$0' expects $1 arguments ($2 provided)")
DIAGNOSTIC(31002, Error, attributeNotApplicable, "attribute '$0' is not valid here")

DIAGNOSTIC(31003, Error, badlyDefinedPatchConstantFunc, "hull shader '$0' has has badly defined 'patchconstantfunc' attribute.")

DIAGNOSTIC(31004, Error, expectedSingleIntArg, "attribute '$0' expects a single int argument")
DIAGNOSTIC(31005, Error, expectedSingleStringArg, "attribute '$0' expects a single string argument")

DIAGNOSTIC(31006, Error, attributeFunctionNotFound, "Could not find function '$0' for attribute'$1'")

DIAGNOSTIC(31100, Error, unknownStageName, "unknown stage name '$0'")
DIAGNOSTIC(31101, Error, unknownImageFormatName, "unknown image format '$0'")
DIAGNOSTIC(31101, Error, unknownDiagnosticName, "unknown diagnostic '$0'")
DIAGNOSTIC(31102, Error, nonPositiveNumThreads, "expected a positive integer in 'numthreads' attribute, got '$0'")

DIAGNOSTIC(31120, Error, invalidAttributeTarget, "invalid syntax target for user defined attribute")

DIAGNOSTIC(31121, Error, anyValueSizeExceedsLimit, "'anyValueSize' cannot exceed $0")

DIAGNOSTIC(31122, Error, associatedTypeNotAllowInComInterface, "associatedtype not allowed in a [COM] interface")

// Enums

DIAGNOSTIC(32000, Error, invalidEnumTagType,        "invalid tag type for 'enum': '$0'")
DIAGNOSTIC(32003, Error, unexpectedEnumTagExpr,     "unexpected form for 'enum' tag value expression")



// 303xx: interfaces and associated types
DIAGNOSTIC(30300, Error, assocTypeInInterfaceOnly, "'associatedtype' can only be defined in an 'interface'.")
DIAGNOSTIC(30301, Error, globalGenParamInGlobalScopeOnly, "'type_param' can only be defined global scope.")
// TODO: need to assign numbers to all these extra diagnostics...
DIAGNOSTIC(39999, Fatal, cyclicReference, "cyclic reference '$0'.")
DIAGNOSTIC(39999, Error, localVariableUsedBeforeDeclared, "local variable '$0' is being used before its declaration.")
DIAGNOSTIC(39999, Error, variableUsedInItsOwnDefinition, "the initial-value expression for variable '$0' depends on the value of the variable itself")

// 304xx: generics
DIAGNOSTIC(30400, Error, genericTypeNeedsArgs, "generic type '$0' used without argument")

// 305xx: initializer lists
DIAGNOSTIC(30500, Error, tooManyInitializers, "too many initializers (expected $0, got $1)")
DIAGNOSTIC(30501, Error, cannotUseInitializerListForArrayOfUnknownSize, "cannot use initializer list for array of statically unknown size '$0'")
DIAGNOSTIC(30502, Error, cannotUseInitializerListForVectorOfUnknownSize, "cannot use initializer list for vector of statically unknown size '$0'")
DIAGNOSTIC(30503, Error, cannotUseInitializerListForMatrixOfUnknownSize, "cannot use initializer list for matrix of statically unknown size '$0' rows")
DIAGNOSTIC(30504, Error, cannotUseInitializerListForType, "cannot use initializer list for type '$0'")

// 306xx: variables
DIAGNOSTIC(30600, Error, varWithoutTypeMustHaveInitializer, "a variable declaration without an initial-value expression must be given an explicit type")

DIAGNOSTIC(30610, Error, ambiguousDefaultInitializerForType, "more than one default initializer was found for type '$0'")

// 307xx: parameters
DIAGNOSTIC(30700, Error, outputParameterCannotHaveDefaultValue, "an 'out' or 'inout' parameter cannot have a default-value expression")

// 308xx: inheritance
DIAGNOSTIC(30810, Error, baseOfInterfaceMustBeInterface, "interface '$0' cannot inherit from non-interface type '$1'")
DIAGNOSTIC(30811, Error, baseOfStructMustBeStructOrInterface, "struct '$0' cannot inherit from type '$1' that is neither a struct nor an interface")
DIAGNOSTIC(30812, Error, baseOfEnumMustBeIntegerOrInterface, "enum '$0' cannot inherit from type '$1' that is neither an interface not a builtin integer type")
DIAGNOSTIC(30813, Error, baseOfExtensionMustBeInterface, "extension cannot inherit from non-interface type '$1'")
DIAGNOSTIC(30814, Error, baseOfClassMustBeClassOrInterface, "class '$0' cannot inherit from type '$1' that is neither a class nor an interface")

DIAGNOSTIC(30820, Error, baseStructMustBeListedFirst, "a struct type may only inherit from one other struct type, and that type must appear first in the list of bases")
DIAGNOSTIC(30821, Error, tagTypeMustBeListedFirst, "an unum type may only have a single tag type, and that type must be listed first in the list of bases")
DIAGNOSTIC(30822, Error, baseClassMustBeListedFirst, "a class type may only inherit from one other class type, and that type must appear first in the list of bases")

DIAGNOSTIC(30830, Error, cannotInheritFromExplicitlySealedDeclarationInAnotherModule, "cannot inherit from type '$0' marked 'sealed' in module '$1'")
DIAGNOSTIC(30831, Error, cannotInheritFromImplicitlySealedDeclarationInAnotherModule, "cannot inherit from type '$0' in module '$1' because it is implicitly 'sealed'; mark the base type 'open' to allow inheritance across modules")
DIAGNOSTIC(30832, Error, invalidTypeForInheritance, "type '$0' cannot be used for inheritance")

DIAGNOSTIC(30850, Error, invalidExtensionOnType, "type '$0' cannot be extended. `extension` can only be used to extend a nominal type.")

// 309xx: subscripts

// 310xx: properties

// 311xx: accessors

DIAGNOSTIC(31100, Error, accessorMustBeInsideSubscriptOrProperty, "an accessor declaration is only allowed inside a subscript or property declaration")

DIAGNOSTIC(31101, Error, nonSetAccessorMustNotHaveParams, "accessors other than 'set' must not have parameters")
DIAGNOSTIC(31102, Error, setAccessorMayNotHaveMoreThanOneParam, "a 'set' accessor may not have more than one parameter")
DIAGNOSTIC(31102, Error, setAccessorParamWrongType, "'set' parameter '$0' has type '$1' which does not match the expected type '$2'")

// 39999 waiting to be placed in the right range

DIAGNOSTIC(39999, Error, expectedIntegerConstantWrongType, "expected integer constant (found: '$0')")
DIAGNOSTIC(39999, Error, expectedIntegerConstantNotConstant, "expression does not evaluate to a compile-time constant")
DIAGNOSTIC(39999, Error, expectedIntegerConstantNotLiteral, "could not extract value from integer constant")

DIAGNOSTIC(39999, Error, noApplicableOverloadForNameWithArgs, "no overload for '$0' applicable to arguments of type $1")
DIAGNOSTIC(39999, Error, noApplicableWithArgs, "no overload applicable to arguments of type $0")

DIAGNOSTIC(39999, Error, ambiguousOverloadForNameWithArgs, "ambiguous call to '$0' with arguments of type $1")
DIAGNOSTIC(39999, Error, ambiguousOverloadWithArgs, "ambiguous call to overloaded operation with arguments of type $0")

DIAGNOSTIC(39999, Note, overloadCandidate, "candidate: $0")
DIAGNOSTIC(39999, Note, moreOverloadCandidates, "$0 more overload candidates")

DIAGNOSTIC(39999, Error, caseOutsideSwitch, "'case' not allowed outside of a 'switch' statement")
DIAGNOSTIC(39999, Error, defaultOutsideSwitch, "'default' not allowed outside of a 'switch' statement")

DIAGNOSTIC(39999, Error, expectedAGeneric, "expected a generic when using '<...>' (found: '$0')")

DIAGNOSTIC(39999, Error, genericArgumentInferenceFailed, "could not specialize generic for arguments of type $0")
DIAGNOSTIC(39999, Note, genericSignatureTried, "see declaration of $0")

DIAGNOSTIC(39999, Error, ambiguousReference, "ambiguous reference to '$0'")
DIAGNOSTIC(39999, Error, ambiguousExpression, "ambiguous reference")

DIAGNOSTIC(39999, Error, declarationDidntDeclareAnything, "declaration does not declare anything")

DIAGNOSTIC(39999, Error, expectedPrefixOperator, "function called as prefix operator was not declared `__prefix`")
DIAGNOSTIC(39999, Error, expectedPostfixOperator, "function called as postfix operator was not declared `__postfix`")

DIAGNOSTIC(39999, Error, notEnoughArguments, "not enough arguments to call (got $0, expected $1)")
DIAGNOSTIC(39999, Error, tooManyArguments, "too many arguments to call (got $0, expected $1)")

DIAGNOSTIC(39999, Error, invalidIntegerLiteralSuffix, "invalid suffix '$0' on integer literal")
DIAGNOSTIC(39999, Error, invalidFloatingPointLiteralSuffix, "invalid suffix '$0' on floating-point literal")

DIAGNOSTIC(39999, Warning, integerLiteralTruncated, "integer literal '$0' too large for type '$1' truncated to '$2'")
DIAGNOSTIC(39999, Warning, floatLiteralUnrepresentable, "$0 literal '$1' unrepresentable, converted to '$2'")
DIAGNOSTIC(39999, Warning, floatLiteralTooSmall, "'$1' is smaller than the smallest representable value for type $0, converted to '$2'")

DIAGNOSTIC(39999, Error, unableToFindSymbolInModule, "unable to find the mangled symbol '$0' in module '$1'")

// 38xxx 

DIAGNOSTIC(38000, Error, entryPointFunctionNotFound, "no function found matching entry point name '$0'")
DIAGNOSTIC(38001, Error, ambiguousEntryPoint, "more than one function matches entry point name '$0'")
DIAGNOSTIC(38002, Note, entryPointCandidate, "see candidate declaration for entry point '$0'")
DIAGNOSTIC(38003, Error, entryPointSymbolNotAFunction, "entry point '$0' must be declared as a function")

DIAGNOSTIC(38004, Error, entryPointTypeParameterNotFound, "no type found matching entry-point type parameter name '$0'")
DIAGNOSTIC(38005, Error, expectedTypeForSpecializationArg, "expected a type as argument for specialization parameter '$0'")

DIAGNOSTIC(38006, Warning, specifiedStageDoesntMatchAttribute, "entry point '$0' being compiled for the '$1' stage has a '[shader(...)]' attribute that specifies the '$2' stage")
DIAGNOSTIC(38007, Error, entryPointHasNoStage, "no stage specified for entry point '$0'; use either a '[shader(\"name\")]' function attribute or the '-stage <name>' command-line option to specify a stage")

DIAGNOSTIC(38008, Error, specializationParameterOfNameNotSpecialized, "no specialization argument was provided for specialization parameter '$0'")
DIAGNOSTIC(38008, Error, specializationParameterNotSpecialized, "no specialization argument was provided for specialization parameter")

DIAGNOSTIC(38009, Error, expectedValueOfTypeForSpecializationArg, "expected a constant value of type '$0' as argument for specialization parameter '$1'")

DIAGNOSTIC(38100, Error, typeDoesntImplementInterfaceRequirement, "type '$0' does not provide required interface member '$1'")
DIAGNOSTIC(38101, Error, thisExpressionOutsideOfTypeDecl, "'this' expression can only be used in members of an aggregate type")
DIAGNOSTIC(38102, Error, initializerNotInsideType, "an 'init' declaration is only allowed inside a type or 'extension' declaration")
DIAGNOSTIC(38103, Error, thisTypeOutsideOfTypeDecl, "'This' type can only be used inside of an aggregate type")

DIAGNOSTIC(38020, Error, mismatchEntryPointTypeArgument, "expecting $0 entry-point type arguments, provided $1.")
DIAGNOSTIC(38021, Error, typeArgumentForGenericParameterDoesNotConformToInterface, "type argument `$0` for generic parameter `$1` does not conform to interface `$2`.")

DIAGNOSTIC(38022, Error, cannotSpecializeGlobalGenericToItself, "the global type parameter '$0' cannot be specialized to itself")
DIAGNOSTIC(38023, Error, cannotSpecializeGlobalGenericToAnotherGenericParam, "the global type parameter '$0' cannot be specialized using another global type parameter ('$1')")


DIAGNOSTIC(38024, Error, invalidDispatchThreadIDType, "parameter with SV_DispatchThreadID must be either scalar or vector (1 to 3) of uint/int but is $0")

DIAGNOSTIC(-1, Note, noteWhenCompilingEntryPoint, "when compiling entry point '$0'")

DIAGNOSTIC(38025, Error, mismatchSpecializationArguments, "expected $0 specialization arguments ($1 provided)")
DIAGNOSTIC(38026, Error, globalTypeArgumentDoesNotConformToInterface, "type argument `$1` for global generic parameter `$0` does not conform to interface `$2`.")

DIAGNOSTIC(38027, Error, mismatchExistentialSlotArgCount, "expected $0 existential slot arguments ($1 provided)")
DIAGNOSTIC(38029, Error,typeArgumentDoesNotConformToInterface, "type argument '$0' does not conform to the required interface '$1'")

DIAGNOSTIC(38200, Error, recursiveModuleImport, "module `$0` recursively imports itself")
DIAGNOSTIC(39999, Error, errorInImportedModule, "import of module '$0' failed because of a compilation error")
DIAGNOSTIC(39999, Fatal, complationCeased, "compilation ceased")

// 39xxx - Type layout and parameter binding.

DIAGNOSTIC(39000, Error, conflictingExplicitBindingsForParameter, "conflicting explicit bindings for parameter '$0'")
DIAGNOSTIC(39001, Warning, parameterBindingsOverlap, "explicit binding for parameter '$0' overlaps with parameter '$1'")


DIAGNOSTIC(39002, Error, shaderParameterDeclarationsDontMatch, "declarations of shader parameter '$0' in different translation units don't match")

DIAGNOSTIC(39003, Note, shaderParameterTypeMismatch, "type is declared as '$0' in one translation unit, and '$0' in another")
DIAGNOSTIC(39004, Note, fieldTypeMisMatch, "type of field '$0' is declared as '$1' in one translation unit, and '$2' in another")
DIAGNOSTIC(39005, Note, fieldDeclarationsDontMatch, "type '$0' is declared with different fields in each translation unit")
DIAGNOSTIC(39006, Note, usedInDeclarationOf, "used in declaration of '$0'")

DIAGNOSTIC(39007, Error, unknownRegisterClass, "unknown register class: '$0'")
DIAGNOSTIC(39008, Error, expectedARegisterIndex, "expected a register index after '$0'")
DIAGNOSTIC(39009, Error, expectedSpace, "expected 'space', got '$0'")
DIAGNOSTIC(39010, Error, expectedSpaceIndex, "expected a register space index after 'space'")
DIAGNOSTIC(39011, Error, componentMaskNotSupported, "explicit register component masks are not yet supported in Slang")
DIAGNOSTIC(39012, Error, packOffsetNotSupported, "explicit 'packoffset' bindings are not yet supported in Slang")
DIAGNOSTIC(39013, Warning, registerModifierButNoVulkanLayout, "shader parameter '$0' has a 'register' specified for D3D, but no '[[vk::binding(...)]]` specified for Vulkan")
DIAGNOSTIC(39014, Error, unexpectedSpecifierAfterSpace, "unexpected specifier after register space: '$0'")
DIAGNOSTIC(39015, Error, wholeSpaceParameterRequiresZeroBinding, "shader parameter '$0' consumes whole descriptor sets, so the binding must be in the form '[[vk::binding(0, ...)]]'; the non-zero binding '$1' is not allowed")

DIAGNOSTIC(39013, Error, dontExpectOutParametersForStage, "the '$0' stage does not support `out` or `inout` entry point parameters")
DIAGNOSTIC(39014, Error, dontExpectInParametersForStage, "the '$0' stage does not support `in` entry point parameters")

DIAGNOSTIC(39016, Warning, globalUniformNotExpected, "'$0' is implicitly a global shader parameter, not a global variable. If a global variable is intended, add the 'static' modifier. If a uniform shader parameter is intended, add the 'uniform' modifier to silence this warning.")

DIAGNOSTIC(39017, Error, tooManyShaderRecordConstantBuffers, "can have at most one 'shader record' attributed constant buffer; found $0.")

DIAGNOSTIC(39018, Error, typeParametersNotAllowedOnEntryPointGlobal, "local-root-signature shader parameter '$0' at global scope must not include existential/interface types")

DIAGNOSTIC(39019, Warning, vkIndexWithoutVkLocation, "ignoring '[[vk::index(...)]]` attribute without a corresponding '[[vk::location(...)]]' attribute")
DIAGNOSTIC(39020, Error, mixingImplicitAndExplicitBindingForVaryingParams, "mixing explicit and implicit bindings for varying parameters is not supported (see '$0' and '$1')")

//
// 4xxxx - IL code generation.
//
DIAGNOSTIC(40001, Error, bindingAlreadyOccupiedByComponent, "resource binding location '$0' is already occupied by component '$1'.")
DIAGNOSTIC(40002, Error, invalidBindingValue, "binding location '$0' is out of valid range.")
DIAGNOSTIC(40003, Error, bindingExceedsLimit, "binding location '$0' assigned to component '$1' exceeds maximum limit.")
DIAGNOSTIC(40004, Error, bindingAlreadyOccupiedByModule, "DescriptorSet ID '$0' is already occupied by module instance '$1'.")
DIAGNOSTIC(40005, Error, topLevelModuleUsedWithoutSpecifyingBinding, "top level module '$0' is being used without specifying binding location. Use [Binding: \"index\"] attribute to provide a binding location.")


DIAGNOSTIC(49999, Error, unknownSystemValueSemantic, "unknown system-value semantic '$0'")

DIAGNOSTIC(40006, Error, needCompileTimeConstant, "expected a compile-time constant")

DIAGNOSTIC(40007, Internal, irValidationFailed, "IR validation failed: $0")

DIAGNOSTIC(40008, Error, invalidLValueForRefParameter, "the form of this l-value argument is not valid for a `ref` parameter")

DIAGNOSTIC(40009, Error, dynamicInterfaceLacksAnyValueSizeAttribute, "interface '$0' is being used in dynamic dispatch code but has no [anyValueSize] attribute defined.")
DIAGNOSTIC(40010, Note, seeInterfaceUsage, "see usage of interface '$0'.")

DIAGNOSTIC(40011, Error, unconstrainedGenericParameterNotAllowedInDynamicFunction, "unconstrained generic paramter '$0' is not allowed in a dynamic function.")


// 41000 - IR-level validation issues

DIAGNOSTIC(41000, Warning, unreachableCode, "unreachable code detected")

DIAGNOSTIC(41010, Warning, missingReturn, "control flow may reach end of non-'void' function")

DIAGNOSTIC(41011, Error, typeDoesNotFitAnyValueSize, "type '$0' does not fit in the size required by its conforming interface.")

DIAGNOSTIC(41012, Error, typeCannotBePackedIntoAnyValue, "type '$0' contains fields that cannot be packed into an AnyValue.")

//
// 5xxxx - Target code generation.
//

DIAGNOSTIC(50010, Internal, missingExistentialBindingsForParameter, "missing argument for existential parameter slot")

DIAGNOSTIC(50020, Error, invalidTessCoordType,          "TessCoord must have vec2 or vec3 type.")
DIAGNOSTIC(50020, Error, invalidFragCoordType,          "FragCoord must be a vec4.")
DIAGNOSTIC(50020, Error, invalidInvocationIdType,       "InvocationId must have int type.")
DIAGNOSTIC(50020, Error, invalidThreadIdType,           "ThreadId must have int type.")
DIAGNOSTIC(50020, Error, invalidPrimitiveIdType,        "PrimitiveId must have int type.")
DIAGNOSTIC(50020, Error, invalidPatchVertexCountType,    "PatchVertexCount must have int type.")
DIAGNOSTIC(50022, Error, worldIsNotDefined, "world '$0' is not defined.")
DIAGNOSTIC(50023, Error, stageShouldProvideWorldAttribute, "'$0' should provide 'World' attribute.")
DIAGNOSTIC(50040, Error, componentHasInvalidTypeForPositionOutput, "'$0': component used as 'loc' output must be of vec4 type.")
DIAGNOSTIC(50041, Error, componentNotDefined, "'$0': component not defined.")

DIAGNOSTIC(50052, Error, domainShaderRequiresControlPointCount, "'DomainShader' requires attribute 'ControlPointCount'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresControlPointCount, "'HullShader' requires attribute 'ControlPointCount'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresControlPointWorld, "'HullShader' requires attribute 'ControlPointWorld'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresCornerPointWorld, "'HullShader' requires attribute 'CornerPointWorld'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresDomain, "'HullShader' requires attribute 'Domain'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresInputControlPointCount, "'HullShader' requires attribute 'InputControlPointCount'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresOutputTopology, "'HullShader' requires attribute 'OutputTopology'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresPartitioning, "'HullShader' requires attribute 'Partitioning'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresPatchWorld, "'HullShader' requires attribute 'PatchWorld'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresTessLevelInner, "'HullShader' requires attribute 'TessLevelInner'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresTessLevelOuter, "'HullShader' requires attribute 'TessLevelOuter'.")

DIAGNOSTIC(50053, Error, invalidTessellationDomian, "'Domain' should be either 'triangles' or 'quads'.")
DIAGNOSTIC(50053, Error, invalidTessellationOutputTopology, "'OutputTopology' must be one of: 'point', 'line', 'triangle_cw', or 'triangle_ccw'.")
DIAGNOSTIC(50053, Error, invalidTessellationPartitioning, "'Partitioning' must be one of: 'integer', 'pow2', 'fractional_even', or 'fractional_odd'.")
DIAGNOSTIC(50053, Error, invalidTessellationDomain,     "'Domain' should be either 'triangles' or 'quads'.")

DIAGNOSTIC(50082, Error, importingFromPackedBufferUnsupported, "importing type '$0' from PackedBuffer is not supported by the GLSL backend.")
DIAGNOSTIC(51090, Error, cannotGenerateCodeForExternComponentType, "cannot generate code for extern component type '$0'.")
DIAGNOSTIC(51091, Error, typeCannotBePlacedInATexture, "type '$0' cannot be placed in a texture.")
DIAGNOSTIC(51092, Error, stageDoesntHaveInputWorld, "'$0' doesn't appear to have any input world")

DIAGNOSTIC(50100, Error, noTypeConformancesFoundForInterface, "No type conformances are found for interface '$0'. Code generation for current target requires at least one implementation type present in the linkage.")

DIAGNOSTIC(52000, Error, multiLevelBreakUnsupported, "control flow appears to require multi-level `break`, which Slang does not yet support")

DIAGNOSTIC(52001, Warning, dxilNotFound, "dxil shared library not found, so 'dxc' output cannot be signed! Shader code will not be runnable in non-development environments.")

DIAGNOSTIC(52002, Error, passThroughCompilerNotFound, "could not find a suitable pass-through compiler for '$0'.")

DIAGNOSTIC(52004, Error, unableToWriteFile, "unable to write file '$0'")
DIAGNOSTIC(52005, Error, unableToReadFile, "unable to read file '$0'")

DIAGNOSTIC(52006, Error, compilerNotDefinedForTransition, "compiler not defined for transition '$0' to '$1'.")

DIAGNOSTIC(53001,Error, invalidTypeMarshallingForImportedDLLSymbol, "invalid type marshalling in imported func $0.")

//
// 8xxxx - Issues specific to a particular library/technology/platform/etc.
//

// 811xx - NVAPI

DIAGNOSTIC(81110, Error, nvapiMacroMismatch, "conflicting definitions for NVAPI macro '$0': '$1' and '$2'")


// 99999 - Internal compiler errors, and not-yet-classified diagnostics.

DIAGNOSTIC(99999, Internal, unimplemented, "unimplemented feature in Slang compiler: $0")
DIAGNOSTIC(99999, Internal, unexpected, "unexpected condition encountered in Slang compiler: $0")
DIAGNOSTIC(99999, Internal, internalCompilerError, "Slang internal compiler error")
DIAGNOSTIC(99999, Error, compilationAborted, "Slang compilation aborted due to internal error")
DIAGNOSTIC(99999, Error, compilationAbortedDueToException, "Slang compilation aborted due to an exception of $0: $1")
DIAGNOSTIC(99999, Internal, serialDebugVerificationFailed, "Verification of serial debug information failed.")

#undef DIAGNOSTIC
