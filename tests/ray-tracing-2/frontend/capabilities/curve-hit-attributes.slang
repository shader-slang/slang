//TEST:SIMPLE: -experimental-feature -entry CurveClosestHit -stage closesthit -target metal -capability metallib_3_1 -restrictive-capability-check -no-codegen
//TEST:SIMPLE: -experimental-feature -entry CurveAnyHit -stage anyhit -target metal -capability metallib_3_1 -restrictive-capability-check -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CLOSEST_HLSL,non-exhaustive): -experimental-feature -entry CurveClosestHit -stage closesthit -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=ANY_HLSL,non-exhaustive): -experimental-feature -entry CurveAnyHit -stage anyhit -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CLOSEST_METAL): -experimental-feature -entry CurveClosestHit -stage closesthit -target metal -capability metallib_3_0 -restrictive-capability-check -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=ANY_METAL): -experimental-feature -entry CurveAnyHit -stage anyhit -target metal -capability metallib_3_0 -restrictive-capability-check -no-codegen

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
/*CLOSEST_HLSL:
       ^^^^^^^^^^^^^^^ unavailable features in entry point
*/
/*CLOSEST_METAL:
       ^^^^^^^^^^^^^^^ entry point uses capabilities not in specified profile
       ^^^^^^^^^^^^^^^ Missing capabilities are: 'metallib_3_1'
*/
{
    typealias Context = CurveContext;

    static void invoke(rt::ClosestHitInput<Context> input)
    {
        rt::CurveData attributes = input.attributes;
        input.payload.value = attributes.parameter;
    }
}

struct CurveAnyHit : rt::IAnyHitShader
/*ANY_HLSL:
       ^^^^^^^^^^^ unavailable features in entry point
*/
/*ANY_METAL:
       ^^^^^^^^^^^ entry point uses capabilities not in specified profile
       ^^^^^^^^^^^ Missing capabilities are: 'metallib_3_1'
*/
{
    typealias Context = CurveContext;

    static void invoke(rt::AnyHitInput<Context> input)
    {
        let attributes = input.attributes;
        input.payload.value = attributes.parameter;
    }
}
