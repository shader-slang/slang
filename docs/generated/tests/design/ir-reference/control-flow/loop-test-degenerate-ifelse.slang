//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=9107af33a205c736447b4c02d66c4032cb7ffae2ebc1f3e72dc1132780edf3e5
//META: purpose=A loop-condition test lowers to a degenerate ifElse whose afterBlock operand names the loop body (its trueBlock) and whose falseBlock is the loop's break label.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second degenerate ifElse shape the doc calls out: a loop
// condition test is emitted with afterBlock == trueBlock rather than four
// distinct blocks. The CHECK binds the loop terminator's breakBlock operand
// and the ifElse's trueBlock operand, then requires the ifElse's falseBlock
// to be that break label and its afterBlock to be the same block as its
// trueBlock. A pass that assumed four distinct operands would be wrong here.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    for (int i = 0; i < n; i++)
    {
        s = s + i;
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// Operand 1 of `loop` is the break label; bind it so the loop test can be
// cross-anchored to this specific loop.
// CHECK: loop(%{{[0-9]+}}, %[[BRK:[0-9]+]], %{{[0-9]+}}, 0 : Int, 0 : Int)
// CHECK: cmpLT(%i, %n)
// CHECK: ifElse(%{{[0-9]+}}, %[[BODY:[0-9]+]], %[[BRK]], %[[BODY]])
