//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -O3 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -minimum-slang-optimization -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -O3 -validate-ir -line-directive-mode none
//TEST:COMPARE_COMPUTE(filecheck-buffer=RESULT): -cuda -compute -output-using-type

struct State
{
    uint unused[644];
    uint a;
    uint b;
    uint3 c;
    uint d;
    uint e;
    uint f;
    int g;
    uint h;
    float2 i;
};

[noinline]
uint nineFields(State snapshot)
{
    return snapshot.a + snapshot.b + snapshot.c.x + snapshot.c.y + snapshot.c.z +
           snapshot.d + snapshot.e + snapshot.f + uint(snapshot.g) + snapshot.h +
           uint(snapshot.i.x + snapshot.i.y);
}

// Keep the exported definition while allowing a compact private call path.
[noinline]
export uint exportedFields(State snapshot, inout State original)
{
    original.a += 7;
    return snapshot.a + snapshot.b + snapshot.c.x + snapshot.c.y + snapshot.c.z +
           snapshot.d + snapshot.e + snapshot.f + uint(snapshot.g) + snapshot.h +
           uint(snapshot.i.x + snapshot.i.y);
}

[noinline]
uint forwardFields(State snapshot)
{
    return nineFields(snapshot);
}

[noinline]
uint liveArray(State snapshot, inout State original, uint index)
{
    original.unused[index] += 2;
    return snapshot.unused[index] + snapshot.a;
}

// A specialized caller must still follow its unmodified aggregate callee when
// value parameters are lowered to borrowed pointers.
[noinline]
export uint mixedCalls(State first, uint middle, State second, State whole, inout State original)
{
    return nineFields(first) + middle + nineFields(second) + liveArray(whole, original, 0);
}

[CudaDeviceExport]
uint deviceBoundary(State snapshot)
{
    return snapshot.a + snapshot.b;
}

// CUDA-DAG: struct ValueFields
// Compact payloads still use the existing borrow-in calling convention.
// CUDA-DAG: __noinline__ uint nineFields_{{[0-9]+}}(ValueFields{{[a-zA-Z0-9_]*}} *
// CUDA-DAG: __noinline__ uint exportedFields_compact_{{[0-9]+}}(ValueFields{{[a-zA-Z0-9_]*}} *
// CUDA-DAG: __noinline__ uint exportedFields_{{[0-9]+}}(State_{{[0-9]+}} *
// CUDA-DAG: __noinline__ uint liveArray_{{[0-9]+}}(State_{{[0-9]+}} *
// CUDA-DAG: __device__ uint deviceBoundary(State_{{[0-9]+}}
// CPP-DAG: nineFields_{{[0-9]+}}(State_{{[0-9]+}} *

//TEST_INPUT:ubuffer(data=[1 100], stride=4):name=inputs
StructuredBuffer<uint> inputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    State state = {};
    state.a = inputs[0];
    state.b = 2;
    state.c = uint3(3, 4, 5);
    state.d = 6;
    state.e = 7;
    state.f = 8;
    state.g = 9;
    state.h = 10;
    state.i = float2(11, 12);
    state.unused[0] = inputs[1];

    State captured = state;
    output[0] = nineFields(captured);
    output[5] = deviceBoundary(captured);
    State second = captured;
    second.b += 20;
    State other = state;
    output[6] = mixedCalls(captured, 3, second, captured, other) + other.unused[0];
    state.a += 100;
    output[1] = exportedFields(captured, state) + state.a;
    output[2] = forwardFields(captured);
    output[3] = liveArray(captured, state, 0) + state.unused[0];

    uint sum = 0;
    [unroll]
    for (uint n = 0; n < 33; ++n)
    {
        captured = state;
        sum += exportedFields(captured, state);
    }
    output[4] = sum;
}

// RESULT: 78
// RESULT: 186
// RESULT: 78
// RESULT: 203
// RESULT: 9801
// RESULT: 3
// RESULT: 382
