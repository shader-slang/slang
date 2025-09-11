//TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry computeMain

// Test for GitHub issue #8412: Avoid unnecessary copy of function parameters
// This test verifies that member function calls on ParameterBlock types
// do not generate unnecessary temporary copies in CUDA output.

struct LargeStruct
{
    float4 data[16];  // 256 bytes - large enough to make copying expensive

    // Non-mutating method that should not create unnecessary copies
    float getValue(int index)
    {
        return data[index].x + data[index].y;
    }

    // Another method to test multiple member function calls
    float4 getVector(int index)
    {
        return data[index];
    }
}

ParameterBlock<LargeStruct> pb;
RWStructuredBuffer<float> outputBuffer;

void myFunc(LargeStruct tmp)
{
    // These calls should NOT generate temporary copies in CUDA output
    float result1 = tmp.getValue(0);
    float4 result2 = tmp.getVector(1);

    outputBuffer[0] = result1;
    outputBuffer[1] = result2.x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    myFunc(pb);
}

// CHECK: __device__ float LargeStruct_getValue
// CHECK: __device__ float4 LargeStruct_getVector
// CHECK: __device__ void myFunc
// CHECK-NOT: LargeStruct{{.*}} {{.*}} = *
// CHECK: LargeStruct_getValue
// CHECK-NOT: LargeStruct{{.*}} {{.*}} = *
// CHECK: LargeStruct_getVector
