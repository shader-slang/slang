//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies a `switch` with shared empty cases and a fall-through case emits shared case labels and pins the per-target fall-through shape (HLSL duplicates the default body, GLSL/SPIR-V do not).
//META: intent=characterization
//META: covers=source/slang/slang-emit-c-like.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins emitSwitchCaseSelectorsImpl / region-based switch emission
// (slang-emit-c-like.cpp:3555, :3718). cases 0 and 1 are empty and share
// one label/body; case 2 has no break and falls through into default.
// The observable per-target divergence: HLSL emits case 2 with the
// default body *duplicated* inline (`+100` then `+1`, with a break),
// whereas SPIR-V keeps a single OpSwitch with cases 0 and 1 pointing at
// the same label. The switch operand is runtime (tid.x + i) so it is not
// folded; result sunk to outputBuffer.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int x = (int)tid.x;
    int r = 0;
    [loop]
    for (int i = 0; i < 4; i++)
    {
        switch (x + i)
        {
        case 0:
        case 1:
            r += 10;
            break;
        case 2:
            r += 100;
            // fallthrough
        default:
            r += 1;
            break;
        }
        if (r > 1000)
            break;
    }
    outputBuffer[0] = r;
}

// HLSL emits cases 0 and 1 sharing one body, then case 2 falling into a
// duplicated default (+100 then +1).
//HLSL-DAG: case int(0):
//HLSL-DAG: case int(1):
//HLSL-DAG: r_0 + int(100)
// SPIR-V keeps one OpSwitch with 0 and 1 mapped to the same case label
// and 2 to its own; the trailing operand is the default label.
//SPV: OpSwitch %{{[0-9]+}} %{{[0-9]+}} 0 %[[C01:[0-9]+]] 1 %[[C01]] 2 %{{[0-9]+}}
