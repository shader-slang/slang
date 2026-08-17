//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=136fa0f20043e0a8db805b7f018d292446250e6117047722c77e7aa7efca0675
//META: purpose=The single-argument matrix constructor `float2x2(x)` lowers to `makeMatrixFromScalar` with one operand, splatting the scalar across the matrix.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the matrix counterpart of the vector splat opcode. The scalar
// is a `uniform float`, and one element of the resulting matrix is read
// back through `getElement` so neither the construction nor the read can
// be eliminated. CHECK asserts the result line is
// `Mat(Float, 2 : Int, 2 : Int, ...)` produced by
// `makeMatrixFromScalar(%f)`, proving the four elements are not spelled
// out as separate `makeMatrix` operands.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float f;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    float2x2 m = float2x2(f);
    buf[0] = m[1][1];
}

// IR-LABEL: func %main
// IR: : Mat(Float, 2 : Int, 2 : Int, {{.*}})	= makeMatrixFromScalar(%f)
