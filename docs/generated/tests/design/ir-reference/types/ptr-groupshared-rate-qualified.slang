//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#rates-and-rate-qualified-types
//META: doc_section_digest=c24e0f57222f24155857b18aba7c3d8c022107d36ac8d499a6b2d26735d4b98b
//META: purpose=A groupshared declaration is typed RateQualified(GroupShared, Ptr(<T>)) in the IR.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A groupshared declaration is typed RateQualified(GroupShared,
// Ptr(<T>)) in the IR. A `groupshared int gs;` declaration surfaces
// `RateQualified(GroupShared, Ptr(Int))` as its IR type in the
// `global_var` declaration of `gs`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

groupshared int gs;
uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0) gs = a;
    GroupMemoryBarrierWithGroupSync();
    buf[tid.x] = gs;
}

// IR: global_var %gs {{.*}} RateQualified(GroupShared, Ptr(Int))
