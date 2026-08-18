//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
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
