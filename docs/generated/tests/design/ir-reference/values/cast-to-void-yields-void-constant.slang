//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#live-but-unproduced-opcodes
//META: doc_section_digest=f400c3e3d85c3f3a43b09285b54b24ce8a795e0f2ad77f193abf0d754c940045
//META: purpose=`castToVoid` is declared but never constructed: a `(void)expr` cast is intercepted and yields the unique `void_constant` value instead, and the operand's side effects survive as their own instructions.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that discarding a value with `(void)expr` produces no
// data-carrying cast instruction. The discarded expression is a call to
// a helper that also writes a buffer element, so its side effect must
// still be emitted; CHECK asserts the `call %sideEffect` is present, the
// `void`-returning function still terminates on `return_val(void_constant)`,
// and `castToVoid` appears nowhere. The absent opcode plus the surviving
// call together show the interception preserves semantics without
// introducing a second spelling of the `void` value.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int sideEffect(int v)
{
    buf[1] = v;
    return v;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a;
    (void)sideEffect(a);
}

// IR-LABEL: func %main
// IR: call %sideEffect(%a)
// IR: return_val(void_constant)
// IR-NOT: castToVoid
