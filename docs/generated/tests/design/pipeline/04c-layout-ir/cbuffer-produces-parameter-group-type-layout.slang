//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:02:55+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=bea4097f3cb99e6205f4e10d4bf9ab240dde03162a3388a24e9a78e55d5c0e65
//META: purpose=When the global scope is wrapped in a parameter group (cbuffer), the module's layout decoration is an IRParameterGroupTypeLayout rather than the raw IRStructTypeLayout; observable as parameterGroupTypeLayout(...) in the layout block.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that wrapping the global scope in a parameter group (a
// cbuffer here) makes the module-level layout an
// IRParameterGroupTypeLayout rather than the raw struct layout: the
// CHECK asserts a parameterGroupTypeLayout(...) instruction appears
// in the first post-link layout block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

cbuffer CB { float4 color; }
uniform RWStructuredBuffer<float4> result;

[numthreads(1, 1, 1)]
void main()
{
    result[0] = color;
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: parameterGroupTypeLayout(
