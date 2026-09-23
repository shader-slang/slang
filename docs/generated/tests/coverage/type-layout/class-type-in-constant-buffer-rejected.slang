//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies laying out a `class` type as a shader parameter is rejected with E39031, reported at the class declaration rather than at the parameter.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `_createTypeLayout`'s `DeclRefType` dispatch has a `ClassDecl` arm that
// diagnoses `ClassTypeNotSupported` and falls back to an empty layout. This is
// the only way to reach that arm: a `class` reaching layout at all requires it
// to be used as a shader parameter type. The diagnostic's location is worth
// pinning too — the arm prefers `context.layoutDeclForDiagnostics`, and with
// none set it falls back to `classDeclRef.getLoc()`, which is why the caret
// lands on `C` in the class declaration and not on `cb`.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

class C { float x; }
/*CHECK:
      ^ class types are not supported in type layout
      ^ class type 'C' is not supported; use 'struct' instead
*/
ConstantBuffer<C> cb;
RWStructuredBuffer<float> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = cb.x;
}
