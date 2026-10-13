//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=1f82b50c62ed58a098b1fecb706a884ddcf3e4428a8e0efd194a2377ef542645
//META: purpose=Default-initializing a fixed-size array field lowers to `makeArrayFromElement`, a one-operand splat of the element's default value, rather than to a `makeArray` listing every element.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the array-splat opcode of the aggregate-constructor table.
// The catalog row gives default-initialization of an array type as one
// of its origins, so `Holder h = {};` — where `Holder`'s only field is
// `int a[4]` — is the shortest surface that reaches it. CHECK asserts a
// single `makeArrayFromElement(0 : Int)` producing `Array(Int, 4 : Int)`:
// one operand for a four-element result is exactly what distinguishes
// this opcode from `makeArray`, which would carry four. A later element
// write and a runtime-indexed read keep the array alive past DCE.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

struct Holder { int a[4]; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Holder h = {};
    h.a[0] = int(tid.y);
    buf[0] = h.a[int(tid.x)];
}

// IR-LABEL: func %main
// IR: : Array(Int, 4 : Int)	= makeArrayFromElement(0 : Int)
