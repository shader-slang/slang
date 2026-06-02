//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#type-aliases
//META: doc_section_digest=f22a5a3e9b185affa7aa79dc81418911e02b79b1e1eaf02eebfdcef5d9b84e6a
//META: purpose=The traditional `typedef` syntax also declares a type alias equivalent to the named type.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "Type aliases can also be declared with traditional C-style
// syntax: `typedef int Height;`"

//TEST:INTERPRET(filecheck=CHECK):

typedef int Height;

void main()
{
    Height h = 200;
    int raw = h;
    // CHECK: 200
    printf("%d\n", raw);
}
