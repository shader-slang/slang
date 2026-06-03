//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MEMORY,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DMEMORY
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_SEMANTIC,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DSEMANTIC
import experimental.workgraph;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main()
{
#if defined(MEMORY)
    Barrier((BarrierMemoryTypeFlags)0xdead, BarrierSemanticFlags.GROUP_SYNC);
//CHECK_MEMORY: E31117
#elif defined(SEMANTIC)
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, (BarrierSemanticFlags)0xdead);
//CHECK_SEMANTIC: E31116
#endif
}
