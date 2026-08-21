//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#backwarddifferentiate
//META: doc_section_digest=12e46df686aab82f431b585fd75feb5a9219b7cce8632e69cae32a8214fb6322
//META: purpose=The user-facing __bwd_diff form does not reach IR as a BackwardDifferentiate inst, because semantic checking resolves the expression earlier.
//META: intent=negative
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out that makes the `BackwardDifferentiate` row's AST
// origin misleading: writing `__bwd_diff(f)` never produces that opcode. The
// entry point uses exactly the surface a reader would expect to produce it,
// and CHECK first pins what is produced instead — the combined legacy request
// over `%f` — then requires that no `= BackwardDifferentiate(` producer exists
// on either side of it within the LOWER-TO-IR section. The `= ` prefix on the
// forbidden pattern matters: without it the `CHECK-NOT` would also be tripped
// by the `LegacyBackwardDifferentiate` and `TrivialBackwardDifferentiate`
// spellings, which contain the shorter name as a substring.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float f(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    DifferentialPair<float> dp = DifferentialPair<float>(a, 0.0);
    __bwd_diff(f)(dp, 1.0);
    buf[0] = dp.d;
}

// CHECK: {{^}}### LOWER-TO-IR:
// CHECK-NOT: = BackwardDifferentiate(
// CHECK: = LegacyBackwardDifferentiate(
// CHECK-NOT: = BackwardDifferentiate(
// CHECK: {{^}}###{{$}}
