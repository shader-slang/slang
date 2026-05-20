//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#rates-and-rate-qualified-types
//META: doc_section_digest=480f214056fa4a8161f1c884e5d3f82bfc07b1bc1c95814f32ba958d315dc859
//META: purpose=A groupshared declaration is typed RateQualified(GroupShared, Ptr(<T>)) in the IR.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Rates and rate-qualified types" table lists `GroupShared`
// as a Rate opcode and `RateQualified` as the type that combines a
// rate with a value type. A `groupshared int gs;` declaration
// surfaces `RateQualified(GroupShared, Ptr(Int))` as its IR type
// in the `global_var` declaration of `gs`.

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
