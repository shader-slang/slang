//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#eliminatephis-with-default-options
//META: doc_section_digest=e0c92ad13ee96c904e5b118ad8e34d030d04052671354fe0520c1057981ecef2
//META: purpose=Metal runs `eliminatePhis` with the default options, so an if/else-merged scalar becomes a function-local variable assigned in each branch.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the shape the default phi-elimination options produce, in
// contrast to the register-allocating variant SPIR-V uses. The condition
// reads a `uniform` so the branch cannot be folded away at -O0; CHECK
// asserts a bare local declaration precedes the `if`, and that the same
// local is the assignment target inside both arms rather than a
// select/ternary expression.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

uniform int u;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v;
    if (u > 0)
    {
        v = int(tid.x);
    }
    else
    {
        v = -int(tid.x);
    }
    outBuf[tid.x] = v;
}

// CHECK: int v_{{[0-9]+}};
// CHECK: if(
// CHECK: v_{{[0-9]+}} = int(
// CHECK: else
// CHECK: v_{{[0-9]+}} =
