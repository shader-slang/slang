// float-literal-default-type.slang

// Test that an unsuffixed floating-point literal defaults to `float`, not `double`.
// Reading the value into a `float` destination cannot prove this (double->float is
// an implicit conversion, so it would pass either way). Instead we resolve an
// overloaded call: `whichType` returns 1 for a `float` argument and 2 for a `double`
// argument, so the returned tag observes the *inferred* literal type directly.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int whichType(float x) { return 1; }
int whichType(double x) { return 2; }

[numthreads(1, 1, 1)]
void computeMain()
{
    // Unsuffixed literal: infers to float -> float overload -> 1.
    var a = 1.0;
    // CHECK: 1
    outputBuffer[0] = whichType(a);

    // Arithmetic on unsuffixed literals stays float -> 1.
    var b = 2.0 + 0.5;
    // CHECK: 1
    outputBuffer[1] = whichType(b);

    var c = 7.0f;
    // CHECK: 1
    outputBuffer[2] = whichType(c);

    // Control: an explicit `double` resolves to the double overload -> 2,
    // proving the discriminator is real and the above are genuinely float.
    double d = 1.0;
    // CHECK: 2
    outputBuffer[3] = whichType(d);
}
