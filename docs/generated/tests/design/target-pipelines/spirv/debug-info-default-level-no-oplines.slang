//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#debug-levels-and--debug-info-include-source
//META: doc_section_digest=a465c2bb21f120c8c3458404a567b476edc5f751bce03ff4c08b17bf8f2b2163
//META: purpose=At the default debug level the module carries only a bare OpSource Slang 1 and no OpLine instructions.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the -g0 end of the debug-level ladder: with no -g option the IR
// carries no debug instructions, so the emitter can only produce the bare
// language/version source record. CHECK asserts the bare OpSource line is
// present, and CHECK-NOT asserts neither OpLine nor an OpString file record
// was emitted, which is what distinguishes this level from -g1.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = int(tid.x) * 3;
    outBuf[tid.x] = v;
}

// CHECK-NOT: OpString
// CHECK-NOT: OpLine
// CHECK: OpSource Slang 1
