//TEST_IGNORE_FILE:

// Helper module for entry-point-duplicate-name-callee.slang. Its two `[shader]` functions named
// `K` are entry points of this module, but the program that imports it only calls them. Its
// ordinary function `Main` shares the name of the importing module's entry point, which `-entry
// Main` must still find, because only functions of the importing module are candidates.

module "entry-point-duplicate-name-callee-helper";

public RWStructuredBuffer<uint> outBuf;

public void Main(uint value)
{
    outBuf[2] = value;
}

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
