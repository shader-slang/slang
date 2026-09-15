//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -O3 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -O3 -validate-ir -line-directive-mode none
//TEST:COMPARE_COMPUTE(filecheck-buffer=RESULT): -cuda -compute -output-using-type
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -minimum-slang-optimization -validate-ir -line-directive-mode none

// Keep the call boundary while removing an unused array from value arguments.
// Values must be captured before mutations through the other parameter.
struct State
{
    uint a;
    uint unused[1024];
    uint b;
};

//TEST_INPUT:ubuffer(data=[11 5 9], stride=4):name=inputs
StructuredBuffer<uint> inputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

[noinline]
uint readFields(State snapshot, inout State original)
{
    original.a += 7;
    return snapshot.a + snapshot.b;
}

[noinline]
uint forwardFields(State snapshot, inout State original)
{
    return readFields(snapshot, original);
}

[noinline]
uint readReorderedFields(State snapshot)
{
    return snapshot.b * 10 + snapshot.a + snapshot.a;
}

[noinline]
uint readTwoSnapshots(State first, uint middle, State second, inout State original)
{
    original.a += 7;
    return first.a + second.b + middle;
}

[noinline]
uint mutateValue(State snapshot, inout State original, uint index)
{
    snapshot.unused[index] += 2;
    original.a += 7;
    return snapshot.a + snapshot.b + snapshot.unused[index];
}

// A compact captured value precedes the remaining mutable pointer. The same
// transformation reaches forwardFields after readFields is specialized.
// CUDA-DAG: static __device__ __noinline__ uint readFields_{{[0-9]+}}(ValueFields{{[a-zA-Z0-9_]*}} {{[a-zA-Z0-9_]+}}, State_{{[0-9]+}} *
// CUDA-DAG: static __device__ __noinline__ uint forwardFields_{{[0-9]+}}(ValueFields{{[a-zA-Z0-9_]*}} {{[a-zA-Z0-9_]+}}, State_{{[0-9]+}} *
// CUDA-DAG: static __device__ __noinline__ uint readReorderedFields_{{[0-9]+}}(ValueFields{{[a-zA-Z0-9_]*}} snapshot
// CUDA-DAG: static __device__ __noinline__ uint readTwoSnapshots_{{[0-9]+}}(ValueFields{{[a-zA-Z0-9_]*}} {{[a-zA-Z0-9_]+}}, uint {{[a-zA-Z0-9_]+}}, ValueFields{{[a-zA-Z0-9_]*}} {{[a-zA-Z0-9_]+}}, State_{{[0-9]+}} *
// CUDA-DAG: static __device__ __noinline__ uint mutateValue_{{[0-9]+}}(State_{{[0-9]+}} *

// CUDA-only optimization: the CPU target retains its existing aggregate ABI.
// CPP: readFields_{{[0-9]+}}(State_{{[0-9]+}} *

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    State original = {};
    original.a = inputs[0];
    original.b = inputs[1];
    original.unused[0] = inputs[2];
    State captured = original;
    output[0] = readFields(captured, original) + original.a;

    // The snapshot predates this mutation as well as the mutation inside readFields.
    original.a = inputs[0];
    captured = original;
    original.a += 100;
    output[1] = readFields(captured, original) + original.a;

    original.a = inputs[0];
    captured = original;
    output[2] = forwardFields(captured, original) + original.a;

    original.a = inputs[0];
    output[3] = readReorderedFields(original);

    captured = original;
    State second = original;
    second.b += 1;
    output[4] = readTwoSnapshots(captured, 3, second, original) + original.a;

    original.a = inputs[0];
    captured = original;
    output[5] = mutateValue(captured, original, 0) + original.a + original.unused[0];

}

// RESULT: 34
// RESULT: 134
// RESULT: 34
// RESULT: 72
// RESULT: 38
// RESULT: 54
