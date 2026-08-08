// integer-literal-inference.slang

// Test type inference rules for integer literals.
// - Unsuffixed decimal literals infer to int; if too large, int64_t.
// - Unsuffixed hex literals infer to int, uint, int64_t, uint64_t (first that fits).
// - Suffix `u` forces uint; `ll` forces int64_t; `ull` forces uint64_t.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Unsuffixed decimal literal fits in int
    var a = 42;
    int checkA = a;

    // CHECK: 42
    outputBuffer[0] = checkA;

    // Unsuffixed hex literal within int range
    var b = 0xFF;
    int checkB = b;

    // CHECK: 255
    outputBuffer[1] = checkB;

    // Suffix 'u' forces uint
    var c = 100u;
    uint checkC = c;

    // CHECK: 100
    outputBuffer[2] = int(checkC);

    // Suffix 'll' forces int64_t
    var d = 50ll;
    int64_t checkD = d;

    // CHECK: 50
    outputBuffer[3] = int(checkD);

    // Suffix 'ull' forces uint64_t
    var e = 75ull;
    uint64_t checkE = e;

    // CHECK: 75
    outputBuffer[4] = int(checkE);

    // Hex literal in uint range (> 0x7FFFFFFF fits uint)
    var f = 0x80000001u;
    uint checkF = f;

    // CHECK: 1
    outputBuffer[5] = int(checkF - 0x80000000u);

    // Arithmetic on inferred int
    var g = 10 + 20;

    // CHECK: 30
    outputBuffer[6] = g;

    // Hex arithmetic
    var h = 0x10 + 0x20;

    // CHECK: 48
    outputBuffer[7] = h;
}
