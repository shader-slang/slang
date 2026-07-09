//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=5e6e6830e19c77586abe7afcb70af45ad2c1dc56341e670eb4abae10eeb65319
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
