// Positive control for the "multiple depth output" diagnostic (E30705): a
// fragment entry point that declares exactly one depth output must still
// compile without error. This guards against the multiplicity check (added in
// validateEntryPoint) firing on the legal single-depth case.
//
// A single depth output across an `out` parameter and a separate non-depth
// return-struct field is fine, so neither E30705 nor any other error should be
// produced.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage fragment

//CHECK-NOT: E30705

struct FSOut
{
    float color : SV_Target;
};

[shader("fragment")]
FSOut main(float x, out float depth : SV_DepthGreaterEqual)
{
    depth = x + 1.0;
    FSOut o;
    o.color = x;
    return o;
}
