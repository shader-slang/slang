//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#builtin-operators-bypass-call-lowering
//META: doc_section_digest=84a14c8c06fb62977462b222065d70ef81479ad93846db3d8c9e968027e956d2
//META: purpose=A builtin elementwise `+` on `float2x2` matrix operands takes the checker fast path and lowers to a single pure `add` inst whose result IR type is the matrix type `Mat(Float, 2 : Int, 2 : Int, ...)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: operand-shape axis (matrix). The fast path covers matrix
// operators as well as scalars and vectors; elementwise matrix `+`
// must lower to one `add` typed as the Mat IR type rather than a call
// to an operator function.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float2x2 a;
uniform float2x2 b;
uniform RWStructuredBuffer<float2x2> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + b;
}

// CHECK-LABEL: func %main
// CHECK-NOT: = call
// CHECK: Mat(Float, 2 : Int, 2 : Int, {{[0-9]+}} : Int){{.*}}= add(%{{.*}}, %{{.*}})
// CHECK-NOT: = call
// CHECK: return_val
