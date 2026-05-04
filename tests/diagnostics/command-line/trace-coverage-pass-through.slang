// trace-coverage-pass-through.slang
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-pass-through glslang -target spirv -stage compute -entry computeMain -trace-coverage

[numthreads(1, 1, 1)]
void computeMain()
{
}

//CHECK: error[E45104]
//CHECK-SAME: cannot be combined with `-pass-through`
