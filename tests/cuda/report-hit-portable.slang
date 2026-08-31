//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry main -stage intersection
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry main -stage intersection
//TEST:SIMPLE(filecheck=CHECK-EMPTY): -target cuda -entry empty -stage intersection

// Portable HLSL `ReportHit(tHit, hitKind, attributes)` compiles for CUDA/OptiX by flattening the
// aggregate field-wise into `optixReportIntersection`, one attribute register per scalar leaf, in
// the same order the reader (`optixGetAttribute_N`) consumes them: struct fields in order, then
// vector components in index order. An integer leaf passes through; a float leaf is
// bit-reinterpreted with `__float_as_uint` (the inverse of the reader's `__int_as_float`).

// The narrow integer fields (`int16_t`, `uint8_t`) exercise that any integer type of 32 bits or
// fewer passes through a single attribute register unchanged; only `float` fields are wrapped in
// `__float_as_uint`.
struct Attributes
{
    int id;
    int16_t code;
    uint8_t tag;
    bool flag;
    float weight;
    float2 bary;
};

[shader("intersection")]
void main()
{
    Attributes attributes = {-3, int16_t(-2), uint8_t(200), true, 0.5f, float2(0.25f, 0.75f)};
    // CHECK: optixReportIntersection(1.0f, 1U,
    // CHECK-SAME: id
    // CHECK-SAME: code
    // CHECK-SAME: tag
    // CHECK-SAME: flag
    // CHECK-SAME: __float_as_uint(
    // CHECK-SAME: weight
    // CHECK-SAME: __float_as_uint(
    // CHECK-SAME: bary
    // CHECK-SAME: .x
    // CHECK-SAME: __float_as_uint(
    // CHECK-SAME: bary
    // CHECK-SAME: .y
    // CHECK-PTX: _optix_report_intersection
    ReportHit(1.0f, 1u, attributes);
}

// An empty attribute struct flattens to zero leaves and uses OptiX's zero-attribute
// `optixReportIntersection(tHit, hitKind)` overload.
struct EmptyAttributes
{
};

[shader("intersection")]
void empty()
{
    EmptyAttributes attributes = {};
    // CHECK-EMPTY: optixReportIntersection(1.0f, 1U)
    ReportHit(1.0f, 1u, attributes);
}
