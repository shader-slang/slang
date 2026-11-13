//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry computeMain

// An empty struct is normally dropped by empty-type legalization, letting a conformer's remaining
// leaves form the contiguous run the whole-object AnyValue bit-cast copies. But a struct that
// legalization preserves wholesale — here the conformers are `__extern_cpp` — is not decomposed, so
// an *undecorated* empty member inside it also survives, taking a nonzero footprint in the emitted
// C++ and shifting the field after it. Leaf-counting still sees two `float` leaves and an exact
// 8-byte box, so eligibility must reject a zero-leaf member that sits inside a preserved enclosing
// struct, not only one that is itself ABI-decorated. All these conformers must stay on the
// per-field path.
//
// Two shapes, in both member orders:
//  - `OuterPreservedA`/`B`: the empty is a *direct* field of the preserved struct — the reject
//    fires on the struct's own preservation, computed locally in that loop.
//  - `OuterNestedA`/`B`: the empty sits inside an *undecorated* `Inner` struct nested one level
//    deeper inside the preserved outer struct — the reject can only fire via the
//    `enclosingPreserved` flag threaded from the outer struct down into `Inner`, since `Inner`
//    itself carries no decoration. Dropping that threading would wrongly accept these onto the
//    whole-object bit-cast (a silent miscompile: the emitted `Empty` inside `Inner` shifts `f`),
//    so this case is what pins the threaded parameter.

struct Empty
{
}

struct Inner
{
    Empty e;
    float f;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[anyValueSize(8)]
interface IOuterEmpty
{
    int measure();
}

__extern_cpp struct OuterPreservedA : IOuterEmpty
{
    float a;
    Empty e;
    float b;
    int measure() { return int(a) + int(b); }
}

__extern_cpp struct OuterPreservedB : IOuterEmpty
{
    Empty e;
    float x;
    float y;
    int measure() { return int(x) + int(y); }
}

IOuterEmpty makeOuterEmpty(int id)
{
    if (id == 0)
    {
        OuterPreservedA s;
        s.a = 1;
        s.b = 2;
        return s;
    }
    else
    {
        OuterPreservedB s;
        s.x = 3;
        s.y = 4;
        return s;
    }
}

[anyValueSize(8)]
interface INestedEmpty
{
    int measure();
}

__extern_cpp struct OuterNestedA : INestedEmpty
{
    Inner inner;
    float g;
    int measure() { return int(inner.f) + int(g); }
}

__extern_cpp struct OuterNestedB : INestedEmpty
{
    float g;
    Inner inner;
    int measure() { return int(inner.f) + int(g); }
}

INestedEmpty makeNested(int id)
{
    if (id == 0)
    {
        OuterNestedA s;
        s.inner.f = 5;
        s.g = 6;
        return s;
    }
    else
    {
        OuterNestedB s;
        s.g = 8;
        s.inner.f = 7;
        return s;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    IOuterEmpty p0 = makeOuterEmpty(0);
    outputBuffer[0] = p0.measure();
    // CHECK: 3

    IOuterEmpty p1 = makeOuterEmpty(1);
    outputBuffer[1] = p1.measure();
    // CHECK: 7

    INestedEmpty n0 = makeNested(0);
    outputBuffer[2] = n0.measure();
    // CHECK: 11

    INestedEmpty n1 = makeNested(1);
    outputBuffer[3] = n1.measure();
    // CHECK: 15
}

// Emission order follows first-boxing order in computeMain: OuterPreservedA, OuterPreservedB,
// OuterNestedA, OuterNestedB. Each conformer must stay field-wise (no whole-object bit-cast).
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(OuterPreservedA
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: OuterPreservedA unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<OuterPreservedA>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(OuterPreservedB
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: OuterPreservedB unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<OuterPreservedB>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(OuterNestedA
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: OuterNestedA unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<OuterNestedA>
// CPP-LABEL: packAnyValue{{[0-9]+}}_{{[0-9]+}}(OuterNestedB
// CPP-NOT: slang_bit_cast<AnyValue
// CPP-LABEL: OuterNestedB unpackAnyValue{{[0-9]+}}_{{[0-9]+}}(AnyValue
// CPP-NOT: slang_bit_cast<OuterNestedB>
