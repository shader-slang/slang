//TEST:SIMPLE: -experimental-feature -entry M<P> -stage miss -no-codegen

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

// These commands pass 1.5 to the integer parameter N. The diagnostic comes from the command
// line, so keep each locationless expectation beside the command that produces it.
//TEST:SIMPLE(filecheck=WARNING): -experimental-feature -entry ValueMiss<1.5> -stage miss -no-codegen
//WARNING: warning[E30081]: implicit conversion not recommended
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ERROR): -experimental-feature -entry ValueMiss<1.5> -stage miss -no-codegen -warnings-as-errors all
//ERROR: error[E30081]: implicit conversion not recommended
struct ValueMiss<let N : int> : rt::IMissShader
{
    typealias Context = MissContext<P>;
    static void invoke(in rt::MissInput<Context> input) {}
}
