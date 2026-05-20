//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=3e412bb1b8df51b023821b69be767e4f040104ad482e9063949a7dd98d9eda38
//META: purpose=Stress: two distinct `float4(...)` constructions feeding a vector `add` lower to two `makeVector(...)` opcodes plus an `add(%a, %b)` on the vector type — pattern-bloat probe for componentwise vector arithmetic.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two `makeVector` calls feeding a vector-typed `add` exercise
// the variadic constructor + componentwise arithmetic pattern.
// The result type of the `add` is `Vec(Float, 4 : Int)`, the same
// type the constructors produce. (Naming note: this file pre-
// existed as a diagnostic probe during expansion; its content was
// rewritten to a real stress test before lint.)

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
