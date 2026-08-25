//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature -target spirv -emit-spirv-directly -entry BadClosestHit -stage closesthit

import slang.raytracing;

struct Payload {}
struct Record {}

struct TraceContext : rt::ITraceContext
{
    typealias Payload = Payload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = Record;
}

struct BadClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    /* CHECK:
         ^^^^^^ E36107
         ^^^^^^ uses features that are not available in '_closesthit' stage
    */
    {
        uint value = WaveGetWaveIndex() + WaveGetNumWaves();
    }
}
