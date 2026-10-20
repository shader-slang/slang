//TEST:SIMPLE: -experimental-feature -entry M<P> -stage miss -no-codegen
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
struct M<PayloadType> : rt::IMissShader
{
    typealias Context = MissContext<PayloadType>;

    static void invoke(in rt::MissInput<Context> input) {}
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
