//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#memory
//META: doc_section_digest=2a7e11ed01098a86b63060423b4c3849bf9aa3b3c45a6bdc1347a679da04c37c
//META: purpose=Pointer `operator+` lowers to `getOffsetPtr(base, offset)`, which advances a pointer by a count of elements rather than by bytes and is a different opcode from `getElementPtr`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that adding an integer to a pointer is its own opcode and
// that it composes with, rather than replaces, element addressing.
// `&arr[0]` first produces a `getElementPtr`; adding 2 to that pointer
// must then produce `getOffsetPtr`, and dereferencing the result must
// read through the offset pointer. CHECK asserts that exact three-step
// chain — `getElementPtr`, `getOffsetPtr(%p, 2 : Int)`, `load(%q)` — so
// a lowering that folded the offset back into the element index would
// be caught.

//TEST:SIMPLE(filecheck=IR):-target cpp -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int arr[4] = { a, a + 1, a + 2, a + 3 };
    int* p = &arr[0];
    int* q = p + 2;
    buf[0] = *q;
}

// IR-LABEL: func %main
// IR: let  %p	: Ptr(Int{{.*}})	= getElementPtr(%arr, 0 : Int)
// IR: let  %q	: Ptr(Int{{.*}})	= getOffsetPtr(%p, 2 : Int)
// IR: load(%q)
