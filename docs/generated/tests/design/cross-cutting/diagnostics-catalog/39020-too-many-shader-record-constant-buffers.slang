//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E39020 (too-many-shader-record-constant-buffers) - too many shader record constant buffers
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39020
//META: catalog_name=too-many-shader-record-constant-buffers
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// At most one constant buffer may be tagged
// `[[vk::shader_record]]`; declaring two triggers E39020.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage closesthit -entry main

[[vk::shader_record]]
cbuffer SR1 { int a; }

[[vk::shader_record]]
cbuffer SR2 { int b; }

struct Payload { float4 c; }

[shader("closesthit")]
void main(inout Payload p, in BuiltInTriangleIntersectionAttributes attrs)
{
    p.c = float4(0);
}

// CHECK: E39020
