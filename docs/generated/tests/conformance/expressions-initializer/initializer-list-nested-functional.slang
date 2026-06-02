//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:11+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-initializer.md#initializer-list-expression
//META: doc_section_digest=68be737ab4b8adb6374f77514e63e2dd70ca264c5b4d8f7ff4cffa68f75e41dc
//META: purpose=Nested initializer lists ({ {a,b}, {c,d} }) initialize nested struct fields in declaration order.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// C4 combination: the doc says "zero or more expressions" — those expressions
// may themselves be initializer lists (nested structs).
// Verify that nested {} braces assign inner fields correctly.

//TEST:INTERPRET(filecheck=CHECK):

struct Inner { int a; int b; };
struct Outer { Inner first; Inner second; };

int passInt(int v) { return v; }

void main()
{
    Outer o = { { passInt(1), passInt(2) }, { passInt(3), passInt(4) } };
    //CHECK: 1 2 3 4
    printf("%d %d %d %d\n", o.first.a, o.first.b, o.second.a, o.second.b);
}
