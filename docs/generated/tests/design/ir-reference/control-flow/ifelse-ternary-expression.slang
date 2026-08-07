//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=9107af33a205c736447b4c02d66c4032cb7ffae2ebc1f3e72dc1132780edf3e5
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
