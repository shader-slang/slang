//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=BorrowInParamType / OutParamType / BorrowInOutParamType distinguish parameter-passing modes; only inout and out make callee mutation visible at the caller.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the three parameter-passing-mode types differ observably
// at the caller: inout threads a value through (caller sees updates),
// out has the callee assign a fresh value the caller picks up, in is
// a local copy that does not propagate. CHECK asserts each caller-side
// value after the call.

//TEST:INTERPRET(filecheck=CHECK):

void addOne(inout int x) { x = x + 1; }
void setTo(out int x) { x = 99; }
void readOnly(in int x) { x = x + 1000; /* local copy */ }

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
