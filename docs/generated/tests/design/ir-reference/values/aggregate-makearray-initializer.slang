//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=38e604e256ff13306b001bc1378ee072317e37c7d88f8bc3fdc3ec5f43311030
//META: purpose=An `InitializerListExpr` assigned to a fixed-size array lvalue lowers to a single `makeArray(...)` opcode carrying one operand per element, which is then stored through the array's address.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `makeArray` is a real value-producing opcode and not an
// element-by-element store loop. Assigning the initializer list to a
// struct field (rather than declaring a fresh local) keeps the array
// value on the value side of the assignment, so the aggregate must be
// built before it can be stored. All three elements derive from a
// `uniform int` so none folds. CHECK asserts the `get_field_addr` for
// the field, the three-operand `makeArray` typed `Array(Int, 3 : Int)`,
// and the `store` that consumes it in that order.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Holder
{
    int arr[3];
}

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Holder h;
    h.arr = { a, a + 1, a + 2 };
    buf[0] = h.arr[2];
}

// IR-LABEL: func %main
// IR: : Ptr(Array(Int, 3 : Int))	= get_field_addr(%h, %arr)
// IR: : Array(Int, 3 : Int)	= makeArray(%a, %{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}})
// IR: store(
