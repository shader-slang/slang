//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/ir-reference/differentiation.md#opcodes
//META: doc_section_digest=aacf05fed4ed4ce34369f80555096390dd0ba483e842c1405e66f4d2a320bfb7
//META: purpose=The no_diff parameter marker is implemented by a unique module-scope let %k : Void = no_diff value that all Attributed(T, %k) wrappers reference.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The `no_diff` marker is a unique module-scope IR value (`let %k :
// Void = no_diff`). Every `no_diff T x` parameter wraps the
// parameter's type in `Attributed(T, %k)` where `%k` is that
// module-scope value. This test pins both the module-scope opcode
// binding and the attribute usage.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float c;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float scale(no_diff float k, float x)
{
    return k * x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(scale)(c, DifferentialPair<float>(a, 1.0)).d;
}

// Module-scope no_diff opcode value is bound at top level...
// CHECK: : Void	= no_diff
// ...and the parameter type wraps `Float` with that Attributed marker.
// CHECK: Attributed(Float,
