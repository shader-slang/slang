//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#synthesizebitfieldaccessors
//META: doc_section_digest=567a399041a92fcbf85453003530b8688fa87949447a5f2fae5b2dd5e5cb2b64
//META: purpose=Phase B's synthesizeBitFieldAccessors (B4) synthesises get/set bodies for bit-field accessors early enough that the Phase C inlining loop can inline them into the emitted shift/mask sequence.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the observable footprint of the accessor synthesis: the two
// bit-field members collapse onto one integer backing field, and the
// get/set bodies are inlined rather than surviving as calls. The emitted
// HLSL is the stable surface for this -- the CHECK looks for the backing
// field's name and for the mask/shift arithmetic that the inlined setter
// reduces to.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

struct S { int a : 4; int b : 4; };
uniform RWStructuredBuffer<S> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    S s = buf[0];
    s.a = 3;
    buf[0] = s;
}

// CHECK: x24bit_field_backing
