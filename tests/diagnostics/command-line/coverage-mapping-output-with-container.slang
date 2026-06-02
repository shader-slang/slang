// coverage-mapping-output-with-container.slang
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -stage compute -entry main -trace-coverage -coverage-mapping-output coverage-mapping-output-container.json -o coverage-mapping-output-container.slang-module

[numthreads(1, 1, 1)]
void main()
{
}

//CHECK: error[E45111]
//CHECK-SAME: is not supported when writing a container output
