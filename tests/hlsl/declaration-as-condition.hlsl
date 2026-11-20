//TEST:COMPARE_COMPUTE(targets=cpu, filecheck-buffer=CHECK): -source-language hlsl -output-using-type -xslang -Wno-30081 -xslang -Wno-41024
//TEST:SIMPLE(filecheck=COMPILE): -no-codegen -Wno-30081 -Wno-41024
// COMPILE: result code = 0

// `testIfCondition` exercises implicit float-to-int and int-to-bool conversions.
// `testCommaExpressionCondition` intentionally uses the comma operator. Both runs suppress
// the warnings for these constructs.
//TEST_INPUT:ubuffer(data=[-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

float nextFraction(inout int evaluations)
{
    ++evaluations;
    return 0.75f;
}

void testIfCondition()
{
    int count = 37;
    int evaluations = 0;

    // `nextFraction` runs once. Conversion to `int` produces zero, so control enters the
    // `else` arm. Both arms can read the inner `count`, and the outer `count` is visible again
    // after the statement.
    if (int count = nextFraction(evaluations))
        outputBuffer[0] = count + 1;
    else
        outputBuffer[0] = count;

    outputBuffer[1] = evaluations;
    outputBuffer[2] = count;
}

void testWhileCondition()
{
    int evaluations = 0;
    int iterations = 0;
    int sum = 0;
    int remaining = 50;

    // The loop evaluates the condition before each iteration, including after `continue`.
    // The body counts its own iterations so the test still terminates if lowering puts the
    // initializer outside the loop.
    while (int remaining = 3 - evaluations++)
    {
        if (++iterations > 3)
        {
            outputBuffer[3] = -1;
            return;
        }
        if (remaining == 2)
            continue;
        sum += remaining;
    }

    outputBuffer[3] = sum;
    outputBuffer[4] = evaluations;
    outputBuffer[5] = remaining;
}

int testStaticCondition()
{
    // A declaration used as a condition has the same storage duration as a local declaration
    // with the same modifiers. The second call reads the value written by the first.
    if (static int previousCalls = 0)
        return previousCalls + 10;
    else
    {
        ++previousCalls;
        return previousCalls;
    }
}

namespace Types
{
    typedef int Count;
}

int testConditionLookahead()
{
    int left = 2;
    int right = 3;

    // The outer test starts with a product of two values. The inner test starts with a
    // qualified type name and declares a variable with a parenthesized declarator.
    if (left * right < 7)
    {
        if (const Types::Count (product) = left * right)
            return product;
    }
    return -1;
}

int testInitializerShadowing()
{
    int count = 3;
    bool hasInnerValue = false;

    // The initializer reads the outer `count`. The arm reads the variable it initializes.
    if (int count = count - 1)
        hasInnerValue = count == 2;

    return hasInnerValue ? count : -1;
}

RWStructuredBuffer<int> getOutputBuffer(int value)
{
    return outputBuffer;
}

RWStructuredBuffer<int> getOutputBuffer(float value)
{
    return outputBuffer;
}

void testCommaExpressionCondition()
{
    int value = 0;

    // The overloaded call supplies the buffer for the assignment. The comparison is false,
    // so the assignment in the condition is the only write to `outputBuffer[10]`.
    if (getOutputBuffer(value)[10] = 23, value > 0)
        outputBuffer[10] = -2;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    testIfCondition();
    testWhileCondition();
    outputBuffer[6] = testStaticCondition();
    outputBuffer[7] = testStaticCondition();
    outputBuffer[8] = testConditionLookahead();
    outputBuffer[9] = testInitializerShadowing();
    testCommaExpressionCondition();
}

// CHECK: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}37{{$}}
// CHECK-NEXT: {{^}}4{{$}}
// CHECK-NEXT: {{^}}4{{$}}
// CHECK-NEXT: {{^}}50{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}11{{$}}
// CHECK-NEXT: {{^}}6{{$}}
// CHECK-NEXT: {{^}}3{{$}}
// CHECK-NEXT: {{^}}23{{$}}
