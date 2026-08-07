//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=9107af33a205c736447b4c02d66c4032cb7ffae2ebc1f3e72dc1132780edf3e5
//META: purpose=An `if` with no `else` lowers to a degenerate four-operand ifElse whose falseBlock operand names the same block as the afterBlock.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the degenerate ifElse shape an else-less `if` produces: the
// convenience emitter reuses the merge block as the false target, so
// operand 2 (falseBlock) and operand 3 (afterBlock) name the same block.
// The CHECK binds the fourth operand to a FileCheck variable and requires
// the third operand to be that same block id, which a lowering that
// synthesized four distinct blocks could not satisfy.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    if (a > 0)
    {
        buf[0] = a;
    }
    buf[1] = 1;
}

// CHECK-LABEL: func %main
// CHECK: cmpGT(%a, 0 : Int)
// falseBlock and afterBlock are bound to the same capture, so this only
// matches when the lowering collapsed them onto one block.
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %[[AFTER:[0-9]+]], %[[AFTER]])
