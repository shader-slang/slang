// A `[raypayload]` struct may contain a member whose type is itself a `[raypayload]` struct.
// Per the DXR payload-access-qualifier spec, such a member carries no qualifier of its own and
// inherits the field-level PAQs of the nested type. This also holds when the member's type is
// spelled through a `typealias` or is a generic specialization — both resolve to the same
// underlying payload struct.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile lib_6_7 -stage raygeneration -entry rayGenShaderA
//TEST:SIMPLE(filecheck=DXIL): -target dxil -profile lib_6_7 -stage raygeneration -entry rayGenShaderA

// DXC accepts the emitted HLSL (struct-typed payload members with no qualifier), producing the
// ray payload annotations metadata:
// DXIL: define void @
// DXIL: !dx.dxrPayloadAnnotations

// The nested payload struct carries its own field-level PAQs.
[raypayload] struct NestedPayload
{
    float4 color : read(caller, anyhit) : write(caller);
};

// A generic [raypayload] struct, specialized below.
[raypayload] struct GenPayload<T>
{
    T value : read(caller) : write(caller);
};

// An alias to a [raypayload] struct resolves to the same underlying type.
typealias NestedAlias = NestedPayload;

// OuterPayload has members whose types are [raypayload] structs spelled three ways — direct, via a
// type alias, and as a generic specialization — none of which may carry a qualifier. A sibling
// scalar member is still validated/emitted with its own PAQs as usual.
[raypayload] struct OuterPayload
{
    NestedPayload nested;
    NestedAlias aliased;
    GenPayload<float4> generic;
    uint flags : read(caller) : write(caller);
};

// Each nested payload struct's own scalar field keeps its qualifiers:
// CHECK-DAG: color{{[_0-9]*}} : read(caller, anyhit) : write(caller);
// CHECK-DAG: value{{[_0-9]*}} : read(caller) : write(caller);

// The three nested-payload-typed members of OuterPayload carry no qualifier (the `;` follows the
// member name directly), while the sibling scalar keeps its own PAQs:
// CHECK: struct [raypayload] OuterPayload
// CHECK: nested{{[_0-9]*}};
// CHECK: aliased{{[_0-9]*}};
// CHECK: generic{{[_0-9]*}};
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
    payload.aliased.color = float4(0, 0, 0, 0);
    payload.generic.value = float4(0, 0, 0, 0);
    payload.flags = 0;
    TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    resultTexture[threadIdx.xy] = payload.nested.color;
}
