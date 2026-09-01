//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#debug-levels-and--debug-info-include-source
//META: doc_section_digest=a465c2bb21f120c8c3458404a567b476edc5f751bce03ff4c08b17bf8f2b2163
//META: purpose=At -g2 the emitter switches to the NonSemantic.Shader.DebugInfo.100 extended instruction set and records the entry point's command line.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Standard (-g2) rung: the module additionally carries local
// variable debug info, so the emitter imports the NonSemantic debug-info
// extended instruction set instead of stopping at OpLine. CHECK asserts the
// extended-instruction import, the producer string, and the recorded command
// line, whose stage and entry-point arguments are derived per entry point from
// its own entry-point decoration. Only the stable leading and trailing
// fragments of that command line are matched.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -g2

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int local = int(tid.x) * 2;
    outBuf[tid.x] = local;
}

// CHECK-DAG: OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
// CHECK-DAG: OpString "slangc"
// CHECK-DAG: OpString "-target spirv{{.*}}-stage compute -entry main"
