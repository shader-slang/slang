// float-literal-default-type.slang

// Test that unsuffixed floating-point literals default to float (not double).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float identity(float x) { return x; }

[numthreads(1, 1, 1)]
void computeMain()
{
    // Unsuffixed literal 1.0 should be float, assignable directly
    var a = 1.0;
    float fa = a;

    // CHECK: 1.0
    outputBuffer[0] = fa;

    // Unsuffixed literal passed to a function expecting float (no conversion needed)
    var b = 3.5;
    float fb = identity(b);

    // CHECK: 3.5
    outputBuffer[1] = fb;

    // Arithmetic with unsuffixed literals produces float
    var c = 2.0 + 0.5;
    float fc = c;

    // CHECK: 2.5
    outputBuffer[2] = fc;

    // Explicit 'f' suffix also yields float
    var d = 7.0f;
    float fd = d;

    // CHECK: 7.0
    outputBuffer[3] = fd;
}
