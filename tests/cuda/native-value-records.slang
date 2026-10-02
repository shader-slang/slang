//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -O3 -validate-ir -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -O3 -validate-ir -line-directive-mode none
//TEST:COMPARE_COMPUTE(filecheck-buffer=RESULT): -cuda -compute -output-using-type

struct Small
{
    uint a;
    uint b;
};

struct Fits
{
    uint4 a;
    uint4 b;
    uint4 c;
    uint4 d;
    uint4 e;
    uint4 f;
    uint4 g;
    uint4 h;
};

struct Larger
{
    uint4 a;
    uint4 b;
    uint4 c;
    uint4 d;
    uint4 e;
    uint4 f;
    uint4 g;
    uint4 h;
    uint tail;
};

struct Nested
{
    Fits fields;
};

struct WithArray
{
    uint data[2];
};

[noinline]
uint mutateCopy(Small snapshot, inout Small original)
{
    snapshot.a += 2;
    original.a += 7;
    return snapshot.a + snapshot.b;
}

[noinline]
uint readCopy(Small snapshot, inout Small original)
{
    original.a += 7;
    return snapshot.a + snapshot.b;
}

[noinline]
uint fitRecord(Fits snapshot)
{
    return snapshot.a.x + snapshot.h.w;
}

[noinline]
uint largeRecord(Larger snapshot)
{
    return snapshot.a.x + snapshot.tail;
}

[noinline]
uint nestedRecord(Nested snapshot)
{
    return snapshot.fields.a.x + snapshot.fields.h.w;
}

[noinline]
export uint exportedSmall(Small snapshot)
{
    return snapshot.a + snapshot.b;
}

[noinline]
uint arrayRecord(WithArray snapshot, uint i)
{
    return snapshot.data[i & 1];
}

// Field compaction must not change borrow-in lowering for ordinary records,
// irrespective of size. None of these arguments contains an unused array field
// eligible for compaction. Mutation must continue to observe the captured value.
// CUDA-DAG: mutateCopy_{{[0-9]+}}(Small_{{[0-9]+}} *
// CUDA-DAG: readCopy_{{[0-9]+}}(Small_{{[0-9]+}} *
// CUDA-DAG: fitRecord_{{[0-9]+}}(Fits_{{[0-9]+}} *
// CUDA-DAG: largeRecord_{{[0-9]+}}(Larger_{{[0-9]+}} *
// CUDA-DAG: nestedRecord_{{[0-9]+}}(Nested_{{[0-9]+}} *
// CUDA-DAG: exportedSmall_{{[0-9]+}}(Small_{{[0-9]+}} *
// CUDA-DAG: arrayRecord_{{[0-9]+}}(WithArray_{{[0-9]+}} *
// CPP-DAG: mutateCopy_{{[0-9]+}}(Small_{{[0-9]+}} *

//TEST_INPUT:ubuffer(data=[11 5], stride=4):name=inputs
StructuredBuffer<uint> inputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;
[numthreads(1, 1, 1)]
void computeMain()
{
    Small original = { inputs[0], inputs[1] };
    Small captured = original;
    original.a += 100;
    output[0] = mutateCopy(captured, original) + original.a;
    Fits record = {};
    record.a = uint4(inputs[0], 0, 0, 0);
    record.h = uint4(0, 0, 0, inputs[1]);
    output[1] = fitRecord(record);
    Larger large = {};
    large.a = record.a;
    large.tail = inputs[1];
    output[2] = largeRecord(large);
    WithArray array = { { inputs[0], inputs[1] } };
    output[3] = arrayRecord(array, 0);
    Nested nested = { record };
    output[4] = nestedRecord(nested) + exportedSmall(captured);
    original.a = inputs[0];
    captured = original;
    original.a += 100;
    output[5] = readCopy(captured, original) + original.a;
}
// RESULT: 136
// RESULT: 16
// RESULT: 16
// RESULT: 11
// RESULT: 32
// RESULT: 134
