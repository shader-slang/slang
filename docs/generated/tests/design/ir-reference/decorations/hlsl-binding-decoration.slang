//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#layout-and-binding
//META: doc_section_digest=fefaa116238af2b286c6cb73dc3224a4704918ed8d0cdbaa5e118e7c7b474c44
//META: purpose=A global parameter declared with a register(uN) binding lowers to a HasExplicitHLSLBinding decoration on the parameter.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `HasExplicitHLSLBinding` is the nullary marker recording
// that a global parameter was given an explicit register binding. The
// parameter also carries `[[vk::binding(3, 0)]]` so the SPIR-V target does
// not warn about a D3D register without a Vulkan binding; the CHECK pins
// the bare decoration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

[[vk::binding(3, 0)]]
uniform RWStructuredBuffer<int> buf : register(u3);

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [HasExplicitHLSLBinding]
