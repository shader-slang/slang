// A member whose type is itself a `[raypayload]` struct inherits that type's access qualifiers and
// must not declare its own; writing one is an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[raypayload] struct NestedPayload
{
    float4 color : read(caller, anyhit) : write(caller);
};

// The error fires whether the member's type is the struct directly or a `typealias` to it.
typealias NestedAlias = NestedPayload;

[raypayload] struct OuterPayload
{
    NestedPayload nested : read(caller) : write(caller);
//CHECK:          ^^^^^^ ray payload field of ray payload struct type must not have access qualifiers
//CHECK:          ^^^^^^ field 'nested' has a ray payload struct type and inherits that type's access qualifiers; it must not declare its own 'read' or 'write' qualifiers
    NestedAlias aliased : read(caller) : write(caller);
//CHECK:        ^^^^^^^ ray payload field of ray payload struct type must not have access qualifiers
//CHECK:        ^^^^^^^ field 'aliased' has a ray payload struct type and inherits that type's access qualifiers; it must not declare its own 'read' or 'write' qualifiers
    uint flags : read(caller) : write(caller);
};

uniform RaytracingAccelerationStructure sceneBVH;

[shader("raygeneration")]
void rayGenShaderA()
{
    OuterPayload payload;
    payload.nested.color = float4(0, 0, 0, 0);
    payload.flags = 0;
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);
}
