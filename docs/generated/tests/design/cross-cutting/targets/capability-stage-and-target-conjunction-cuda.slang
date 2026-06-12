//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#vocabulary
//META: doc_section_digest=e6ee8d6a0ad9b6be0dfa1e2bef7fa142d6866067cb581476b3dadc1f13bb47e4
//META: purpose=The stage / target keyhole independence claim extends to `cuda`: a `compute` entry point conjoined with the `cuda` target atom is a valid combination (different keyholes), mirroring the C-01 smoke probe at a single specific target.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the `target` × `stage` keyhole conjunction
// described in the Vocabulary section, fixing the target axis at
// `cuda` and stressing that `compute` + `cuda` is a legal pair (the
// smoke test exercises every text target generically; this probe pins
// CUDA, which is the most idiosyncratic of the seven). The
// `__global__` marker in the emitted text is the visible footprint
// of the cuda atom populating the target keyhole.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 7;
}

// CHECK: __global__
// CHECK: 7
