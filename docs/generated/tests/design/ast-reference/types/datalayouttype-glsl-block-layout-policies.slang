//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=The data-layout policy types are the second generic argument of a uniform parameter group and each selects a distinct GLSL block layout, with the scalar policy additionally requiring the GL_EXT_scalar_block_layout extension.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the layout-policy rows of the node table are distinguishable
// branches rather than one interchangeable marker family. Three constant
// buffers over the *same* element struct are declared side by side: one with
// no policy argument, one with the std430 policy, and one with the scalar
// policy. If the policy did not reach layout the three blocks would carry the
// same qualifier; the CHECKs assert three different ones, plus the extension
// line that the scalar row names explicitly as its GLSL emit requirement.
// The declaration order is the emission order, so plain CHECKs are used.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain

struct Params
{
    float a;
    float3 b;
}

ConstantBuffer<Params> cbDefault;
ConstantBuffer<Params, Std430DataLayout> cb430;
ConstantBuffer<Params, ScalarDataLayout> cbScalar;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = cbDefault.a + cb430.a + cbScalar.b.x;
}

// The scalar policy is the one that needs an extension.
//GLSL: #extension GL_EXT_scalar_block_layout : require
// Default policy -> std140; explicit std430 and scalar policies -> their own.
//GLSL: layout(std140) uniform block_Params
//GLSL: layout(std430) uniform block_Params
//GLSL: layout(scalar) uniform block_Params
