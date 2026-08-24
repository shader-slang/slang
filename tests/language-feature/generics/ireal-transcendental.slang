// Regression test for shader-slang/slang#12591: a generic function constrained to `IReal` can call
// the math functions (pow/exp/exp2/log/log2/log10/sqrt/rsqrt/sin/cos/tan) and componentwise
// min/max/saturate uniformly across scalar, vector, and matrix; `ISupportsInnerProduct` adds `dot`;
// `IFloatingPoint` adds `rcp`. Math is spelled with method syntax (`x.pow(y)`).
//
// Where an exact CPU result exists, each requirement is checked with an input/output a wrong or
// no-op witness would fail (e.g. `exp(0)=1`, `log(1)=0`, `sqrt(9)=3`, base-2/10 at distinct values,
// non-cancelling `saturate`). Some checks are necessarily weaker: `sin`/`cos`/`tan` are evaluated at
// 0 (the only exactly-representable transcendental points on CPU), so they catch an identity or a
// swap onto a function that is nonzero at 0, but not swaps among functions sharing that value
// (e.g. `cos`<->`exp`, both 1, or `sin`<->`tan`, both 0); and `mathMix`'s `exp(log(x))` is a
// round-trip, so it is the isolated `applyExp`/`applyLog` checks that distinguish those two.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// One generic `pow` usable across scalar, vector, and matrix.
T computePow<T : IReal>(T base, T exponent)
{
    return base.pow(exponent);
}

// sqrt / log / exp round-trip through the interface (also the #6961 `exp` case), scalar and matrix.
T mathMix<T : IReal>(T x)
{
    return x.sqrt().add(x.log().exp());
}

// Each transcendental isolated to its own check with a distinct expected value.
T applyExp<T : IReal>(T x)    { return x.exp(); }
T applyLog<T : IReal>(T x)    { return x.log(); }
T applyExp2<T : IReal>(T x)   { return x.exp2(); }
T applyLog2<T : IReal>(T x)   { return x.log2(); }
T applyLog10<T : IReal>(T x)  { return x.log10(); }
T applySqrt<T : IReal>(T x)   { return x.sqrt(); }
T applyRsqrt<T : IReal>(T x)  { return x.rsqrt(); }
T applyCos<T : IReal>(T x)    { return x.cos(); }
T applySin<T : IReal>(T x)    { return x.sin(); }
T applyTan<T : IReal>(T x)    { return x.tan(); }

// Componentwise min/max on the generic interface, scalar+vector+matrix.
T clampReal<T : IReal>(T x, T lo, T hi)
{
    return x.min(hi).max(lo);
}

// Componentwise saturate (clamp to [0,1]) on the generic interface.
T saturateReal<T : IReal>(T x)
{
    return x.saturate();
}

// Inner product via the separate capability.
T.Element innerProduct<T : ISupportsInnerProduct>(T a, T b)
{
    return a.dot(b);
}

// `rcp` is a field-like op on the broad IFloatingPoint tier (not IReal).
T reciprocal<T : IFloatingPoint>(T x)
{
    return x.rcp();
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Scalar: pow(3, 2) = 9.
    float s = computePow(3.0f, 2.0f);
    // CHECK: 9.0
    outputBuffer[0] = s;

    // Vector: element-wise pow((2,3,4), (2,3,4)) = (4, 27, 256); sum = 287.
    float3 v = computePow(float3(2.0f, 3.0f, 4.0f), float3(2.0f, 3.0f, 4.0f));
    // CHECK: 287.0
    outputBuffer[1] = v.x + v.y + v.z;

    // double conforms too: pow(2, 10) = 1024.
    double d = computePow(2.0, 10.0);
    // CHECK: 1024.0
    outputBuffer[2] = float(d);

    // sqrt(4) + exp(log(4)) = 2 + 4 = 6 (round-trip; isolated exp/log are checked below).
    float m = mathMix(4.0f);
    // CHECK: 6.0
    outputBuffer[3] = m;

    // Componentwise clamp on a vector: clamp((5,-1,2),0,3) = (3,0,2); sum 5.
    float3 c = clampReal(float3(5, -1, 2), float3(0, 0, 0), float3(3, 3, 3));
    // CHECK: 5.0
    outputBuffer[4] = c.x + c.y + c.z;

    // Same generic over a matrix: clamp((5,-1,2,7),0,3) row-wise = ((3,0),(2,3)); sum = 8.
    float2x2 mm = clampReal(float2x2(5, -1, 2, 7), float2x2(0, 0, 0, 0), float2x2(3, 3, 3, 3));
    // CHECK: 8.0
    outputBuffer[5] = mm[0][0] + mm[0][1] + mm[1][0] + mm[1][1];

    // Inner product (dot) via ISupportsInnerProduct: (1,2,3)·(4,5,6) = 32.
    float ip = innerProduct(float3(1, 2, 3), float3(4, 5, 6));
    // CHECK: 32.0
    outputBuffer[6] = ip;

    // IFloat is now an alias of IFloatingPoint: an existing `T : IFloat` generic still works.
    float f = scaleByTwo(2.5f);
    // CHECK: 5.0
    outputBuffer[7] = f;

    // rcp on the broad tier: reciprocal of a vector, 1/(2,4) = (0.5, 0.25); sum = 0.75.
    float2 rv = reciprocal(float2(2.0f, 4.0f));
    // CHECK: 0.75
    outputBuffer[8] = rv.x + rv.y;

    // Scalars also conform to ISupportsInnerProduct (degenerate dot): dot(2,3) = 6.
    float sip = innerProduct(2.0f, 3.0f);
    // CHECK: 6.0
    outputBuffer[9] = sip;

    // rcp over a matrix (matrix overrides the broad-tier default with elementwise rcp):
    // 1/((2,4),(5,10)) = ((0.5,0.25),(0.2,0.1)); sum = 1.05.
    float2x2 mr = reciprocal(float2x2(2, 4, 5, 10));
    // CHECK: 1.05
    outputBuffer[10] = mr[0][0] + mr[0][1] + mr[1][0] + mr[1][1];

    // Isolated transcendentals. Those with an exact CPU value use distinct results (a swap fails);
    // the trig checks at 0 only exercise the witness (see the file header for which swaps they miss).
    // exp(0) = 1.
    // CHECK: 1.0
    outputBuffer[11] = applyExp(0.0f);
    // log(1) = 0.
    // CHECK: 0.0
    outputBuffer[12] = applyLog(1.0f);
    // exp2(3) = 8.
    // CHECK: 8.0
    outputBuffer[13] = applyExp2(3.0f);
    // log2(32) = 5.
    // CHECK: 5.0
    outputBuffer[14] = applyLog2(32.0f);
    // log10(100) = 2.
    // CHECK: 2.0
    outputBuffer[15] = applyLog10(100.0f);
    // rsqrt(16) = 0.25.
    // CHECK: 0.25
    outputBuffer[16] = applyRsqrt(16.0f);
    // sqrt(9) = 3 (distinct from the values above, so an isolated sqrt swap is caught).
    // CHECK: 3.0
    outputBuffer[17] = applySqrt(9.0f);
    // cos(0) = 1.
    // CHECK: 1.0
    outputBuffer[18] = applyCos(0.0f);
    // sin(0) = 0.
    // CHECK: 0.0
    outputBuffer[19] = applySin(0.0f);
    // tan(0) = 0.
    // CHECK: 0.0
    outputBuffer[20] = applyTan(0.0f);

    // Matrix transcendentals: mathMix over all-4 matrix = 6 per element; sum over 4 elements = 24.
    float2x2 mtx = mathMix(float2x2(4, 4, 4, 4));
    // CHECK: 24.0
    outputBuffer[21] = mtx[0][0] + mtx[0][1] + mtx[1][0] + mtx[1][1];

    // Componentwise saturate, non-cancelling inputs (an identity/no-op `saturate` changes the sum).
    // Vector saturate((-1,-1,2)) = (0,0,1); sum = 1.0 (identity would sum to 0.0).
    float3 sv = saturateReal(float3(-1.0f, -1.0f, 2.0f));
    // CHECK: 1.0
    outputBuffer[22] = sv.x + sv.y + sv.z;
    // Matrix saturate((-1,-1,2,3)) = (0,0,1,1); sum = 2.0 (identity would sum to 3.0).
    float2x2 ms = saturateReal(float2x2(-1.0f, -1.0f, 2.0f, 3.0f));
    // CHECK: 2.0
    outputBuffer[23] = ms[0][0] + ms[0][1] + ms[1][0] + ms[1][1];

    // Scalar min/max: clamp(5, 0, 3) = min(5,3)=3 then max(3,0)=3.
    // CHECK: 3.0
    outputBuffer[24] = clampReal(5.0f, 0.0f, 3.0f);

    // Scalar rcp: 1/4 = 0.25.
    // CHECK: 0.25
    outputBuffer[25] = reciprocal(4.0f);

    // Scalar saturate: saturate(1.5) = 1.
    // CHECK: 1.0
    outputBuffer[26] = saturateReal(1.5f);

    // double through a transcendental other than pow: log2(256) = 8.
    double dl = applyLog2(256.0);
    // CHECK: 8.0
    outputBuffer[27] = float(dl);

    // The headline of #12591 — one generic `pow` over a matrix: pow((2,3,4,5),(2,2,2,2)) =
    // (4,9,16,25); sum = 54.
    float2x2 mp = computePow(float2x2(2, 3, 4, 5), float2x2(2, 2, 2, 2));
    // CHECK: 54.0
    outputBuffer[28] = mp[0][0] + mp[0][1] + mp[1][0] + mp[1][1];
}

T scaleByTwo<T : IFloat>(T x)
{
    return x + x;
}
