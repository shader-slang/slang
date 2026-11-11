//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#rates-and-rate-qualified-types
//META: doc_section_digest=1c6a8f0e5bba8df21836ec9c7740986d01e6ef7ccfa3cbf1a6ab88d0febf8224
//META: purpose=A groupshared declaration is typed RateQualified(GroupShared, Ptr(<T>)) in the IR.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A groupshared declaration is typed RateQualified(GroupShared,
// Ptr(<T>)) in the IR. A `groupshared int gs;` declaration surfaces
// `RateQualified(GroupShared, Ptr(Int))` as its IR type in the
// `global_var` declaration of `gs`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

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
