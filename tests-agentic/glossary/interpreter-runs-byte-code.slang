//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=interpreter (slangi): the `slangi` byte-code interpreter runs a compiled program end-to-end; its `printf` output is the user-visible evidence that compilation + lowering + bytecode-emit + execution all succeeded.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "interpreter" mention in the glossary: the
// INTERPRET directive routes through `slangi`, which executes the
// program and reports printed output.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 2;
    int b = 3;
    int c = a * b + 1;
    // CHECK: interp=7
    printf("interp=%d\n", c);
}
