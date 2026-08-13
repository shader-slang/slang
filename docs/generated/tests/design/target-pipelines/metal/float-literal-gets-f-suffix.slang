//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=0c4ba248635633393b51ae889f226e3b5f86a549ad5be4af6f827414819fb357
//META: purpose=A finite `float` literal is emitted with an explicit `f` suffix so MSL does not type the bare decimal as `double`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `emitSimpleValueImpl` appends the float suffix. Without it MSL
// would give a bare decimal `double` type, which changes overload
// resolution and the result type of surrounding arithmetic. The literal is
// multiplied by a dispatch-derived value so it survives to emit as a
// literal operand; CHECK asserts the emitted constant is `3.5f`, not
// `3.5`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float(tid.x) * 3.5f;
}

// CHECK: 3.5f
