// The `parallelFirstLLVMCodegen` unit test compiles this shader to LLVM IR from several threads at
// once, each thread making the first LLVM code generation request of a fresh global session. The
// directive below covers the same compile on a single thread.

//TEST:SIMPLE(filecheck=CHECK): -target llvm-shader-ir -emit-cpu-via-llvm -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

// CHECK: define {{.*}}@computeMain(
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = float(tid.x) * 2.0;
}
