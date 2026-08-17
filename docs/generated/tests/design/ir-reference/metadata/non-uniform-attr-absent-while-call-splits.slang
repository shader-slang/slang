//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#nonuniform
//META: doc_section_digest=82806b4f8363ddaff36b0a51ebfe48d692f3149558d9e87f8a43db03bb5d0b95
//META: purpose=The nonuniform attribute never reaches a dump; its only visible effect is that the call site splits into two specialized functions with an identical signature.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one attribute on this page with a producer but no operand use.
// The shader is the worked example: the same helper is called once through a
// `NonUniformResourceIndex` subscript and once through a plain one. The
// snapshot is taken straight after the pass that specializes on resource usage
// so the whole dump is a single section, which lets the CHECK-NOTs bracket the
// two positive matches and so assert absence over the entire text. The two
// matches are the observable consequence: two distinct functions carrying the
// identical `Func(Vec(Float, 4 : Int), UInt)` signature, distinguishable only
// by a key that is never written into the IR.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-after specializeResourceUsage -o /dev/null -entry main -stage compute

struct Params
{
    float4 v;
}

ConstantBuffer<Params> gCB[64];
RWStructuredBuffer<float4> gOut;

float4 f(ConstantBuffer<Params> cb)
{
    return cb.v;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    gOut[0] = f(gCB[NonUniformResourceIndex(i)]) + f(gCB[i]);
}

// The attribute's mnemonic is all-lowercase `nonuniform`; the intrinsic that
// triggers it prints as `nonUniformResourceIndex` and is a different token.
// CHECK-NOT: nonuniform
// CHECK-NOT: Attributed(
// CHECK: {{^}}func %f{{[0-9]*}} : Func(Vec(Float, 4 : Int), UInt)
// CHECK: {{^}}func %f{{[0-9]*}} : Func(Vec(Float, 4 : Int), UInt)
// CHECK-NOT: nonuniform
// CHECK-NOT: Attributed(
