// Test that WaveGetLaneIndex() returns correct values for 3D thread blocks (z > 1).
// With numthreads(4,4,2), the 32 threads form exactly 1 warp.
// The linearized index is threadIdx.z * (4*4) + threadIdx.y * 4 + threadIdx.x,
// so the lane index should equal the SV_GroupIndex directly.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

uniform RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(4,4,2)]
void computeMain(uint lane : SV_GroupIndex)
{
    uint i = lane * 2;
    outputBuffer[i + 0] = WaveIsFirstLane() ? 1.0 : 0.0;
    outputBuffer[i + 1] = float(WaveGetLaneIndex());
}

// 32 threads in one warp. WaveIsFirstLane()=true only for lane 0.
// WaveGetLaneIndex() should be 0..31.

// CHECK: 1.0
// CHECK: 0.0
// CHECK: 0.0
// CHECK: 1.0
// CHECK: 0.0
// CHECK: 2.0
// CHECK: 0.0
// CHECK: 3.0
// CHECK: 0.0
// CHECK: 4.0
// CHECK: 0.0
// CHECK: 5.0
// CHECK: 0.0
// CHECK: 6.0
// CHECK: 0.0
// CHECK: 7.0
// CHECK: 0.0
// CHECK: 8.0
// CHECK: 0.0
// CHECK: 9.0
// CHECK: 0.0
// CHECK: 10.0
// CHECK: 0.0
// CHECK: 11.0
// CHECK: 0.0
// CHECK: 12.0
// CHECK: 0.0
// CHECK: 13.0
// CHECK: 0.0
// CHECK: 14.0
// CHECK: 0.0
// CHECK: 15.0
// CHECK: 0.0
// CHECK: 16.0
// CHECK: 0.0
// CHECK: 17.0
// CHECK: 0.0
// CHECK: 18.0
// CHECK: 0.0
// CHECK: 19.0
// CHECK: 0.0
// CHECK: 20.0
// CHECK: 0.0
// CHECK: 21.0
// CHECK: 0.0
// CHECK: 22.0
// CHECK: 0.0
// CHECK: 23.0
// CHECK: 0.0
// CHECK: 24.0
// CHECK: 0.0
// CHECK: 25.0
// CHECK: 0.0
// CHECK: 26.0
// CHECK: 0.0
// CHECK: 27.0
// CHECK: 0.0
// CHECK: 28.0
// CHECK: 0.0
// CHECK: 29.0
// CHECK: 0.0
// CHECK: 30.0
// CHECK: 0.0
// CHECK: 31.0
