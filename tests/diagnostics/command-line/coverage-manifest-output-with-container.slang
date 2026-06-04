// coverage-manifest-output-with-container.slang
// Regression test that verifies E45111 fires for explicit manifest paths with container output.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -stage compute -entry main -trace-coverage -coverage-manifest-output coverage-manifest-output-container.json -o coverage-manifest-output-container.slang-module

[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: error[E45111]
//CHECK-SAME: is not supported when writing a container output
