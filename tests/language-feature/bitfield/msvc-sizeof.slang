//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -compile-arg -msvc-style-bitfield-packing

// MSVC starts new backing fields when type sizes change
// CHECK:      4
// CHECK-NEXT: 12
// CHECK-NEXT: 8
// CHECK-NEXT: 16

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct S {
    int foo : 8;
    int bar : 24;
};

// MSVC will use separate backing fields for different sized types
struct T {
    int64_t foo : 33;  // Uses int64_t backing
    int bar : 24;      // Uses separate int backing due to type size change
};

// This still takes two ints to store all the bits
struct P {
    int foo : 24;
    int bar : 24;
};

// MSVC will use separate backing fields due to type size difference
struct Q {
    int8_t foo : 1;    // Uses int8_t backing
    int64_t bar : 63;  // Uses separate int64_t backing
};

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sizeof(S);
    outputBuffer[1] = sizeof(T);
    outputBuffer[2] = sizeof(P);
    outputBuffer[3] = sizeof(Q);
}

