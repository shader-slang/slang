//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

// Semantic checking propagates a selected instance method's receiver through parentheses.
// Direct-call lowering must therefore pass the same receiver that it would pass for the
// unparenthesized `value.read()` call.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

struct Value
{
    int payload;

    [nonmutating]
    int read()
    {
        return payload;
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Value value = { 7 };
    outputBuffer[0] = (value.read)();
}

// CHECK: type: int32_t
// CHECK-NEXT: 7
