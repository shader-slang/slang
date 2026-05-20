//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#pointer-reference-parameter-passing-types
//META: doc_section_digest=9632f1067419e79bbad1711bd742d58d70173e0ac1cfe04bef2332c85dbcb231
//META: purpose=BorrowInParamType / OutParamType / BorrowInOutParamType distinguish parameter-passing modes; only `inout` and `out` make callee mutation visible at the caller.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The three parameter-passing-mode types observably differ at the
// caller: `inout` threads the value through (caller sees updates),
// `out` requires the callee to assign and the caller picks up the
// assigned value, `in` is a local copy that does not propagate.

//TEST:INTERPRET(filecheck=CHECK):

void addOne(inout int x) { x = x + 1; }
void setTo(out int x) { x = 99; }
void readOnly(in int x) { /* x is a local copy */ }

void main()
{
    int a = 5;
    addOne(a);
    // CHECK: a=6
    printf("a=%d\n", a);

    int b;
    setTo(b);
    // CHECK: b=99
    printf("b=%d\n", b);

    int c = 7;
    readOnly(c);
    // CHECK: c=7
    printf("c=%d\n", c);
}
