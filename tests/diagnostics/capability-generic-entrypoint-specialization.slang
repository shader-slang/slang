// An unused specialization argument still contributes its capability requirement.
// Selecting a specialized entry point must check it just like an ordinary call.
//DIAGNOSTIC_TEST:SIMPLE(diag=UNAVAILABLE,non-exhaustive): -target hlsl -entry genericMain<MetalOnly>
//DIAGNOSTIC_TEST:SIMPLE(diag=UNAVAILABLE,non-exhaustive): -target hlsl -entry genericMain -specialize MetalOnly
//DIAGNOSTIC_TEST:SIMPLE(diag=UNAVAILABLE,non-exhaustive): -target hlsl -entry genericMain<MetalOnly> -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=UNAVAILABLE,non-exhaustive): -target hlsl -entry genericMain -specialize MetalOnly -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=UNAVAILABLE,non-exhaustive): -target hlsl -entry wrapperMain
//TEST:SIMPLE: -target hlsl -entry genericMain -no-codegen
//TEST:SIMPLE: -target hlsl -entry genericMain<Plain> -no-codegen
//TEST:SIMPLE: -target hlsl -entry genericMain -specialize Plain -no-codegen
//TEST:SIMPLE: -target metal -entry genericMain<MetalOnly> -no-codegen
//TEST:SIMPLE: -target metal -entry genericMain -specialize MetalOnly -no-codegen
//TEST:SIMPLE: -target metal -entry wrapperMain -no-codegen

// Incompatible requirements must remain incompatible regardless of argument order.
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target hlsl -entry pairMain<HlslOnly,MetalOnly>
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target hlsl -entry pairMain -specialize HlslOnly -specialize MetalOnly
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target hlsl -entry pairMain<MetalOnly,HlslOnly>
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target hlsl -entry pairMain -specialize MetalOnly -specialize HlslOnly
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target metal -entry pairMain<HlslOnly,MetalOnly>
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target metal -entry pairMain -specialize HlslOnly -specialize MetalOnly
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target metal -entry pairMain<MetalOnly,HlslOnly>
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target metal -entry pairMain -specialize MetalOnly -specialize HlslOnly
// An argument requirement must also be combined with requirements from the body.
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target hlsl -entry bodyMain<MetalOnly>
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target hlsl -entry bodyMain -specialize MetalOnly
// Generic signature types contribute requirements before the selected arguments are combined.
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target hlsl -entry signatureMain<MetalOnly>
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target hlsl -entry signatureMain -specialize MetalOnly
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target metal -entry signatureMain<MetalOnly>
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFLICT,non-exhaustive): -target metal -entry signatureMain -specialize MetalOnly

//UNAVAILABLE: E36107
//UNAVAILABLE: MetalOnly
//CONFLICT: E36107

[require(metal)]
struct MetalOnly
{
    uint value;
}

[require(hlsl)]
struct HlslOnly
{
    uint value;
}

struct Plain
{
    uint value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void genericMain<T>()
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void wrapperMain()
{
    genericMain<MetalOnly>();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void pairMain<T, U>()
{
}

[require(hlsl)]
void requireHlsl()
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void bodyMain<T>()
{
    requireHlsl();
}

[require(hlsl)]
struct GenericSignature<T>
{
    uint value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void signatureMain<T>(uniform GenericSignature<int> value)
{
}
