// Test reinterpret_cast of RWStructuredBuffer<half> to RWStructuredBuffer<half2>.
// Verifies that AnyValue marshalling correctly handles the full size of
// StructuredBuffer on CUDA (16 bytes: T* data + size_t count).

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// Input buffer: 4 half values = [1.0, 2.0, 3.0, 4.0]
//TEST_INPUT: ubuffer(data=[0x40003C00 0x44004200], stride=2, count=4):name=inputBuffer
uniform RWStructuredBuffer<half>.Handle inputBuffer;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=2, count=4):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
    RWStructuredBuffer<half2> vecBuffer = reinterpret<RWStructuredBuffer<half2>>(*inputBuffer);

    __atomic_reduce_add(vecBuffer[0], half2(1.0h, 2.0h));
    
    half2 v0 = vecBuffer[0];
    half2 v1 = vecBuffer[1];

    outputBuffer[0] = v0.x;
    outputBuffer[1] = v0.y;
    outputBuffer[2] = v1.x;
    outputBuffer[3] = v1.y;
}

// CHECK: 2.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 4.0
