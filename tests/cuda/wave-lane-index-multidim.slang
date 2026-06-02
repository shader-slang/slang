// Test that WaveGetLaneIndex() returns correct values for multi-dimensional thread blocks.
// With numthreads(8,8,1), the 64 threads form 2 warps of 32.
// Previously _getLaneId() used `threadIdx.x & 31` which only returned 0-7,
// ignoring threadIdx.y. The fix uses the linearized thread index.

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

uniform RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(8,8,1)]
void computeMain(uint lane : SV_GroupIndex)
{
    uint i = lane * 2;
    outputBuffer[i + 0] = WaveIsFirstLane() ? 1.0 : 0.0;
    outputBuffer[i + 1] = float(WaveGetLaneIndex());
}

// With 64 threads and CUDA warp size 32, we get 2 warps.
// Warp 0 (lanes 0-31): WaveIsFirstLane()=true only for lane 0
// Warp 1 (lanes 32-63): WaveIsFirstLane()=true only for lane 32

// Warp 0
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

// Warp 1
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
