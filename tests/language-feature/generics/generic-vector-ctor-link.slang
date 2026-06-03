// Regression test for #11182: linking a module that contains a generic
// constrained on `__BuiltinFloatingPointType` and constructs a `vector<T,N>`
// from a scalar literal would crash in `addHoistableInst` /
// `mergeCandidateParentsForHoistableInst` during IR linking, because the
// inlined constructor body produced a hoistable instruction whose operands
// lived in two unrelated source-IR generics.
//
// Note: `cs_main`'s body is empty and `f<T,N>` is never explicitly called.
// The patched code path is still exercised because `f` is a module-scope
// symbol that goes through `prelinkIR`'s `cloneInst` — which is where the
// malformed hoistable inst was constructed and the crash fired. DCE is an
// IR-level pass that runs *after* linking on the already-linked module,
// so it cannot mask this regression at the link stage. (Verified by
// reverting the fix on master: the test fails with the original
// `SLANG_ASSERT(parentNonBlock)` in `slang-ir.cpp` at line 1468.)

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -profile spirv_1_6 -emit-spirv-directly -stage compute -entry cs_main

vector<T, N> f<T, int N>(vector<T, N> v) where T : __BuiltinFloatingPointType
{
    return vector<T, N>(0.0f);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void cs_main()
{
}

// Confirm slangc produced a SPIR-V module (an internal compiler error from
// the malformed-IR crash would not include any of these markers) and that
// the entry point survived linking.
//
// CHECK: OpCapability Shader
// CHECK: OpEntryPoint GLCompute
// CHECK: OpExecutionMode {{.*}} LocalSize 1 1 1
// CHECK: OpReturn
