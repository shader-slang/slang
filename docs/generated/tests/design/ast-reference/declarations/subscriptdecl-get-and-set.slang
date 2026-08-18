//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A SubscriptDecl with both get and set accessors makes an indexed expression assignable as well as readable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a subscript declaring both accessors supports index syntax on
// either side of an assignment. Two elements are written through the setter
// and read back through the getter; the CHECK asserts their sum, which
// requires both accessors to be wired to the same backing storage.

//TEST:INTERPRET(filecheck=CHECK):

struct Cells
{
    int a[3];
    __subscript(int i) -> int
    {
        get { return a[i]; }
        set { a[i] = newValue; }
    }
}

void main()
{
    Cells c;
    c[0] = 5;
    c[1] = 6;
    //CHECK: 11
    printf("%d\n", c[0] + c[1]);
}
