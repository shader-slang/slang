//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry compute_main

// RWStructuredBuffer<int*> wrapped in a struct (RWValueRef), which is
// then a field of a ParameterBlock struct alongside a bare pointer field.
// Mirrors the pattern used by slangpy's RWValueRef type.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct RWValueRef
{
    RWStructuredBuffer<int*> value;
};

struct CallData
{
    uint3 _thread_count;
    int* ptr;
    RWValueRef _result;
};

ParameterBlock<CallData> call_data;

// The argument-buffer struct must not contain any pointer-to-pointer types.
// Single-level pointer (ptr) stays as a typed device pointer.
// Resource pointers (RWStructuredBuffer<int*>) are lowered via the resource-element
// path, so their inner pointers become ulong.
// CHECK-LABEL: struct CallData
// CHECK: int device* ptr
// CHECK-NOT: device{{.*}}device
// CHECK: };

// CHECK-LABEL: compute_main

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_main(uint3 tid : SV_DispatchThreadID)
{
    call_data._result.value[0] = call_data.ptr;
}
