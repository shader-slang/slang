//TEST:COMPILE: tests/bugs/link-time-constant-array-size-lib.slang -o tests/bugs/link-time-constant-array-size-lib.slang-module
//TEST:COMPILE: tests/bugs/link-time-constant-array-size-main.slang -o tests/bugs/link-time-constant-array-size-main.slang-module
//TEST:SIMPLE(filecheck=SPIRV): -r tests/bugs/link-time-constant-array-size-main.slang-module -r tests/bugs/link-time-constant-array-size-lib.slang-module -target spirv -o out.spv -stage compute -entry computeMain

// Check that linked constant array sizes still lower into SPIR-V.

extern static const int N;

// SPIRV: warning[E31010]
struct S { int xs[N]; }

RWStructuredBuffer<S> b;

ParameterBlock<S> p;

[numthreads(1, 1, 1)]
void computeMain()
{
    // check that we multiply by our linked constant
    // SPIRV: [[arraySize:%[a-zA-Z0-9_]+]] = OpConstant %int 1024
    // SPIRV: OpIMul %int {{.*}}[[arraySize]]{{.*}}
    for(int i = 0; i < N; ++i)
        b[0].xs[i] = p.xs[i] * N;
}
