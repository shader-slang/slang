// Trivial compute shader used by unit-test-llvm-version-skew.cpp to drive an
// LLVM-IR codegen that routes through createLLVMBuilder in slang-emit-llvm.cpp.
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
}
