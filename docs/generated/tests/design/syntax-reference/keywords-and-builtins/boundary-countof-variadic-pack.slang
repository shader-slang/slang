//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=13c9d348ad0da645a7829d3d113c15ae4df72fa2a418b0b196c8b655d8ea1d92
//META: purpose=Boundary: `countof` is documented in the expression-keywords group with `sizeof`/`alignof`; for a variadic value pack of three elements, `countof(D)` is 3.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Exercises the third member of the `sizeof`/`alignof`/`countof`
// triad. `countof` is most cleanly observable on a variadic value
// pack — array-typed `countof` is not constant-folded in the
// interpreter for plain locals, so the pack form is the reliable
// probe of the keyword spelling.

//TEST:INTERPRET(filecheck=CHECK):

struct Dims<let each D : int>
{
    static const int dimCount = countof(D);
}

void main()
{
    // CHECK: c=3
    printf("c=%d\n", Dims<10, 20, 30>.dimCount);
}
