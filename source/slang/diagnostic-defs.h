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
//
// 0xxxx -  Command line and interaction with host platform APIs.
//

DIAGNOSTIC(    1, Error, cannotOpenFile, "cannot open file '$0'.")
DIAGNOSTIC(    2, Error, cannotFindFile, "cannot find file '$0'.")
DIAGNOSTIC(    2, Error, unsupportedCompilerMode, "unsupported compiler mode.")
DIAGNOSTIC(    4, Error, cannotWriteOutputFile, "cannot write output file '$0'.")

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

DIAGNOSTIC(-1, Note, seeOpeningToken, "see opening '$0'")

// 153xx - #include
DIAGNOSTIC(15300, Error, includeFailed, "failed to find include file '$0'")
DIAGNOSTIC(15301, Error, importFailed, "failed to find imported file '$0'")
DIAGNOSTIC(-1, Error, noIncludeHandlerSpecified, "no `#include` handler was specified")

// 154xx - macro definition
DIAGNOSTIC(15400, Warning, macroRedefinition, "redefinition of macro '$0'")
DIAGNOSTIC(15401, Warning, macroNotDefined, "macro '$0' is not defined")
DIAGNOSTIC(15403, Error, expectedTokenInMacroParameters, "expected '$0' in macro parameters")

// 155xx - macro expansion
DIAGNOSTIC(15500, Warning, expectedTokenInMacroArguments, "expected '$0' in macro invocation")

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

DIAGNOSTIC(30001, Error, functionRedefinitionWithArgList, "'$0$1': function redefinition.")
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
DIAGNOSTIC(30016, Error, parameterCannotBeVoid, "'void' can not be parameter type.")
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
DIAGNOSTIC(30051, Error, invalidValueForArgument, "invalid value for argument '$0'")
DIAGNOSTIC(30052, Error, ordinaryFunctionAsModuleArgument, "ordinary functions not allowed as argument to function-typed module parameter.")
DIAGNOSTIC(30079, Error, selectPrdicateTypeMismatch, "selector must evaluate to bool.");
DIAGNOSTIC(30080, Error, selectValuesTypeMismatch, "the two value expressions in a select clause must have same type.");
DIAGNOSTIC(31040, Error, undefinedTypeName, "undefined type name: '$0'.")
DIAGNOSTIC(32013, Error, circularReferenceNotAllowed, "'$0': circular reference is not allowed.");
DIAGNOSTIC(32014, Error, shaderDoesProvideRequirement, "shader '$0' does not provide '$1' as required by '$2'.")
DIAGNOSTIC(32015, Error, argumentNotAvilableInWorld, "argument '$0' is not available in world '$1' as required by '$2'.")
DIAGNOSTIC(32015, Error, componentNotAvilableInWorld, "component '$0' is not available in world '$1' as required by '$2'.")
DIAGNOSTIC(32047, Error, firstArgumentToImportNotComponent, "first argument of an import operator call does not resolve to a component.");
DIAGNOSTIC(32051, Error, componentTypeNotWhatPipelineRequires, "component '$0' has type '$1', but pipeline '$2' requires it to be '$3'.")
DIAGNOSTIC(32052, Error, shaderDoesNotDefineComponentAsRequiredByPipeline, "shader '$0' does not define '$1' as required by pipeline '$2''.")
DIAGNOSTIC(33001, Error, worldNameAlreadyDefined, "world '$0' is already defined.")
DIAGNOSTIC(33002, Error, explicitPipelineSpecificationRequiredForShader, "explicit pipeline specification required for shader '$0' because multiple pipelines are defined in current context.")
DIAGNOSTIC(33003, Error, cannotDefineComponentsInAPipeline, "cannot define components in a pipeline.")
DIAGNOSTIC(33004, Error, undefinedWorldName, "undefined world name '$0'.")
DIAGNOSTIC(33005, Error, abstractWorldAsTargetOfImport, "abstract world cannot appear as target as an import operator.")

// Note(tfoley): This is a duplicate of 33004 above.
DIAGNOSTIC(33006, Error, undefinedWorldName2, "undefined world name '$0'.")

DIAGNOSTIC(33007, Error, importOperatorCircularity, "import operator '$0' creates a circular dependency between world '$1' and '$2'")
DIAGNOSTIC(33009, Error, parametersOnlyAllowedInModules, "parameters can only be defined in modules.")
DIAGNOSTIC(33010, Error, undefinedPipelineName, "pipeline '$0' is undefined.")
DIAGNOSTIC(33011, Error, shaderCircularity, "shader '$0' involves circular reference.")
DIAGNOSTIC(33012, Error, worldIsNotDefinedInPipeline, "'$0' is not a defined world in '$1'.")
DIAGNOSTIC(33013, Error, abstractWorldCannotAppearWithOthers, "abstract world cannot appear with other worlds.")
DIAGNOSTIC(33014, Error, nonAbstractComponentMustHaveImplementation, "non-abstract component must have an implementation.")
DIAGNOSTIC(33016, Error, usingInComponentDefinition, "'using': importing not allowed in component definition.")
DIAGNOSTIC(33018, Error, nameAlreadyDefined, "'$0' is already defined.")
DIAGNOSTIC(33018, Error, shaderAlreadyDefined, "shader '$0' has already been defined.")
DIAGNOSTIC(33019, Error, componentMarkedExportMustHaveWorld, "component '$0': definition marked as 'export' must have an explicitly specified world.")
DIAGNOSTIC(33020, Error, componentIsAlreadyDefined, "'$0' is already defined.")
DIAGNOSTIC(33020, Error, componentIsAlreadyDefinedInThatWorld, "'$0' is already defined at '$1'.")
DIAGNOSTIC(33021, Error, inconsistentSignatureForComponent, "'$0': inconsistent signature.")
DIAGNOSTIC(33022, Error, nameAlreadyDefinedInCurrentScope, "'$0' is already defined in current scope.")
DIAGNOSTIC(33022, Error, parameterNameConflictsWithExistingDefinition, "'$0': parameter name conflicts with existing definition.")
DIAGNOSTIC(33023, Error, parameterOfModuleIsUnassigned, "parameter '$0' of module '$1' is unassigned.")
DIAGNOSTIC(33027, Error, argumentTypeDoesNotMatchParameterType, "argument type ($0) does not match parameter type ($1)")
DIAGNOSTIC(33028, Error, nameIsNotAParameterOfCallee, "'$0' is not a parameter of '$1'.")
DIAGNOSTIC(33029, Error, requirementsClashWithPreviousDef, "'$0': requirement clash with previous definition.")
DIAGNOSTIC(33030, Error, positionArgumentAfterNamed, "positional argument cannot appear after a named argument.")
DIAGNOSTIC(33032, Error, functionRedefinition, "'$0': function redefinition.")
DIAGNOSTIC(33034, Error, recordTypeVariableInImportOperator, "cannot declare a record-typed variable in an import operator.")
DIAGNOSTIC(33037, Error, componetMarkedExportCannotHaveParameters, "component '$0': definition marked as 'export' cannot have parameters.")
DIAGNOSTIC(33039, Error, componentInInputWorldCantHaveCode, "'$0': no code allowed for component defined in input world.")
DIAGNOSTIC(33040, Error, requireWithComputation, "'require': cannot define computation on component requirements.")
DIAGNOSTIC(33042, Error, paramWithComputation, "'param': cannot define computation on parameters.")
DIAGNOSTIC(33041, Error, pipelineOfModuleIncompatibleWithPipelineOfShader, "pipeline '$0' targeted by module '$1' is incompatible with pipeline '$2' targeted by shader '$3'.")
DIAGNOSTIC(33070, Error, expectedFunction, "expression preceding parenthesis of apparent call must have function type.")
DIAGNOSTIC(33071, Error, importOperatorCalledFromAutoPlacedComponent, "cannot call an import operator from an auto-placed component '$0'. try qualify the component with explicit worlds.")
DIAGNOSTIC(33072, Error, noApplicableImportOperator, "'$0' is an import operator defined in pipeline '$1', but none of the import operator overloads converting to world '$2' matches argument list ($3).")
DIAGNOSTIC(33073, Error, importOperatorCalledFromMultiWorldComponent, "cannot call an import operator from a multi-world component definition. consider qualify the component with only one explicit world.")
DIAGNOSTIC(33080, Error, componentTypeDoesNotMatchInterface, "'$0': component type does not match definition in interface '$1'.")
DIAGNOSTIC(33081, Error, shaderDidNotDefineComponentFunction, "shader '$0' did not define component function $1 as required by interface '$2'.")
DIAGNOSTIC(33082, Error, shaderDidNotDefineComponent, "shader '$0' did not define component '$1' as required by interface '$2'.")
DIAGNOSTIC(33083, Error, interfaceImplMustBePublic, "'$0': component fulfilling interface '$1' must be declared as 'public'.")
DIAGNOSTIC(33084, Error, defaultParamNotAllowedInInterface, "'$0': default parameter value not allowed in interface definition.")

DIAGNOSTIC(33100, Error, componentCantBeComputedAtWorldBecauseDependentNotAvailable, "'$0' cannot be computed at '$1' because the dependent component '$2' is not accessible.")
DIAGNOSTIC(33101, Warning, worldIsNotAValidChoiceForKey, "'$0' is not a valid choice for '$1'.")
DIAGNOSTIC(33102, Error, componentDefinitionCircularity, "component definition '$0' involves circular reference.")
DIAGNOSTIC(34024, Error, componentAlreadyDefinedWhenCompiling, "component named '$0' is already defined when compiling '$1'.")
DIAGNOSTIC(34025, Error, globalComponentConflictWithPreviousDeclaration, "'$0': global component conflicts with previous declaration.")
DIAGNOSTIC(34026, Warning, componentIsAlreadyDefinedUseRequire, "'$0': component is already defined when compiling shader '$1'. use 'require' to declare it as a parameter.")
DIAGNOSTIC(34062, Error, cylicReference, "cyclic reference: $0");
DIAGNOSTIC(34064, Error, noApplicableImplicitImportOperator, "cannot find import operator to import component '$0' to world '$1' when compiling '$2'.")
DIAGNOSTIC(34065, Error, resourceTypeMustBeParamOrRequire, "'$0': resource typed component must be declared as 'param' or 'require'.");
DIAGNOSTIC(34066, Error, cannotDefineComputationOnResourceType, "'$0': cannot define computation on resource typed component.");

DIAGNOSTIC(35001, Error, fragDepthAttributeCanOnlyApplyToOutput, "FragDepth attribute can only apply to an output component.");
DIAGNOSTIC(35002, Error, fragDepthAttributeCanOnlyApplyToFloatComponent, "FragDepth attribute can only apply to a float component.");


DIAGNOSTIC(36001, Error, insufficientTemplateShaderArguments, "instantiating template shader '$0': insufficient arguments.");
DIAGNOSTIC(36002, Error, tooManyTemplateShaderArguments, "instantiating template shader '$0': too many arguments.");
DIAGNOSTIC(36003, Error, templateShaderArgumentIsNotDefined, "'$0' provided as template shader argument to '$1' is not a defined module.");
DIAGNOSTIC(36004, Error, templateShaderArgumentDidNotImplementRequiredInterface, "module '$0' provided as template shader argument to '$1' did not implement required interface '$2'.");

// TODO: need to assign numbers to all these extra diagnostics...

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

//
// 4xxxx - IL code generation.
//
DIAGNOSTIC(40001, Error, bindingAlreadyOccupiedByComponent, "resource binding location '$0' is already occupied by component '$1'.")
DIAGNOSTIC(40002, Error, invalidBindingValue, "binding location '$0' is out of valid range.")
DIAGNOSTIC(40003, Error, bindingExceedsLimit, "binding location '$0' assigned to component '$1' exceeds maximum limit.")
DIAGNOSTIC(40004, Error, bindingAlreadyOccupiedByModule, "DescriptorSet ID '$0' is already occupied by module instance '$1'.")
DIAGNOSTIC(40005, Error, topLevelModuleUsedWithoutSpecifyingBinding, "top level module '$0' is being used without specifying binding location. Use [Binding: \"index\"] attribute to provide a binding location.")
//
// 5xxxx - Target code generation.
//

DIAGNOSTIC(50020, Error, unknownStageType,              "Unknown stage type '$0'.")
DIAGNOSTIC(50020, Error, invalidTessCoordType,          "TessCoord must have vec2 or vec3 type.")
DIAGNOSTIC(50020, Error, invalidFragCoordType,          "FragCoord must be a vec4.")
DIAGNOSTIC(50020, Error, invalidInvocationIdType,       "InvocationId must have int type.")
DIAGNOSTIC(50020, Error, invalidThreadIdType,           "ThreadId must have int type.")
DIAGNOSTIC(50020, Error, invalidPrimitiveIdType,        "PrimitiveId must have int type.")
DIAGNOSTIC(50020, Error, invalidPatchVertexCountType,    "PatchVertexCount must have int type.")
DIAGNOSTIC(50022, Error, worldIsNotDefined, "world '$0' is not defined.");
DIAGNOSTIC(50023, Error, stageShouldProvideWorldAttribute, "'$0' should provide 'World' attribute.");
DIAGNOSTIC(50040, Error, componentHasInvalidTypeForPositionOutput, "'$0': component used as 'Position' output must be of vec4 type.")
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


// 99999 - Internal compiler errors, and not-yet-classified diagnostics.

DIAGNOSTIC(99999, Internal, internalCompilerError, "internal compiler error")
DIAGNOSTIC(99999, Internal, unimplemented, "unimplemented feature: $0")

#undef DIAGNOSTIC
