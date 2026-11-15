// Resource-type legalization separates `Pair` into its texture fields. `specializeResourceUsage`
// exposes the fields returned by `selStruct` at its call site. `specializeResourceSelections` must
// then branch to sample `t0` or `t1`, without storing a texture in a function-local variable.
// The implicit sample still warrants a warning about the introduced conditional execution.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=OPT):-target spirv-asm -entry main -stage fragment -validate-ir -validate-ir-detailed -O1
//CHECK: warning[E56020]
//CHECK: %[[INPUT:[^ ]+]] = OpLoad %int %b{{( |$)}}
//CHECK: %[[CONDITION:[^ ]+]] = OpINotEqual %bool %[[INPUT]] %int_0{{( |$)}}
//CHECK: %[[INVERSE:[^ ]+]] = OpLogicalNot %bool %[[CONDITION]]{{( |$)}}
//CHECK: %[[RESTORED:[^ ]+]] = OpLogicalNot %bool %[[INVERSE]]{{( |$)}}
//CHECK: OpSelectionMerge %[[MERGE:[^ ]+]] None
//CHECK: OpBranchConditional %[[RESTORED]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
//CHECK: %[[THEN]] = OpLabel
//CHECK-DAG: %[[TEX1:[^ ]+]] = OpLoad {{[^ ]+}} %t1{{( |$)}}
//CHECK-DAG: %[[SAMPLER1:[^ ]+]] = OpLoad {{[^ ]+}} %samp{{( |$)}}
//CHECK: %[[PAIR1:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEX1]] %[[SAMPLER1]]{{( |$)}}
//CHECK: OpImageSampleImplicitLod {{[^ ]+}} %[[PAIR1]]{{( |$)}}
//CHECK: OpBranch %[[MERGE]]{{( |$)}}
//CHECK: %[[ELSE]] = OpLabel
//CHECK-DAG: %[[TEX0:[^ ]+]] = OpLoad {{[^ ]+}} %t0{{( |$)}}
//CHECK-DAG: %[[SAMPLER0:[^ ]+]] = OpLoad {{[^ ]+}} %samp{{( |$)}}
//CHECK: %[[PAIR0:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEX0]] %[[SAMPLER0]]{{( |$)}}
//CHECK: OpImageSampleImplicitLod {{[^ ]+}} %[[PAIR0]]{{( |$)}}
//CHECK: OpBranch %[[MERGE]]{{( |$)}}
//CHECK: %[[MERGE]] = OpLabel
// SPIR-V optimization cancels the two negations, while the sampler and texture alternatives
// must still reach the same conditional samples.
//OPT: warning[E56020]
//OPT: %[[INPUT:[^ ]+]] = OpLoad %int %b{{( |$)}}
//OPT: %[[CONDITION:[^ ]+]] = OpINotEqual %bool %[[INPUT]] %int_0{{( |$)}}
//OPT: OpSelectionMerge %[[MERGE:[^ ]+]] None
//OPT: OpBranchConditional %[[CONDITION]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
//OPT: %[[THEN]] = OpLabel
//OPT-DAG: %[[TEX1:[^ ]+]] = OpLoad {{[^ ]+}} %t1{{( |$)}}
//OPT-DAG: %[[SAMPLER1:[^ ]+]] = OpLoad {{[^ ]+}} %samp{{( |$)}}
//OPT: %[[PAIR1:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEX1]] %[[SAMPLER1]]{{( |$)}}
//OPT: OpImageSampleImplicitLod {{[^ ]+}} %[[PAIR1]]{{( |$)}}
//OPT: OpBranch %[[MERGE]]{{( |$)}}
//OPT: %[[ELSE]] = OpLabel
//OPT-DAG: %[[TEX0:[^ ]+]] = OpLoad {{[^ ]+}} %t0{{( |$)}}
//OPT-DAG: %[[SAMPLER0:[^ ]+]] = OpLoad {{[^ ]+}} %samp{{( |$)}}
//OPT: %[[PAIR0:[^ ]+]] = OpSampledImage {{[^ ]+}} %[[TEX0]] %[[SAMPLER0]]{{( |$)}}
//OPT: OpImageSampleImplicitLod {{[^ ]+}} %[[PAIR0]]{{( |$)}}
//OPT: OpBranch %[[MERGE]]{{( |$)}}
//OPT: %[[MERGE]] = OpLabel

Texture2D<float> t0, t1;
SamplerState samp;

struct Pair
{
    Texture2D<float> a;
    Texture2D<float> b;
};

[noinline]
Pair selStruct(bool b)
{
    Pair p0 = {t0, t0};
    Pair p1 = {t1, t1};
    return b ? p1 : p0;
}

[shader("fragment")]
float main(bool b) : SV_Target
{
    Pair p = selStruct(b);
    return p.a.Sample(samp, float2(0, 0));
}
