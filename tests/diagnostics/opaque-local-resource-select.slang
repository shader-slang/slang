// `specializeResourceUsage` exposes `returnHelper`'s conditional texture result at its call site,
// even with `[noinline]`. `specializeResourceSelections` must then branch to fetch from `tex0` or
// `tex1` and merge the float result, without storing a texture in a function-local variable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=OPT):-target spirv-asm -validate-ir -validate-ir-detailed -O1
// The unoptimized condition explicitly negates `useSecond`; SPIR-V optimization folds that
// pair of operations into an equality test. Both forms must fetch from the corresponding binding.
//CHECK: %[[INPUT:[^ ]+]] = OpLoad %int %useSecond{{( |$)}}
//CHECK: %[[USE_SECOND:[^ ]+]] = OpINotEqual %bool %[[INPUT]] %int_0{{( |$)}}
//CHECK: %[[USE_FIRST:[^ ]+]] = OpLogicalNot %bool %[[USE_SECOND]]{{( |$)}}
//CHECK: OpSelectionMerge %[[MERGE:[^ ]+]] None
//CHECK: OpBranchConditional %[[USE_FIRST]] %[[FIRST:[^ ]+]] %[[SECOND:[^ ]+]]{{( |$)}}
//CHECK: %[[FIRST]] = OpLabel
//CHECK: %[[TEX0:[^ ]+]] = OpLoad {{[^ ]+}} %tex0{{( |$)}}
//CHECK: OpImageFetch {{[^ ]+}} %[[TEX0]]{{( |$)}}
//CHECK: OpBranch %[[MERGE]]{{( |$)}}
//CHECK: %[[SECOND]] = OpLabel
//CHECK: %[[TEX1:[^ ]+]] = OpLoad {{[^ ]+}} %tex1{{( |$)}}
//CHECK: OpImageFetch {{[^ ]+}} %[[TEX1]]{{( |$)}}
//CHECK: OpBranch %[[MERGE]]{{( |$)}}
//CHECK: %[[MERGE]] = OpLabel
//OPT: %[[INPUT:[^ ]+]] = OpLoad %int %useSecond{{( |$)}}
//OPT: %[[USE_FIRST:[^ ]+]] = OpIEqual %bool %[[INPUT]] %int_0{{( |$)}}
//OPT: OpSelectionMerge %[[MERGE:[^ ]+]] None
//OPT: OpBranchConditional %[[USE_FIRST]] %[[FIRST:[^ ]+]] %[[SECOND:[^ ]+]]{{( |$)}}
//OPT: %[[FIRST]] = OpLabel
//OPT: %[[TEX0:[^ ]+]] = OpLoad {{[^ ]+}} %tex0{{( |$)}}
//OPT: OpImageFetch {{[^ ]+}} %[[TEX0]]{{( |$)}}
//OPT: OpBranch %[[MERGE]]{{( |$)}}
//OPT: %[[SECOND]] = OpLabel
//OPT: %[[TEX1:[^ ]+]] = OpLoad {{[^ ]+}} %tex1{{( |$)}}
//OPT: OpImageFetch {{[^ ]+}} %[[TEX1]]{{( |$)}}
//OPT: OpBranch %[[MERGE]]{{( |$)}}
//OPT: %[[MERGE]] = OpLabel

Texture1D<float> tex0, tex1;

[noinline]
Texture1D<float> returnHelper(Texture1D<float> a, Texture1D<float> b, bool useSecond)
{
    return useSecond ? b : a;
}

[shader("fragment")]
float main(bool useSecond) : SV_Target
{
    Texture1D<float> returned = returnHelper(tex0, tex1, useSecond);
    return returned[0];
}
