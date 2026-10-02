//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -xslang -DSLANG_ENABLE_BOUND_ZERO_INDEX
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type -xslang -DSLANG_ENABLE_BOUND_ZERO_INDEX

// Arrays retain their lengths even when CUDA buffer references contain only a pointer.
// Only array accesses go out of bounds; output buffer accesses remain in range.
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    int values[3] = {2, 5, 9};
    outputBuffer[index] = values[index];
    values[index] = 17;
    outputBuffer[index + 4] = values[0];
}

// CHECK: type: int32_t
// CHECK-NEXT: 2
// CHECK-NEXT: 5
// CHECK-NEXT: 9
// CHECK-NEXT: 2
// CHECK-NEXT: 17
// CHECK-NEXT: 2
// CHECK-NEXT: 2
// CHECK-NEXT: 17
