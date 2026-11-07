// Generic member aliases contribute their target requirements before specialization.
// Regression for https://github.com/shader-slang/slang/issues/13793.
//DIAGNOSTIC_TEST:SIMPLE(diag=DIRECT,non-exhaustive): -target hlsl -entry directMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC,non-exhaustive): -target hlsl -entry genericMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry directMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry genericMain -stage compute -no-codegen
//TEST:SIMPLE: -target hlsl -entry unrestrictedMain -stage compute -no-codegen

[require(metal)]
struct MetalElement<U>
{
}

struct GenericAliasContext
{
    typealias Element<U> = MetalElement<U>;
}

struct Input<T>
{
}

[numthreads(1, 1, 1)]
void directMain(uniform GenericAliasContext input)
/*DIRECT:
     ^^^^^^^^^^ error E36107
*/
{
}

[numthreads(1, 1, 1)]
void genericMain(uniform Input<GenericAliasContext> input)
/*GENERIC:
     ^^^^^^^^^^^ error E36107
*/
{
}

// An unconstrained generic parameter does not impose a target requirement.
struct UnrestrictedAliasContext
{
    typealias Element<U> = U;
}

[numthreads(1, 1, 1)]
void unrestrictedMain(uniform UnrestrictedAliasContext input)
{
}
