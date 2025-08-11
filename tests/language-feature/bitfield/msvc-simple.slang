//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -compile-arg -msvc-style-bitfield-packing

// With MSVC packing, bitfields are packed from MSB to LSB
// and new backing fields start when type sizes change
// CHECK:      123
// CHECK-NEXT: 4567
// CHECK-NEXT: 0
// CHECK-NEXT: 0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct S {
    int foo : 8;   // Packed in bits 31-24
    uint bar : 24; // Packed in bits 23-0
};

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.foo = 123;
    s.bar = 4567;
    outputBuffer[0] = s.foo;
    outputBuffer[1] = s.bar;
    s.foo = 0;
    s.bar = 0;
    outputBuffer[2] = s.foo;
    outputBuffer[3] = s.bar;
}

