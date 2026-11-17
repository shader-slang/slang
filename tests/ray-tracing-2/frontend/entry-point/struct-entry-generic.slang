//TEST:SIMPLE: -experimental-feature -entry M<P> -stage miss -no-codegen
//TEST:SIMPLE: -experimental-feature -entry M<P>.invoke -stage miss -no-codegen
//TEST:SIMPLE(filecheck=WARNING): -experimental-feature -entry ValueMiss<1.5> -stage miss -no-codegen
//TEST:SIMPLE(filecheck=WARNING): -experimental-feature -entry ValueMiss<1.5>.invoke -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ERROR,exit-code=-1): -experimental-feature -entry ValueMiss<1.5> -stage miss -no-codegen -warnings-as-errors all
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ERROR,exit-code=-1): -experimental-feature -entry ValueMiss<1.5>.invoke -stage miss -no-codegen -warnings-as-errors all
// Selecting the bare stage name with separate specialization arguments chooses the same
// specialization as binding the arguments in the name. An unspecialized selection is legal
// while no code will be generated, but must be specialized for a full compilation.
//TEST:SIMPLE: -experimental-feature -entry M -stage miss -no-codegen
//TEST:SIMPLE: -experimental-feature -entry M -specialize P -stage miss -no-codegen
//TEST:SIMPLE: -experimental-feature -entry M.invoke -specialize P -stage miss -no-codegen
//TEST:SIMPLE: -experimental-feature -entry ValueMiss -specialize 2 -stage miss -no-codegen
//TEST:SIMPLE(filecheck=WARNING): -experimental-feature -entry ValueMiss -specialize 1.5 -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ERROR,exit-code=-1): -experimental-feature -entry ValueMiss -specialize 1.5 -stage miss -no-codegen -warnings-as-errors all
//DIAGNOSTIC_TEST:SIMPLE(diag=UNSPEC): -experimental-feature -entry M -stage miss -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=COUNT): -experimental-feature -entry M -specialize P -specialize R -stage miss -no-codegen

import slang.raytracing.portable;

struct P {}
struct R {}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContext<PayloadType> : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadType;
    typealias Record = R;
}

// `M<P>` chooses `P` for `PayloadType`. Selecting bare `M` and supplying separate
// specialization arguments chooses the same specialization. The `-no-codegen` tests check
// both spellings, and an unspecialized selection stays checkable while no code is generated.
struct M<PayloadType> : rt::IMissShader
{
    typealias Context = MissContext<PayloadType>;

    static void invoke(in rt::MissInput<Context> input) {}
/*UNSPEC:
generic entry point 'M' must be specialized with concrete generic arguments
*/
/*COUNT:
expected 1 specialization arguments (2 provided)
*/
}

// The tests pass 1.5 to the integer parameter N below, either bound in the entry name or
// supplied as a separate specialization argument. That conversion produces a warning, or an
// error with -warnings-as-errors. The command-line diagnostic has no source location.
struct ValueMiss<let N : int> : rt::IMissShader
//WARNING: warning[E30081]: implicit conversion not recommended
//ERROR: error[E30081]: implicit conversion not recommended
{
    typealias Context = MissContext<P>;
    static void invoke(in rt::MissInput<Context> input) {}
}
