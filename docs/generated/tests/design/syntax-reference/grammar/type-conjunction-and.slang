//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#types
//META: doc_section_digest=29130252ebb8566d4abf275df716b28c339ea28ecaa9f518d2ddee6bfdd47125
//META: purpose=`CoreType` accepts the infix conjunction form `CoreType '&' CoreType`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `&` in type position parses as the conjunction type
// rather than a bitwise operator or a reference marker. The constraint
// `T : IA & IB` is used as the bound of a generic parameter, and the
// body calls a method from each conjunct; both calls only resolve if
// the single constraint carried both interfaces, so the combined value
// 34 (a()==3, b()==4) is the evidence.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int a(); }
interface IB { int b(); }

struct S : IA, IB
{
    int a() { return 3; }
    int b() { return 4; }
}

int both<T : IA & IB>(T v) { return v.a() * 10 + v.b(); }

void main()
{
    S s;
    // CHECK: r=34
    printf("r=%d\n", both<S>(s));
}
