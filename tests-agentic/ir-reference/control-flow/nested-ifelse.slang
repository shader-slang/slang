//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=7e284974bac81cfe7a4ad420d50d47c3ab731d9a15b876fdbff0eeaf9972d4ef
//META: purpose=Nested if-statements produce nested structured ifElse terminators — each level carries its own four-operand cond/true/false/merge tuple.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two nested conditionals on uniforms produce two distinct ifElse
// terminators in the same function — the outer arm contains an
// inner ifElse of its own.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    if (a > 0)
    {
        if (b > 0)
        {
            buf[0] = a + b;
        }
        else
        {
            buf[0] = a;
        }
    }
    else
    {
        buf[0] = -1;
    }
}

// CHECK-LABEL: func %main
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
