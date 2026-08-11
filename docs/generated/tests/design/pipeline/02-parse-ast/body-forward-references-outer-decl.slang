//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#two-stage-parsing
//META: doc_section_digest=21478edfded8ded74c2720ad076754bbce4b66844ce82b5c2e2eec2008cb3819
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
