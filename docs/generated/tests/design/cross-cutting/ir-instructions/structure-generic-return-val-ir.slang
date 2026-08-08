//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=d6d21d15e8b14ba2b57495447b3188f156d4c4168b8574097962151b591b1497
//META: purpose=A `generic` body ends in `return_val` returning the specialized value, not in a `yield` terminator.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `generic`/`IRGeneric` row's explicit correction that a
// generic's body ends in `return_val`, not `yield` (`yield` closes an
// `expand` body instead). A generic function constrained on an interface is
// declared and called; the CHECKs walk the dump from the `generic` header
// into the nested `func`, then assert the generic block's terminator is
// `return_val(%applyTwice)` — returning the specialized function value. The
// `IR-NOT: yield(` line asserts no `yield` terminator is produced anywhere
// for this program, which is the half of the claim that a positive CHECK
// alone cannot establish.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IAdder { int addTo(int v); }
struct Plus3 : IAdder { int addTo(int v) { return v + 3; } }

int applyTwice<T : IAdder>(T t, int v)
{
    return t.addTo(t.addTo(v));
}

uniform int a;
uniform RWStructuredBuffer<int> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Plus3 p;
    outbuf[0] = applyTwice<Plus3>(p, a);
}

// The `IR-NOT` lines are interleaved so every region of the dump — before
// the first generic, between the landmarks, and after the last one — is
// covered by the "no `yield` terminator" half of the claim.
// IR-NOT: yield(
// IR: generic %{{.*}}
// IR-NOT: yield(
// IR: func %applyTwice
// IR-NOT: yield(
// IR: return_val(%applyTwice)
// IR-NOT: yield(
