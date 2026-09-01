//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature

// Test that an unresolved NodeLaunch attribute does not cascade into missing
// NodeLaunch or numthreads diagnostics.

[shader("node")]
[NodeLaunch("broadcasting")]
//CHECK: unknown attribute 'NodeLaunch'
//CHECK-NOT: must specify a '[NodeLaunch]' attribute
//CHECK-NOT: must specify a '[numthreads]' attribute
void main()
{
}
