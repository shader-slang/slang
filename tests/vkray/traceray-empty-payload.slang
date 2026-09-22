// An unannotated TraceRay payload receives its IRRayPayloadDecoration when the D3D-only
// ForceVarIntoRayPayloadStructTemporarily marker is resolved. Ray-tracing legalization must do
// that before collecting empty payload structs, or general type legalization erases the payload
// and leaves an invalid seven-argument TraceRay call.

//TEST:SIMPLE(filecheck=HLSL): -stage raygeneration -entry main -target hlsl -profile sm_6_7
//TEST:SIMPLE(filecheck=DXIL): -stage raygeneration -entry main -target dxil -profile sm_6_7

RaytracingAccelerationStructure scene;

struct EmptyPayload
{
}

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;

    EmptyPayload payload;
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, payload);
}

// HLSL-DAG: struct {{.*}}EmptyPayload
// HLSL-DAG: int _slang_dummy{{.*}};
// HLSL: EmptyPayload{{[A-Za-z0-9_]*}} [[payload:[A-Za-z0-9_]+]];
// HLSL: TraceRay({{.*}}, [[payload]]);

// A successful DXIL compile is the end-to-end check that DXC accepts the eight-argument call.
// DXIL: define void @
