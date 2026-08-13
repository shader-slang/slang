//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=c0e97396ea139ceab54212053b6d443692d19f0faf2847c4dc505373ae8c4861
//META: purpose=Stress: a diagnostic raised inside six levels of nested control flow still decodes to the correct line and renders a caret.
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
