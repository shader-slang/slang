//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=7e284974bac81cfe7a4ad420d50d47c3ab731d9a15b876fdbff0eeaf9972d4ef
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
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
