//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage raygeneration -g2 -emit-spirv-directly

RaytracingAccelerationStructure rtas;

struct HitPayload
{
    float3 hitValue;
};

void main()
{
    uint rayFlags = RAY_FLAG_FORCE_OPAQUE;

    RayDesc desc;
    desc.Origin = float3(0, 0, 0);
    desc.Direction = float3(1, 0, 0);
    desc.TMin = 0.0;
    desc.TMax = 1.0;

    HitPayload payload;

    TraceRay(rtas,
             rayFlags,
             0xFF,
             0,
             0,
             0,
             desc,
             payload);
}

// CHECK: %[[TYPENAME:[0-9]+]] = OpString "RaytracingAccelerationStructure"
// CHECK: %[[TYPE:[0-9]+]] = OpExtInst %void %{{[a-zA-Z0-9_]+}} DebugTypeComposite %[[TYPENAME]]
// CHECK: OpExtInst %void %{{[0-9]+}} DebugGlobalVariable %{{[0-9]+}} %[[TYPE]] %{{.*}} %rtas
