//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#var-vs-globalvar-vs-alloca
//META: doc_section_digest=545914b38f219893a8c0b8cfa2e347efdf0f6474fc9476df2449d7564150b623
//META: purpose=A local variable becomes a nullary `var` whose result type is `Ptr<T>`, so member and element access chain off it through address opcodes rather than through the value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the pointer-typed result of `var`, which is what makes the
// lvalue chain possible. A local struct holding an array is written
// field-by-element and read back, so the local survives lowering (a
// plain scalar local would be collapsed). CHECK asserts the `var` line
// itself is typed `Ptr(%Holder)`, and that the field and element steps
// hanging off it are the address forms `get_field_addr` /
// `getElementPtr` ending in a `load` — the chain only type-checks if
// `var` produced a pointer.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Holder
{
    int arr[4];
}

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Holder h;
    h.arr[1] = a;
    buf[0] = h.arr[1];
}

// IR-LABEL: func %main
// IR: let  %h	: Ptr(%Holder)	= var
// IR: get_field_addr(%h, %arr)
// IR: getElementPtr(
// IR: load(
