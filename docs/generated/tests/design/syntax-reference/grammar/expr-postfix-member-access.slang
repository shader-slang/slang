//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=dc101149ff9b445d781bc38dc82164f3a5d08e36d3931fcf7594b780eca06062
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
