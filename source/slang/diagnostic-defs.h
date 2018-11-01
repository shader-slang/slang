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

DIAGNOSTIC(-1, Note, alsoSeePipelineDefinition, "also see pipeline definition");
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
DIAGNOSTIC(    11, Error, glslIsNotSupported, "the Slang compiler does not support GLSL as a source language");
DIAGNOSTIC(    12, Error, cannotDeduceSourceLanguage, "can't deduce language for input file '$0'");
DIAGNOSTIC(    13, Error, unknownCodeGenerationTarget, "unknown code generation target '$0'");
DIAGNOSTIC(    14, Error, unknownProfile, "unknown profile '$0'");
DIAGNOSTIC(    15, Error, unknownStage, "unknown stage '$0'");
DIAGNOSTIC(    16, Error, unknownPassThroughTarget, "unknown pass-through target '$0'");
DIAGNOSTIC(    17, Error, unknownCommandLineOption, "unknown command-line option '$0'");
DIAGNOSTIC(    20, Error, entryPointsNeedToBeAssociatedWithTranslationUnits, "when using multiple source files, entry points must be specified after their corresponding source file(s)");
DIAGNOSTIC(    21, Error, expectedArgumentForOption, "expected an argument for command-line option '$0'");

DIAGNOSTIC(    24, Error, unknownLineDirectiveMode, "unknown '#line' directive mode '$0'");
DIAGNOSTIC(    25, Error, unknownFloatingPointMode, "unknown floating-point mode '$0'");

DIAGNOSTIC(    30, Error, sameStageSpecifiedMoreThanOnce, "the stage '$0' was specified more than once for entry point '$1'")
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

//
// 1xxxx - Lexical anaylsis
//

DIAGNOSTIC(10000, Error, illegalCharacterPrint, "illegal character '$0'");
DIAGNOSTIC(10000, Error, illegalCharacterHex, "illegal character (0x$0)");
DIAGNOSTIC(10001, Error, illegalCharacterLiteral, "illegal character literal");

DIAGNOSTIC(10002, Warning, octalLiteral, "'0' prefix indicates octal literal")
DIAGNOSTIC(10003, Error, invalidDigitForBase, "invalid digit for base-$1 literal: '$0'")

DIAGNOSTIC(10004, Error, endOfFileInLiteral, "end of file in literal");
DIAGNOSTIC(10005, Error, newlineInLiteral, "newline in literal");

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
DIAGNOSTIC(15200, Error, expectedTokenInPreprocessorExpression, "expected '$0' in preprocessor expression");
DIAGNOSTIC(15201, Error, syntaxErrorInPreprocessorExpression, "syntax error in preprocessor expression");
DIAGNOSTIC(15202, Error, divideByZeroInPreprocessorExpression, "division by zero in preprocessor expression");
DIAGNOSTIC(15203, Error, expectedTokenInDefinedExpression, "expected '$0' in 'defined' expression");
DIAGNOSTIC(15204, Warning, directiveExpectsExpression, "'$0' directive requires an expression");
DIAGNOSTIC(15205, Warning, undefinedIdentifierInPreprocessorExpression, "undefined idenfier '$0' in preprocessor expression will evaluate to zero")

DIAGNOSTIC(-1, Note, seeOpeningToken, "see opening '$0'")

// 153xx - #include
DIAGNOSTIC(15300, Error, includeFailed, "failed to find include file '$0'")
DIAGNOSTIC(15301, Error, importFailed, "failed to find imported file '$0'")
DIAGNOSTIC(-1, Error, noIncludeHandlerSpecified, "no `#include` handler was specified")
DIAGNOSTIC(15302, Error, noCanonicalPath, "`#include` handler didn't generate a canonical path for '$0'")


// 154xx - macro definition
DIAGNOSTIC(15400, Warning, macroRedefinition, "redefinition of macro '$0'")
DIAGNOSTIC(15401, Warning, macroNotDefined, "macro '$0' is not defined")
DIAGNOSTIC(15403, Error, expectedTokenInMacroParameters, "expected '$0' in macro parameters")

// 155xx - macro expansion
DIAGNOSTIC(15500, Warning, expectedTokenInMacroArguments, "expected '$0' in macro invocation")
DIAGNOSTIC(15501, Error, wrongNumberOfArgumentsToMacro, "wrong number of arguments to macro (expected $0, got $1)")

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

DIAGNOSTIC(20003, Error, unexpectedToken, "unexpected $0");
DIAGNOSTIC(20001, Error, unexpectedTokenExpectedTokenType, "unexpected $0, expected $1");
DIAGNOSTIC(20001, Error, unexpectedTokenExpectedTokenName, "unexpected $0, expected '$1'");

DIAGNOSTIC(0, Error, tokenNameExpectedButEOF, "\"$0\" expected but end of file encountered.");
DIAGNOSTIC(0, Error, tokenTypeExpectedButEOF, "$0 expected but end of file encountered.");
DIAGNOSTIC(20001, Error, tokenNameExpected, "\"$0\" expected");
DIAGNOSTIC(20001, Error, tokenNameExpectedButEOF2, "\"$0\" expected but end of file encountered.");
DIAGNOSTIC(20001, Error, tokenTypeExpected, "$0 expected");
DIAGNOSTIC(20001, Error, tokenTypeExpectedButEOF2, "$0 expected but end of file encountered.");
DIAGNOSTIC(20001, Error, typeNameExpectedBut, "unexpected $0, expected type name");
DIAGNOSTIC(20001, Error, typeNameExpectedButEOF, "type name expected but end of file encountered.");
DIAGNOSTIC(20001, Error, unexpectedEOF, " Unexpected end of file.");
DIAGNOSTIC(20002, Error, syntaxError, "syntax error.");
DIAGNOSTIC(20004, Error, unexpectedTokenExpectedComponentDefinition, "unexpected token '$0', only component definitions are allowed in a shader scope.")
DIAGNOSTIC(20008, Error, invalidOperator, "invalid operator '$0'.");
DIAGNOSTIC(20011, Error, unexpectedColon, "unexpected ':'.")

//
// 3xxxx - Semantic analysis
//

DIAGNOSTIC(30002, Error, parameterAlreadyDefined, "parameter '$0' already defined.")
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
DIAGNOSTIC(30015, Error, undefinedIdentifier, "'$0': undefined identifier.")
DIAGNOSTIC(30015, Error, undefinedIdentifier2, "undefined identifier '$0'.")
DIAGNOSTIC(30017, Error, componentNotAccessibleFromShader, "component '$0' is not accessible from shader '$1'.")
DIAGNOSTIC(30019, Error, typeMismatch, "expected an expression of type '$0', got '$1'")
DIAGNOSTIC(30020, Error, importOperatorReturnTypeMismatch, "import operator should return '$1', but the expression has type '$0''. do you forget 'project'?")
DIAGNOSTIC(30021, Error, noApplicationFunction, "$0: no overload takes arguments ($1)")
DIAGNOSTIC(30022, Error, invalidTypeCast, "invalid type cast between \"$0\" and \"$1\".")
DIAGNOSTIC(30023, Error, typeHasNoPublicMemberOfName, "\"$0\" does not have public member \"$1\".");
DIAGNOSTIC(30025, Error, invalidArraySize, "array size must be larger than zero.")
DIAGNOSTIC(30026, Error, returnInComponentMustComeLast, "'return' can only appear as the last statement in component definition.")
DIAGNOSTIC(30027, Error, noMemberOfNameInType, "'$0' is not a member of '$1'.");
DIAGNOSTIC(30028, Error, forPredicateTypeError, "'for': predicate expression must evaluate to bool.")
DIAGNOSTIC(30030, Error, projectionOutsideImportOperator, "'project': invalid use outside import operator.")
DIAGNOSTIC(30031, Error, projectTypeMismatch, "'project': expression must evaluate to record type '$0'.")
DIAGNOSTIC(30033, Error, invalidTypeForLocalVariable, "cannot declare a local variable of this type.")
DIAGNOSTIC(30035, Error, componentOverloadTypeMismatch, "'$0': type of overloaded component mismatches previous definition.")
DIAGNOSTIC(30041, Error, bitOperationNonIntegral, "bit operation: operand must be integral type.")
DIAGNOSTIC(30047, Error, argumentExpectedLValue, "argument passed to parameter '$0' must be l-value.")
DIAGNOSTIC(30048, Note,  implicitCastUsedAsLValue, "argument was implicitly cast from '$0' to '$1', and Slang does not support using an implicit cast as an l-value")
DIAGNOSTIC(30049, Note,  thisIsImmutableByDefault, "a 'this' parameter is an immutable parameter by default in Slang; apply the `[mutating]` attribute to the function declaration to opt in to a mutable `this`")

DIAGNOSTIC(30051, Error, invalidValueForArgument, "invalid value for argument '$0'")
DIAGNOSTIC(30052, Error, invalidSwizzleExpr, "invalid swizzle pattern '$0' on type '$1'")

DIAGNOSTIC(30100, Error, staticRefToNonStaticMember, "type '$0' cannot be used to refer to non-static member '$1'")

DIAGNOSTIC(30201, Error, functionRedefinition, "function '$0' already has a body")
DIAGNOSTIC(30202, Error, functionRedeclarationWithDifferentReturnType, "function '$0' declared to return '$1' was previously declared to return '$2'")


DIAGNOSTIC(33070, Error, expectedFunction, "expression preceding parenthesis of apparent call must have function type.")
DIAGNOSTIC(33071, Error, expectedAStringLiteral, "expected a string literal")



// Attributes
DIAGNOSTIC(31000, Error, unknownAttributeName, "unknown attribute '$0'")
DIAGNOSTIC(31001, Error, attributeArgumentCountMismatch, "attribute '$0' expects $1 arguments ($2 provided)")
DIAGNOSTIC(31002, Error, attributeNotApplicable, "attribute '$0' is not valid here")

DIAGNOSTIC(31003, Error, badlyDefinedPatchConstantFunc, "hull shader '$0' has has badly defined 'patchconstantfunc' attribute.");

DIAGNOSTIC(31004, Error, expectedSingleIntArg, "attribute '$0' expects a single int argument")
DIAGNOSTIC(31005, Error, expectedSingleStringArg, "attribute '$0' expects a single string argument")

DIAGNOSTIC(31006, Error, attributeFunctionNotFound, "Could not find function '$0' for attribute'$1'")


DIAGNOSTIC(31100, Error, unknownStageName,  "unknown stage name '$0'")



// Enums

DIAGNOSTIC(32000, Error, invalidEnumTagType,        "invalid tag type for 'enum': '$0'")
DIAGNOSTIC(32001, Error, enumTypeAlreadyHasTagType, "'enum' type has already declared a tag type")
DIAGNOSTIC(32002, Note,  seePreviousTagType,        "see previous tag type declaration")
DIAGNOSTIC(32003, Error, unexpectedEnumTagExpr,     "unexpected form for 'enum' tag value expression")



// 303xx: interfaces and associated types
DIAGNOSTIC(30300, Error, assocTypeInInterfaceOnly, "'associatedtype' can only be defined in an 'interface'.")
DIAGNOSTIC(30301, Error, globalGenParamInGlobalScopeOnly, "'__generic_param' can only be defined global scope.")
// TODO: need to assign numbers to all these extra diagnostics...
DIAGNOSTIC(39999, Fatal, cyclicReference, "cyclic reference '$0'.")
DIAGNOSTIC(39999, Fatal, localVariableUsedBeforeDeclared, "local variable '$0' is being used before its declaration.")

// 304xx: generics
DIAGNOSTIC(30400, Error, genericTypeNeedsArgs, "generic type '$0' used without argument")

DIAGNOSTIC(39999, Error, expectedIntegerConstantWrongType, "expected integer constant (found: '$0')")
DIAGNOSTIC(39999, Error, expectedIntegerConstantNotConstant, "expression does not evaluate to a compile-time constant")
DIAGNOSTIC(39999, Error, expectedIntegerConstantNotLiteral, "could not extract value from integer constant")

DIAGNOSTIC(39999, Error, noApplicableOverloadForNameWithArgs, "no overload for '$0' applicable to arguments of type $1")
DIAGNOSTIC(39999, Error, noApplicableWithArgs, "no overload applicable to arguments of type $0")

DIAGNOSTIC(39999, Error, ambiguousOverloadForNameWithArgs, "ambiguous call to '$0' operation with arguments of type $1")
DIAGNOSTIC(39999, Error, ambiguousOverloadWithArgs, "ambiguous call to overloaded operation with arguments of type $0")

DIAGNOSTIC(39999, Note, overloadCandidate, "candidate: $0")
DIAGNOSTIC(39999, Note, moreOverloadCandidates, "$0 more overload candidates")

DIAGNOSTIC(39999, Error, caseOutsideSwitch, "'case' not allowed outside of a 'switch' statement")
DIAGNOSTIC(39999, Error, defaultOutsideSwitch, "'default' not allowed outside of a 'switch' statement")

DIAGNOSTIC(39999, Error, expectedAGeneric, "expected a generic when using '<...>' (found: '$0')")

DIAGNOSTIC(39999, Error, genericArgumentInferenceFailed, "could not specialize generic for arguments of type $0")
DIAGNOSTIC(39999, Note, genericSignatureTried, "see declaration of $0")

DIAGNOSTIC(39999, Error, expectedAnInterfaceGot, "expected an interface, got '$0'")

DIAGNOSTIC(39999, Error, ambiguousReference, "amiguous reference to '$0'");

DIAGNOSTIC(39999, Error, declarationDidntDeclareAnything, "declaration does not declare anything");


DIAGNOSTIC(39999, Error, expectedPrefixOperator, "function called as prefix operator was not declared `__prefix`")
DIAGNOSTIC(39999, Error, expectedPostfixOperator, "function called as postfix operator was not declared `__postfix`")

DIAGNOSTIC(39999, Error, notEnoughArguments, "not enough arguments to call (got $0, expected $1)")
DIAGNOSTIC(39999, Error, tooManyArguments, "too many arguments to call (got $0, expected $1)")

DIAGNOSTIC(39999, Error, invalidIntegerLiteralSuffix, "invalid suffix '$0' on integer literal")
DIAGNOSTIC(39999, Error, invalidFloatingPOintLiteralSuffix, "invalid suffix '$0' on floating-point literal")



DIAGNOSTIC(38000, Error, entryPointFunctionNotFound, "no function found matching entry point name '$0'")
DIAGNOSTIC(38001, Error, ambiguousEntryPoint, "more than one function matches entry point name '$0'")
DIAGNOSTIC(38002, Note, entryPointCandidate, "see candidate declaration for entry point '$0'")
DIAGNOSTIC(38003, Error, entryPointSymbolNotAFunction, "entry point '$0' must be declared as a function")

DIAGNOSTIC(38004, Error, entryPointTypeParameterNotFound, "no type found matching entry-point type parameter name '$0'")
DIAGNOSTIC(38005, Error, entryPointTypeSymbolNotAType, "entry-point type parameter '$0' must be declared as a type")

DIAGNOSTIC(38006, Warning, specifiedStageDoesntMatchAttribute, "entry point '$0' being compiled for the '$1' stage has a '[shader(...)]' attribute that specifies the '$2' stage")
DIAGNOSTIC(38007, Error, entryPointHasNoStage, "no stage specified for entry point '$0'; use either a '[shader(\"name\")]' function attribute or the '-stage <name>' command-line option to specify a stage")

DIAGNOSTIC(38100, Error, typeDoesntImplementInterfaceRequirement, "type '$0' does not provide required interface member '$1'")
DIAGNOSTIC(38101, Error, thisExpressionOutsideOfTypeDecl, "'this' expression can only be used in members of an aggregate type")
DIAGNOSTIC(38102, Error, initializerNotInsideType, "an 'init' declaration is only allowed inside a type or 'extension' declaration")
DIAGNOSTIC(38102, Error, accessorMustBeInsideSubscriptOrProperty, "an accessor declaration is only allowed inside a subscript or property declaration")

DIAGNOSTIC(38020, Error, mismatchEntryPointTypeArgument, "expecting $0 entry-point type arguments, provided $1.")
DIAGNOSTIC(38021, Error, typeArgumentDoesNotConformToInterface, "type argument `$1` for generic parameter `$0` does not conform to interface `$1`.")

DIAGNOSTIC(38022, Error, cannotSpecializeGlobalGenericToItself, "the global type parameter '$0' cannot be specialized to itself")
DIAGNOSTIC(38023, Error, cannotSpecializeGlobalGenericToAnotherGenericParam, "the global type parameter '$0' cannot be specialized using another global type parameter ('$1')")
DIAGNOSTIC(-1, Note, noteWhenCompilingEntryPoint, "when compiling entry point '$0'")

DIAGNOSTIC(38200, Error, recursiveModuleImport, "module `$0` recursively imports itself")
DIAGNOSTIC(39999, Fatal, errorInImportedModule, "error in imported module, compilation ceased.")


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

DIAGNOSTIC(39013, Error, dontExpectOutParametersForStage, "the '$0' stage does not support `out` or `inout` entry point parameters")
DIAGNOSTIC(39014, Error, dontExpectInParametersForStage, "the '$0' stage does not support `in` entry point parameters")

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

// 41000 - IR-level validation issues

DIAGNOSTIC(41000, Warning, unreachableCode, "unreachable code detected")

DIAGNOSTIC(41010, Warning, missingReturn, "control flow may reach end of non-'void' function")

//
// 5xxxx - Target code generation.
//
DIAGNOSTIC(50020, Error, invalidTessCoordType,          "TessCoord must have vec2 or vec3 type.")
DIAGNOSTIC(50020, Error, invalidFragCoordType,          "FragCoord must be a vec4.")
DIAGNOSTIC(50020, Error, invalidInvocationIdType,       "InvocationId must have int type.")
DIAGNOSTIC(50020, Error, invalidThreadIdType,           "ThreadId must have int type.")
DIAGNOSTIC(50020, Error, invalidPrimitiveIdType,        "PrimitiveId must have int type.")
DIAGNOSTIC(50020, Error, invalidPatchVertexCountType,    "PatchVertexCount must have int type.")
DIAGNOSTIC(50022, Error, worldIsNotDefined, "world '$0' is not defined.");
DIAGNOSTIC(50023, Error, stageShouldProvideWorldAttribute, "'$0' should provide 'World' attribute.");
DIAGNOSTIC(50040, Error, componentHasInvalidTypeForPositionOutput, "'$0': component used as 'loc' output must be of vec4 type.")
DIAGNOSTIC(50041, Error, componentNotDefined, "'$0': component not defined.")

DIAGNOSTIC(50052, Error, domainShaderRequiresControlPointCount, "'DomainShader' requires attribute 'ControlPointCount'.");
DIAGNOSTIC(50052, Error, hullShaderRequiresControlPointCount, "'HullShader' requires attribute 'ControlPointCount'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresControlPointWorld, "'HullShader' requires attribute 'ControlPointWorld'.");
DIAGNOSTIC(50052, Error, hullShaderRequiresCornerPointWorld, "'HullShader' requires attribute 'CornerPointWorld'.");
DIAGNOSTIC(50052, Error, hullShaderRequiresDomain, "'HullShader' requires attribute 'Domain'.");
DIAGNOSTIC(50052, Error, hullShaderRequiresInputControlPointCount, "'HullShader' requires attribute 'InputControlPointCount'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresOutputTopology, "'HullShader' requires attribute 'OutputTopology'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresPartitioning, "'HullShader' requires attribute 'Partitioning'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresPatchWorld, "'HullShader' requires attribute 'PatchWorld'.");
DIAGNOSTIC(50052, Error, hullShaderRequiresTessLevelInner, "'HullShader' requires attribute 'TessLevelInner'.")
DIAGNOSTIC(50052, Error, hullShaderRequiresTessLevelOuter, "'HullShader' requires attribute 'TessLevelOuter'.")

DIAGNOSTIC(50053, Error, invalidTessellationDomian, "'Domain' should be either 'triangles' or 'quads'.");
DIAGNOSTIC(50053, Error, invalidTessellationOutputTopology, "'OutputTopology' must be one of: 'point', 'line', 'triangle_cw', or 'triangle_ccw'.");
DIAGNOSTIC(50053, Error, invalidTessellationPartitioning, "'Partitioning' must be one of: 'integer', 'pow2', 'fractional_even', or 'fractional_odd'.")
DIAGNOSTIC(50053, Error, invalidTessellationDomain,     "'Domain' should be either 'triangles' or 'quads'.")

DIAGNOSTIC(50082, Error, importingFromPackedBufferUnsupported, "importing type '$0' from PackedBuffer is not supported by the GLSL backend.")
DIAGNOSTIC(51090, Error, cannotGenerateCodeForExternComponentType, "cannot generate code for extern component type '$0'.")
DIAGNOSTIC(51091, Error, typeCannotBePlacedInATexture, "type '$0' cannot be placed in a texture.")
DIAGNOSTIC(51092, Error, stageDoesntHaveInputWorld, "'$0' doesn't appear to have any input world");

DIAGNOSTIC(52000, Error, multiLevelBreakUnsupported, "control flow appears to require multi-level `break`, which Slang does not yet support");


// 99999 - Internal compiler errors, and not-yet-classified diagnostics.

DIAGNOSTIC(99999, Internal, unimplemented, "unimplemented feature in Slang compiler: $0")
DIAGNOSTIC(99999, Internal, unexpected, "unexpected condition encountered in Slang compiler: $0")
DIAGNOSTIC(99999, Internal, internalCompilerError, "Slang internal compiler error")
DIAGNOSTIC(99999, Error, compilationAborted, "Slang compilation aborted due to internal error");
DIAGNOSTIC(99999, Error, compilationAbortedDueToException, "Slang compilation aborted due to an exception of $0: $1");
DIAGNOSTIC(99999, Note, noteLocationOfInternalError, "the Slang compiler threw an exception while working on code near this location");

#undef DIAGNOSTIC
