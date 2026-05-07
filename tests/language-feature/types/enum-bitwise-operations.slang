// enum-bitwise-operations.slang

// Test that enums conform to ILogical and support bitwise operations (|, &, ~).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[Flags]
enum Mask
{
    None = 0,
    A,    // = 1
    B,    // = 2
    C,    // = 4
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Mask combined = Mask.A | Mask.C;  // 1 | 4 = 5
    // CHECK: 5
    outputBuffer[0] = int(combined);

    Mask masked = combined & Mask.A;  // 5 & 1 = 1
    // CHECK: 1
    outputBuffer[1] = int(masked);

    Mask inverted = ~Mask.A;  // ~1 = -2 (all bits except bit 0)
    // CHECK: -2
    outputBuffer[2] = int(inverted);

    // Test that & correctly filters: (A|B|C) & (A|C) = A|C
    Mask all = Mask.A | Mask.B | Mask.C;  // 7
    Mask filter = Mask.A | Mask.C;         // 5
    // CHECK: 5
    outputBuffer[3] = int(all & filter);
}
