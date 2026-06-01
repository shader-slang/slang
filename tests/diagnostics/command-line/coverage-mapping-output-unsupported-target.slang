// coverage-mapping-output-unsupported-target.slang
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target wgsl -stage compute -entry main -trace-coverage -coverage-mapping-output sidecar.json

[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: error[E45112]
//CHECK-SAME: did not produce coverage metadata
