//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=136fa0f20043e0a8db805b7f018d292446250e6117047722c77e7aa7efca0675
//META: purpose=Stress: two distinct `float4(...)` constructions feeding a vector `add` lower to two `makeVector(...)` opcodes plus an `add(%a, %b)` on the vector type — pattern-bloat probe for componentwise vector arithmetic.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that two independent `float4(...)` constructor calls each
// lower to their own `makeVector(...)` opcode and that the vector `+`
// between them lowers to a single `add` on the vector type. Both
// constructor argument lists are built from `uniform` floats so nothing
// folds; CHECK asserts the two distinct operand orders and the `add`,
// which is only reachable if the aggregate constructor and the
// componentwise arithmetic opcode compose without an intermediate.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform float fb;
uniform float fc;
uniform float fd;
uniform RWStructuredBuffer<float4> v4buf;

[numthreads(1, 1, 1)]
void main()
{
    float4 a = float4(fa, fb, fc, fd);
    float4 b = float4(fd, fc, fb, fa);
    float4 c = a + b;
    v4buf[0] = c;
}

// IR-LABEL: func %main
// IR: makeVector(%fa, %fb, %fc, %fd)
// IR: makeVector(%fd, %fc, %fb, %fa)
// IR: add(%a, %b)
