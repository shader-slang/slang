//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#eliminatephis-with-default-options
//META: doc_section_digest=2b0ff9304692be73626bf7bd3c50f028f4c346a134dc0949d2343d82c090c772
//META: purpose=Stress: deeply nested if/else chains (>= 5 levels) reduce through default-option `eliminatePhis` to per-branch assignments to a single function-local variable.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: deep control flow (5 nested if/else levels). Metal
// uses default `eliminatePhis` options, so each branch writes to
// the same local. The emit must still produce a `kernel` entry
// and assign `x_0` from inside the nest.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;
uniform int level;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = int(tid.x);
    if (level > 0)
    {
        if (level > 1)
        {
            if (level > 2)
            {
                if (level > 3)
                {
                    if (level > 4)
                    {
                        x = 5;
                    }
                    else { x = 4; }
                }
                else { x = 3; }
            }
            else { x = 2; }
        }
        else { x = 1; }
    }
    outBuf[tid.x] = x;
}

// CHECK: kernel
// CHECK: void main_0
// CHECK: x_0
