//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:00Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#downstream-apple-metal-compiler
//META: doc_section_digest=47dc1979b06f0478ae36fb28d851c48cf8597970c427e3df4913d3436b7c39a0
//META: purpose=`-target metallib-asm` first compiles the emitted MSL to an intermediate `.metallib` via Apple's `metal` compiler (MetalC) and then disassembles it with `metal-objdump --disassemble`; the resulting disassembly carries the renamed entry point `main_0`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: requires-tool=metal-toolchain
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two-step MetalLibAssembly downstream chain: the request
// routes through an intermediate MetalLib (Apple metal compile) and the
// final artifact is metal-objdump disassembly text. The CHECK asserts
// the disassembled output still names the Slang-renamed kernel main_0,
// which proves the compile-then-disassemble chain ran end to end.
// Requires the Apple Metal toolchain; skipped on runners without it.

//TEST:SIMPLE(filecheck=CHECK):-target metallib-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) * 2;
}

// CHECK: main_0
