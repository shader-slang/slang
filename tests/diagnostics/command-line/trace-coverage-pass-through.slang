// trace-coverage-pass-through.slang
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-pass-through glslang -target spirv -stage compute -entry computeMain -trace-coverage

[numthreads(1, 1, 1)]
void computeMain()
{
}

//CHECK: -trace-coverage cannot be combined with -pass-through
