//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=96146019aaea16661ac66159cf8328c2ae1406fa65d0708df73480a207c02e9c
//META: purpose=A `__target_switch` with zero arms is accepted by the parser and dropped by `slang-ir-specialize-target-switch.cpp` — the degenerate lower edge of arm count.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `__target_switch` arm-count zero boundary: an empty body is the
// degenerate lower edge. The doc's specialization pass is named in the
// "How target choice affects IR" section; with no arms it has nothing
// to select, and control flows past the construct to the explicit
// return below. The CHECK confirms the function body compiles to
// emitted text for the matching target.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

int after()
{
    __target_switch
    {
    }
    return 11;
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = after();
}

// HLSL: 11
