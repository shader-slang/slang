// The C-like emitters for CPU, CUDA and Metal give every entry point named `main` a fresh name
// (warning E40100), so two `main` entry points in different namespaces do not collide there
// (shader-slang/slang#14053). SPIR-V and the LLVM emitter keep the name `main`, so the same pair
// collides there.

//TEST:SIMPLE(filecheck=CHECK):-target cuda
//TEST:SIMPLE(filecheck=CHECK):-target ptx -whole-program
//TEST:SIMPLE(filecheck=CHECK):-target metal
//TEST:SIMPLE(filecheck=CHECK):-target cpp -whole-program
//DIAGNOSTIC_TEST:SIMPLE(diag=COLLIDE):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=COLLIDE):-target llvm-ir -whole-program

// CHECK-NOT: E38016
// CHECK-DAG: warning[E40100]: entry point 'main' has been renamed to 'main_0'
// CHECK-DAG: warning[E40100]: entry point 'main' has been renamed to 'main_1'

RWStructuredBuffer<uint> outBuf;

namespace A
{
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
/*COLLIDE:
     ^^^^ the other entry point emitted as 'main' is here
*/
{
    outBuf[0] = 1;
}
}

namespace B
{
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
/*COLLIDE:
     ^^^^ more than one entry point would be emitted as 'main' for target
     ^^^^ this entry point would also be emitted as 'main'
*/
{
    outBuf[0] = 2;
}
}
