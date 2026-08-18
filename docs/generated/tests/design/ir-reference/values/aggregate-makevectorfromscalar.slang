//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=136fa0f20043e0a8db805b7f018d292446250e6117047722c77e7aa7efca0675
//META: purpose=The single-argument vector constructor `float3(x)` lowers to `MakeVectorFromScalar` with exactly one operand, not to a `makeVector` with the scalar repeated.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the splat constructor has its own opcode and that the
// element type and count come from the result type rather than from
// extra operands. The scalar is a `uniform float` so it cannot fold; the
// vector is stored so it cannot be dead-coded. CHECK asserts a
// `Vec(Float, 3 : Int)` result produced by `MakeVectorFromScalar(%f)` —
// a single operand — and that no `makeVector` with three copies of `%f`
// was emitted instead.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float f;
uniform RWStructuredBuffer<float3> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = float3(f);
}

// IR-LABEL: func %main
// IR: : Vec(Float, 3 : Int)	= MakeVectorFromScalar(%f)
// IR-NOT: makeVector(%f, %f, %f)
