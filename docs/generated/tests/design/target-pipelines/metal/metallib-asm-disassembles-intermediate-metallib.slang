//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#downstream-apple-metal-compiler
//META: doc_section_digest=65e900d0cbf2c6b34b856edc81e93bce49e3fc22fad6946b0d7bc5d0b4974910
//META: purpose=`-target metallib-asm` produces an intermediate `.metallib` with Apple's `metal` compiler and then disassembles it with `metal-objdump --disassemble`.
//META: intent=expansion
//META: pipeline_stage=runtime
//META: requires-tool=metal-toolchain
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two-step `MetalLib` -> `MetalLibAssembly` chain: the
// request cannot be served directly, so Slang compiles to a `.metallib`
// first and disassembles that artifact. CHECK asserts the disassembly text
// still names the Slang-renamed kernel, which is only reachable if both
// downstream steps ran. Requires the Apple toolchain; ignored elsewhere.

//TEST:SIMPLE(filecheck=CHECK):-target metallib-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) * 2;
}

// CHECK: main_0
