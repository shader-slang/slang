//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/02-parse-ast.md#two-stage-parsing
//META: doc_section_digest=533454eadd55380449930cf39d9145725c95943e6be5773e59633f3f506b93cf
//META: purpose=Because function bodies are deferred to a second parse pass that runs after decl-stage has populated the outer scope, a body can forward-reference an outer declaration that appears later in the source file.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `main` calls `later`, but `later` is declared after `main`. This is
// only possible if the body of `main` is parsed after both top-level
// decls have been added to the outer scope — exactly the contract the
// `## Two-stage parsing` section names.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = later(6);
    // CHECK: v=12
    printf("v=%d\n", v);
}

int later(int n) { return n + n; }
