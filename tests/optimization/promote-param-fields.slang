// Large aggregate `in` parameters of which a callee reads only a few fields are
// split into just those fields, so call sites do not copy the whole aggregate.

//TEST:SIMPLE(filecheck=CUDA): -stage compute -entry computeMain -target cuda -line-directive-mode none -O3
//TEST:SIMPLE(filecheck=HLSL): -stage compute -entry computeMain -target hlsl -line-directive-mode none -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type -Xslang -O3

struct Inner
{
    uint x;
    uint y;
};

struct State
{
    uint a;
    uint unused[64];
    Inner inner;
    uint arr[4];
    uint b;
};

struct Small
{
    uint x;
    uint y;
};

//TEST_INPUT:ubuffer(data=[3 5 11 13], stride=4):name=inputs
StructuredBuffer<uint> inputs;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

// Only `a` and `b` are read: the parameter becomes two `uint`s.
// CUDA: readFields{{[_0-9]*}}(uint snapshot_a{{[_0-9]*}}, uint snapshot_b{{[_0-9]*}}, State{{[_0-9]*}} * original
// HLSL: readFields{{[_0-9]*}}(uint snapshot_a{{[_0-9]*}}, uint snapshot_b{{[_0-9]*}}, inout State{{[_0-9]*}} original
[noinline]
uint readFields(State snapshot, inout State original)
{
    original.a += 7;
    return snapshot.a + snapshot.b;
}

// Nested field and constant-index paths are promoted individually.
// CUDA: readNested{{[_0-9]*}}(uint s_inner_y{{[_0-9]*}}, uint s_arr_2{{[_0-9]*}})
[noinline]
uint readNested(State s)
{
    return s.inner.y * 100 + s.arr[2];
}

// A parameter that is only forwarded to a callee whose parameter was promoted
// is promoted too.
// CUDA: forward{{[_0-9]*}}(uint s_inner_y{{[_0-9]*}}, uint s_arr_2{{[_0-9]*}})
[noinline]
uint forward(State s)
{
    return readNested(s) + 1;
}

// A parameter used as a whole is left alone.
// CUDA: useWhole{{[_0-9]*}}(State{{[_0-9]*}} * s
[noinline]
State useWhole(State s)
{
    State r = s;
    r.b += 1;
    return r;
}

// A dynamic index can read any element of `unused`, so the whole array would have
// to be passed. That is most of `State`, so the parameter is left alone.
// CUDA: readDynamic{{[_0-9]*}}(State{{[_0-9]*}} * s{{[_0-9]*}}, uint i
[noinline]
uint readDynamic(State s, uint i)
{
    return s.unused[i] + s.a;
}

// Small aggregates are cheap to copy and are left alone.
// CUDA: readSmall{{[_0-9]*}}(Small{{[_0-9]*}} * s
[noinline]
uint readSmall(Small s)
{
    return s.x;
}

// A parameter that is never read is removed.
// CUDA: ignoreState{{[_0-9]*}}(uint v
[noinline]
uint ignoreState(State s, uint v)
{
    return v;
}

// Call sites read the promoted fields directly instead of copying `State`
// into a temporary.
// CUDA: readFields{{[_0-9]*}}({{.*}}original{{[_0-9]*}})->a{{[_0-9]*}}, {{.*}}original{{[_0-9]*}})->b{{[_0-9]*}}, &original
// CUDA: readNested{{[_0-9]*}}({{.*}}->y{{[_0-9]*}}, {{.*}}->arr{{[_0-9]*}}[int(2)])
// CUDA: forward{{[_0-9]*}}({{.*}}->y{{[_0-9]*}}, {{.*}}->arr{{[_0-9]*}}[int(2)])

[numthreads(1, 1, 1)]
void computeMain()
{
    State original = {};
    original.a = inputs[0];
    original.b = inputs[1];
    original.inner.y = inputs[2];
    original.arr[2] = inputs[3];

    // `snapshot` sees the values at the call, before the callee adds 7 to `original`.
    State captured = original;
    // BUF: 8
    output[0] = readFields(captured, original);
    // BUF-NEXT: 10
    output[1] = original.a;
    // BUF-NEXT: 1113
    output[2] = readNested(original);
    // BUF-NEXT: 1114
    output[3] = forward(original);
    // BUF-NEXT: 6
    output[4] = useWhole(original).b;
    // BUF-NEXT: 0
    output[5] = original.unused[0];
    // BUF-NEXT: 10
    output[6] = readDynamic(original, inputs[3] - 13);

    Small small = { inputs[0], inputs[1] };
    // BUF-NEXT: 3
    output[7] = readSmall(small);
    // BUF-NEXT: 42
    output[8] = ignoreState(original, 42);
}
