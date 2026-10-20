//TEST:SIMPLE: -experimental-feature -entry M<P> -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CODEGEN): -experimental-feature -entry M<P> -stage miss -target hlsl
//TEST:SIMPLE(filecheck=WARNING): -experimental-feature -entry ValueMiss<1.5> -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ERROR): -experimental-feature -entry ValueMiss<1.5> -stage miss -no-codegen -warnings-as-errors all

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

// `M<P>` chooses `P` for `PayloadType`. The `-no-codegen` test checks that this succeeds.
//
// The separate HLSL test checks CODEGEN below. Its two lines match one error: the short title
// and the explanation at `invoke`. "Not lowered" means that this method has not been converted
// into the shader entry function HLSL expects. That conversion is not implemented yet.
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

// The `ValueMiss<1.5>` commands pass a float to the integer parameter `N` below. This conversion
// must produce a warning, or an error with `-warnings-as-errors all`. The diagnostic comes from
// the command-line entry-point name, so it has no source-file location to check with a caret.
//WARNING: warning[E30081]: implicit conversion not recommended
//ERROR: error[E30081]: implicit conversion not recommended
struct ValueMiss<let N : int> : rt::IMissShader
{
    typealias Context = MissContext<P>;
    static void invoke(in rt::MissInput<Context> input) {}
}
