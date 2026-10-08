// Regression: a capability requirement carried by a decl used as an *operand* of a fast-path
// builtin operator must still be observed by the capability walk. `BuiltinOperatorExpr` is not an
// `InvokeExpr`, so `SemanticsDeclReferenceVisitor` (the base of `CapabilityDeclReferenceVisitor`)
// needs its own recursion into the operands; without it, `main`'s use of `needsNonUniform()` inside
// `needsNonUniform() + 1u` would be silently missed and no "undeclared capability" diagnostic
// would fire.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -emit-spirv-directly -entry main -stage compute

RWStructuredBuffer<uint> outputBuffer;

[require(spvShaderNonUniform)]
uint needsNonUniform()
/*CHECK:
     ^^^^^^^^^^^^^^^ see definition of 'needsNonUniform'
*/
{
    return 1;
}

[require(spirv, shaderclock)]
void main()
/*CHECK:
     ^^^^ undeclared capability used
     ^^^^ 'main' uses undeclared capability 'spvShaderNonUniform'
*/
{
    // The `+` is a fast-path BuiltinOperatorExpr; the capability walk must recurse into its
    // operand `needsNonUniform()` to observe the requirement.
    outputBuffer[0] = needsNonUniform() + 1u;
/*CHECK:
                      ^^^^^^^^^^^^^^^ see using of 'needsNonUniform'
*/
}
