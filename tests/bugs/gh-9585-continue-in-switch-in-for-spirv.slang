// Regression test for issue #9585:
// "For loop `continue` inside a `switch` breaks structured control flow"
//
// A `continue` inside a switch (with no other cases) inside a `for` loop was producing
// invalid SPIRV. This is the simpler variant of #10198 — same root cause, same fix.
//
// Fixed in slang-ir-eliminate-multilevel-break.cpp by adding the loop's continueBlock
// to region->exitBlocks when continueBlock != targetBlock (for-loops), so that branches
// to it from inside nested switch constructs are treated as multi-level branches and
// transformed into valid SPIRV structured control flow via eliminateContinueBlocksInFunc.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -emit-spirv-directly
//TEST:COMPARE_COMPUTE(filecheck-buffer=CBUF): -output-using-type -cpu

// CHECK: OpEntryPoint

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int sum = 0;

    for (int i = 0; i < 3; ++i)
    {
        sum += 1;

        switch (i)
        {
        case 0:
            {
                continue; // continue from switch inside for-loop: the bug
            }
        }

        sum += 10; // skipped when i==0 due to continue, executed for i==1,2
    }

    // i=0: sum += 1, continue (skip +10) -> sum=1
    // i=1: sum += 1, no case match, sum += 10 -> sum=12
    // i=2: sum += 1, no case match, sum += 10 -> sum=23
    outputBuffer[0] = sum;
    // CBUF: 23
}
