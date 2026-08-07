//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=a505032fe1ced813d290018b087d7ba67b9a672a9b25244d2cf6a5fa887fe795
//META: purpose=`Param` accepts the modern name-first form `IDENT ':' Type ('=' Expr)?` with a default-value clause.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the modern (name-first) parameter production including its
// default-value clause: `factor : int = 2`. Calling with and without
// the second argument yields different products, proving the default
// clause parsed and applies only when the argument is omitted.

//TEST:INTERPRET(filecheck=CHECK):

func scale(x : int, factor : int = 2) -> int
{
    return x * factor;
}

void main()
{
    // CHECK: a=42
    printf("a=%d\n", scale(21));
    // CHECK: b=63
    printf("b=%d\n", scale(21, 3));
}
