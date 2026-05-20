//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T10:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=9ce4d053661212c26b75e2e5610820fbcc296703c400512972d2afca16087b34
//META: purpose=A cbuffer containing a single scalar lowers to a `ConstantBuffer<...>` global parameter whose payload struct carries exactly one `field(...)` row.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: cbuffer single-field lower edge. The doc's
// "Layout intent on global parameters" implies a per-field
// breakdown; with one field, the lowered struct has exactly one
// `field(%x, Int)` row.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

cbuffer CB { int x; }

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = x;
}

// CHECK-LABEL: struct %SLANGx5FParameterGroupx5FCB
// CHECK: field(%x, Int)
// CHECK: let  %CB	: ConstantBuffer({{.*}})	= global_param
