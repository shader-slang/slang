//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=1fb3516e49ddbb5d4259751275e6b594d9d69aa0b1266c092e549ec5de2b449a
//META: purpose=A second lookahead for `let` after `if` routes the statement to the `if let` binding form, which enters its body only when the tested `Optional` holds a value and binds the unwrapped payload inside it.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `if` followed by `let` parses as the binding form rather
// than an ordinary conditional. Two `Optional<int>` values are tested:
// the present one must enter its body with the payload bound to `v`,
// and the `none` one must skip its body entirely. CHECK asserts the
// accumulator ends at exactly the present payload, which is only true
// if the binding form both unwrapped the value and suppressed the
// empty arm.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<int> present = 42;
    Optional<int> empty = none;

    int r = 0;
    if (let v = present) { r = r + v; }
    if (let w = empty) { r = r + 1000; }

    // CHECK: r=42
    printf("r=%d\n", r);
}
