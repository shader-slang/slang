//TEST_IGNORE_FILE:

// Companion module for conflicting-export-13319.slang: a link-time function
// whose definitions are specialized for different targets.

public extern uint pickForTarget();

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
public void computeMain()
{
    output[0] = pickForTarget();
}
