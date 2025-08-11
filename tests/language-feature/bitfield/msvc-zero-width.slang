//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -compile-arg -msvc-style-bitfield-packing

// MSVC packing with zero-width bitfields
// CHECK:      123
// CHECK-NEXT: 4567
// CHECK-NEXT: 1
// CHECK-NEXT: 0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct S {
    int foo : 8;      // First backing field
    int breaker : 0;  // Forces new backing field
    int bar : 24;     // Second backing field
};

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.foo = 123;
    s.bar = 4567;
    s.breaker = 9999; // Zero-width field, assignment has no effect
    outputBuffer[0] = s.foo;
    outputBuffer[1] = s.bar;
    outputBuffer[2] = sizeof(S) == sizeof(int) * 2; // Two backing fields
    outputBuffer[3] = s.breaker; // Always returns 0
}

