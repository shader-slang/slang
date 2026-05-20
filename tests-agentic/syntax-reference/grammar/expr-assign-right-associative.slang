//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#expressions
//META: doc_section_digest=da8363e39914cfb7152b84e17c34f7e9f5e6ae76aad2de17997f38aecabfc651
//META: purpose=Assignment `=` (level 14) is right-associative; `a = b = 5` parses as `a = (b = 5)`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies right-associativity of assignment. `a = b = 5` is parsed as
// `a = (b = 5)` so both variables end up at 5. Left associativity would
// be a parse error (`(a = b) = 5` writes to a non-lvalue).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 0, b = 0;
    a = b = 5;
    // CHECK: a=5,b=5
    printf("a=%d,b=%d\n", a, b);
}
