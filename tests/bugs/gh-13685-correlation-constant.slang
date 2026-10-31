// Regression test for https://github.com/shader-slang/slang/issues/13685
//
// The constant variant of gh-13685-correlation.slang. Here `t` is `5` or the never-written `t`, and
// `before` reads the never-written `t`, so `t == before` must hold whenever `c` is false. The
// front end's prelinking peephole forwards the merge to `5` without refining the never-written
// value, so the `!c` path compares `5` with an independent arbitrary value. Refining during
// prelinking would change the front end's uninitialized-use warnings, so the test stays disabled
// until https://github.com/shader-slang/slang/issues/13727 is resolved.

//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -Wno-41035 -xslang -Wno-41016
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -Wno-41035 -xslang -Wno-41016

//TEST_INPUT: set inputBuffer = ubuffer(data=[0 1], stride=4)
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

int mergeAndCompare(bool c)
{
    int t;
    int before = t;
    if (c)
        t = 5;
    return (c ? t == 5 : t == before) ? 1 : 0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: type: int32_t
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    outputBuffer[0] = mergeAndCompare(inputBuffer[0] != 0);
    outputBuffer[1] = mergeAndCompare(inputBuffer[1] != 0);
}
