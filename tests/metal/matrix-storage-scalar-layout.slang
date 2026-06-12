//TEST:SIMPLE(filecheck=SCALAR): -target metal -stage compute -entry computeMain -force-glsl-scalar-layout
//TEST:SIMPLE(filecheck=ROWMAJOR): -target metal -stage compute -entry computeMain -force-glsl-scalar-layout -matrix-layout-row-major
//TEST:SIMPLE(filecheck=DEFAULT): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain -force-glsl-scalar-layout

// With `-force-glsl-scalar-layout`, matrices in device buffers use natural
// (scalar-aligned, tightly packed) layout. MSL `float3` has a 16-byte stride,
// so matrix storage uses arrays of packed vectors (`packed_float3`: 12-byte
// stride) instead of arrays of native vectors. Without the option, the native
// vector storage is kept.
//
// The dynamic row index accesses exercise the whole-row load/store paths of
// `lowerMatrixAddresses`, which with packed-vector storage must convert
// between the row vector and its `packed_float3` storage.

StructuredBuffer<float3x3> mats;
RWStructuredBuffer<float3x3> outMats;
RWStructuredBuffer<float> output;

void addOne(inout float3 v)
{
    v += 1.0;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3x3 m = mats[tid.x];
    output[tid.x] = m[1][2];
    outMats[tid.x] = m;
    outMats[tid.x][2] = float3(1.0, 2.0, 3.0);
    outMats[tid.x][0][1] = 4.0;

    // Dynamic row index: whole-row store and whole-row load (through the
    // inout parameter) on the lowered storage.
    uint row = tid.x % 3;
    outMats[tid.x][row] = float3(5.0, 6.0, 7.0);
    addOne(outMats[tid.x][row]);
}

// SCALAR: struct _MatrixStorage_float3x3
// SCALAR-NOT: array<float3,
// SCALAR: array<packed_float3, int(3)> data
// SCALAR-NOT: array<float3,

// With the (default) column-major layout, the dynamic whole-row store
// decomposes into per-component stores into the packed columns, and the
// whole-row load reconstructs a `float3` from them.
// SCALAR-DAG: = 5.0
// SCALAR-DAG: = 6.0
// SCALAR-DAG: = 7.0
// SCALAR-DAG: float3({{.*}}data

// With row-major layout the row maps directly onto one packed vector, so the
// dynamic whole-row store becomes a single packed-vector store and the
// whole-row load reconstructs a `float3` from the packed row.
// ROWMAJOR: struct _MatrixStorage_float3x3
// ROWMAJOR-NOT: array<float3,
// ROWMAJOR: array<packed_float3, int(3)> data
// ROWMAJOR-NOT: array<float3,
// ROWMAJOR-DAG: = packed_float3(5.0, 6.0, 7.0)
// ROWMAJOR-DAG: float3({{.*}}data

// DEFAULT: struct _MatrixStorage_float3x3
// DEFAULT: array<float3, int(3)> data

// The metallib target compiles the generated MSL with the Metal toolchain and
// FileCheck runs over the LLVM-IR-style text of the resulting metallib. The
// pattern below matches only if that compilation succeeded: malformed MSL
// (e.g. a leftover whole-row store of a vector into the lowered storage
// struct, which `lowerMatrixAddresses` must remove) fails the Metal compile
// and produces no metallib text to match.
// METALLIB: define void @computeMain
