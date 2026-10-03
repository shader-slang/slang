// Boxing a byte-compatible conforming type into its flat AnyValue payload should
// be a single whole-object bit-cast, not one slang_bit_cast<uint> per 4 bytes.
// See shader-slang/slang#12606. Correctness is checked on the CPU target (which
// shares the C++ emitter with CUDA); the CUDA and CPP passes assert the emit-form
// win, and that types which are NOT byte-compatible stay on the safe per-field path.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
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

// --- Byte-compatible on every target, exercising the leaf kinds the fast-path predicate
// accepts beyond bare floats: an all-`uint` struct, an array leaf, and an enum leaf (the
// array/enum recursion arms of `countWordScalarLeaves`). All are 4-byte word scalars,
// densely packed at 16 bytes, so each conformer fast-paths on both CUDA and CPP — positive
// controls confirming these leaf kinds count as verbatim words. (The field-wise `uint`
// store/load branch is covered separately by the rejected `IScalarLeaf` conformers.) ---

enum Tag : uint
{
    A = 1,
    B = 7,
}

[anyValueSize(16)]
interface IWordKinds
{
    int measure();
}

struct UintQuad : IWordKinds
{
    uint x;
    uint y;
    uint z;
    uint w;
    int measure() { return int(x + y + z + w); }
}

struct ArrayLeaf : IWordKinds
{
    float[3] vals;
    int tag;
    int measure() { return int(vals[0] + vals[1] + vals[2]) + tag; }
}

struct EnumLeaf : IWordKinds
{
    Tag t;
    float a;
    float b;
    float c;
    int measure() { return int(t) + int(a) + int(b) + int(c); }
}

IWordKinds makeWordKinds(int id)
{
    if (id == 0)
    {
        UintQuad q;
        q.x = 1;
        q.y = 2;
        q.z = 3;
        q.w = 4;
        return q;
    }
    else if (id == 1)
    {
        ArrayLeaf a;
        a.vals = { 5.0, 6.0, 7.0 };
        a.tag = 8;
        return a;
    }
    else
    {
        EnumLeaf e;
        e.t = Tag.B;
        e.a = 2;
        e.b = 3;
        e.c = 4;
        return e;
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

// --- Not byte-compatible on any target. These conformers are `column_major`, which
// `emitMarshallingCode` walks column-by-column while the emitted `Matrix<T,R,C>` stores
// rows contiguously, so a whole-object copy would reorder the elements. (A row-major matrix
// would not be reordered; the predicate still rejects it, conservatively.) Boxed at 24 bytes
// so its fallback can be asserted independently. ---

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

// Reject: a non-word scalar leaf. `HasBool` is boxed at its exact layout size, so only
// the leaf kind fails the fast path — `bool` is normalized, not a verbatim word. The `uint`
// leaf makes these conformers also cover the field-wise `uint` direct-store/load branch of
// `marshalBasicType` (the whole-object fast path never runs `marshalBasicType`, so a
// rejected conformer is the only way to exercise that per-leaf branch).

[anyValueSize(12)]
interface IScalarLeaf
{
    int measure();
}

struct HasBool : IScalarLeaf
{
    float a;
    bool flag;
    uint u;
    int measure() { return int(a) + (flag ? 100 : 0) + int(u); }
}

struct HasBoolAlt : IScalarLeaf
{
    bool flag;
    uint a;
    float b;
    int measure() { return (flag ? 10 : 0) + int(a) + int(b); }
}

IScalarLeaf makeScalarLeaf(int id)
{
    if (id == 0)
    {
        HasBool h;
        h.a = 1;
        h.flag = true;
        h.u = 3;
        return h;
    }
    else
    {
        HasBoolAlt h;
        h.flag = true;
        h.a = 2;
        h.b = 4;
        return h;
    }
}

// Correctness coverage of the field-wise 64-bit split/reassembly path. Both 32-bit halves
// of `big` are nonzero and both observed in `measure()`, so dropping either word fails.

[anyValueSize(16)]
interface IWideScalar
{
    int measure();
}

struct HasI64 : IWideScalar
{
    int64_t big;
    float pad;
    int measure() { return int(big) + int(big >> 32) + int(pad); }
}

struct HasI64Alt : IWideScalar
{
    float pad;
    int64_t big;
    int measure() { return int(big) + int(big >> 32) + int(pad); }
}

IWideScalar makeWideScalar(int id)
{
    if (id == 0)
    {
        HasI64 h;
        h.big = (7LL << 32) + 5LL;
        h.pad = 3;
        return h;
    }
    else
    {
        HasI64Alt h;
        h.big = (2LL << 32) + 9LL;
        h.pad = 1;
        return h;
    }
}

// Fast-path with a transitively-empty aggregate. The empty member legalizes to `none` and
// carries no payload words, so the object stays byte-compatible and boxes with a single
// whole-object bit-cast (see shader-slang/slang#12609). `HasEmpty2` puts the empty member
// first, exercising that the surviving `float` leaves are still placed at offsets 0 and 4.

struct Empty
{
}

[anyValueSize(8)]
interface IEmpty
{
    int measure();
}

struct HasEmpty : IEmpty
{
    float a;
    Empty e;
    float b;
    int measure() { return int(a) + int(b); }
}

struct HasEmpty2 : IEmpty
{
    Empty e;
    float x;
    float y;
    int measure() { return int(x) + int(y); }
}

IEmpty makeEmpty(int id)
{
    if (id == 0)
    {
        HasEmpty h;
        h.a = 3;
        h.b = 4;
        return h;
    }
    else
    {
        HasEmpty2 h;
        h.x = 5;
        h.y = 6;
        return h;
    }
}

// Isolator for the vector-case guard. `HasZeroVec` is two `float`s plus a `vector<float,0>`. The
// zero-width vector contributes no payload words; because the struct case accepts a zero-leaf
// member, only the vector case's own reject keeps the enclosing struct off the fast path. The box
// is exact (two `float` leaves = 8 bytes), so that reject is the *only* thing keeping this value
// field-wise, and the `-NOT` checks below pin it.

[anyValueSize(8)]
interface IZeroVec
{
    int measure();
}

struct HasZeroVec : IZeroVec
{
    float a;
    vector<float, 0> z;
    float b;
    int measure() { return int(a) + int(b); }
}

struct HasZeroVec2 : IZeroVec
{
    vector<float, 0> z;
    float x;
    float y;
    int measure() { return int(x) + int(y); }
}

IZeroVec makeZeroVec(int id)
{
    if (id == 0)
    {
        HasZeroVec h;
        h.a = 3;
        h.b = 4;
        return h;
    }
    else
    {
        HasZeroVec2 h;
        h.x = 5;
        h.y = 6;
        return h;
    }
}

// Reject: an ABI-preserved empty member. `AbiEmpty` is empty but marked `__extern_cpp`, so
// empty-type legalization keeps it instead of dropping it. Slang layout still counts it as zero
// bytes — two `float` leaves, an exact 8-byte box — but the emitted C++ retains `AbiEmpty` with a
// nonzero footprint and shifts the second `float`, so a whole-object copy would move the wrong
// bytes. The conformer must therefore stay field-wise; the `-NOT` checks below pin that.
// `HasAbiEmpty2` puts the surviving empty first, so the reject cannot depend on member order.

__extern_cpp struct AbiEmpty
{
}

[anyValueSize(8)]
interface IAbiEmpty
{
    int measure();
}

struct HasAbiEmpty : IAbiEmpty
{
    float a;
    AbiEmpty e;
    float b;
    int measure() { return int(a) + int(b); }
}

struct HasAbiEmpty2 : IAbiEmpty
{
    AbiEmpty e;
    float x;
    float y;
    int measure() { return int(x) + int(y); }
}

IAbiEmpty makeAbiEmpty(int id)
{
    if (id == 0)
    {
        HasAbiEmpty h;
        h.a = 3;
        h.b = 4;
        return h;
    }
    else
    {
        HasAbiEmpty2 h;
        h.x = 5;
        h.y = 6;
        return h;
    }
}

// Reject: an inexact fit. `InexactSmall` (4 bytes) is smaller than its 12-byte box, so a
// bulk copy would leave the high payload words the field-wise path zero-inits undefined.
// `InexactBig` (12 bytes) fits exactly and fast-paths, as a positive control.

[anyValueSize(12)]
interface IInexact
{
    int measure();
}

struct InexactBig : IInexact
{
    float a;
    float b;
    float c;
    int measure() { return int(a) + int(b) + int(c); }
}

struct InexactSmall : IInexact
{
    float a;
    int measure() { return int(a); }
}

IInexact makeInexact(int id)
{
    if (id == 0)
    {
        InexactBig s;
        s.a = 2;
        s.b = 3;
        s.c = 4;
        return s;
    }
    else
    {
        InexactSmall s;
        s.a = 13;
        return s;
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

    // Reject-path conformers: correctness must survive the field-wise round-trip.
    IScalarLeaf e = makeScalarLeaf(0);
    outputBuffer[5] = e.measure();
    // CHECK: 104

    IScalarLeaf f = makeScalarLeaf(1);
    outputBuffer[6] = f.measure();
    // CHECK: 16

    IWideScalar ws0 = makeWideScalar(0);
    outputBuffer[7] = ws0.measure();
    // CHECK: 15

    IWideScalar ws1 = makeWideScalar(1);
    outputBuffer[8] = ws1.measure();
    // CHECK: 12

    IEmpty g = makeEmpty(0);
    outputBuffer[9] = g.measure();
    // CHECK: 7

    IEmpty h = makeEmpty(1);
    outputBuffer[10] = h.measure();
    // CHECK: 11

    IInexact i = makeInexact(0);
    outputBuffer[11] = i.measure();
    // CHECK: 9

    IInexact j = makeInexact(1);
    outputBuffer[12] = j.measure();
    // CHECK: 13

    IZeroVec k = makeZeroVec(0);
    outputBuffer[13] = k.measure();
    // CHECK: 7

    IZeroVec l = makeZeroVec(1);
    outputBuffer[14] = l.measure();
    // CHECK: 11

    IWordKinds m = makeWordKinds(0);
    outputBuffer[15] = m.measure();
    // CHECK: 10

    IWordKinds n = makeWordKinds(1);
    outputBuffer[16] = n.measure();
    // CHECK: 26

    IWordKinds o = makeWordKinds(2);
    outputBuffer[17] = o.measure();
    // CHECK: 16

    IAbiEmpty p = makeAbiEmpty(0);
    outputBuffer[18] = p.measure();
    // CHECK: 7

    IAbiEmpty q = makeAbiEmpty(1);
    outputBuffer[19] = q.measure();
    // CHECK: 11
}

// Each check is anchored to a specific pack/unpack function via CHECK-LABEL so the
// whole-object cast (or its absence) is bound to the concrete type that owns it, rather
// than counting casts module-wide. The generated function-name suffix (`_i`) depends on
// marshalling-function emission order, so every group matches it with a `{{[0-9]+}}` regex
// and binds the function by its concrete parameter/return type instead.

// --- IShape (Circle, Trio): 4-byte aligned, fast path on BOTH targets, both
// directions. The pack returns bit_cast<AnyValueN>; the unpack returns
// bit_cast<ConcreteType>. ---
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(Circle_0
// CUDA: slang_bit_cast<AnyValue16>
// CUDA-LABEL: Circle_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA: slang_bit_cast<Circle_0>
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(Trio_0
// CUDA: slang_bit_cast<AnyValue16>
// CUDA-LABEL: Trio_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA: slang_bit_cast<Trio_0>

// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(Circle_0
// CPP: slang_bit_cast<AnyValue16>
// CPP-LABEL: Circle_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP: slang_bit_cast<Circle_0>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(Trio_0
// CPP: slang_bit_cast<AnyValue16>
// CPP-LABEL: Trio_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP: slang_bit_cast<Trio_0>

// --- IWide (Wide, WideAlt): a float4 is 16-byte aligned on CUDA (payload is only
// 4-byte aligned), so the fast path must NOT fire — the CUDA-NOT lines are scoped to
// the Wide/WideAlt pack functions by the surrounding CHECK-LABELs. On CPP a float4 is
// 4-byte aligned, so the fast path DOES fire in that pack function. ---
// Each CUDA-NOT is bounded by the following CHECK-LABEL, so it is scoped to exactly
// one function; the unpack labels also assert the unpack side does not fast-path.
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(Wide_0
// CUDA-NOT: slang_bit_cast<AnyValue16>
// CUDA-LABEL: Wide_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<Wide_0>
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(WideAlt_0
// CUDA-NOT: slang_bit_cast<AnyValue16>
// CUDA-LABEL: WideAlt_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<WideAlt_0>

// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(Wide_0
// CPP: slang_bit_cast<AnyValue16>
// CPP-LABEL: Wide_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP: slang_bit_cast<Wide_0>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(WideAlt_0
// CPP: slang_bit_cast<AnyValue16>
// CPP-LABEL: WideAlt_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP: slang_bit_cast<WideAlt_0>

// --- IMatrix (MatA, MatB): column-major, not byte-compatible on any target, so the
// pack stays field-wise (no whole-object AnyValue24 cast) in the pack functions, and
// the unpack returns no whole-object concrete cast, on both targets. ---
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(MatA_0
// CUDA-NOT: slang_bit_cast<AnyValue
// CUDA-LABEL: MatA_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<MatA_0>
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(MatB_0
// CUDA-NOT: slang_bit_cast<AnyValue
// CUDA-LABEL: MatB_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<MatB_0>

// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(MatA_0
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: MatA_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<MatA_0>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(MatB_0
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: MatB_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<MatB_0>

// The generated function-name suffix (`_i`) depends on marshalling-function emission
// order, so the labels below match it with a `{{[0-9]+}}` regex and bind each function
// by its concrete parameter/return type instead, so an unrelated conformer added later
// cannot shift a suffix and cause a false failure.

// IScalarLeaf (bool leaf, above): the -NOT targets only the whole-object cast; the
// field-wise bodies still contain per-leaf `slang_bit_cast<uint>`, which is the safe path.
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasBool_0
// CUDA-NOT: slang_bit_cast<AnyValue
// CUDA-LABEL: HasBool_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<HasBool_0>
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasBoolAlt_0
// CUDA-NOT: slang_bit_cast<AnyValue
// CUDA-LABEL: HasBoolAlt_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<HasBoolAlt_0>

// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasBool_0
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: HasBool_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<HasBool_0>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasBoolAlt_0
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: HasBoolAlt_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<HasBoolAlt_0>

// IWideScalar (int64 leaf, above): the split path must stay field-wise.
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasI64_0
// CUDA-NOT: slang_bit_cast<AnyValue
// CUDA-LABEL: HasI64_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<HasI64_0>
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasI64Alt_0
// CUDA-NOT: slang_bit_cast<AnyValue
// CUDA-LABEL: HasI64Alt_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<HasI64Alt_0>

// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasI64_0
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: HasI64_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<HasI64_0>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasI64Alt_0
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: HasI64Alt_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<HasI64Alt_0>

// IEmpty (empty-aggregate leaf, above): the empty member legalizes away, leaving a
// byte-compatible object, so pack/unpack fast-path with a single whole-object bit-cast.
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasEmpty_0
// CUDA: slang_bit_cast<AnyValue8>
// CUDA-LABEL: HasEmpty_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA: slang_bit_cast<HasEmpty_0>
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasEmpty2_0
// CUDA: slang_bit_cast<AnyValue8>
// CUDA-LABEL: HasEmpty2_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA: slang_bit_cast<HasEmpty2_0>

// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasEmpty_0
// CPP: slang_bit_cast<AnyValue8>
// CPP-LABEL: HasEmpty_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP: slang_bit_cast<HasEmpty_0>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasEmpty2_0
// CPP: slang_bit_cast<AnyValue8>
// CPP-LABEL: HasEmpty2_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP: slang_bit_cast<HasEmpty2_0>

// IInexact (above): InexactBig fits and fast-paths (control); InexactSmall must not.
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(InexactBig_0
// CUDA: slang_bit_cast<AnyValue
// CUDA-LABEL: InexactBig_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA: slang_bit_cast<InexactBig_0>
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(InexactSmall_0
// CUDA-NOT: slang_bit_cast<AnyValue
// CUDA-LABEL: InexactSmall_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<InexactSmall_0>

// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(InexactBig_0
// CPP: slang_bit_cast<AnyValue
// CPP-LABEL: InexactBig_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP: slang_bit_cast<InexactBig_0>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(InexactSmall_0
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: InexactSmall_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<InexactSmall_0>

// IZeroVec (nested zero-width vector, above): pack/unpack must stay field-wise. Without the
// `total > 0 ? total : -1` guard on the *vector* case, the zero-width vector's zero leaves would
// sum into the struct total — which now accepts a zero-leaf member — so `HasZeroVec`/`HasZeroVec2`
// would take the whole-object bit-cast.
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasZeroVec_0
// CUDA-NOT: slang_bit_cast<AnyValue
// CUDA-LABEL: HasZeroVec_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<HasZeroVec_0>
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasZeroVec2_0
// CUDA-NOT: slang_bit_cast<AnyValue
// CUDA-LABEL: HasZeroVec2_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<HasZeroVec2_0>

// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasZeroVec_0
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: HasZeroVec_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<HasZeroVec_0>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasZeroVec2_0
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: HasZeroVec2_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<HasZeroVec2_0>

// --- IWordKinds (UintQuad, ArrayLeaf, EnumLeaf): positive controls for the uint
// direct-store branch and the array/enum recursion arms — all 4-byte word leaves, dense at
// 16 bytes, so each fast-paths on BOTH targets in both directions. ---
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(UintQuad_0
// CUDA: slang_bit_cast<AnyValue16>
// CUDA-LABEL: UintQuad_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA: slang_bit_cast<UintQuad_0>
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(ArrayLeaf_0
// CUDA: slang_bit_cast<AnyValue16>
// CUDA-LABEL: ArrayLeaf_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA: slang_bit_cast<ArrayLeaf_0>
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(EnumLeaf_0
// CUDA: slang_bit_cast<AnyValue16>
// CUDA-LABEL: EnumLeaf_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA: slang_bit_cast<EnumLeaf_0>

// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(UintQuad_0
// CPP: slang_bit_cast<AnyValue16>
// CPP-LABEL: UintQuad_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP: slang_bit_cast<UintQuad_0>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(ArrayLeaf_0
// CPP: slang_bit_cast<AnyValue16>
// CPP-LABEL: ArrayLeaf_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP: slang_bit_cast<ArrayLeaf_0>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(EnumLeaf_0
// CPP: slang_bit_cast<AnyValue16>
// CPP-LABEL: EnumLeaf_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP: slang_bit_cast<EnumLeaf_0>

// IAbiEmpty (ABI-preserved empty member, above): the `__extern_cpp` empty survives legalization and
// occupies a nonzero footprint in the emitted struct, so pack/unpack must stay field-wise on both
// targets (the reject is target-independent — `AbiEmpty` is 4-byte-compatible in size otherwise).
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasAbiEmpty_0
// CUDA-NOT: slang_bit_cast<AnyValue
// CUDA-LABEL: HasAbiEmpty_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<HasAbiEmpty_0>
// CUDA-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasAbiEmpty2_0
// CUDA-NOT: slang_bit_cast<AnyValue
// CUDA-LABEL: HasAbiEmpty2_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CUDA-NOT: slang_bit_cast<HasAbiEmpty2_0>

// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasAbiEmpty_0
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: HasAbiEmpty_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<HasAbiEmpty_0>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(HasAbiEmpty2_0
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: HasAbiEmpty2_0 unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<HasAbiEmpty2_0>
