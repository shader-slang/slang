//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test that an invalid NodeLaunch mode does not cascade into a missing-numthreads diagnostic.

[shader("node")]
[NodeLaunch("invalid")]
//CHECK: E31113
//CHECK-NOT: must specify a '[numthreads]' attribute
void main(RWBuffer<int> output)
{
    output[0] = 0;
}
