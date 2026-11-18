//TEST:SIMPLE(filecheck=CHECK):-target llvm-host-ir -emit-cpu-via-llvm -whole-program

// A compute kernel with varying and uniform parameters legalizes to the same
// three-pointer CPU ABI (threadInput, entryPointParams, globalParams): the
// dispatch-thread-ID is folded into the thread-input struct, the cbuffer into
// the entry-point parameter block, and the kernel signature itself stays the
// three context pointers the emitted wrappers hardcode.
cbuffer Uniforms
{
    float g_scale;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x == 0 && g_scale == 1.0f)
    {
    }
}
//CHECK-NOT: define void @_S{{[^()]*}}()
//CHECK: define void @_S{{[^()]*}}(ptr {{[^,%]*}}%{{[^,)]*}}, ptr {{[^,%]*}}%{{[^,)]*}}, ptr {{[^,%]*}}%{{[^,)]*}})
//CHECK: define void @main_Group(ptr
//CHECK: define void @main(ptr
