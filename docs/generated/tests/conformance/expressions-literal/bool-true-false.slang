//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-literal.md#boolean-literal-expressions
//META: doc_section_digest=769b627a9edfb98d3dcd82a049d5720ec282bc1a720cf4d1da09d5420919dd11
//META: purpose=Boolean literal: `true` and `false` are the only spellings; each is a `bool` r-value with the documented truth value.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The reference's `BooleanLiteral` production is exactly
// `'true' | 'false'`. Verify both values parse, are bool-typed
// (via overload resolution), and print the documented value.

//TEST:INTERPRET(filecheck=CHECK):

int probe(bool x) { return x ? 1 : 0; }
int probe(int x) { return -1; }

void main()
{
    int t = probe(true);
    int f = probe(false);
    //CHECK: t=1 f=0
    printf("t=%d f=%d\n", t, f);
}
