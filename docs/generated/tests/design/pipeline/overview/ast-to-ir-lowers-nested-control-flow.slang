//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#ast--ir-lowering
//META: doc_section_digest=d2bb343d99876dfd1d05e29a3d730266a1a5c82ccace506601773d10ac4885df
//META: purpose=Five levels of nested if/else lower to five nested conditional branches that all survive to the emitted target text.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pushes the statement-to-basic-block claim to a nesting depth where the
// join blocks have to be stacked rather than merged pairwise: five
// conditions, each derived from a different dispatch component so none can
// be folded, with a distinct result value per leaf so no arm can be
// eliminated. The CHECKs count five conditional branches per target, which
// only holds if every nesting level produced its own merge point instead of
// collapsing into a flattened select chain.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int r = 0;
    if (tid.x > 0u)
        if (tid.y > 1u)
            if (tid.z > 2u)
                if (tid.x > tid.y)
                    if (tid.y > tid.z)
                        r = 5;
                    else
                        r = 4;
                else
                    r = 3;
            else
                r = 2;
        else
            r = 1;
    gOut[0] = r;
}

// HLSL-COUNT-5: if(
// HLSL: int(5)

// SPIRV-COUNT-5: OpBranchConditional

// WGSL-COUNT-5: if(
// WGSL: i32(5)

// METAL-COUNT-5: if(
// METAL: int(5)

// CPP-COUNT-5: if(
// CPP: int(5)
