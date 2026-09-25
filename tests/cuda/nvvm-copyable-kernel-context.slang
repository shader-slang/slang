//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[5 8 11 14 17 20 23 26 29 32 35 38 41 44 47 50 53 56 59 62 65 68 71 74 77 80 83 86 89 92 95 98], stride=4):name=inputs
StructuredBuffer<uint> inputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct InnerState
{
    bool enabled;
    double amount;
}
struct State
{
    InnerState inner;
    float2 values;
    uint payload[2];
    bool other;
}
static State state;
static bool flag = false;
static uint visits = 0;

[noinline]
bool initializeAndSet(uint value)
{
    bool ok = !flag && !state.inner.enabled && !state.other && visits == 0;
    ok &= state.inner.amount == 0.0 && all(state.values == float2(0));
    ok &= state.payload[0] == 0 && state.payload[1] == 0;
    flag = (value & 1) != 0;
    state.inner.enabled = (value & 2) != 0;
    // Distinguish adjacent doubles by preserving the dynamically loaded low word.
    state.inner.amount = bit_cast<double>((uint64_t(0x3FF00000u) << 32) | uint64_t(value));
    state.values = float2(float(value) + 0.5, -float(value) - 0.5);
    state.payload[0] = value * 3 + 1;
    state.payload[1] = value ^ 0x55u;
    state.other = !flag;
    visits = 1;
    return ok;
}

[noinline]
void mutateState()
{
    visits += 2;
    flag = !flag;
    state.inner.enabled = !state.inner.enabled;
    state.inner.amount = -state.inner.amount;
    state.values = state.values.yx;
    uint temporary = state.payload[0];
    state.payload[0] = state.payload[1];
    state.payload[1] = temporary;
    state.other = !state.other;
}

[noinline]
bool checkState(uint value, bool mutated)
{
    bool expectedFlag = ((value & 1) != 0) != mutated;
    bool expectedEnabled = ((value & 2) != 0) != mutated;
    uint64_t expectedBits = (uint64_t(mutated ? 0xBFF00000u : 0x3FF00000u) << 32) | uint64_t(value);
    float first = float(value) + 0.5;
    bool ok = flag == expectedFlag && state.inner.enabled == expectedEnabled;
    ok &= state.other == !expectedFlag && visits == (mutated ? 3 : 1);
    ok &= bit_cast<uint64_t>(state.inner.amount) == expectedBits;
    ok &= all(state.values == (mutated ? float2(-first, first) : float2(first, -first)));
    ok &= state.payload[0] == (mutated ? value ^ 0x55u : value * 3 + 1);
    ok &= state.payload[1] == (mutated ? value * 3 + 1 : value ^ 0x55u);
    return ok;
}

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint value = inputs[tid.x];
    bool ok = initializeAndSet(value);
    ok &= checkState(value, false);
    mutateState();
    ok &= checkState(value, true);
    outputBuffer[tid.x] = ok ? 6400 + tid.x : 0;
}

// CHECK: 6400
// CHECK: 6401
// CHECK: 6402
// CHECK: 6403
// CHECK: 6404
// CHECK: 6405
// CHECK: 6406
// CHECK: 6407
// CHECK: 6408
// CHECK: 6409
// CHECK: 6410
// CHECK: 6411
// CHECK: 6412
// CHECK: 6413
// CHECK: 6414
// CHECK: 6415
// CHECK: 6416
// CHECK: 6417
// CHECK: 6418
// CHECK: 6419
// CHECK: 6420
// CHECK: 6421
// CHECK: 6422
// CHECK: 6423
// CHECK: 6424
// CHECK: 6425
// CHECK: 6426
// CHECK: 6427
// CHECK: 6428
// CHECK: 6429
// CHECK: 6430
// CHECK: 6431
