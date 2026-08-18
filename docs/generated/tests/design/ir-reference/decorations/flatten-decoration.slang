//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=4b61bbe5577fdd277138f6c76e47bb415b4e0d9c6365f3fd1961205026f01739
//META: purpose=A [flatten] attribute on a conditional lowers to a flatten decoration with no operands attached to the conditional inst.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `flatten` row's operand shape: the decoration is nullary, so
// the dump prints a bare `[flatten]`. As in the `branch` sibling the
// condition is thread-id dependent and the result escapes through a buffer
// write, so the conditional is still present when the dump is taken.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = 0;
    [flatten]
    if (tid.x == 0u) v = 10;
    else v = 20;
    buf[0] = v;
}

// CHECK: [flatten]
