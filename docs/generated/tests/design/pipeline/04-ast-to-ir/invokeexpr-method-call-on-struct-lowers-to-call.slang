//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A method-call InvokeExpr `s.act(v)` on a struct lowers to a `call %S_act(...)` (the same `IRCall` family the doc lists) with the struct-typed receiver passed as the first argument.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: InvokeExpr-callee-shape axis. Free-function calls are
// covered by `invokeexpr-lowers-to-call`. A method-call expression
// `s.act(v)` resolves to the same IRCall opcode in lowering; the
// callee is the method's IRFunc and the receiver is the explicit
// first argument.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct S {
    int side;
    int act(int v) { return side + v; }
}

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    S s;
    s.side = a;
    buf[0] = s.act(a);
}

// CHECK-LABEL: func %main
// CHECK: call %{{.*}}act(%{{.*}})
