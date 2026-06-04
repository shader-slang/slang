// coverage-manifest-output-unsupported-target.slang
// Regression test that verifies E45112 fires when the selected target emits no coverage metadata.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target wgsl -stage compute -entry main -trace-coverage -coverage-manifest-output sidecar.json

[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: error[E45112]
//CHECK-SAME: did not produce coverage metadata
