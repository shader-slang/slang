// Boxing a byte-compatible conforming type into its flat AnyValue payload should
// be a single whole-object bit-cast, not one slang_bit_cast<uint> per 4 bytes.
// See shader-slang/slang#12606. Correctness is checked on the CPU target (which
// shares the C++ emitter with CUDA); the CUDA and CPP passes assert the emit-form
// win, and that types which are NOT byte-compatible stay on the safe per-field path.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// --- Byte-compatible on every target: 16 bytes, all 4-byte float leaves, densely
// packed, and 4-byte aligned (no vector wider than float3) => a single whole-object
// bit-cast on both CUDA and CPP, in both directions. ---

[anyValueSize(16)]
interface IShape
{
    int measure();
}

struct Circle : IShape
{
    float3 center;
    float radius;
    int measure() { return int(center.x + center.y + center.z + radius); }
}

struct Trio : IShape
{
    float3 a;
    int tag;
    int measure() { return int(a.x + a.y + a.z) + tag; }
}

IShape makeShape(int id)
{
    if (id == 0)
    {
        Circle c;
        c.center = float3(1, 2, 3);
        c.radius = 4.0;
        return c;
    }
    else
    {
        Trio t;
        t.a = float3(5, 6, 7);
        t.tag = 8;
        return t;
    }
}

// --- Byte-compatible in size, but a `float4` is 16-byte aligned on CUDA while the
// AnyValue payload is only 4-byte aligned. The whole-object bit-cast is therefore
// alignment-safe on CPP (C-rules give float4 4-byte alignment) but NOT on CUDA, so
// the fast path must fire only on CPP here. Boxed at 16 bytes. ---

[anyValueSize(16)]
interface IWide
{
    int measure();
}

struct Wide : IWide
{
    float4 v;
    int measure() { return int(v.x + v.y + v.z + v.w); }
}

// A second conformer so the interface is a genuine multi-conformer dynamic dispatch.
struct WideAlt : IWide
{
    float4 w;
    int measure() { return int(w.x - w.y); }
}

IWide makeWide(int id)
{
    if (id == 0)
    {
        Wide a;
        a.v = float4(1, 2, 3, 4);
        return a;
    }
    else
    {
        WideAlt b;
        b.w = float4(9, 5, 0, 0);
        return b;
    }
}

// --- Not byte-compatible on any target: a matrix is walked column-by-column but
// stored row-contiguously, so a whole-object copy would reorder its elements.
// Boxed at 24 bytes so its fallback can be asserted independently. ---

[anyValueSize(24)]
interface IMatrix
{
    int measure();
}

struct MatA : IMatrix
{
    column_major float3x2 m;
    int measure() { return int(m[0][0]); }
}

struct MatB : IMatrix
{
    column_major float2x3 n;
    int measure() { return int(n[0][0]); }
}

IMatrix makeMatrix(int id)
{
    if (id == 0)
    {
        MatA a;
        a.m = float3x2(1, 2, 3, 4, 5, 6);
        return a;
    }
    else
    {
        MatB b;
        b.n = float2x3(7, 8, 9, 10, 11, 12);
        return b;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    IShape a = makeShape(0);
    outputBuffer[0] = a.measure();
    // CHECK: 10

    IShape b = makeShape(1);
    outputBuffer[1] = b.measure();
    // CHECK: 26

    IWide w = makeWide(0);
    outputBuffer[2] = w.measure();
    // CHECK: 10

    IMatrix c = makeMatrix(0);
    outputBuffer[3] = c.measure();
    // CHECK: 1

    IMatrix d = makeMatrix(1);
    outputBuffer[4] = d.measure();
    // CHECK: 7
}

// Each check is anchored to a specific pack/unpack function via CHECK-LABEL so the
// whole-object cast (or its absence) is bound to the concrete type that owns it,
// rather than counting casts module-wide. The generated function names are
// `[un]packAnyValueN_i(<ConcreteType> *)`.

// --- IShape (Circle, Trio): 4-byte aligned, fast path on BOTH targets, both
// directions. The pack returns bit_cast<AnyValueN>; the unpack returns
// bit_cast<ConcreteType>. ---
// CUDA-LABEL: packAnyValue16_0(Circle_0
// CUDA: slang_bit_cast<AnyValue16>
// CUDA-LABEL: unpackAnyValue16_0(AnyValue16
// CUDA: slang_bit_cast<Circle_0>
// CUDA-LABEL: packAnyValue16_1(Trio_0
// CUDA: slang_bit_cast<AnyValue16>
// CUDA-LABEL: unpackAnyValue16_1(AnyValue16
// CUDA: slang_bit_cast<Trio_0>

// CPP-LABEL: packAnyValue16_0(Circle_0
// CPP: slang_bit_cast<AnyValue16>
// CPP-LABEL: unpackAnyValue16_0(AnyValue16
// CPP: slang_bit_cast<Circle_0>
// CPP-LABEL: packAnyValue16_1(Trio_0
// CPP: slang_bit_cast<AnyValue16>
// CPP-LABEL: unpackAnyValue16_1(AnyValue16
// CPP: slang_bit_cast<Trio_0>

// --- IWide (Wide, WideAlt): a float4 is 16-byte aligned on CUDA (payload is only
// 4-byte aligned), so the fast path must NOT fire — the CUDA-NOT lines are scoped to
// the Wide/WideAlt pack functions by the surrounding CHECK-LABELs. On CPP a float4 is
// 4-byte aligned, so the fast path DOES fire in that pack function. ---
// Each CUDA-NOT is bounded by the following CHECK-LABEL, so it is scoped to exactly
// one function; the unpack labels also assert the unpack side does not fast-path.
// CUDA-LABEL: packAnyValue16_2(Wide_0
// CUDA-NOT: slang_bit_cast<AnyValue16>
// CUDA-LABEL: unpackAnyValue16_2(AnyValue16
// CUDA-NOT: slang_bit_cast<Wide_0>
// CUDA-LABEL: packAnyValue16_3(WideAlt_0
// CUDA-NOT: slang_bit_cast<AnyValue16>
// CUDA-LABEL: unpackAnyValue16_3(AnyValue16
// CUDA-NOT: slang_bit_cast<WideAlt_0>

// CPP-LABEL: packAnyValue16_2(Wide_0
// CPP: slang_bit_cast<AnyValue16>
// CPP-LABEL: unpackAnyValue16_2(AnyValue16
// CPP: slang_bit_cast<Wide_0>
// CPP-LABEL: packAnyValue16_3(WideAlt_0
// CPP: slang_bit_cast<AnyValue16>
// CPP-LABEL: unpackAnyValue16_3(AnyValue16
// CPP: slang_bit_cast<WideAlt_0>

// --- IMatrix (MatA, MatB): column-major, not byte-compatible on any target, so the
// pack stays field-wise (no whole-object AnyValue24 cast) in the pack functions, and
// the unpack returns no whole-object concrete cast, on both targets. ---
// CUDA-LABEL: packAnyValue24_0(MatA_0
// CUDA-NOT: slang_bit_cast<AnyValue24>
// CUDA-LABEL: unpackAnyValue24_0(AnyValue24
// CUDA-NOT: slang_bit_cast<MatA_0>
// CUDA-LABEL: packAnyValue24_1(MatB_0
// CUDA-NOT: slang_bit_cast<AnyValue24>
// CUDA-LABEL: unpackAnyValue24_1(AnyValue24
// CUDA-NOT: slang_bit_cast<MatB_0>

// CPP-LABEL: packAnyValue24_0(MatA_0
// CPP-NOT: slang_bit_cast<AnyValue24>
// CPP-LABEL: unpackAnyValue24_0(AnyValue24
// CPP-NOT: slang_bit_cast<MatA_0>
// CPP-LABEL: packAnyValue24_1(MatB_0
// CPP-NOT: slang_bit_cast<AnyValue24>
// CPP-LABEL: unpackAnyValue24_1(AnyValue24
// CPP-NOT: slang_bit_cast<MatB_0>
