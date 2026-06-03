//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8

// Test that unrecognized BarrierMemoryTypeFlags bit patterns produce diagnostic E31407.

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(RWBuffer<int> output)
{
    Barrier((BarrierMemoryTypeFlags)0x100, BarrierSemanticFlags.GROUP_SYNC);
    output[0] = 0;
}

//CHECK: E31407
