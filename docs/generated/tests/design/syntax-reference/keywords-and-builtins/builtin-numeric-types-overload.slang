//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=61eae4cb09aaf55ca13e1b5adcdbe1bcd8d876e194e57de10e1861ffeb9396c7
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
