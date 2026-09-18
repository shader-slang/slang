// A malformed bracket-attribute name must produce a clean diagnostic and must not crash the
// checker. Regression guard for the expr-based attribute-name path (shader-slang/slang#12674): the
// parser must not synthesize a name expression from a failed identifier read, or the checker would
// later type-check a `VarExpr` built from an error token with no name.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -no-codegen

// CHECK: unexpected {{.*}}expected identifier
[]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
