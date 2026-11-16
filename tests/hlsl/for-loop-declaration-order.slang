//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -source-language hlsl -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -source-language hlsl -output-using-type

// HLSL loop initializers become visible in declaration order and remain visible after their
// loops. Each result distinguishes which declaration was selected before and after a loop.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

static const int globalIndex = 23;

// The first loop must use its own `i`, and the post-loop read must use the second loop's `i`.
int siblingLoops()
{
    int sum = 0;
    for (int i = 0; i < 2; ++i)
        sum += i;
    for (int i = 2; i < 5; ++i)
        sum += i;
    return sum * 10 + i;
}

// An initializer in the inner block must not hide the outer `i` before its declaration.
int readBeforeLoop()
{
    int i = 17;
    {
        int before = i;
        for (int i = 0; i < 3; ++i)
        {}
        return before * 10 + i;
    }
}

// A loop variable replaces an earlier local after its initializer has been checked.
int shadowLocal()
{
    int i = 11;
    int before = i;
    for (int i = 0; i < 2; ++i)
    {}
    return before * 10 + i;
}

// The post-loop read selects the initializer instead of the parameter.
int shadowParameter(int i)
{
    int before = i;
    for (int i = 0; i < 3; ++i)
    {}
    return before * 10 + i;
}

// The post-loop read selects the initializer instead of the global.
int shadowGlobal()
{
    int before = globalIndex;
    for (int globalIndex = 0; globalIndex < 4; ++globalIndex)
    {}
    return before * 10 + globalIndex;
}

// An inner block's initializer must not replace a variable belonging to the outer block.
int bracedLoop()
{
    int i = 29;
    {
        for (int i = 0; i < 2; ++i)
        {}
    }
    return i;
}

// The unbraced body has its own scope: the inner initializer must not replace the outer `i`
// in the outer predicate, update, or post-loop read.
int unbracedNestedLoops()
{
    int sum = 0;
    for (int i = 0; i < 2; ++i)
        for (int i = 0; i < 3; ++i)
            sum += i;
    return sum * 10 + i;
}

// Each grouped declaration becomes visible before the next initializer is checked. Repeating
// the group must select the new `i` in `j`'s initializer and the new `j` in the loop body.
int groupedInitializers()
{
    int sum = 0;
    for (int i = 0, j = i + 2; i < 3; ++i)
        sum += j + i;
    for (int i = 1, j = i + 3; i < 3; ++i)
        sum += j + i;
    return sum * 100 + i * 10 + j;
}

// An expression initializer in a later loop can continue using the earlier loop's variable.
int continueLoop()
{
    int sum = 0;
    for (int i = 0; i < 2; ++i)
        sum += i;
    for (i = 2; i < 4; ++i)
        sum += i;
    return sum * 10 + i;
}

// Each unbraced control-flow body has its own scope. Initializers inside those bodies must
// leave the enclosing `i` visible after the bodies end.
int controlBodyScopes()
{
    int i = 31;
    int sum = 0;
    if (i > 0)
        for (int i = 0; i < 2; ++i)
            sum += i;

    int iteration = 0;
    while (iteration++ < 1)
        for (int i = 0; i < 3; ++i)
            sum += i;

    do
        for (int i = 0; i < 2; ++i)
            sum += i;
    while (false);
    return i * 10 + sum;
}

// A later declarator in the same HLSL loop initializer also replaces the earlier declaration.
int repeatedInitializerDeclarator()
{
    int sum = 0;
    for (int i = 0, i = 2; i < 4; ++i)
        sum += i;
    return sum * 10 + i;
}

// A comparison on a vector member can require type lookup while parsing `<`. Both loop headers
// must select their own declaration, and the post-loop reference must select the later variable.
int memberPredicates()
{
    int sum = 0;
    for (int2 value = 0; value.x < 3; ++value.x)
        sum += value.x;
    for (int2 value = 5; value.x < 7; ++value.x)
        sum += value.x;
    return sum * 10 + value.x;
}

// A completed declarator must shadow the earlier local before a later initializer in the
// same group needs parser-time type lookup for a comparison on that declarator's member.
int groupedMemberInitializer()
{
    int2 value = 9;
    for (int2 value = 1, comparison = int2(value.x < 2 ? 1 : 0, 0); value.x < 4; ++value.x)
    {}
    return value.x * 10 + comparison.x;
}

// Excluding the variable being initialized must leave the earlier binding available, even
// when parser-time type lookup checks the new declaration before statement checking reaches it.
int initializerReadsEarlierBinding()
{
    int2 value = 3;
    int digits = 0;
    for (int2 value = value + 1; value.x < 6; ++value.x)
        digits = digits * 10 + value.x;
    return digits * 10 + value.x;
}

// We store each result separately so an incorrect binding cannot be hidden by combining outputs.
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = siblingLoops();
    outputBuffer[1] = readBeforeLoop();
    outputBuffer[2] = shadowLocal();
    outputBuffer[3] = shadowParameter(19);
    outputBuffer[4] = shadowGlobal();
    outputBuffer[5] = bracedLoop();
    outputBuffer[6] = unbracedNestedLoops();
    outputBuffer[7] = groupedInitializers();
    outputBuffer[8] = continueLoop();
    outputBuffer[9] = controlBodyScopes();
    outputBuffer[10] = repeatedInitializerDeclarator();
    outputBuffer[11] = memberPredicates();
    outputBuffer[12] = groupedMemberInitializer();
    outputBuffer[13] = initializerReadsEarlierBinding();
}

// CHECK: 105
// CHECK-NEXT: 173
// CHECK-NEXT: 112
// CHECK-NEXT: 193
// CHECK-NEXT: 234
// CHECK-NEXT: 29
// CHECK-NEXT: 62
// CHECK-NEXT: 2034
// CHECK-NEXT: 64
// CHECK-NEXT: 315
// CHECK-NEXT: 54
// CHECK-NEXT: 147
// CHECK-NEXT: 41
// CHECK-NEXT: 456
