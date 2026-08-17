//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=58ff39ee4b5eb63d9fd7dd839e0f9f67e7ef5727cd278b7eee0411b98e5d64fe
//META: purpose=Boundary: an if with an empty then-arm but a non-empty else still lowers to the four-operand ifElse, with the true-arm operand pointing at the same block as the merge.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on the ifElse axis: the then-arm is degenerate (empty) but
// the else-arm has a side effect. The structured ifElse terminator is
// still emitted with all four operands.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    if (a > 0)
    {
        // empty then-arm
    }
    else
    {
        buf[1] = a;
    }
    buf[0] = a;
}

// CHECK-LABEL: func %main
// CHECK: cmpGT(%a, 0 : Int)
// The true-arm operand and the afterBlock operand share one capture, so this
// only matches when the empty arm was collapsed onto the merge block.
// CHECK: ifElse(%{{[0-9]+}}, %[[AFTER:[0-9]+]], %{{[0-9]+}}, %[[AFTER]])
