//TEST(compute):COMPARE_COMPUTE(targets=cpu+vk,filecheck-buffer=CHECK):-output-using-type

// GitHub issue #13978: IR SCCP folded `uint64_t` comparisons as signed, so
// `0xFFFFFFFFFFFFFFFFull > 5ull` folded to false. SCCP now compares the operands in their own
// types. The comparisons below are in ordinary code, which the front end does not fold, so only
// SCCP folds them. Each store writes 1 when SCCP compares the unsigned operands as unsigned and 2
// otherwise.
// `gh-13978-int-compare-folding-ast.slang` covers the same comparison in the front end.

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

static const uint64_t X = 0xFFFFFFFFFFFFFFFFull;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint64_t x = 0xFFFFFFFFFFFFFFFFull;
    outputBuffer[0] = (x > 5ull) ? 1 : 2;                     // IR SCCP
    outputBuffer[1] = (0x8000000000000000ull > 1ull) ? 1 : 2; // IR SCCP
    outputBuffer[2] = (x >= 5ull) ? 1 : 2;                    // IR SCCP
}
// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
