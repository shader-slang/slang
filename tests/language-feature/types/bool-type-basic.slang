// bool-type-basic.slang

// Test bool type: true/false values, logical operations, and conversion to int.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Basic true/false values
    bool t = true;
    bool f = false;

    // CHECK: 1
    outputBuffer[0] = int(t);

    // CHECK: 0
    outputBuffer[1] = int(f);

    // Logical AND
    bool andResult = t && f;

    // CHECK: 0
    outputBuffer[2] = int(andResult);

    // Logical OR
    bool orResult = t || f;

    // CHECK: 1
    outputBuffer[3] = int(orResult);

    // Logical NOT
    bool notResult = !f;

    // CHECK: 1
    outputBuffer[4] = int(notResult);

    // Comparison producing bool
    int x = 5;
    int y = 3;
    bool cmpResult = x > y;

    // CHECK: 1
    outputBuffer[5] = int(cmpResult);

    // Bool from equality
    bool eqResult = (x == y);

    // CHECK: 0
    outputBuffer[6] = int(eqResult);

    // Compound logical expression
    bool compound = (x > y) && (y > 0) && !(x == y);

    // CHECK: 1
    outputBuffer[7] = int(compound);
}
