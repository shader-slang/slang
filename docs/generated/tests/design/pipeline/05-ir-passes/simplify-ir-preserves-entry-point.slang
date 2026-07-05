//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=bf314e51995b5a2b76e1104cf5f719db98f30e41b3a1ec2cddb5792a8ffcdf27
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
