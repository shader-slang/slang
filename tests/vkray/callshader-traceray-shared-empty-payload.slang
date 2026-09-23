// Regression test for the payload-collection dedup in `legalizeRayTracingPayloads`. An empty struct
// used as BOTH a TraceRay ray payload and a CallShader callable payload is matched by two of the
// per-kind collectors, yet must be padded exactly once. The collectors feed one shared set that
// deduplicates it; padding the same struct from two separate sets would re-enter
// `padEmptyStructWithDummyField` on an already-padded (no longer empty) struct and trip its
// empty-struct assert. A successful compile plus a single `_slang_dummy` field is the signal.

//TEST:SIMPLE(filecheck=HLSL): -stage raygeneration -entry rgen -target hlsl -profile sm_6_7

struct Shared
{
}

RaytracingAccelerationStructure accel;

[shader("raygeneration")]
void rgen()
{
    Shared p;
    RayDesc ray;
    ray.Origin = float3(0.0, 0.0, 0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;
    TraceRay(accel, 0, 0xff, 0, 0, 0, ray, p);
    CallShader(0, p);
}

// The shared empty payload is padded exactly once — not once per collector.
// HLSL-COUNT-1: _slang_dummy
