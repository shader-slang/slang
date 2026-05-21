//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A FuncDecl that calls itself lowers to a single IRFunc whose body contains a `call %self(...)` instruction targeting the same IRFunc — the doc's `InvokeExpr -> IRCall` row applied to a self-reference.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: function call-shape axis — recursion. The "InvokeExpr ->
// IRCall" row says nothing about whether the callee is the enclosing
// function. A recursive callee is observable as the same `%factorial`
// name appearing both on `func %factorial` and inside `call
// %factorial(...)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int factorial(int n)
{
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = factorial(a);
}

// CHECK-LABEL: func %factorial
// CHECK: call %factorial(%{{.*}})
