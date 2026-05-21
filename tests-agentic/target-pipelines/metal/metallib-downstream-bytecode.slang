//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#downstream-apple-metal-compiler
//META: doc_section_digest=a18030895beb62382852be4a9918954cf153c6a439ce82644504b2f07adb8d72
//META: purpose=`-target metallib` invokes Apple's `metal` command-line compiler on the emitted MSL: the downstream Metal toolchain converts the Slang-generated source into a `.metallib` artifact whose disassembly carries the entry-point definition (`define void @main_0`).
//META: intent=expansion
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `-target metallib` triggers the downstream Apple `metal` compiler
// (PassThroughMode::MetalC). The resulting `.metallib` bytecode is
// LLVM-bitcode-flavored; the disassembled form carries an
// LLVM-style entry-point definition for the Slang-renamed kernel
// `main_0`. Apple's toolchain is required on the runner — the
// no-GPU agent runtime skips this case.

//TEST:SIMPLE(filecheck=CHECK):-target metallib -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) + 1;
}

// CHECK: define
// CHECK-SAME: main_0
