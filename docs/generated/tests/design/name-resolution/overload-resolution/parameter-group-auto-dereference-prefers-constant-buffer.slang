//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversions-that-do-not-have-their-own-cost-level
//META: doc_section_digest=234f96bc98a7bac5b1f6d6a6347fc524534856924818bfab5d35d45f80856a61
//META: purpose=A ConstantBuffer source is implicitly dereferenced at kConversionCost_ImplicitDereference (10) before converting onward, so an overload taking ConstantBuffer<Foo> beats one taking Foo.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the auto-dereference of a parameter-group type is charged a
// real cost, so the overload that takes the parameter group itself stays
// cheaper than the one that takes the dereferenced element. The two overloads
// return distinct sentinels; CHECK asserts the emitted HLSL contains the
// ConstantBuffer overload's sentinel and never the element overload's.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute

struct Foo { int v; }

ConstantBuffer<Foo> gFoo;
RWStructuredBuffer<int> outputBuffer;

int pickCB(ConstantBuffer<Foo> c) { return 4242; }
int pickCB(Foo f)                 { return 7777; }

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = pickCB(gFoo);
}

//HLSL: 4242
//HLSL-NOT: 7777
