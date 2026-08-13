//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#module-level-outputs
//META: doc_section_digest=0b34b0fb1813131149d3616a45cc3577cb15e3fd3da49c3edccfff26407d2c0a
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
