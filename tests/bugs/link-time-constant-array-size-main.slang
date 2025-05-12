//TEST:COMPILE: tests/bugs/link-time-constant-array-size-lib.slang -o tests/bugs/link-time-constant-array-size-lib.slang-module
//TEST:COMPILE: tests/bugs/link-time-constant-array-size-main.slang -o tests/bugs/link-time-constant-array-size-main.slang-module
//TEST:COMPILE: tests/bugs/link-time-constant-array-size-main.slang-module tests/bugs/link-time-constant-array-size-lib.slang-module -target spirv -o out.spv

extern static const int N;

struct S { int xs[N]; }

RWStructuredBuffer<S> b;

ParameterBlock<S> p;

[numthreads(1, 1, 1)]
void computeMain()
{
    for(int i = 0; i < N; ++i)
        b[0].xs[i] = p.xs[i] * N;
}
