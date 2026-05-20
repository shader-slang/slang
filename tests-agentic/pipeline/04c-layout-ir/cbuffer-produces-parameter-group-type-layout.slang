//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:02:55+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=21e70fc9b432ffb7540b46ba9ead6c9d1a1b5e21edaccd9090e6fe0740eaa77e
//META: purpose=When the global scope is wrapped in a parameter group (cbuffer), the module's layout decoration is an IRParameterGroupTypeLayout rather than the raw IRStructTypeLayout; observable as parameterGroupTypeLayout(...) in the layout block.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says "When the global scope is wrapped in a parameter
// group (a constant buffer or push-constant block), the module's
// layout decoration is an IRParameterGroupTypeLayout rather than
// the raw struct layout (lines 15401-15419)." A cbuffer introduces
// the parameterGroupTypeLayout(...) instruction in the layout block.

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
