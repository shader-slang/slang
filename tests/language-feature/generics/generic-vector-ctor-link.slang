// Regression test for #11182: linking a module that contains a generic
// constrained on `__BuiltinFloatingPointType` and constructs a `vector<T,N>`
// from a scalar literal would crash in `addHoistableInst` /
// `mergeCandidateParentsForHoistableInst` during IR linking, because the
// inlined constructor body produced a hoistable instruction whose operands
// lived in two unrelated source-IR generics.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -profile spirv_1_6 -emit-spirv-directly -stage compute -entry cs_main

vector<T, N> f<T, int N>(vector<T, N> v) where T : __BuiltinFloatingPointType
{
    return vector<T, N>(0.0f);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void cs_main()
{
}

// CHECK: OpEntryPoint
