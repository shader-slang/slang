//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL,non-exhaustive): -experimental-feature -entry MetalOnlyMiss -stage miss -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -entry MetalOnlyMiss -stage miss -target metal -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CURVE_HLSL,non-exhaustive): -experimental-feature -entry CurveClosestHit -stage closesthit -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -entry CurveClosestHit -stage closesthit -target metal -capability metallib_3_1 -no-codegen

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Payload {}
struct Record {}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

[require(metal)]
void useMetalOnlyFeature() {}

struct MetalOnlyMiss : rt::IMissShader
/*HLSL:
       ^^^^^^^^^^^^^ unavailable features in entry point
       ^^^^^^^^^^^^^ entrypoint 'MetalOnlyMiss' uses features that are not available in '_miss' stage for 'hlsl' compilation target.
*/
{
    typealias Context = MissContext;

    void invoke(in rt::MissInput<Context> input)
    {
        useMetalOnlyFeature();
//HLSL: ^^^^^^^^^^^^^^^^^^^ see using of 'useMetalOnlyFeature'
    }
}

struct CurveHitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::CurvePrimitive;
    typealias Record = ::Record;
}

// Curve selection is part of the checked structural contract even when `invoke` does not read a
// CurveData property, so entry-point target validation must retain its Metal-only requirement.
struct CurveClosestHit : rt::IClosestHitShader
/*CURVE_HLSL:
       ^^^^^^^^^^^^^^^ unavailable features in entry point
       ^^^^^^^^^^^^^^^ entrypoint 'CurveClosestHit' uses features that are not available in '_closesthit' stage for 'hlsl' compilation target.
*/
{
    typealias Context = CurveHitContext;

    void invoke(in rt::ClosestHitInput<Context> input) {}
}
