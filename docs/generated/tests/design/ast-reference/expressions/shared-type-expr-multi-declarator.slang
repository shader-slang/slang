//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Verifies that one type spelling shared by several declarators in `int a = 1, b = 2, c = 3;` types every declarator identically.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A declarator list writes the type once and reuses that single type
// expression for each name. All three locals are initialised and read back
// separately; the CHECKs fail if any declarator ended up untyped or bound to a
// different type than the leading `int`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 1, b = 2, c = 3;
    // CHECK: a=1 b=2 c=3
    printf("a=%d b=%d c=%d\n", a, b, c);
}
