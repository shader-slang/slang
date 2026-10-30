//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -source-language hlsl -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -source-language hlsl -output-using-type

// In HLSL, as in C++ and DXC, the variable declared in a `for` initializer is scoped to the
// loop. Sibling loops can each declare `i`, a declaration after a loop does not conflict with
// the loop variable, and code outside a loop sees the enclosing `i`, never the loop's.
// https://github.com/shader-slang/slang/issues/14099

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

static int g = 3;

int siblingLoops()
{
    int value = 0;
    for (int i = 0; i < 2; ++i) { value += i; }
    for (int i = 0; i < 3; ++i) { value += i * 10; }
    for (int i = 0; i < 4; ++i) { value += i * 100; }
    return value;
}

int siblingLoopsWithTwoVariables()
{
    int value = 0;
    for (int i = 0, j = 10; i < 2; ++i) { value += j; }
    for (int i = 0, j = 20; i < 2; ++i) { value += j; }
    return value;
}

int nestedLoopsWithoutBraces()
{
    int value = 0;
    for (int i = 0; i < 2; ++i)
        for (int i = 0; i < 3; ++i)
            value += i;
    return value;
}

int outerVariableBeforeLoop()
{
    int i = 7;
    int value = 0;
    {
        value = i;
        for (int i = 0; i < 2; ++i) { value += 100; }
    }
    return value;
}

int declarationAfterLoop()
{
    for (int i = 0; i < 4; ++i) {}
    int i = 5;
    return i;
}

int outerVariableAfterLoop()
{
    int i = 50;
    int value = i;
    for (int i = 0; i < 3; ++i) { value += i; }
    return value + i * 1000;
}

int parameterAfterLoop(int i)
{
    for (int i = 0; i < 3; ++i) {}
    return i;
}

int globalAfterLoop()
{
    for (int g = 0; g < 5; ++g) {}
    return g;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: {{^}}631{{$}}
    outputBuffer[0] = siblingLoops();
    // CHECK-NEXT: {{^}}60{{$}}
    outputBuffer[1] = siblingLoopsWithTwoVariables();
    // CHECK-NEXT: {{^}}6{{$}}
    outputBuffer[2] = nestedLoopsWithoutBraces();
    // CHECK-NEXT: {{^}}207{{$}}
    outputBuffer[3] = outerVariableBeforeLoop();
    // CHECK-NEXT: {{^}}5{{$}}
    outputBuffer[4] = declarationAfterLoop();
    // CHECK-NEXT: {{^}}50053{{$}}
    outputBuffer[5] = outerVariableAfterLoop();
    // CHECK-NEXT: {{^}}9{{$}}
    outputBuffer[6] = parameterAfterLoop(9);
    // CHECK-NEXT: {{^}}3{{$}}
    outputBuffer[7] = globalAfterLoop();
}
