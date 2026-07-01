//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#synthesizebitfieldaccessors
//META: doc_section_digest=58fdc92508daf1d634e71de3c3efda4535d7c1e0dddc262515ee34d0afb791dd
//META: purpose=Phase B's synthesizeBitFieldAccessors (B4) synthesises get/set bodies for bit-field accessors so the early-inlining loop can inline them.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct with bit-field members produces a "backing" integer
// field after Phase B. The accessor bodies are inlined into the
// emit by the mandatory-early-inlining loop. We verify the
// backing field name appears in the HLSL emit (the doc anchors
// at the Phase B pass; the backing-field token is its post-pass
// footprint).

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
