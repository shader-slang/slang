//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda -compute
//TEST:SIMPLE(filecheck=CUDA): -stage compute -entry computeMain -target cuda

struct Data
{
    int val;

    __init(int val)
    {
        this.val = val;
    }
};

struct DataWrapped
{
    Data field;
    Data element[2];

    __init(int val)
    {
        field.val = val;
        element[0].val = val;
        element[1].val = val;
    }
}

// Passed as const-ref due to un-mutated `data`
// CUDA: addCopyElision{{.*}}const{{.*}}Data{{.*}}*{{.*}}data
int addCopyElision(Data data, int val)
{
    return data.val + val;
}

//TEST_INPUT:uniform(data=[1]):name=globalData
uniform Data globalData;

//TEST_INPUT: set input = ubuffer(data=[1 2 3 4], stride=4)
RWStructuredBuffer<int> input;

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Data data = Data(input[0]);
    int val1 = addCopyElision(data, input[1]);

    DataWrapped dataWrapped = DataWrapped(input[0]);
    int val2 = addCopyElision(dataWrapped.field, input[1]);
    int val3 = addCopyElision(dataWrapped.element[0], input[1]);

    int val4 = addCopyElision(globalData, input[1]);

    output[0] = 
        val1 == 3 &&
        val2 == 3 &&
        val3 == 3 &&
        val4 == 3
        ? 1 : 0;
}

//BUF: 1