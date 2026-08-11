//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=a505032fe1ced813d290018b087d7ba67b9a672a9b25244d2cf6a5fa887fe795
//META: purpose=`FuncDecl` in `func` form takes its result type via `('->' Type)?` after the parameter list.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the arrow result clause of the modern `func` declaration
// form: `func add(...) -> int`. The returned value is printed, proving
// the header (name-first params plus `->` result type) parsed as one
// FuncDecl.

//TEST:INTERPRET(filecheck=CHECK):

func add(a : int, b : int) -> int
{
    return a + b;
}

void main()
{
    // CHECK: r=42
    printf("r=%d\n", add(40, 2));
}
