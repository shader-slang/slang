//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members
//META: doc_section_digest=5af69869d1bb41b9ec6da86b628cac4884b654398df91d571e67dbe22a034ace
//META: purpose=An `AccessorDecl` inherits visibility from its enclosing subscript/property; a private subscript's accessors are private and rejected from outside the struct.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `getDeclVisibility` returns the enclosing decl's visibility for
// `AccessorDecl`. A `private __subscript` therefore makes the get
// accessor private, and `s[0]` from a free function is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S
{
    int value;
    __init(int v) { value = v; }

    private __subscript(int i)->int
    {
        get { return value + i; }
    }
}

void test(S s)
{
    int x = s[0];
/*CHECK:
             ^ declaration not accessible
             ^ 'operator[]' is not accessible from the current context.
*/
// CHECK: E30600
}
