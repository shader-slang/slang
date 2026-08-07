//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-general-path
//META: doc_section_digest=c62ab6e8891fadd07f2fdee0a7c5e69e5cb4d6cd3f3350ca265514ab39eb4f4a
//META: purpose=Two user-declared operator+ overloads on different struct types are separate candidates in one operator-keyed candidate set and each dispatches on its own operand types.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the operator-keyed lookup gathers every user overload of the
// name and the per-argument type check separates them: neither call is
// ambiguous even though both candidates share the operator name. CHECK asserts
// both results in one line.

//TEST:INTERPRET(filecheck=CHECK):

struct Money { int cents; }
struct Time  { int seconds; }

Money operator+(Money a, Money b) { Money r; r.cents = a.cents + b.cents; return r; }
Time  operator+(Time a, Time b)   { Time r; r.seconds = a.seconds + b.seconds; return r; }

void main()
{
    Money m1; m1.cents = 100;
    Money m2; m2.cents = 50;
    Money mSum = m1 + m2;

    Time t1; t1.seconds = 9;
    Time t2; t2.seconds = 3;
    Time tSum = t1 + t2;

    // CHECK: m=150 t=12
    printf("m=%d t=%d\n", mSum.cents, tSum.seconds);
}
