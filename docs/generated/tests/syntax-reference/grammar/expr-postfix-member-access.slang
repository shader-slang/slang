//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=da8363e39914cfb7152b84e17c34f7e9f5e6ae76aad2de17997f38aecabfc651
//META: purpose=`PostfixSuffix` accepts `'.' IDENT` member access, binding tighter than arithmetic.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `.member` postfix binds at level 1 (tighter than
// arithmetic). `p.a + p.b` parses as `(p.a) + (p.b)`.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair { int a; int b; }

void main()
{
    Pair p;
    p.a = 10;
    p.b = 32;
    int r = p.a + p.b;
    // CHECK: r=42
    printf("r=%d\n", r);
}
