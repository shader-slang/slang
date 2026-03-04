//TEST:SIMPLE(filecheck=CHECK): -target hpp -line-directive-mode none

//CHECK: #pragma once

//CHECK: #include "{{.*}}slang-cpp-prelude.h"

//CHECK: #ifdef SLANG_PRELUDE_NAMESPACE
//CHECK: using namespace SLANG_PRELUDE_NAMESPACE;
//CHECK: #endif

//CHECK: struct Struct
//CHECK: {
//CHECK:     float x;
//CHECK:     RWStructuredBuffer<float> vec;
//CHECK:     RWStructuredBuffer<float> result;
//CHECK: };
__extern_cpp struct Struct
{
    __extern_cpp float x;
    __extern_cpp RWStructuredBuffer<float> vec;
    __extern_cpp RWStructuredBuffer<float> result;
};

// must be in global scope, otherwise KernelContext is not generated properly
//CHECK: struct KernelContext
//CHECK: {
//CHECK:     FixedArray<float, 4>  * sharedMemory;
//CHECK: };
__extern_cpp groupshared float sharedMemory[4];

//CHECK: void kernel(Vector<uint32_t, 3>  {{[[:alnum:]_]+}}, Struct * {{[[:alnum:]_]+}}, KernelContext * {{[[:alnum:]_]+}});
export __extern_cpp void kernel(uint3 globalIdx, Struct s)
{
    sharedMemory[globalIdx.x] = s.vec[globalIdx.x] + s.x;
    s.result[0] += sharedMemory[globalIdx.x];
}

// needs dummy entrypoint as no output is generated to stdout
// otherwise it only works when slangc -o output.hpp is specified
[numthreads(1, 1, 1)]
void dummyEntrypoint_(uint3 dispatchThreadID: SV_DispatchThreadID) { }
