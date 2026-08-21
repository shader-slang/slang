// Portable `ReportHit(tHit, hitKind, attributes)` on CUDA/OptiX flattens the aggregate field-wise
// into `optixReportIntersection` attribute registers. Two cases are rejected rather than
// miscompiled: an attribute that needs more than 8 32-bit registers (OptiX's 32-byte limit), and a
// scalar leaf that is not a supported single-register type (here `double`, which cannot fit one
// 32-bit register).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry tooLarge -stage intersection
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-TYPE): -target cuda -entry badType -stage intersection
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-HALF): -target cuda -entry badHalf -stage intersection
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-PTR): -target cuda -entry badPtr -stage intersection

struct NineFloats
{
    float a0, a1, a2, a3, a4, a5, a6, a7, a8;
};

[shader("intersection")]
void tooLarge()
{
    NineFloats attributes = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    ReportHit(1.0f, 1u, attributes);
    //CHECK: E55217
    //CHECK: requires 9 32-bit attribute registers, but OptiX supports at most 8
}

struct HasDouble
{
    double value;
};

[shader("intersection")]
void badType()
{
    HasDouble attributes = {1.5};
    ReportHit(1.0f, 1u, attributes);
    //CHECK-TYPE: E55216
    //CHECK-TYPE: each scalar field must fit in one 32-bit register
}

struct HasHalf
{
    half value;
};

[shader("intersection")]
void badHalf()
{
    HasHalf attributes = {half(1.5)};
    // `half` is 16 bits but has no bit-preserving read counterpart, so it is rejected too.
    ReportHit(1.0f, 1u, attributes);
    //CHECK-HALF: E55216
}

struct PointerAttr
{
    float x;
};

[shader("intersection")]
void badPtr()
{
    PointerAttr attr = {0.5f};
    PointerAttr* p = &attr;
    // A pointer cannot round-trip through a scalar attribute register; reject rather than
    // dereference (which would change which value is reported).
    ReportHit(1.0f, 1u, p);
    //CHECK-PTR: E55216
}
