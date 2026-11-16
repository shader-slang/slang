// Reflection must report the member offsets that the generated code uses for a buffer with
// int/uint/bool matrices. `tests/spirv/int-matrix-buffer-layout-offsets.slang` checks the offsets
// in the emitted SPIR-V for the same struct, so the two tests together check that the emitted
// offsets and the reflection data agree under both matrix layout defaults.

//TEST:REFLECTION(filecheck=COLMAJOR):-stage compute -entry computeMain -target spirv -no-codegen -matrix-layout-column-major
//TEST:REFLECTION(filecheck=ROWMAJOR):-stage compute -entry computeMain -target spirv -no-codegen -matrix-layout-row-major

struct S
{
    column_major int2x3 m;
    int2x3 k;
    row_major int2x3 r;
    column_major bool2x3 b;
    int tail;
}

RWStructuredBuffer<S> buffer;
RWStructuredBuffer<int> output;

// The member `k` has no qualifier, so it follows the default layout.
// COLMAJOR: "name": "m"
// COLMAJOR: "binding": {"kind": "uniform", "offset": 0, "size": 24
// COLMAJOR: "name": "k"
// COLMAJOR: "binding": {"kind": "uniform", "offset": 24, "size": 24
// COLMAJOR: "name": "r"
// COLMAJOR: "binding": {"kind": "uniform", "offset": 48, "size": 32
// COLMAJOR: "name": "b"
// COLMAJOR: "binding": {"kind": "uniform", "offset": 80, "size": 24
// COLMAJOR: "name": "tail"
// COLMAJOR: "binding": {"kind": "uniform", "offset": 104, "size": 4

// ROWMAJOR: "name": "m"
// ROWMAJOR: "binding": {"kind": "uniform", "offset": 0, "size": 24
// ROWMAJOR: "name": "k"
// ROWMAJOR: "binding": {"kind": "uniform", "offset": 32, "size": 32
// ROWMAJOR: "name": "r"
// ROWMAJOR: "binding": {"kind": "uniform", "offset": 64, "size": 32
// ROWMAJOR: "name": "b"
// ROWMAJOR: "binding": {"kind": "uniform", "offset": 96, "size": 24
// ROWMAJOR: "name": "tail"
// ROWMAJOR: "binding": {"kind": "uniform", "offset": 120, "size": 4

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = buffer[0].m[0][1] + buffer[0].k[0][1] + buffer[0].r[0][1] +
                (buffer[0].b[1][2] ? 1 : 0) + buffer[0].tail;
}
