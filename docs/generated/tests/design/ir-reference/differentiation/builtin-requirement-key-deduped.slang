//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#builtinrequirementkey
//META: doc_section_digest=82acd463d8cb41f494c18915dac62b4eeb436a684f415f6a51bfca02f2381039
//META: purpose=builtinRequirementKey is hoistable and deduplicated by construction from its kind operand, so one logical built-in requirement is a single key inst with no linkage decoration.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the identity-by-operand claim that distinguishes a built-in
// requirement key from an ordinary requirement key. The shader touches the
// `IDifferentiable` family from two different user types, so every recognized
// requirement is keyed more than once during lowering. CHECK captures the kind
// operand of the first key inst as a FileCheck variable and then forbids any
// second inst with that same kind for the rest of the LOWER-TO-IR section; a
// key that carried per-declaration identity instead of operand identity would
// produce a second inst here. The closing `###` pattern bounds the assertion to
// that one dump section, because the later per-pass dumps repeat the module.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

struct P : IDifferentiable
{
    typealias Differential = P;
    float v;
}

[Differentiable]
float f(float x)
{
    return x * x;
}

[Differentiable]
float g(P p)
{
    return p.v + p.v;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(DifferentialPair<float>(a, 1.0)).d;
    buf[1] = __fwd_diff(g)(DifferentialPair<P>({a}, {1.0})).d;
}

// CHECK: {{^}}### LOWER-TO-IR:
// CHECK: = builtinRequirementKey([[KIND:[0-9]+]] : Int)
// CHECK-NOT: = builtinRequirementKey([[KIND]] : Int)
// CHECK: {{^}}###{{$}}
