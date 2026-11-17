// Regression test for #13466: dead-store removal drops a store when a side-effect-free call
// between it and a later store to the same address reads that address through a pointer. `rd`
// is inferred NoSideEffect (it only loads), and it reads `*q` without receiving `q`, so the
// first store is not dead: `rd` must observe 1, and the expected result is 1.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -g0
//TEST(compute, cuda):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -output-using-type -g0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-directly -Xslang -g0

struct P { uint* p; }
//TEST_INPUT: set cb = new P{ ubuffer(data=[0], stride=4) }
ConstantBuffer<P> cb;
//TEST_INPUT: set outb = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outb;

[noinline] uint rd() { return *cb.p; }

[noinline] uint pointerRoot()
{
    uint* q = cb.p;
    *q = 1;
    uint r = rd();     // rd reads *q: expected 1
    *q = 2;
    return r;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outb[0] = pointerRoot();
}
// BUF: type: uint32_t
// BUF-NEXT: {{^}}1{{$}}
