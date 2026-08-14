//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A FuncDecl with eight parameters lowers to an IRFunc whose entry block carries eight `param` instructions in declaration order.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: function-parameter "many" edge (>= 8 args). Each
// AST-level parameter shows up as a `param %name` at the start of
// the entry block; the IRFunc's function type carries them as
// trailing operands in `Func(Int, Int, Int, ...)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int x;
uniform RWStructuredBuffer<int> buf;

int mp(int a, int b, int c, int d, int e, int f, int g, int h)
{
    return a + b + c + d + e + f + g + h;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = mp(x, x, x, x, x, x, x, x);
}

// CHECK-LABEL: func %mp
// CHECK: param %a
// CHECK: param %b
// CHECK: param %c
// CHECK: param %d
// CHECK: param %e
// CHECK: param %f
// CHECK: param %g
// CHECK: param %h
