//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-general-path
//META: doc_section_digest=eea2bfede346e737fa7d2e2da728c31f2fe2d0f31fe7abb55e6567711bbf3361
//META: purpose=visitIndexExpr looks __subscript up on the indexed value's type and passes that value to createLookupResultExpr as its explicit base, so a[i] dispatches to the type's getter.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `visitIndexExpr` resolves `__subscript` on the indexed value's type and passes that value to
// `createLookupResultExpr` as the explicit base. The getter can therefore read the receiver's
// field. The result combines the stored base with the index, confirming that the getter used the
// indexed `Grid` value.

//TEST:INTERPRET(filecheck=CHECK):

struct Grid
{
    int base;
    __subscript(int i) -> int { get { return base + i; } }
}

void main()
{
    Grid g;
    g.base = 100;
    int tag = g[7];
    // CHECK: tag=107
    printf("tag=%d\n", tag);
}
