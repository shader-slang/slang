//TEST:COMPARE_COMPUTE(targets=cpu, filecheck-buffer=CHECK): -source-language hlsl -output-using-type -xslang -Wno-30081 -xslang -Wno-41024
//TEST:SIMPLE(filecheck=COMPILE): -no-codegen -Wno-30081 -Wno-41024
// COMPILE: result code = 0

// The CPU run checks how HLSL `if` and `while` initialize and test a variable declared in
// the condition, and where that variable is visible. `computeMain` writes the results in
// test order to consecutive elements of `outputBuffer`. The compile-only run checks that
// the `.hlsl` extension selects the dialect that accepts the declarations.
//
// `testIfCondition` exercises implicit float-to-int and int-to-bool conversions.
// `testCommaExpressionCondition` intentionally uses the comma operator. Both runs suppress
// the warnings for these constructs.
//TEST_INPUT:ubuffer(data=[-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Counts an initializer call and returns a fraction that converts to zero as an `int`.
float nextFraction(inout int evaluations)
{
    ++evaluations;
    return 0.75f;
}

// Records the selected arm's value, initializer call count, and outer `count` in elements
// 0 through 2 of `outputBuffer`.
void testIfCondition()
{
    int count = 37;
    int evaluations = 0;

    // `nextFraction` runs once. Conversion to `int` produces zero, so the `else` arm
    // records the inner `count`. After the statement we record the outer `count`.
    if (int count = nextFraction(evaluations))
        outputBuffer[0] = count + 1;
    else
        outputBuffer[0] = count;

    outputBuffer[1] = evaluations;
    outputBuffer[2] = count;
}

// Records the loop's sum, condition evaluation count, and outer `remaining` in elements 3
// through 5 of `outputBuffer`.
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

// Returns 1 on the first call and 11 on the second if the condition variable retains its value.
int testStaticCondition()
{
    // The first call sees zero and stores one. The second call sees one because the condition
    // declares a `static` local. We add 10 on the true path to record which arm ran.
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

// Returns the product if the expression and declaration conditions both evaluate to true.
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

// Returns the outer `count` if the condition initializes the inner `count` to 2.
int testInitializerShadowing()
{
    int count = 3;
    bool hasInnerValue = false;

    // The initializer reads the outer `count`, and the arm reads the inner `count`.
    // The flag lets us check the inner value while returning the value visible after the `if`.
    if (int count = count - 1)
        hasInnerValue = count == 2;

    return hasInnerValue ? count : -1;
}

// Both overloads return the same buffer so the expected output is independent of overload
// selection.
RWStructuredBuffer<int> getOutputBuffer(int value)
{
    return outputBuffer;
}

RWStructuredBuffer<int> getOutputBuffer(float value)
{
    return outputBuffer;
}

// Records the assignment from the comma expression in `outputBuffer[10]`.
void testCommaExpressionCondition()
{
    int value = 0;

    // `getOutputBuffer(value)[10]` could start a declaration with a type name followed by
    // a parenthesized declarator and an array suffix. In the comma expression, the assignment
    // writes 23 and the false comparison keeps the arm from overwriting it.
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
