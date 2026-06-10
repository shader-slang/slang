//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -stage compute -entry main -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target metal -stage compute -entry main -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target wgsl -stage compute -entry main -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target glsl -stage compute -entry main -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target cuda -stage compute -entry main -experimental-feature

import experimental.workgraph;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
//CHECK: entrypoint 'main' uses features that are not available
{
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, BarrierSemanticFlags.GROUP_SYNC);
    //CHECK: see using of 'Barrier'
}
