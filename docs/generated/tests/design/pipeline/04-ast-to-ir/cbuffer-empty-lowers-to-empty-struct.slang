//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=155d8d8ed14757fb02e5f6854e31cd2d2f05a5782b23f664d4b18331af96a126
//META: purpose=An empty `cbuffer CB {}` lowers to a `ConstantBuffer(%SLANG_ParameterGroup_CB, ...)` global parameter and a body-less struct type; layout-intent annotation appears later in the pipeline.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: layout-intent marker lower edge — empty cbuffer. The
// doc's "Layout intent on global parameters" note describes the
// global-param-with-layout shape, here at zero fields. The struct
// `%SLANG_ParameterGroup_CB` is declared but carries no
// `field(...)` rows.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -stage compute -entry main

cbuffer CB { }

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = 0;
}

// CHECK: struct %SLANGx5FParameterGroupx5FCB	: Type;
// CHECK: let  %CB	: ConstantBuffer({{.*}})	= global_param
