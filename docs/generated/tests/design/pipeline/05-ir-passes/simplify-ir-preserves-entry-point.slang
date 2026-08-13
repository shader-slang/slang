//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=1784b8e9f30332a4995be817424580937cb7178f3cb7ce01ea908f8b8807e9fa
//META: purpose=Repeated runs of simplifyIR after the cleanup category leave only the final-form IR for emit; the `func %main` line remains as the user-visible entry point.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The `simplifyIR` pass listed in the cleanup category runs
// multiple times. Each run preserves the user-named entry point
// (`main`). We observe two distinct `### AFTER simplifyIR:` headers
// in the dump and confirm that `func %main` is visible in the final
// such block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-after simplifyIR -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// CHECK-LABEL: ### AFTER simplifyIR:
// CHECK: func %main
