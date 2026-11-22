//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission
//META: doc_section_digest=94881c573723d5b98cfbc42adedec7f412824c3ab543d9cbbddd55d495627a3d
//META: purpose=Phase A's lowerFrontEndEntryPointToIR (A6) produces an IRFunc carrying the entryPoint and numThreads decorations in the LOWER-TO-IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the per-entry-point lowering step of Phase A leaves an
// `IRFunc` behind that still carries the stage/thread-group metadata the
// front end attached. The CHECKs read the post-Phase-D IR snapshot for the
// `entryPoint` and `numThreads` decorations sitting on `func %main`; that
// pair can only be there if the entry point was lowered as a function
// rather than, say, kept as an AST-only decl.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

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
