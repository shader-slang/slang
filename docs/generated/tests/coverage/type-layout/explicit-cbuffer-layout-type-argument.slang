//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies the explicit `Std430DataLayout` and `ScalarDataLayout` type arguments on `ConstantBuffer` select those rule sets, packing a scalar into a float3's tail at offset 12 instead of the std140 offset 16.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `GLSLLayoutRulesFamilyImpl::getConstantBufferRules` switches on the second
// type argument of `ConstantBuffer<T, Layout>` before consulting any global
// option, but only the `Std140DataLayout` and `CDataLayout` cases are exercised
// by the shipped suites. `float3 a; float b;` is the shape that distinguishes
// the remaining two from the default: under std140 the vector's 16-byte
// alignment pushes `b` to offset 16, whereas std430 and scalar layout both
// allow `b` to occupy the float3's fourth component slot at offset 12. The
// emitted type-name suffixes (`_std430` / `_natural`) confirm which rule set
// each buffer actually selected, so the test cannot pass by accident on the
// default rules.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute

struct S { float3 a; float b; }
ConstantBuffer<S, Std430DataLayout> cb430;
ConstantBuffer<S, ScalarDataLayout> cbScalar;
RWStructuredBuffer<float> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = cb430.b + cbScalar.b;
}

//SPV-DAG: OpMemberDecorate %S_std430 0 Offset 0
//SPV-DAG: OpMemberDecorate %S_std430 1 Offset 12
//SPV-DAG: OpMemberDecorate %S_natural 0 Offset 0
//SPV-DAG: OpMemberDecorate %S_natural 1 Offset 12
