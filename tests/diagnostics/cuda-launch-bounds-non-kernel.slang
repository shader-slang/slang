// [CudaLaunchBounds] / [CudaMaxRegisters] only take effect on a function emitted as a
// `__global__` kernel. On an ordinary device function they are ignored, with a warning;
// on an entry point or a [CudaKernel] function there is no warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

[CudaLaunchBounds(64)]
/*CHECK:
 ^^^^^^^^^^^^^^^^ 'CudaLaunchBounds' only applies to a CUDA kernel and is ignored on 'helper'
 ^^^^^^^^^^^^^^^^ only an entry point other than a callable shader, or a function marked [CudaKernel], is emitted as a '__global__' kernel
*/
[noinline]
uint helper(uint x)
{
    return x * 2;
}

[CudaMaxRegisters(32)]
/*CHECK:
 ^^^^^^^^^^^^^^^^ 'CudaMaxRegisters' only applies to a CUDA kernel and is ignored on 'otherHelper'
 ^^^^^^^^^^^^^^^^ only an entry point other than a callable shader, or a function marked [CudaKernel], is emitted as a '__global__' kernel
*/
uint otherHelper(uint x)
{
    return x + 1;
}

// A generic method of a generic type is nested in two generics.
struct Wrapper<T>
{
    [CudaMaxRegisters(32)]
/*CHECK:
     ^^^^^^^^^^^^^^^^ 'CudaMaxRegisters' only applies to a CUDA kernel and is ignored on 'Wrapper.scaled'
     ^^^^^^^^^^^^^^^^ only an entry point other than a callable shader, or a function marked [CudaKernel], is emitted as a '__global__' kernel
*/
    static uint scaled<U>(uint x)
    {
        return x * 3;
    }
}

[CudaKernel]
[CudaLaunchBounds(128)]
void kernel(uint x)
{
}

// Lowering puts [CudaKernel] on the outermost generic, not on the inner function.
[CudaKernel]
[CudaMaxRegisters(32)]
void genericKernel<T>(T x)
{
}

[CudaLaunchBounds(64)]
[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = helper(tid.x) + otherHelper(tid.x) + Wrapper<int>.scaled<float>(tid.x);
}
