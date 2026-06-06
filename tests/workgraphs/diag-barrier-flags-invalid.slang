//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MEMORY,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DMEMORY
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MEMORY_ZERO): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DMEMORY_ZERO
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MEMORY_RUNTIME): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DMEMORY_RUNTIME
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SEMANTIC,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DSEMANTIC
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SEMANTIC_RUNTIME): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DSEMANTIC_RUNTIME
import experimental.workgraph;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(RWBuffer<int> values)
{
#if defined(MEMORY)
    Barrier((BarrierMemoryTypeFlags)0xdead, BarrierSemanticFlags.GROUP_SYNC);
//CHECK_MEMORY: E31117
#elif defined(MEMORY_ZERO)
    Barrier((BarrierMemoryTypeFlags)0, BarrierSemanticFlags.GROUP_SYNC);
//CHECK_MEMORY_ZERO: E31117
//CHECK_MEMORY_ZERO: unrecognized BarrierMemoryTypeFlags value '0x0'
#elif defined(MEMORY_RUNTIME)
    BarrierMemoryTypeFlags memoryFlags = (BarrierMemoryTypeFlags)values[0];
    Barrier(memoryFlags, BarrierSemanticFlags.GROUP_SYNC);
//CHECK_MEMORY_RUNTIME: expected a compile-time constant
#elif defined(SEMANTIC)
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, (BarrierSemanticFlags)0xdead);
//CHECK_SEMANTIC: E31116
#elif defined(SEMANTIC_RUNTIME)
    BarrierSemanticFlags semanticFlags = (BarrierSemanticFlags)values[0];
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, semanticFlags);
//CHECK_SEMANTIC_RUNTIME: expected a compile-time constant
#endif
}
