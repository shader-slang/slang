//TEST_IGNORE_FILE:

// Helper module for entry-point-duplicate-name-callee.slang. Its two `[shader]` functions named
// `K` are entry points of this module, but the program that imports it only calls them.

module "entry-point-duplicate-name-callee-helper";

public RWStructuredBuffer<uint> outBuf;

namespace A
{
[shader("compute")]
[numthreads(1, 1, 1)]
public void K()
{
    outBuf[0] = 1;
}
}

namespace B
{
[shader("compute")]
[numthreads(1, 1, 1)]
public void K()
{
    outBuf[1] = 2;
}
}
