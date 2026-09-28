//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#eliminatephis-with-default-options
//META: doc_section_digest=952b4b8bab84fb048622082284a7ab2ba264b8c1eb3e38f8c5aabf3b7aad5169
//META: purpose=Stress: six levels of nested if/else still resolve through eliminatePhis default options into a single named local with branch-wise assignments.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: deeply nested control flow (six levels). `eliminatePhis`
// default options still produce a single local merged at each join,
// not a phi inst in the emitted text.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = int(tid.x);
    int r = 0;
    if (x > 0) {
        if (x > 1) {
            if (x > 2) {
                if (x > 3) {
                    if (x > 4) {
                        if (x > 5) {
                            r = x;
                        } else { r = 5; }
                    } else { r = 4; }
                } else { r = 3; }
            } else { r = 2; }
        } else { r = 1; }
    }
    outBuf[tid.x] = r;
}

// CHECK: extern "C" __global__ void main
// CHECK: if(
// CHECK-NOT: phi
