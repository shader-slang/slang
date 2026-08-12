//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
//META: purpose=A uint+uint addition near the uint MAX boundary emits an OpIAdd on %uint and a uint MAX OpConstant; the wrap is left to the runtime (IEEE/two's-complement modular arithmetic on 32-bit unsigned).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: probes that an arithmetic op at the uint MAX edge
// emits as OpIAdd %uint. Slang does not pre-fold runtime
// operands, so the constant 0xFFFFFFFFu survives and the add
// is emitted as a real SPIR-V instruction.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint x = tid.x;
    outBuf[tid.x] = x + 0xFFFFFFFFu;
}

// CHECK-DAG: %uint_4294967295 = OpConstant %uint 4294967295
// CHECK: OpIAdd %uint
