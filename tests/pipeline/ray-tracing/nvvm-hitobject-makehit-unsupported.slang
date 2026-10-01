// OptiX 9 cannot construct arbitrary hits from authored identity and attribute fields.
//TEST:SIMPLE(filecheck=BAD): -target ptx -emit-cuda-via-nvvm -entry main -stage raygeneration -capability cuda_sm_8_0 -DFORM=0 -o make-hit.ptx
//TEST:SIMPLE(filecheck=BAD): -target ptx -emit-cuda-via-nvvm -entry main -stage raygeneration -capability cuda_sm_8_0 -DFORM=1 -o make-motion-hit.ptx
//TEST:SIMPLE(filecheck=BAD): -target ptx -emit-cuda-via-nvvm -entry main -stage raygeneration -capability cuda_sm_8_0 -DFORM=2 -o make-hit-record.ptx
//TEST:SIMPLE(filecheck=BAD): -target ptx -emit-cuda-via-nvvm -entry main -stage raygeneration -capability cuda_sm_8_0 -DFORM=3 -o make-motion-hit-record.ptx
// BAD: result code = {{-?[1-9][0-9]*}}
// BAD: OptiX 9 does not support arbitrary HitObject MakeHit construction

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
{
    RayDesc ray = { float3(0, 0, 1), 0, float3(0, 0, -1), 10 };
    BuiltInTriangleIntersectionAttributes attributes = { float2(0.25, 0.125) };
#if FORM == 0
    HitObject object = HitObject.MakeHit(scene, 0, 0, 0, 254, 0, 1, ray, attributes);
#elif FORM == 1
    HitObject object = HitObject.MakeMotionHit(scene, 0, 0, 0, 254, 0, 1, ray, 0.5, attributes);
#elif FORM == 2
    HitObject object = HitObject.MakeHit(0, scene, 0, 0, 0, 254, ray, attributes);
#else
    HitObject object = HitObject.MakeMotionHit(0, scene, 0, 0, 0, 254, ray, 0.5, attributes);
#endif
    output[0] = object.IsHit();
}
