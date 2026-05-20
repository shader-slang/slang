//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=entry point: a function selected by `-entry NAME -stage STAGE` runs as the kernel; the byte-code interpreter dispatches `main` and produces its observable output.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "entry point" glossary entry: an entry point is a
// function plus a pipeline stage that defines a kernel to compile.
// `slangi` runs the entry point `main` and we observe its printed
// output.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: entry-ok
    printf("entry-ok\n");
}
