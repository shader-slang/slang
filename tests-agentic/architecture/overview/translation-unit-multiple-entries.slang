//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/architecture/overview.md#compilation-request-lifecycle
//META: doc_section_digest=4dae689ab71645e4934f3d5e29afb21da2d6bcdf308ea84e96e25981a6c6dd7b
//META: purpose=A `TranslationUnitRequest` is "a collection of source files that share a namespace"; multiple entry points in one Slang translation unit each pick up their own emit when `-entry` selects them.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that two entry points declared in the same translation
// unit can both compile, and that they share access to file-scope
// declarations (here the helper function `helper` and the resource
// `outBuf`). Two distinct compilations select two distinct entries
// from the same source file.

//TEST:SIMPLE(filecheck=ENTRY_A):-target hlsl -entry entryA -stage compute
//TEST:SIMPLE(filecheck=ENTRY_B):-target hlsl -entry entryB -stage compute

RWStructuredBuffer<int> outBuf;

int helper(int x) { return x + int('!'); }

void entryA()
{
    //ENTRY_A: void entryA
    outBuf[0] = helper(int('A'));
}

void entryB()
{
    //ENTRY_B: void entryB
    outBuf[0] = helper(int('B'));
}
