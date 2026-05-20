//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#breadcrumbs
//META: doc_section_digest=d65add87a553b75b168020a969cb739b5014e573c8568f2ebfe82d91d3709072
//META: purpose=When an unqualified name inside a method resolves to a field of the enclosing struct, lookup records a `This` breadcrumb so the synthesized expression reads `this.field`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's breadcrumb section: "For an unqualified `g` defined on
// `Self` inside a method, the breadcrumb is just `This`, marking
// that the rewritten expression needs an implicit `this.g`." Verify
// by reading a field by unqualified name in the method body.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    int g;
    int read() { return g; }   // unqualified `g` rewritten to this.g
}

void main()
{
    S s;
    s.g = 33;
    // CHECK: r=33
    printf("r=%d\n", s.read());
}
