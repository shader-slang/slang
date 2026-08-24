//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#loweredvalinfo-and-the-lowering-environment
//META: doc_section_digest=9127ecfc310ffdc4aecbad1b96d176cf66095ac4dcbdc84a1ac4106e6bf98dd2
//META: purpose=An implicit conversion on an `inout` argument stays symbolic as the `ImplicitCastedLValue` flavor and materializes at the call site as a single `inOutImplicitCast` on the base pointer, not as a read-cast / write-cast pair around a temporary.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the l-value flavor produced when an argument's declared type does
// not match the parameter's. Passing a `uint` local to an `inout int` parameter
// cannot be a plain address hand-off, because the callee will write back an
// `int`. Lowering keeps the conversion attached to the l-value and emits one
// `inOutImplicitCast(%x)` whose result is the `Ptr(Int)` the call consumes; the
// CHECK pins that inst plus the `call` that takes it, which together show the
// conversion happening on the pointer rather than on a copied-in value. The
// local is seeded from a `uniform` and read back into the output buffer so
// neither the variable nor the call can be eliminated.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<uint> outputBuffer;
uniform uint seed;

void bump(inout int v) { v = v + 1; }

[numthreads(1, 1, 1)]
void main()
{
    uint x = seed;
    bump(x);
    outputBuffer[0] = x;
}

// CHECK-LABEL: func %main
// CHECK: inOutImplicitCast(%x{{[0-9]*}})
// CHECK: call %bump{{[0-9]*}}(
