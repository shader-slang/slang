//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#hlsl-semantics
//META: doc_section_digest=924ecf08334941e0ce19332c62256f9af73bd09c738c20e634f3eefd0c64d11f
//META: purpose=A `: register(...)` binding semantic propagates to emitted HLSL.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `: register(...)` records a register class plus index that layout
// honours instead of assigning the default `u0`. The buffer asks for `u3` and
// CHECK asserts the emitted HLSL declaration carries exactly that binding.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf : register(u3);

[numthreads(1,1,1)]
void main()
{
    outBuf[0] = 1;
}

//CHECK: register(u3)
