//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#variable--binding-declarations
//META: doc_section_digest=34ae2014df6f94e86e6f1c0103e2b1bc5da580d48dd7ba67686044b7b740ee79
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
