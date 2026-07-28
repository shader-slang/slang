//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=6dd59415374c5af65d07340c8540b315850c081483ba8c4243ef7e32cac24212
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
