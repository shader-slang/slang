//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=585639ca71660c2af61e0642d91c54c3088c5a4152aa87e34e0f76e055122b45
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

//CHECK: E39020
