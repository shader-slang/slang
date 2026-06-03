//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8

// Test that Barrier flag operands must be compile-time constants.

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(RWBuffer<int> output)
{
    uint value = output[0];
    Barrier((BarrierMemoryTypeFlags)value, BarrierSemanticFlags.GROUP_SYNC);
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, (BarrierSemanticFlags)value);
    output[0] = value;
}

//CHECK: E31409
//CHECK: E31409
