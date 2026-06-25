//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: OpLoad {{.*}} Aligned 8
// CHECK: OpStore {{.*}} Aligned 16

// CHECK: OpLoad {{.*}} Aligned 16
// CHECK: OpStore {{.*}} Aligned 16

// PTX: ld.global.v2.f32
// PTX: ld.global.v4.f32
// PTX: ld.global.nc.v4.f32

uniform float4* data;

struct C { float4 v0; float4 v1; }
uniform C* data2;

uniform ImmutablePtr<C> data3;

[numthreads(1,1,1)]
void computeMain()
{
    var v = loadAligned<8>((float2x4*)data);
    storeAligned<16>((float2x4*)data+1, v);

    var v1 = loadAligned<16>(data2);
    v1.v0 += 1.0f;
    storeAligned<16>(data2, v1);

    var v2 = loadAligned<16>(data3);
    v2.v0 += 1.0f;
    storeAligned<16>(data2, v2);
}