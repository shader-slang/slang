//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=d4c1621bec12a7398cb1a9674411d74b7313361b2b6d60447dbaae79eb52f58e
//META: purpose=Stress: a diagnostic fired inside deeply nested control-flow (>= 6 levels) still resolves to the correct source line and renders with full caret alignment.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probe the boundary of source-location decoding under deep
// lexical nesting. The undefined identifier sits 6 levels deep
// inside `if` blocks; the renderer must still produce the
// correct file:line:column triple naming the actual offending
// line, not the outer entry-point line.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x > 0) {
        if (tid.x > 1) {
            if (tid.x > 2) {
                if (tid.x > 3) {
                    if (tid.x > 4) {
                        if (tid.x > 5) {
                            int z = deeplyNestedUndef;
                        }
                    }
                }
            }
        }
    }
}

// CHECK: undefined identifier 'deeplyNestedUndef'
