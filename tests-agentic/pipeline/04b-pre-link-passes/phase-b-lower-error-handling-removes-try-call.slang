//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:55+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/pipeline/04b-pre-link-passes.md#lowererrorhandling
//META: doc_section_digest=7e68ed1de0a3d5bc28a1de54c7a0a98fe90fa1fb9db55a8f84e2ce2db2170dc4
//META: purpose=Phase B's lowerErrorHandling (B2) rewrites throwing functions to return Result<T,E>; after Phase B no tryCall opcode survives in the IR.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `throws` function plus a `try` call become an ordinary call
// followed by an `ifElse` on the result tag. The `tryCall` opcode
// is gone from the LOWER-TO-IR dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

enum MyError { Fail }

uniform int a;
uniform RWStructuredBuffer<int> buf;

int doIt(int x) throws MyError
{
    if (x < 0) throw MyError.Fail;
    return x + 1;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    do { buf[0] = try doIt(a); } catch (MyError e) { buf[0] = -1; }
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-NOT: = tryCall(
