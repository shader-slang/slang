// Calling a method on a member of a uniform struct copies only that member into the temporary
// the method's `this` points to, not the whole uniform struct (#13732). The `WGSL-NOT` lines
// check that no copy of the whole `Push_std140` value is made.

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -output-using-type

//TEST_INPUT:set gPool = ubuffer(data=[10 11 12 13 20 21 22 23], stride=4)
ByteAddressBuffer gPool;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct Span
{
    uint offset;
    __subscript(uint i)->uint { get { return gPool.Load<uint>(offset + i * 4); } }
};

struct Push
{
    Span a;
    Span b;
    uint n;
};

//TEST_INPUT:set push.a.offset = 0
//TEST_INPUT:set push.b.offset = 16
//TEST_INPUT:set push.n = 1
uniform Push push;

// WGSL-LABEL: fn computeMain
// WGSL-NOT: : Push_std140{{.*}} = {{.*}}push
// WGSL: Span_std140{{.*}} = {{.*}}push{{.*}}.b
// WGSL-NOT: : Push_std140{{.*}} = {{.*}}push
// WGSL: Span_operatorx5Bx5D_get
// WGSL-NOT: : Push_std140{{.*}} = {{.*}}push

[numthreads(4, 1, 1)]
void computeMain(uint3 t : SV_DispatchThreadID)
{
    outputBuffer[t.x] = push.b[t.x] + push.n;
}

// BUF: 21
// BUF-NEXT: 22
// BUF-NEXT: 23
// BUF-NEXT: 24
