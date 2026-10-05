//TEST:SIMPLE: -experimental-feature -entry CurveClosestHit -stage closesthit -target metal -capability metallib_3_1 -restrictive-capability-check -no-codegen
//TEST:SIMPLE: -experimental-feature -entry CurveAnyHit -stage anyhit -target metal -capability metallib_3_1 -restrictive-capability-check -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry CurveClosestHit -stage closesthit -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry CurveAnyHit -stage anyhit -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry CurveClosestHit -stage closesthit -target metal -capability metallib_3_0 -restrictive-capability-check -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry CurveAnyHit -stage anyhit -target metal -capability metallib_3_0 -restrictive-capability-check -no-codegen

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Payload
{
    float value;
}

struct CurveContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::CurvePrimitive;
    typealias Record = void;
}

// The unified accessor retains the view's type and Metal version requirement in both hit stages.
struct CurveClosestHit : rt::IClosestHitShader
{
    typealias Context = CurveContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        rt::CurveData attributes = input.attributes;
        input.payload.value = attributes.parameter;
    }
}

struct CurveAnyHit : rt::IAnyHitShader
{
    typealias Context = CurveContext;

    void invoke(rt::AnyHitInput<Context> input)
    {
        let attributes = input.attributes;
        input.payload.value = attributes.parameter;
    }
}

//HLSL: unavailable features in entry point
//METAL: entry point uses capabilities not in specified profile
//METAL: Missing capabilities are: 'metallib_3_1'
