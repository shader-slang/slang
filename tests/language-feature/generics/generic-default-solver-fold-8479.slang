//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

// The constraint solver fills a generic function's defaulted value parameter
// when the call omits it (partial application → inference → default
// constraint). The solver-folded default and an explicitly written argument
// must denote the identical type, so the nested-type readback through the
// returned value unifies with the explicit spelling.

struct Box<uint32_t N>
{
    struct Tag
    {
        uint32_t t;
    }
    uint32_t data[N];
    Tag tags[N];
}

Box<N> makeBox<uint32_t N = 16>()
{
    Box<N> b;
    b.data[0] = 1;
    b.tags[0].t = 0;
    return b;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Box<16>.Tag tg = makeBox().tags[0]; // N solver-filled from the default
    Box<16> x = makeBox();              // whole-struct spelling must unify too
    outputBuffer[0] = int(tg.t) + int(x.data[0]);
    // CHECK: 1
}
