//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#capabilities-and-extensions-decided-at-emit-time
//META: doc_section_digest=42b60cbc03a13fbb7144f6605c9a5b8ebf0ab604187ecf3ce7e27dab3df6b393
//META: purpose=A precise-qualified value also marks the feeding temporary arithmetic (computePreciseInsts), so both the OpFMul and the OpFAdd carry NoContraction without -fp-mode precise.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `precise` qualifier decorates only the final value in the IR,
// and the emitter's backward walk (computePreciseInsts) extends NoContraction to the
// temporary `x * y` feeding it. No -fp-mode flag is passed, so the decorations can only
// come from the qualifier. CHECK expects two separate NoContraction decorations.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float> outBuf;
StructuredBuffer<float> inBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float x = inBuf[0];
    float y = inBuf[1];
    float z = inBuf[2];
    precise float s = x * y + z;
    outBuf[tid.x] = s;
}

// CHECK: OpDecorate %{{[A-Za-z0-9_]+}} NoContraction
// CHECK: OpDecorate %{{[A-Za-z0-9_]+}} NoContraction
