//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `type_param` global generic parameter reaching layout with no concrete binding is rejected with E38207.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `_createTypeLayout` has a `GlobalGenericParamDecl` arm that first asks
// `findGlobalGenericSpecializationArg` whether the program layout carries a
// concrete argument for the parameter, and, finding none, builds a
// `GenericParamTypeLayout` placeholder via
// `_createTypeLayoutForGlobalGenericTypeParam`. Compiling a `type_param` from
// the CLI always takes the second path, because there is no way to supply a
// global generic argument without the host API; the placeholder layout is then
// rejected downstream. The test pins that this combination is an error rather
// than a silently zero-sized parameter.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

interface IColor { float4 getColor(); }
type_param TColor : IColor;
//CHECK:   ^^^^^^ global generic parameter used in code without a concrete binding
//CHECK:   ^^^^^^ a global generic parameter ('type_param' / '__generic_value_param') cannot be used in shader code without a concrete binding; such global-scope declarations are intended for reflection and external specialization, not direct use in shader bodies
struct Wrapper { TColor c; float scale; }
ConstantBuffer<Wrapper> cb;
RWStructuredBuffer<float4> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = cb.c.getColor() * cb.scale;
}
