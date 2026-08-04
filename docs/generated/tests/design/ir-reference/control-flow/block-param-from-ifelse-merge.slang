//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#block-and-param
//META: doc_section_digest=6464d00693cef2b4a0aff4d2217833f482b5d7a83cc29251b96ae2aaa6d765d4
//META: purpose=Boundary: a phi at an if-merge (each arm of a ternary supplying a different value) appears as a `param` on the merge block, fed by argument operands on each predecessor's unconditionalBranch.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A ternary `(a > b) ? a : b` produces an if-merge phi: the merge
// block declares a single `param` whose value each arm supplies via
// its `unconditionalBranch` argument operand. This is the if-merge
// counterpart to the existing switch-merge phi test.

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
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: unconditionalBranch(%{{[0-9]+}}, %a)
// CHECK: unconditionalBranch(%{{[0-9]+}}, %b)
// CHECK: param %{{[a-zA-Z0-9_]+}}	: Int
