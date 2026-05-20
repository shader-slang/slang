//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission
//META: doc_section_digest=e769492b0e2a17642d0c2176c69b9635c5d888e1e9fd7ee6420efd9df5b09e62
//META: purpose=Phase A row A6 calls lowerFrontEndEntryPointToIR once per entry point so the entry-point IRFunc skeleton exists before the full declaration walk.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Phase A iterates `for entryPoint in translationUnit.entryPoints`
// (row A6). Compiling with `-entry main` produces the `main` IRFunc.
// A second `[shader("compute")]` entry point in the same module
// also gets lowered (so the declaration walk in A7 can reference
// it); both `func %main` and `func %other` appear in LOWER-TO-IR
// (the other entry stays alive because it carries `[entryPoint(...)]`).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void other(uint3 tid : SV_DispatchThreadID)
{
    buf[1] = int(tid.x);
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK: func %other
// CHECK: func %main
