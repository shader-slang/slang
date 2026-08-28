//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// Regression test for shader-slang/slang#11075.
//
// A generic `min`/`max` (constrained by `IComparable`) specialized on a vector of a pointer-sized
// integer (`intptr_t`/`uintptr_t`) decomposes element-wise on cpp/cuda, like the other integer
// element types: `intptr_t`/`uintptr_t` have scalar `IPTR_min`/`UPTR_min` prelude helpers, and the
// vector helpers apply them per element. (The pointer-sized vector types are cpp/cuda-only, so only
// those two backends run this test.)

T genMinC<T : IComparable>(T a, T b)
{
    return min(a, b);
}
T genMaxC<T : IComparable>(T a, T b)
{
    return max(a, b);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    vector<intptr_t, 2> ai = {intptr_t(3), intptr_t(9)};
    vector<intptr_t, 2> bi = {intptr_t(5), intptr_t(2)};
    vector<intptr_t, 2> mni = genMinC(ai, bi);
    vector<intptr_t, 2> mxi = genMaxC(ai, bi);

    vector<uintptr_t, 3> au = {uintptr_t(7), uintptr_t(1), uintptr_t(4)};
    vector<uintptr_t, 3> bu = {uintptr_t(2), uintptr_t(6), uintptr_t(4)};
    vector<uintptr_t, 3> mnu = genMinC(au, bu);
    vector<uintptr_t, 3> mxu = genMaxC(au, bu);

    // Arity 4 exercises the `longlong4` / `Vector<intptr_t,4>` helper.
    vector<intptr_t, 4> a4 = {intptr_t(8), intptr_t(5), intptr_t(3), intptr_t(1)};
    vector<intptr_t, 4> b4 = {intptr_t(7), intptr_t(2), intptr_t(6), intptr_t(4)};
    vector<intptr_t, 4> mn4 = genMinC(a4, b4);

    // Values above 2^32 confirm the pointer-sized element is preserved at full 64-bit width (a
    // helper that narrowed to 32 bits would pick the wrong element / wrong value). Compared as
    // exact integers, since `float` cannot represent these magnitudes precisely. Signed and
    // unsigned are checked separately so a narrowing in either IPTR or UPTR is caught.
    intptr_t bigLo = intptr_t(0x0000000100000002); // 2^32 + 2
    intptr_t bigHi = intptr_t(0x0000000100000009); // 2^32 + 9
    vector<intptr_t, 2> wa = {bigHi, bigLo};
    vector<intptr_t, 2> wb = {bigLo, bigHi};
    vector<intptr_t, 2> wmin = genMinC(wa, wb);
    vector<intptr_t, 2> wmax = genMaxC(wa, wb);

    uintptr_t ubigLo = uintptr_t(0x0000000100000002); // 2^32 + 2
    uintptr_t ubigHi = uintptr_t(0x0000000100000009); // 2^32 + 9
    vector<uintptr_t, 2> uwa = {ubigHi, ubigLo};
    vector<uintptr_t, 2> uwb = {ubigLo, ubigHi};
    vector<uintptr_t, 2> uwmin = genMinC(uwa, uwb);
    vector<uintptr_t, 2> uwmax = genMaxC(uwa, uwb);

    int i = 0;
    outputBuffer[i++] = float(mni.x); // CHECK: 3.0
    outputBuffer[i++] = float(mni.y); // CHECK: 2.0
    outputBuffer[i++] = float(mxi.x); // CHECK: 5.0
    outputBuffer[i++] = float(mxi.y); // CHECK: 9.0

    outputBuffer[i++] = float(mnu.x); // CHECK: 2.0
    outputBuffer[i++] = float(mnu.y); // CHECK: 1.0
    outputBuffer[i++] = float(mnu.z); // CHECK: 4.0
    outputBuffer[i++] = float(mxu.x); // CHECK: 7.0
    outputBuffer[i++] = float(mxu.y); // CHECK: 6.0
    outputBuffer[i++] = float(mxu.z); // CHECK: 4.0

    outputBuffer[i++] = float(mn4.x); // CHECK: 7.0
    outputBuffer[i++] = float(mn4.y); // CHECK: 2.0
    outputBuffer[i++] = float(mn4.z); // CHECK: 3.0
    outputBuffer[i++] = float(mn4.w); // CHECK: 1.0

    // min/max pick bigLo/bigHi element-wise; check full-width equality, not the float value.
    outputBuffer[i++] = (wmin.x == bigLo && wmin.y == bigLo && wmax.x == bigHi && wmax.y == bigHi &&
                         uwmin.x == ubigLo && uwmin.y == ubigLo && uwmax.x == ubigHi &&
                         uwmax.y == ubigHi)
                            ? 1.0f
                            : 0.0f; // CHECK: 1.0
}
