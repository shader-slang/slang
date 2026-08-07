//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=0c4ba248635633393b51ae889f226e3b5f86a549ad5be4af6f827414819fb357
//META: purpose=Boundary: negative zero, the largest finite float and the smallest positive normal are all finite, so each keeps the `f` suffix.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Sweeps the finite end of the float-literal axis at the magnitudes where
// a lost suffix would matter most: `-0.0f` has a distinct bit pattern from
// `0.0f`, and both the largest finite and the smallest positive normal
// value overflow or flush if the literal is first typed as `double` and
// then narrowed. Each is multiplied by a dispatch-derived value so none is
// folded; CHECK asserts every one of the three reaches emit with an `f`
// suffix.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = float(tid.x) * -0.0f;
    outBuf[1] = float(tid.x) * 3.40282347e+38f;
    outBuf[2] = float(tid.x) * 1.17549435e-38f;
}

// CHECK-DAG: -0.0f
// CHECK-DAG: 3.402823466{{[0-9]*}}e+38f
// CHECK-DAG: 1.175494350{{[0-9]*}}e-38f
