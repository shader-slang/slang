// Ordering folds treat `intptr_t` as signed and `uintptr_t` as unsigned (#13978), in SCCP and in
// the front end. The pointer-sized types are not available on every target, so this test is
// separate from uint64-compare-fold.slang.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // IR SCCP on local values.
    intptr_t p = intptr_t(-1);
    uintptr_t q = uintptr_t(0) - uintptr_t(1);
    outputBuffer[0] = (p < intptr_t(0)) ? 1 : 0;
    outputBuffer[1] = (q > uintptr_t(5)) ? 1 : 0;

    // Front-end folding of array sizes.
    int a[(intptr_t(-1) < intptr_t(0)) ? 1 : 2];
    int b[((uintptr_t(0) - uintptr_t(1)) > uintptr_t(5)) ? 1 : 2];
    outputBuffer[2] = a.getCount();
    outputBuffer[3] = b.getCount();
}

// CHECK: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
