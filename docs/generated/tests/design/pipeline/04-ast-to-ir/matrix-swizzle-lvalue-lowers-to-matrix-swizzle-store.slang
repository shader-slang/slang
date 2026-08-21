//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#loweredvalinfo-and-the-lowering-environment
//META: doc_section_digest=9127ecfc310ffdc4aecbad1b96d176cf66095ac4dcbdc84a1ac4106e6bf98dd2
//META: purpose=A matrix swizzle in l-value position (`m._m00_m11 = ...`) stays symbolic as the `SwizzledMatrixLValue` flavor and resolves at the write site to a single `matrixSwizzleStore` carrying the row/column index pair per selected component.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the matrix half of the deferred-l-value machinery, the sibling of
// the vector `swizzledStore` case. Assigning through `m._m00_m11` selects two
// components identified by (row, column) pairs rather than by a single index,
// so the write cannot be a plain store into a computed address; lowering keeps
// the selection symbolic and emits one `matrixSwizzleStore` whose trailing
// operands are the flattened index pairs. The CHECK pins those four operands —
// `0, 0` then `1, 1` — because they are what distinguishes this from a whole-
// matrix store. The source value is read from a `uniform` so the assignment is
// not folded away, and the matrix is read back into the output buffer so the
// store survives dead-code elimination.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<float> outputBuffer;
uniform float a;

[numthreads(1, 1, 1)]
void main()
{
    float2x2 m = float2x2(0.0, 0.0, 0.0, 0.0);
    m._m00_m11 = float2(a, a + 1.0);
    outputBuffer[0] = m[0][0] + m[1][1];
}

// CHECK: matrixSwizzleStore(%m{{[0-9]*}}, %{{[A-Za-z0-9_]+}}, 0 : Int, 0 : Int, 1 : Int, 1 : Int)
