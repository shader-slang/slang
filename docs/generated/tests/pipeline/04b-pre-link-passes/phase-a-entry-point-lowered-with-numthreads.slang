//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission
//META: doc_section_digest=e769492b0e2a17642d0c2176c69b9635c5d888e1e9fd7ee6420efd9df5b09e62
//META: purpose=Phase A's lowerFrontEndEntryPointToIR (A6) produces an IRFunc with [entryPoint(...)] and [numThreads(...)] decorations visible in the LOWER-TO-IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Phase A row A6 of the doc table establishes the entry-point
// `IRFunc` before the full declaration walk. The compute entry
// point appears in the `### LOWER-TO-IR:` block carrying both
// `entryPoint` and `numThreads` decorations.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(4, 2, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK: entryPoint
// CHECK: numThreads(4 : Int, 2 : Int, 1 : Int)
// CHECK: func %main
