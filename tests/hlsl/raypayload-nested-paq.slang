// A `[raypayload]` struct may contain a member whose type is itself a `[raypayload]` struct.
// Per the DXR payload-access-qualifier spec, such a member carries no qualifier of its own and
// inherits the field-level PAQs of the nested type.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile lib_6_7 -stage raygeneration -entry rayGenShaderA
//TEST:SIMPLE(filecheck=DXIL): -target dxil -profile lib_6_7 -stage raygeneration -entry rayGenShaderA

// DXC accepts the emitted HLSL (a struct-typed payload member with no qualifier), producing the
// ray payload annotations metadata:
// DXIL: define void @
// DXIL: !dx.dxrPayloadAnnotations

// The nested payload struct carries its own field-level PAQs.
[raypayload] struct NestedPayload
{
    float4 color : read(caller, anyhit) : write(caller);
};

// The outer payload has a member whose TYPE is the nested payload struct; that member must carry
// no qualifier. A sibling scalar member is still validated/emitted with its own PAQs as usual.
[raypayload] struct OuterPayload
{
    NestedPayload nested;
    uint flags : read(caller) : write(caller);
};

// The nested struct's own field keeps its qualifiers:
// CHECK: struct [raypayload]
// CHECK: float4 color{{[_0-9]*}} : read(caller, anyhit) : write(caller);

// The outer member of nested-payload type carries no qualifier (the `;` follows the member name
// directly), while the sibling scalar keeps its own PAQs:
// CHECK: struct [raypayload]
// CHECK: NestedPayload{{[_0-9]*}} nested{{[_0-9]*}};
// CHECK: uint flags{{[_0-9]*}} : read(caller) : write(caller);

uniform RWTexture2D<float4> resultTexture;
uniform RaytracingAccelerationStructure sceneBVH;

[shader("raygeneration")]
void rayGenShaderA()
{
    int2 threadIdx = DispatchRaysIndex().xy;

    RayDesc ray;
    ray.Origin = float3((threadIdx.x * 2) - 1, (threadIdx.y * 2) - 1, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    OuterPayload payload;
    payload.nested.color = float4(0, 0, 0, 0);
    payload.flags = 0;
    TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    resultTexture[threadIdx.xy] = payload.nested.color;
}
