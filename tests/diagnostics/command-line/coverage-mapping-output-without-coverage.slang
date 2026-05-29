// coverage-mapping-output-without-coverage.slang
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -stage compute -entry main -coverage-mapping-output sidecar.json

[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: error[E45108]
//CHECK-SAME: requires a coverage tracing option
