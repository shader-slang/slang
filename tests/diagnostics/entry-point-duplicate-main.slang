// The C-like emitters for CUDA and Metal give every entry point named `main` a fresh name
// (warning E40100), so two `main` entry points in different namespaces do not collide there
// (shader-slang/slang#14053).

//TEST:SIMPLE(filecheck=CHECK):-target cuda
//TEST:SIMPLE(filecheck=CHECK):-target metal

// CHECK-NOT: E38016
// CHECK-DAG: warning[E40100]: entry point 'main' has been renamed to 'main_0'
// CHECK-DAG: warning[E40100]: entry point 'main' has been renamed to 'main_1'

RWStructuredBuffer<uint> outBuf;

namespace A
{
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = 1;
}
}

namespace B
{
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = 2;
}
}
