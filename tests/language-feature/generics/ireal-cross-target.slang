// Cross-target compile guard for the #12591 interface tower. No GPU needed — this only checks that
// the generic math emits valid Metal/WGSL source, guarding the two target-specific witness choices:
//
//  * Matrix `rcp` overrides the broad-tier default `This(1.0).div(this)` (a matrix/matrix division
//    that Metal and WGSL reject) with a row-by-row reciprocal, so `reciprocal<matrix>` must compile.
//  * Scalar `dot` lowers to `this * other`, not the free `dot(...)` (WGSL's `dot` requires vector
//    operands), so `innerProduct<scalar>` must not emit a `dot(` call.

//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL):  -target wgsl  -stage compute -entry computeMain

RWStructuredBuffer<float> gOut;

T reciprocal<T : IFloatingPoint>(T x)
{
    return x.rcp();
}

T.Element innerProduct<T : ISupportsInnerProduct>(T a, T b)
{
    return a.dot(b);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float2x2 mr = reciprocal(float2x2(2, 4, 5, 10));
    float sd = innerProduct(2.0f, 3.0f);
    gOut[0] = mr[0][0] + mr[0][1] + mr[1][0] + mr[1][1] + sd;
}

// Guard for the matrix-`rcp` override: if it regressed to the broad-tier default `This(1.0).div(this)`,
// slangc rejects the matrix/matrix division at emit for these targets (E56102,
// DivisionByMatrixNotSupported) and produces no target source — so the entry point would be absent.
// Requiring `computeMain` to appear therefore fails such a regression; there is no emitted
// matrix-division token to `NOT`-match because the compile aborts before emission.
// METAL: computeMain
// WGSL: computeMain

// Guard for the scalar-`dot` witness: it must lower to a multiply, never a `dot(` call (WGSL's `dot`
// requires vector operands), so no `dot(` may appear in the WGSL output.
// WGSL-NOT: dot(
