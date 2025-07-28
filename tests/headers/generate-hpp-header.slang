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

//CHECK: SLANG_PRELUDE_EXPORT
//CHECK: void kernel(Vector<uint32_t, 3>  {{[[:alnum:]_]+}}, Struct {{[[:alnum:]_]+}}, KernelContext * {{[[:alnum:]_]+}});
[DllExport]
void kernel(uint3 globalIdx, Struct s)
{
    sharedMemory[globalIdx.x] = s.vec[globalIdx.x] + s.x;
    s.result[0] += sharedMemory[globalIdx.x];
}
