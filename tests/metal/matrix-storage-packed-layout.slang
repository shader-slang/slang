//TEST:SIMPLE(filecheck=PACKED): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=PACKED): -target metal -stage compute -entry computeMain -force-glsl-scalar-layout
//TEST:SIMPLE(filecheck=ROWMAJOR): -target metal -stage compute -entry computeMain -matrix-layout-row-major
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Matrices in Metal device buffers are stored as arrays of packed vectors
// (`packed_float3`) so the storage matches natural layout. The first two
// commands must agree: `-force-glsl-scalar-layout` requests the default.
// The dynamic row index exercises the whole-row load/store paths of
// `lowerMatrixAddresses`, which convert between the row vector and its
// packed storage.

StructuredBuffer<float3x3> mats;
RWStructuredBuffer<float3x3> outMats;
RWStructuredBuffer<float> output;

// An explicit `ScalarDataLayout` argument requests the natural layout Metal
// device buffers already use, so it produces the same packed storage.
StructuredBuffer<float3x3, ScalarDataLayout> matsScalar;

void addOne(inout float3 v)
{
    v += 1.0;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3x3 m = mats[tid.x] + matsScalar[tid.x];
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

// PACKED: struct _MatrixStorage_float3x3
// PACKED-NOT: array<float3,
// PACKED: array<packed_float3, int(3)> data
// PACKED-NOT: array<float3,

// The `ScalarDataLayout`-annotated buffer maps to the same packed storage.
// PACKED-DAG: _MatrixStorage_float3x3{{[A-Za-z0-9_]*}} device* matsScalar

// Column-major (default): a row is not contiguous, so the whole-row store
// scatters into per-component stores and the load gathers a `float3`.
// PACKED-DAG: = 5.0
// PACKED-DAG: = 6.0
// PACKED-DAG: = 7.0
// PACKED-DAG: float3({{.*}}data

// Row-major: a row maps onto one packed vector, so the store is a single
// packed-vector store.
// ROWMAJOR: struct _MatrixStorage_float3x3
// ROWMAJOR-NOT: array<float3,
// ROWMAJOR: array<packed_float3, int(3)> data
// ROWMAJOR-NOT: array<float3,
// ROWMAJOR-DAG: = packed_float3(5.0, 6.0, 7.0)
// ROWMAJOR-DAG: float3({{.*}}data

// The metallib target compiles the generated MSL with the Metal toolchain;
// the pattern below matches only if that compilation succeeded (malformed MSL
// produces no metallib text).
// METALLIB: define void @computeMain
