//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#capabilities-and-extensions-decided-at-emit-time
//META: doc_section_digest=a1bda696e594d0d008416384d3cd2b7b449a57153c72f78ceef01aa513a35c5e
//META: purpose=Without -fp-mode precise the same float multiply-add carries no NoContraction decoration.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative side of the NoContraction rule: the decoration is a
// consequence of precise mode, not something the emitter always attaches. The
// shader is the same buffer-fed multiply-add as the precise-mode companion but
// is compiled at the default fp mode; CHECK asserts the float arithmetic is
// still emitted while CHECK-NOT asserts no NoContraction appears anywhere.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float> outBuf;
StructuredBuffer<float> inBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float x = inBuf[tid.x];
    float y = inBuf[tid.x + 2];
    outBuf[tid.x] = x * y + y;
}

// CHECK-NOT: NoContraction
// CHECK: OpFMul %float
