//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/overview.md#ast-ir-lowering
//META: doc_section_digest=8186b395b3684ed4e6e22f74790f541ad117cdf9f69fed4adc7bd83bf333f222
//META: purpose=Statements in the AST (here an if-statement) become basic blocks in IR and re-emerge as target-appropriate branching in the emitted text.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the overview claim that "statements become basic blocks
// with parameters; expressions become SSA value instructions". An
// if/else with two distinct buffer writes forces both arms to
// survive DCE, so the branch structure has to reach the target
// emit. HLSL keeps it as an `if`, GLSL keeps it as an `if`, and
// SPIR-V emits an OpSelectionMerge plus OpBranchConditional.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> gOut;
cbuffer Cb { int gThreshold; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    if (gThreshold > 0)
    {
        gOut[0] = 1;
    }
    else
    {
        gOut[0] = -1;
    }
}

// HLSL: if
// GLSL: if
// SPIRV: OpBranchConditional
