//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#modifiers
//META: doc_section_digest=62c5ab5b3369626ddfff4ef9a04078345deb0feb7a2dca8f821cfbac3ba44d82
//META: purpose=`ModifierKeyword` includes `inout`, which attaches to a `Param`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an `inout` modifier on a parameter parses and produces
// a pass-by-reference binding observable at the call site.

//TEST:INTERPRET(filecheck=CHECK):

void inc(inout int x) { x = x + 1; }

void main()
{
    int v = 10;
    inc(v);
    // CHECK: v=11
    printf("v=%d\n", v);
}
