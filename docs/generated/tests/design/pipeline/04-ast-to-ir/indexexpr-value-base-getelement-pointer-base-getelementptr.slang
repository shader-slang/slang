//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=An `IndexExpr` lowers to `getElement` when its base is a value and to `getElementPtr` when its base is a pointer — the same subscript syntax picks a different opcode from the base's l-value-ness.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both arms of the subscript-lowering split in one compile. Inside
// `readFromValue` the array arrives as a by-value parameter, so `arr[i]` has
// nothing to take an address of and lowers to `getElement`. In `main` the array
// is a local variable with storage, so `local[idx]` in assignment position and
// `local[0]` on the read side both lower to `getElementPtr`. Having both in a
// single dump is what makes the opcode choice attributable to the base's form
// rather than to the index being dynamic — `idx` is a `uniform` in one case and
// a literal in the other, and both still produce `getElementPtr`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<int> outputBuffer;
uniform int idx;

int readFromValue(int arr[4], int i) { return arr[i]; }

[numthreads(1, 1, 1)]
void main()
{
    int local[4] = { 1, 2, 3, 4 };
    local[idx] = 9;
    int fixed[4] = { 5, 6, 7, 8 };
    outputBuffer[0] = readFromValue(fixed, idx) + local[0];
}

// A by-value array base yields the value form.
// CHECK: getElement(%arr{{[0-9]*}}, %i{{[0-9]*}})
// CHECK-LABEL: func %main
// A base with storage yields the pointer form, for a dynamic index ...
// CHECK: getElementPtr(%local{{[0-9]*}}, %idx{{[0-9]*}})
// ... and for a constant one.
// CHECK: getElementPtr(%local{{[0-9]*}}, 0 : Int)
