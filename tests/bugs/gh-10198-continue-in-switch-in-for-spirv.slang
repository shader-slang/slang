// Regression test for issue #10198:
// "Invalid SPIRV: `continue` inside `switch` inside `for` produces unstructured control flow"
//
// A `continue` inside a switch case inside a `for` loop was producing invalid SPIRV.
// The root cause was in eliminateMultiLevelBreak: the loop's continue block (which is
// a distinct block from the loop's target in a for-loop) was not being added to the
// region's exitBlocks, so branches from inside the switch to the continue block were
// not detected as multi-level branches requiring transformation.
//
// This test verifies both correctness and that SPIRV validation passes.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -emit-spirv-directly
//TEST:COMPARE_COMPUTE(filecheck-buffer=CBUF): -output-using-type -cpu

// CHECK: OpEntryPoint

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int value)
{
    int result = 0;
    for (int i = 0; i < 3; i++)
    {
        switch (value)
        {
        case 0:
            result += 1;
            continue; // triggers the bug: continue from switch inside for-loop
        case 1:
            result += 2;
            break;
        default:
            result += 3;
            break;
        }
        result += 10;
    }
    return result;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // test(0): case 0 hits, result += 1, continue (skips +10) -> 3 iterations -> result = 3
    // test(1): case 1 hits, result += 2, break, then +10 -> 3 iterations -> result = 36
    // test(2): default hits, result += 3, break, then +10 -> 3 iterations -> result = 39
    // total = 3 + 36 + 39 = 78
    outputBuffer[0] = test(0) + test(1) + test(2);
    // CBUF: 78
}
