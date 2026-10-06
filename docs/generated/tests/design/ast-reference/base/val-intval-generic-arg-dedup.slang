//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#val-nodebase
//META: doc_section_digest=17754b4ad55bb3bbeb3b934dd617e4a3120a865a281a81848bf3fc5ab03948d7
//META: purpose=The IntVal family are Vals too, so two spellings of the same compile-time integer produce one deduplicated generic argument and therefore one specialized type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that compile-time integers participate in the same value
// deduplication as types. `Fixed<2 + 2>` and `Fixed<4>` must denote a
// single specialization: the test assigns one to the other (only legal
// if the two types are identical) and passes both to an overload set
// that separates `Fixed<4>` from `Fixed<5>`. CHECK asserts both calls
// pick the `Fixed<4>` overload.

//TEST:INTERPRET(filecheck=CHECK):

struct Fixed<let N : int> { int n() { return N; } };

int which(Fixed<4> f) { return 40; }
int which(Fixed<5> f) { return 50; }

void main()
{
    Fixed<2 + 2> folded;
    Fixed<4>     literal;
    folded = literal;
    //CHECK: folded=40
    printf("folded=%d\n", which(folded));
    //CHECK: literal=40
    printf("literal=%d\n", which(literal));
    //CHECK: n=4
    printf("n=%d\n", folded.n());
}
