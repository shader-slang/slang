//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=950d7a7efcea5da748ec49fa6499555097f3093abb07ac97945a51be1ce2339c
//META: purpose=`int`, `uint`, `float`, and `bool` are core-module-supplied type names; overload resolution against typed parameters distinguishes all four.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Declare four overloads of `kind`, each accepting a different
// built-in scalar type, and call each with a literal that should
// match exactly one. If any built-in name were missing from the
// environment, the corresponding overload would not even compile.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int v)   { return 1; }
int kind(uint v)  { return 2; }
int kind(float v) { return 3; }
int kind(bool v)  { return 4; }

void main()
{
    int i = 1;
    uint u = 1u;
    float f = 1.0f;
    bool b = true;
    // CHECK: i=1 u=2 f=3 b=4
    printf("i=%d u=%d f=%d b=%d\n", kind(i), kind(u), kind(f), kind(b));
}
