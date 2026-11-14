// Regression test for https://github.com/shader-slang/slang/issues/10477
// DescriptorHandle<T> in dynamic dispatch must not emit as_type<> casts
// between pointer and non-pointer types on Metal. Instead, the pack/unpack
// should go through ulong (C-style cast) to convert pointer <-> integer.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry computeMain -stage compute

interface ISource
{
    float get(int i);
}

struct BufSource : ISource
{
    StructuredBuffer<float>.Handle h;
    float get(int i) { return h[i]; }
}

struct ConstSource : ISource
{
    float v;
    float get(int i) { return v; }
}

ISource make(int kind, StructuredBuffer<float>.Handle h)
{
    if (kind == 0)
    {
        BufSource s;
        s.h = h;
        return s;
    }
    ConstSource c;
    c.v = 1.0;
    return c;
}

uniform StructuredBuffer<float>.Handle buf;
RWStructuredBuffer<float> output;

[numthreads(1,1,1)]
void computeMain()
{
    ISource src = make(0, buf);
    output[0] = src.get(0);
}

// Verify pack uses (ulong) cast, not as_type
// CHECK: (ulong)(
// Verify unpack uses pointer cast from ulong, not as_type
// CHECK: (float device*)(
// Verify no as_type casts involving pointers appear
// CHECK-NOT: as_type<uint2>({{.*}}device
// CHECK-NOT: as_type<float device
