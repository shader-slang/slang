//TEST:SIMPLE(filecheck=CHECK):-target llvm-host-ir -emit-cpu-via-llvm -whole-program

// A compute kernel with varying and uniform parameters legalizes to the same
// three-pointer CPU ABI (threadInput, entryPointParams, globalParams): the
// dispatch-thread-ID is folded into the thread-input struct, the cbuffer and
// the buffer into their parameter structs, and the kernel signature itself
// stays the three context pointers the emitted wrappers hardcode. The
// getelementptr CHECK pins that the kernel body actually reads through the
// global parameter pointer (whole-program places the cbuffer there).
cbuffer Uniforms
{
    float g_scale;
}
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = g_scale;
}
//CHECK-NOT: define void @_S{{[^()]*}}()
//CHECK: define void @_S{{[^()]*}}(ptr {{[^,%]*}}%{{[^,)]*}}, ptr {{[^,%]*}}%entryPointParams, ptr {{[^,%]*}}%globalParams)
//CHECK: getelementptr i8, ptr %globalParams
//CHECK: define void @main_Group(ptr
//CHECK: define void @main(ptr
