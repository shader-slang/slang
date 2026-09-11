//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#undoparametercopy-and-transformparamstoconstref
//META: doc_section_digest=2daf73e31e7ca500dd32820aabeebff047d750c36d45ddc66ec8d467f09ff675
//META: purpose=Verifies an inout int parameter still lowers to pass-by-pointer when the supplied value is int MAX (boundary value does not change the pass shape).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: inout pointer parameter carrying an int MAX
// value. The `undoParameterCopy` rewrite produces an `inc_0(int *)`
// signature regardless of the address-space-edge value passed in.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

void inc(inout int x) { x = x + 1; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = int(2147483647);
    inc(s);
    outBuf[tid.x] = s;
}

// CHECK: inc{{[_0-9]*}}(int *
// CHECK: int(2147483647)
