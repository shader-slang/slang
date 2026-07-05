// coverage-manifest-output-without-coverage.slang
// Regression test that verifies E45108 fires when the manifest path is set without coverage.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -stage compute -entry main -coverage-manifest-output sidecar.json

[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: error[E45108]
//CHECK-SAME: requires a coverage tracing option
