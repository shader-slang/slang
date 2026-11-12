//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:50:00Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#trycall-and-throw
//META: doc_section_digest=316affe4711df67f83b285c59efaaea11c26c7af447b6bd0e6539bca6fcf0b4b
//META: purpose=A throwing function reaches the first dump as a Result-returning function: throw becomes makeResultError, a normal return becomes makeResultValue, and the caller's try becomes a call plus isResultError / getResultValue, with no throw or tryCall opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// lowerErrorHandling runs before the first -dump-ir snapshot, so the
// error flow is already expressed with the Result opcodes.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute
//TEST:SIMPLE(filecheck=NOOP):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

enum MyError { Fail = 7 }

int risky(int v) throws MyError
{
    if (v < 0)
        throw MyError.Fail;
    return v + 1;
}

void guarded(int v)
{
    do
    {
        buf[0] = try risky(v);
    }
    catch (e: MyError)
    {
        buf[0] = -1;
    }
}

[numthreads(1, 1, 1)]
void main()
{
    guarded(n);
}

// CHECK-LABEL: func %risky
// CHECK: makeResultError(
// CHECK: makeResultValue(
// CHECK-LABEL: func %guarded
// CHECK: call %risky(
// CHECK: isResultError(
// CHECK: getResultValue(

// The opcodes the doc says are consumed by lowerErrorHandling are absent.
// NOOP-NOT: tryCall
// NOOP-NOT: throw(
