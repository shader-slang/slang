//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#builtin-operators-bypass-call-lowering
//META: doc_section_digest=32b31a97f5e15fb2fec9486a881ce13cddbd02cb72566084734c0db4939ddf9b
//META: purpose=A builtin `==` on `bool` operands is fast-pathed like numeric operands and lowers to a single pure `cmpEQ` inst — no call instruction precedes it in the body.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: operand-type axis (bool). The fast path is documented for
// builtin numeric *or bool* operands; `==` on two bools must lower to
// one `cmpEQ` reached without any `call` (callable resolution is
// skipped). The compared result feeds an `if` so the value escapes
// through an int buffer without any bool storage or cast.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform bool a;
uniform bool b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int r = 0;
    if (a == b)
        r = 1;
    buf[0] = r;
}

// CHECK-LABEL: func %main
// CHECK-NOT: = call
// CHECK: cmpEQ(%{{.*}}, %{{.*}})
