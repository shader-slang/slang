//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=7e284974bac81cfe7a4ad420d50d47c3ab731d9a15b876fdbff0eeaf9972d4ef
//META: purpose=Boundary: the ternary `?:` expression also lowers to the structured ifElse terminator — the surface-level expression form produces the same four-operand structured tuple as the statement form.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A ternary `(a > b) ? a : b` is a value-producing expression but the
// LOWER-TO-IR pass still emits the structured ifElse, with the two
// arms branching to a merge block whose block-param receives the
// per-arm value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = (a > b) ? a : b;
}

// CHECK-LABEL: func %main
// CHECK: cmpGT(%a, %b)
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: unconditionalBranch(%{{[0-9]+}}, %a)
// CHECK: unconditionalBranch(%{{[0-9]+}}, %b)
