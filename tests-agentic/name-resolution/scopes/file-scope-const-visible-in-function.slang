//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
//META: purpose=ModuleDecl/FileDecl members are reachable from any function in the file via the parent-scope walk.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ModuleDecl is listed as scope-bearing. A file-scope const declared
// in the module is reachable from a function body via the parent
// chain (function scope -> file/module scope).

//TEST:INTERPRET(filecheck=CHECK):

static const int kAnswer = 42;

void main()
{
    // CHECK: kAnswer=42
    printf("kAnswer=%d\n", kAnswer);
}
