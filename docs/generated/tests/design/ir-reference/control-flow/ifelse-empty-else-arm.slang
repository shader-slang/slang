//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=7e284974bac81cfe7a4ad420d50d47c3ab731d9a15b876fdbff0eeaf9972d4ef
//META: purpose=An if statement without an else still lowers to a four-operand ifElse with a synthesized merge block as the explicit structured join.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Even with no else arm, the front-end emits the structured ifElse
// terminator with a four-operand shape; the false-arm and the merge
// are the same target.

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
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
