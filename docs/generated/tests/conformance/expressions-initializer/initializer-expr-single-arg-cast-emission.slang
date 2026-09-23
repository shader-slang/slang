//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:11+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-initializer.md#initializer-expressions
//META: doc_section_digest=4baa222d5b95c67d63377172d8a9ca06fa2e19f939661ef5099353e8c61b3470
//META: purpose=A single-argument initializer expression int(f) emits as an int() cast in HLSL, matching the C-style cast form.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a single-argument initializer expression `int(f)` and its
// C-style cast equivalent `(int)f` both emit as `int(...)` in HLSL.
// Two separate helper functions prevent DCE; both must appear in the emitted
// HLSL with `int(` casts, confirming the initializer-expression form is equivalent.
//
// The same claim is fanned out to the other text-emit back-ends, because the
// "single argument initializer expression is a cast" rule is realized
// independently by each emitter: HLSL/GLSL/Metal all spell the conversion with
// the C-like functional-cast syntax `int(...)`, and WGSL spells it with its own
// scalar-constructor syntax `i32(...)`. In every case the initializer-expression
// helper and the C-style-cast helper must emit the identical construct, which is
// what makes the two forms observably equivalent.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

float passFloat(float f) { return f; }

int castViaInitExpr(float f)
{
    // single-argument initializer expression form
    return int(f);
}

int castViaCStyle(float f)
{
    // C-style cast form — must be equivalent
    return (int)f;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(RWStructuredBuffer<int> buf)
{
    buf[0] = castViaInitExpr(passFloat(buf[1]));
    buf[2] = castViaCStyle(passFloat(buf[3]));
}

// Both functions should emit int(...) in HLSL
// CHECK: int(
// CHECK: int(

// GLSL emits the functional-cast form for both helpers.
// GLSL: int castViaInitExpr_{{[0-9]+}}(float
// GLSL: return int(
// GLSL: int castViaCStyle_{{[0-9]+}}(float
// GLSL: return int(

// Metal likewise emits `int(...)` for both helpers.
// METAL: int castViaInitExpr_{{[0-9]+}}(float
// METAL: return int(
// METAL: int castViaCStyle_{{[0-9]+}}(float
// METAL: return int(

// WGSL spells the same conversion with its scalar-constructor syntax `i32(...)`.
// WGSL: fn castViaInitExpr_{{[0-9]+}}({{.*}}f32{{.*}}) -> i32
// WGSL: return i32(
// WGSL: fn castViaCStyle_{{[0-9]+}}({{.*}}f32{{.*}}) -> i32
// WGSL: return i32(
