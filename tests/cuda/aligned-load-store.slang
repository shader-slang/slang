//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

// CUDA-NOT: __align__(4)
// CUDA: __align__(16)
// CUDA-NOT: __align__(4)

// PTX: ld.global.v2.f32
// PTX: st.global.v2.f32
// PTX: ld.global.v4.f32
// PTX-NOT: ld.global.nc.v4.f32
// PTX: st.global.v4.f32
// PTX: ld.global.nc.v4.f32

uniform float4* data;
uniform float* scalarData;

struct C { float4 v0; float4 v1; }
uniform C* mutableData;
uniform C* outputData;
uniform ImmutablePtr<C> readOnlyData;
groupshared float sharedData;

[numthreads(1,1,1)]
void computeMain()
{
    var v = loadAligned<8>((float2x4*)data);
    storeAligned<16>((float2x4*)data + 1, v);

    var scalarValue = loadAligned<4>(scalarData);
    storeAligned<4>(scalarData, scalarValue + 1.0f);

    sharedData = 0.0f;
    var sharedValue = loadAligned<4>(&sharedData);
    storeAligned<4>(&sharedData, sharedValue + 1.0f);

    var mutableValue = loadAligned<16>(mutableData);
    mutableValue.v0 += 1.0f;
    storeAligned<16>(mutableData, mutableValue);

    var immutableValue = loadAligned<16>(readOnlyData);
    immutableValue.v0 += 2.0f;
    storeAligned<16>(outputData, immutableValue);
}
