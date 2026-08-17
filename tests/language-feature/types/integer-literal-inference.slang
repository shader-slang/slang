// integer-literal-inference.slang

// Test the type an integer literal infers to. Reading a literal into an
// explicitly-typed variable cannot prove its inferred type (implicit integer
// conversions make the assignment compile either way), so we resolve an
// overloaded call: `whichType` returns a distinct tag per parameter type, and the
// returned tag observes the inferred type directly. An unsuffixed decimal in int
// range infers to `int`; an unsuffixed hex literal greater than INT_MAX infers to
// `uint` (the first unsigned type that fits); the `u`/`ll`/`ull` suffixes force
// `uint`/`int64_t`/`uint64_t`.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int whichType(int x) { return 1; }
int whichType(uint x) { return 2; }
int whichType(int64_t x) { return 3; }
int whichType(uint64_t x) { return 4; }

[numthreads(1, 1, 1)]
void computeMain()
{
    // Unsuffixed decimal in int range -> int.
    var a = 42;
    // CHECK: 1
    outputBuffer[0] = whichType(a);

    // Unsuffixed hex greater than INT_MAX -> uint (first unsigned type that fits).
    var b = 0x80000001;
    // CHECK: 2
    outputBuffer[1] = whichType(b);

    // Suffix 'u' -> uint.
    var c = 100u;
    // CHECK: 2
    outputBuffer[2] = whichType(c);

    // Suffix 'll' -> int64_t.
    var d = 50ll;
    // CHECK: 3
    outputBuffer[3] = whichType(d);

    // Suffix 'ull' -> uint64_t.
    var e = 75ull;
    // CHECK: 4
    outputBuffer[4] = whichType(e);

    // Sanity: the uint-inferred hex value round-trips with its high bit intact.
    uint checkB = b;
    // CHECK: 1
    outputBuffer[5] = int(checkB - 0x80000000u);

    // Arithmetic on unsuffixed decimals stays int.
    var g = 10 + 20;
    // CHECK: 1
    outputBuffer[6] = whichType(g);

    // CHECK: 30
    outputBuffer[7] = g;
}
