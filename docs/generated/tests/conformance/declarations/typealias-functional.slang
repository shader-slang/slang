//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#type-aliases
//META: doc_section_digest=f22a5a3e9b185affa7aa79dc81418911e02b79b1e1eaf02eebfdcef5d9b84e6a
//META: purpose=A `typealias` declaration defines a name that is equivalent to the type on the right of `=`; the alias and the original type are interchangeable.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "A type alias defines a name that will be equivalent to the
// type to the right of `=`."

//TEST:INTERPRET(filecheck=CHECK):

typealias Height = int;
typealias Width = float;

void main()
{
    Height h = 180;
    Width w = 75.5;
    // CHECK: 180
    // CHECK: 75.5
    printf("%d\n%g\n", h, w);
}
