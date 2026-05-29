//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#decorations
//META: doc_section_digest=1b5a75ff3886dcb59a3adffbb216b97498738abd9bb7db936eee68e3ef99f8e8
//META: purpose=A function marked as a compute entry point carries an `entryPoint` decoration in the IR and emits a target-specific entry-point declaration.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `[shader("compute")]` function (the entry-point selected by
// `-entry main`) appears in the IR dump with an `entryPoint(...)`
// decoration just above the `func %main` line — the doc's
// "Marks a function as a pipeline entry point" claim. On SPIR-V the
// same decoration lowers to `OpEntryPoint GLCompute %main "main"`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = 1;
}

// IR: [entryPoint({{.*}}"main"{{.*}})]
// IR-NEXT: [numThreads(1 : Int, 1 : Int, 1 : Int)]

// SPV: OpEntryPoint GLCompute %main "main"
