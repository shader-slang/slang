//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: %{{.*}} = OpBitcast %v2uint %{{.*}}

RWTexture2D<uint3> tex;
void fn(inout int2 origPos, uint offset)
{
  origPos.x += offset;
  origPos.y += offset;
}

uint3 id(float3 coord)
{
  return uint3(coord);
}

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    uint3 index = id(threadId);
    // Passing a uint3 swizzle as a parameter to a function expecting a int2 should result
    // in an explicit typecast from unsigned to signed
    fn(index.xy, 1);
    tex[threadId.xy] = index;
}