// generic-subscript-constraint.slang

// Test generic subscript operators with where clause constraints,
// including both getter and setter functionality.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> output;

interface IIndexable
{
    int toIndex();
}

struct MyIndex : IIndexable
{
    int idx;
    int toIndex() { return idx; }
}

struct MyArray
{
    float data0;
    float data1;
    float data2;
    float data3;

    __subscript<I>(I index) -> float
        where I : IIndexable
    {
        get
        {
            int i = index.toIndex();
            if (i == 0) return data0;
            if (i == 1) return data1;
            if (i == 2) return data2;
            return data3;
        }
        set
        {
            int i = index.toIndex();
            if (i == 0) data0 = newValue;
            else if (i == 1) data1 = newValue;
            else if (i == 2) data2 = newValue;
            else data3 = newValue;
        }
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    MyArray arr;
    arr.data0 = 0.0;
    arr.data1 = 0.0;
    arr.data2 = 0.0;
    arr.data3 = 0.0;

    MyIndex i0; i0.idx = 0;
    MyIndex i1; i1.idx = 1;
    MyIndex i2; i2.idx = 2;

    // Test setter via generic subscript
    arr[i0] = 10.0;
    arr[i1] = 20.0;
    arr[i2] = 30.0;

    // Test getter via generic subscript
    // CHECK: 10.0
    output[0] = arr[i0];

    // CHECK: 20.0
    output[1] = arr[i1];

    // CHECK: 30.0
    output[2] = arr[i2];

    // CHECK: 60.0
    output[3] = arr[i0] + arr[i1] + arr[i2];
}
