//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#variable-binding-declarations
//META: doc_section_digest=b0a9ef0c603cd408e940f93b864d2b441f72e4bf4f04f967c2f2f359024af70e
//META: purpose=`Declarator` folds a trailing `ArraySuffix` onto a `typedef` alias type, so `typedef int Pair[2];` parses.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the shared Declarator machinery in TypedefDecl: the alias
// name carries a trailing `[2]` suffix that must fold onto the alias
// type. A variable of the alias type is indexed like an int[2], which
// only works if the suffix became part of the aliased type.

//TEST:INTERPRET(filecheck=CHECK):

typedef int Pair[2];

void main()
{
    Pair p;
    p[0] = 40;
    p[1] = 2;
    // CHECK: r=42
    printf("r=%d\n", p[0] + p[1]);
}
