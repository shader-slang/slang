//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#emit
//META: doc_section_digest=244236b868f771e25853f0063ebf0ce241c2c1af2fde156ad5b70a5ed4f242f6
//META: purpose=Non-source targets are dispatched away from the C-like source emitters, so the same front end plus IR passes also feed the VM bytecode path that the interpreter executes.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that emit has at least one dispatch path that is not a C-like
// source emitter: the interpreter consumes VM bytecode, not target text.
// The program is deliberately non-trivial for the shared front end -- a
// struct decl, a by-value struct argument, and a callee -- so reaching the
// printed value requires the same lex/parse/check/lower/IR-pass chain the
// text targets use, terminating in a different back end. CHECK asserts the
// executed result, which no amount of successful source emission could
// produce on its own.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair
{
    int a;
    int b;
};

int combine(Pair p)
{
    return p.a * 10 + p.b;
}

void main()
{
    Pair p;
    p.a = 4;
    p.b = 2;
    printf("vm=%d\n", combine(p));
}

// CHECK: vm=42
