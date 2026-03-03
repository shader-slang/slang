//TEST:COMPILE: tests/bugs/link-time-constant-array-size-lib.slang -o tests/bugs/link-time-constant-array-size-lib.slang-module
//TEST:COMPILE: tests/bugs/link-time-constant-array-size-main.slang -o tests/bugs/link-time-constant-array-size-main.slang-module
//TEST:SIMPLE(filecheck=SPIRV): -r tests/bugs/link-time-constant-array-size-main.slang-module -r tests/bugs/link-time-constant-array-size-lib.slang-module -target spirv -o out.spv -stage compute -entry computeMain

extern static const int N;

// SPIRV: ([[# @LINE+1]]): warning 31000
struct S { int xs[N]; }

RWStructuredBuffer<S> b;

ParameterBlock<S> p;

[numthreads(1, 1, 1)]
void computeMain()
{
    // check that we multiply by our special number
    // SPIRV: [[fisqr:%[a-zA-Z0-9_]+]] = OpConstant %int 1597463007
    // SPIRV: {{%[0-9]+}} = OpIMul %int {{%[0-9]+}} [[fisqr]]
    for(int i = 0; i < N; ++i)
        b[0].xs[i] = p.xs[i] * N;
}
