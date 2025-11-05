//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: OpLoad {{.*}} Aligned 8
// CHECK: OpStore {{.*}} Aligned 16

// CHECK: OpLoad {{.*}} Aligned 16
// CHECK: OpStore {{.*}} Aligned 16

uniform float4* data;

struct C { float4 v0; float4 v1; }
uniform C* data2;


[numthreads(1,1,1)]
void computeMain()
{
    var v = loadAligned<8>((float2x4*)data);
    storeAligned<16>((float2x4*)data+1, v);

    var v1 = loadAligned<16>(data2);
    v1.v0 += 1.0f;
    storeAligned<16>(data2, v1);
}