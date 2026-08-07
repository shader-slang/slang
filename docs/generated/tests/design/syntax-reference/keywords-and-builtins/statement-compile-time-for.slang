//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=1fb3516e49ddbb5d4259751275e6b594d9d69aa0b1266c092e549ec5de2b449a
//META: purpose=A leading `$` at statement head selects the compile-time statement parser, which then requires `for` and parses the compile-time loop form instead of the ordinary counted loop.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `$`-prefixed spelling reaches the compile-time `for`
// parse path rather than the ordinary statement-entry `for`. The loop
// variable is expanded by the parser, so the body runs once per index
// and the accumulated sum 0+1+2+3 is 6. CHECK asserts that sum; the
// same body written without the `$` would not compile, because the
// compile-time form is the only one whose header is
// `(i in Range(N))` rather than `(init; cond; update)`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int s = 0;
    $for(i in Range(4))
    {
        s = s + i;
    }
    // CHECK: s=6
    printf("s=%d\n", s);
}
