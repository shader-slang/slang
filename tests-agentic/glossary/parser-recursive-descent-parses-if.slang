//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=parser: the parser consumes tokens and produces an AST that the checker / lowerer can execute; a nested `if`/`else` statement parses into a control-flow tree that selects the matching branch at run time.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "parser" glossary entry: nested `if`/`else if`/`else`
// parses into a working selection structure. We exercise each
// branch by varying the static condition's value.

//TEST:INTERPRET(filecheck=CHECK):

int classify(int n)
{
    if (n < 0) return -1;
    else if (n == 0) return 0;
    else return 1;
}

void main()
{
    // CHECK: -1,0,1
    printf("%d,%d,%d\n", classify(-5), classify(0), classify(5));
}
