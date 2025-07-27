//TEST:SIMPLE(filecheck=DXIL):-target dxil-assembly -entry main -stage closesthit -profile sm_6_5 -DNV_SHADER_EXTN_SLOT=u0

// DXIL: main

// DXIL call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %80, i32 %79, i32 0, i32 93, i32 undef, i32 undef, i32 undef, i8 1, i32 4)  ; RawBufferStore(uav,index,elementOffset,value0,value1,value2,value3,mask,alignment)

[[vk::binding(0)]]
uniform RaytracingAccelerationStructure accelStruct : register(t0);

struct RayPayload
{
    float RayHitT;
};

struct CallableParams
{
    float value;
};

#define T_MIN 0.01f
#define T_MAX 1e4f

[shader("closesthit")]
void main( inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs )
{
    float2 dir = (DispatchRaysIndex().xy / DispatchRaysDimensions().xy) * 2.0f - 1.0f;
    float aspectRatio = DispatchRaysDimensions().x / DispatchRaysDimensions().y;

    RayDesc rayDesc;
    rayDesc.Origin = float3(0.0f, 0.0f, 0.0f);
    rayDesc.Direction = normalize(float3(dir.x * aspectRatio, -dir.y, 1)); ;
    rayDesc.TMin = T_MIN;
    rayDesc.TMax = T_MAX;

    //RayPayload payload;
    payload.RayHitT = T_MAX;

    val += GetClusterID();

    payload.RayHitT = val;
}
