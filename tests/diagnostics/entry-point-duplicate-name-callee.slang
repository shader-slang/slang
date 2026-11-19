// A `[shader]` function that the program calls as an ordinary function is not an entry point of
// the output, so two such functions named `K` do not collide (shader-slang/slang#14053). Only the
// entry points of the program being compiled are compared. The imported module also declares an
// ordinary `Main`, which does not make `-entry Main` ambiguous.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry Main -stage compute
//TEST:SIMPLE(filecheck=CHECK-CUDA):-target cuda -entry Main -stage compute

// CHECK-NOT: E38016
// CHECK: OpEntryPoint GLCompute
// CHECK-NOT: OpEntryPoint
// CHECK-CUDA-NOT: E38016
// CHECK-CUDA: __global__ void Main(

import "entry-point-duplicate-name-callee-helper";

[shader("compute")]
[numthreads(1, 1, 1)]
void Main(uint3 tid: SV_DispatchThreadID)
{
    A::K();
    B::K();
}
