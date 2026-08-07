//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#operator-overloading
//META: doc_section_digest=6c67481fccb6a5d96551c93af7509ffafa480d11451126832bec4a0352c4b938
//META: purpose=Operator candidate sets are keyed by operand basic-type pair; two distinct user-defined operator+ overloads for different struct types each fire on their respective operand types without ambiguity.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "A ResolvedOperatorOverload ... cache memoizes the result, keyed
// by OperatorOverloadCacheKey (which captures the operator name
// and the operand BasicType pair)". Two struct types each define
// their own operator+; each call dispatches without ambiguity.

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

    Time  t1; t1.seconds = 9;
    Time  t2; t2.seconds = 3;
    Time  tSum = t1 + t2;

    // CHECK: m=150 t=12
    printf("m=%d t=%d\n", mSum.cents, tSum.seconds);
}
