//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#conditionalbranch-vs-ifelse
//META: doc_section_digest=3716e77c564043cb9f0c5b8c964ce43385211b156222579c14f4aba6721548d8
//META: purpose=The front-end emits structured ifElse — not conditionalBranch — as the in-loop condition test; the loop body's predicate becomes an ifElse with the break label as one target.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A for-loop with a predicate emits ifElse inside the body block —
// the structured form. conditionalBranch is the synthesized lower-
// level form that the front-end does not produce directly from
// surface code.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    for (int i = 0; i < n; i++)
    {
        s = s + 1;
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: cmpLT(%{{[a-zA-Z0-9_]+}}, %n)
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
