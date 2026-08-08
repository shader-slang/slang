//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=b3a4828968ba320e73bd253425b6f464bf34ceea7d61ab6e1933e1f91bcb19a9
//META: purpose=Assignment to a matrix-swizzle lvalue lowers to `matrixSwizzleStore`, whose trailing operands come in (row, col) pairs — two operands per selected element, not one.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand shape that separates `matrixSwizzleStore` from
// the vector `swizzledStore`: selecting two matrix elements produces
// four trailing index operands, because each element is addressed by a
// row and a column. `m._11_22 = v` selects the (0,0) and (1,1) entries,
// so CHECK asserts exactly `0 : Int, 0 : Int, 1 : Int, 1 : Int` after
// the destination pointer and the source vector. A reader (or a test)
// that assumed one index per element would see only two.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float f;
uniform float2 v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    float2x2 m = float2x2(f, f + 1, f + 2, f + 3);
    m._11_22 = v;
    buf[0] = m[0][1];
}

// IR-LABEL: func %main
// IR: matrixSwizzleStore(%m, %v, 0 : Int, 0 : Int, 1 : Int, 1 : Int)
