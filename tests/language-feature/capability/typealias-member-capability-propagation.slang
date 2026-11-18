// A member alias propagates its target type's requirements to the containing struct.
// Regression for https://github.com/shader-slang/slang/issues/13793.
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC,non-exhaustive): -target hlsl -entry main -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=DIRECT,non-exhaustive): -target hlsl -entry directMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=TRANSITIVE,non-exhaustive): -target hlsl -entry transitiveMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=SUBSTITUTED,non-exhaustive): -target hlsl -entry substitutedMain -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=ALIAS_GENERIC,non-exhaustive): -target hlsl -entry aliasGenericMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry main -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry transitiveMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry substitutedMain -stage compute -no-codegen
//TEST:SIMPLE: -target metal -entry aliasGenericMain -stage compute -no-codegen

[require(metal)]
struct MetalOnly
{
}

struct Context
{
    typealias Element = MetalOnly;
}

struct Input<T>
{
}

[numthreads(1, 1, 1)]
void main(uniform Input<Context> input)
/*GENERIC:
     ^^^^ error E36107
*/
{
}

[numthreads(1, 1, 1)]
void directMain(uniform Context input)
/*DIRECT:
     ^^^^^^^^^^ error E36107
*/
{
}

struct OuterContext
{
    typealias Inner = Context;
}

[numthreads(1, 1, 1)]
void transitiveMain(uniform Input<OuterContext> input)
/*TRANSITIVE:
     ^^^^^^^^^^^^^^ error E36107
*/
{
}

[require(metal)]
struct MetalElement<T>
{
}

struct GenericContext<T>
{
    typealias Element = MetalElement<T>;
}

[numthreads(1, 1, 1)]
void substitutedMain(uniform GenericContext<int> input)
/*SUBSTITUTED:
     ^^^^^^^^^^^^^^^ error E36107
*/
{
}

// The alias's generic target has requirements beyond those of its argument `int`.
struct AliasOfGeneric
{
    typealias Element = GenericContext<int>;
}

[numthreads(1, 1, 1)]
void aliasGenericMain(uniform Input<AliasOfGeneric> input)
/*ALIAS_GENERIC:
     ^^^^^^^^^^^^^^^^ error E36107
*/
{
}
