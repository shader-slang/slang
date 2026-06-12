//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
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
