//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BARRIER,non-exhaustive): -target spirv -stage compute -entry main -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BARRIER,non-exhaustive): -target metal -stage compute -entry main -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BARRIER,non-exhaustive): -target wgsl -stage compute -entry main -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BARRIER,non-exhaustive): -target glsl -stage compute -entry main -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BARRIER,non-exhaustive): -target cuda -stage compute -entry main -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NODE,non-exhaustive): -target spirv -stage node -entry nodeMain -profile lib_6_8 -experimental-feature -DNODE_ENTRY
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NODE,non-exhaustive): -target metal -stage node -entry nodeMain -profile lib_6_8 -experimental-feature -DNODE_ENTRY
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NODE,non-exhaustive): -target wgsl -stage node -entry nodeMain -profile lib_6_8 -experimental-feature -DNODE_ENTRY
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NODE,non-exhaustive): -target glsl -stage node -entry nodeMain -profile lib_6_8 -experimental-feature -DNODE_ENTRY
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NODE,non-exhaustive): -target cuda -stage node -entry nodeMain -profile lib_6_8 -experimental-feature -DNODE_ENTRY

import experimental.workgraph;

#if defined(NODE_ENTRY)
struct Record
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void nodeMain(DispatchNodeInputRecord<Record> input)
//CHECK_NODE: entrypoint 'nodeMain' uses features that are not available
{
    input.Get();
    //CHECK_NODE: see using of 'Get'
}
#else
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
//CHECK_BARRIER: entrypoint 'main' uses features that are not available
{
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, BarrierSemanticFlags.GROUP_SYNC);
    //CHECK_BARRIER: see using of 'Barrier'
}
#endif
