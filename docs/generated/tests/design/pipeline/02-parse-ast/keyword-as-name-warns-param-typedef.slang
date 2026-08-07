//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=cc891d0f95a64fa8f584e7573e2e02ccc1d048aeae30b2a876b01210f6bd45a3
//META: purpose=Declaring a function parameter or a typedef alias whose name is a reserved type keyword parses but warns with E20103 (warning severity).
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the remaining two declaration shapes the doc says the
// shared declarator path covers uniformly: a parameter named `struct`
// and a typedef alias named `class` each draw the keyword-used-as-name
// warning. Both sites pin the code E20103 at warning severity.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int useParam(int struct)
//CHECK:         ^^^^^^ warning E20103
//CHECK:         ^^^^^^ 'struct' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts
{
    return 0;
}

typedef int class;
//CHECK:    ^^^^^ warning E20103
//CHECK:    ^^^^^ 'class' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts
