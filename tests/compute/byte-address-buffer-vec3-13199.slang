//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// Value-level regression for shader-slang/slang#13199. On Vulkan a byte-address buffer is
// reinterpreted as a typed structured buffer, and a `float3` load must read the tight bytes its
// offset names — word indices `byteOffset / 4` — not the padded std430 element positions
// (`byteOffset / 12` indices into a stride-16 element). #13199 is silent data corruption, so this
// leg observes the bytes that land rather than the emitted asm: it loads a `float3` from byte 12
// and, so a stride mismatch cannot cancel, surfaces the components through a typed buffer at a
// different offset. The required tight access reads words 3,4,5 (values 3,4,5); a wide std430
// access would instead read words 4,5,6 (bytes 16,20,24).

//TEST_INPUT:ubuffer(data=[0.0 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4):name=inputBuffer
RWByteAddressBuffer inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
    float3 v = inputBuffer.Load<float3>(12);
    outputBuffer[0] = v.x;
    outputBuffer[1] = v.y;
    outputBuffer[2] = v.z;
    // CHECK: 3.0
    // CHECK: 4.0
    // CHECK: 5.0
}
