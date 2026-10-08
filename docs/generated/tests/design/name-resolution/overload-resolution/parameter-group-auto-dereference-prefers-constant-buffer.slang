//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversions-that-do-not-have-their-own-cost-level
//META: doc_section_digest=ecd3a6f3ea79e7ef3d125546849e46357976725d2a9d4de69b69741a1f69fac4
//META: purpose=A ConstantBuffer source is implicitly dereferenced at kConversionCost_ImplicitDereference (10) before converting onward, so an overload taking ConstantBuffer<Foo> beats one taking Foo.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the auto-dereference of a parameter-group type is charged a
// real cost, so the overload that takes the parameter group itself stays
// cheaper than the one that takes the dereferenced element. The two overloads
// return distinct sentinels; each per-target CHECK asserts the emitted code
// contains the ConstantBuffer overload's sentinel and never the element
// overload's. Overload resolution runs before codegen, but the chosen
// callee is what every back-end actually emits, so the fan-out below pins
// the sentinel in each emitter's own output form.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute

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

//GLSL: return 4242;
//GLSL-NOT: 7777

//METAL: return int(4242);
//METAL-NOT: 7777

//WGSL: return i32(4242);
//WGSL-NOT: 7777

//SPIRV: OpConstant %int 4242
//SPIRV-NOT: 7777
