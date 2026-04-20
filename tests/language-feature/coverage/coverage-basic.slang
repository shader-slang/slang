//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type -xslang -trace-coverage

// Verifies that the prototype shader coverage pass activates under
// `-trace-coverage`. The user also declares their own
// `RWStructuredBuffer<uint> __slang_coverage`; the compiler binds the
// synthesized counters to it instead of manufacturing a new buffer.
//
// The loop iterates 4 times. Two iterations take the then-branch
// (i=0, 2) and two take the else-branch (i=1, 3). The `BUF` check is
// spread across both buffers — outputBuffer first, then __slang_coverage.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT: set __slang_coverage = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<uint> __slang_coverage;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = 0;
    for (uint i = 0; i < 4; ++i)
    {
        if ((i & 1u) == 0u)
            accum += i;
        else
            accum += i * 2u;
    }
    outputBuffer[0] = accum;
    // Kernel result: 0 + 2 + 2 + 6 = 10
    // BUF: 10

    // Coverage buffer — one counter per (file, line) pair hit.
    // Under the -trace-coverage placeholder model, each statement
    // entry increments the slot for its source line; multiple
    // statements on the same line accumulate.
    //   counter 0: line 23 (var init + 2 for-init parts) = 3
    //   counter 1: line 24 (for-header + 2 iteration markers) = 2
    //   counter 2: line 26 (if check + increment per iteration) = 8
    //   counter 3: line 27 (then-branch body)              = 2
    //   counter 4: line 29 (else-branch body)              = 2
    //   counter 5: line 32 (store to outputBuffer)         = 1
    // BUF: 3
    // BUF: 2
    // BUF: 8
    // BUF: 2
    // BUF: 2
    // BUF: 1
}
