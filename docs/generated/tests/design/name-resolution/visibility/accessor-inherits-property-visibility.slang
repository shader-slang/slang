//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members
//META: doc_section_digest=ec08120339d3d14caf888d6a0bddd84667a5c4a5996a43c90cd4d9dd60c37807
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
