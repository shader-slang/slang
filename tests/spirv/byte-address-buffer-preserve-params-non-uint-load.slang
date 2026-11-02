//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -preserve-params -O0

// A byte-address buffer that is read only through `Load<float>` is translated to a `float`
// structured buffer and then has no uses. The unused-buffer translation skips it, so the
// buffer is declared once, not also as a `uint` structured buffer at the same binding (#14039).

ByteAddressBuffer floatOnlyBuffer;
RWStructuredBuffer<float> outputBuffer;

// CHECK: OpName %floatOnlyBuffer "floatOnlyBuffer"
// CHECK-NOT: "floatOnlyBuffer"
// CHECK: %floatOnlyBuffer = OpVariable
// CHECK-NOT: %floatOnlyBuffer_{{[0-9]+}} = OpVariable

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = floatOnlyBuffer.Load<float>(0);
}
