// Regression test for the SPIR-V ArrayStride on pointers to sized arrays.
//
// Dynamic indexing into a sized array through a Ptr previously read the wrong
// memory on drivers that consume the pointer type's ArrayStride decoration
// (the decoration carried the whole array size instead of the element
// stride). This test binds a real buffer to a Ptr inside a cbuffer and
// dynamically indexes into the sized array field, verifying the element
// addresses at runtime. Also checks whole-object stepping (d + 1 advances by
// the whole struct).

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -emit-spirv-directly -compute
//TEST_INPUT:ubuffer(data=[1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 2.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 3.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 6.0 7.0 7.0 7.0 7.0 7.0 7.0 7.0 7.0 7.0 7.0 7.0 7.0 7.0 7.0 7.0 7.0 8.0 8.0 8.0 8.0 8.0 8.0 8.0 8.0 8.0 8.0 8.0 8.0 8.0 8.0 8.0 8.0], stride=4):name Globals.d
//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)

struct Data
{
    float4x4 mats[4];
};

cbuffer Globals
{
    LayoutPtr<Data, Std430DataLayout> d;
}

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x & 3;

    // Element indexing through the sized array field: matrix i holds i+1, so
    // the first component of column 0 is idx + 1.
    float4x4 m = d.mats[idx];
    outputBuffer[dispatchThreadID.x] = int(m[0][0]);
    // CHECK: 1
    // CHECK: 2
    // CHECK: 3
    // CHECK: 4

    // Whole-object stepping: d + 1 advances by sizeof(Data) = 256 bytes, and
    // dereferencing the stepped pointer reads the second Data (first matrix
    // holds 5.0). Guarded to a single invocation to avoid overlapping writes.
    if (dispatchThreadID.x == 0)
    {
        outputBuffer[4] = int(reinterpret<uint64_t>(d+1)-reinterpret<uint64_t>(d)) == 256;
        // CHECK: 1
        Data next = *(d + 1);
        float4x4 m2 = next.mats[0];
        outputBuffer[5] = int(m2[0][0]) == 5;
        // CHECK: 1
    }
}