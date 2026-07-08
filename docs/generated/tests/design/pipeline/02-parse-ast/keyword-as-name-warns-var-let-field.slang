//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=cc891d0f95a64fa8f584e7573e2e02ccc1d048aeae30b2a876b01210f6bd45a3
//META: purpose=Declaring a variable, let-binding, or struct field whose name is a reserved type keyword parses but warns with W20103.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the keyword-used-as-name warning fires from the shared
// declarator path for three of the declaration shapes the doc lists:
// a global var named `enum`, a let-binding named `class`, and a
// struct field named `typealias`. Each site pins W20103, and the
// warning text names the offending keyword.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static int enum = 0;
//CHECK: W20103
//CHECK: 'enum' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts

struct Holder
{
    int typealias;
//CHECK: W20103
//CHECK: 'typealias' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts
}

void main()
{
    let class = 3;
//CHECK: W20103
//CHECK: 'class' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts
}
