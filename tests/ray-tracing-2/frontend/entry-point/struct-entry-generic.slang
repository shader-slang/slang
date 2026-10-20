//TEST:SIMPLE: -experimental-feature -entry M<P> -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CODEGEN): -experimental-feature -entry M<P> -stage miss -target hlsl
//TEST:SIMPLE(filecheck=WARNING): -experimental-feature -entry ValueMiss<1.5> -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ERROR): -experimental-feature -entry ValueMiss<1.5> -stage miss -no-codegen -warnings-as-errors all

// Resolving the entry-point name converts its value argument from float to int. Structural
// fallback must preserve that warning, including when warnings are treated as errors.
//WARNING: warning[E30081]: implicit conversion not recommended
//ERROR: error[E30081]: implicit conversion not recommended

import slang.raytracing;

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

// The first two commands select `M<P>`, so the compiler must use `P` wherever this entry point
// uses `PayloadType`, including inside `MissContext<PayloadType>`.
//
// The first command uses `-no-codegen`: it checks the program without producing target shader
// code and should succeed. This catches the compiler forgetting `P` while selecting `invoke`.
//
// The second command asks for HLSL output. Generating an HLSL ray-tracing entry point from
// `M<P>.invoke` is not implemented yet, so this command should report the error below.
// Reaching that specific error also checks that the compiler still recognizes `M<P>.invoke`
// as a structural ray-tracing entry point after replacing `PayloadType` with `P`.
struct M<PayloadType> : rt::IMissShader
{
    typealias Context = MissContext<PayloadType>;

    static void invoke(in rt::MissInput<Context> input)
/*CODEGEN:
                ^^^^^^ structural ray-tracing entry point was not lowered
                ^^^^^^ structural ray-tracing entry point 'M.invoke' must be converted to a native adapter before target code generation
*/
    {}
}

struct ValueMiss<let N : int> : rt::IMissShader
{
    typealias Context = MissContext<P>;
    static void invoke(in rt::MissInput<Context> input) {}
}
