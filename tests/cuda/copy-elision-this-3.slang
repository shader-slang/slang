//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda -compute
//TEST:SIMPLE(filecheck=CUDA): -stage compute -entry computeMain -target cuda

struct Data
{
    int val[10];

    __init(int val)
    {
        this.val[0] = val;
        this.val[1] = val;
    }
};

// Passed as const-ref due to un-mutated `data`
// CUDA: addCopyElision{{.*}}const{{.*}}Array{{.*}}*{{.*}}data
int addCopyElision(int data[10], int val)
{
    return data[1] + val;
}

void someOutFunc(out int data[10])
{
    data[1] = 10;
}

// CUDA-NOT: addNotCopyElision{{.*}}const{{.*}}Array{{.*}}*{{.*}}data
// CUDA: addNotCopyElision{{.*}}Array{{.*}}data
int addNotCopyElision(int data[10], int val)
{
    someOutFunc(data);
    return data[1] + val;
}

//TEST_INPUT: set input = ubuffer(data=[1 2 3 4], stride=4)
RWStructuredBuffer<int> input;

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Data data = Data(input[0]);
    int val1 = addCopyElision(data.val, input[1]);
    int val2 = addNotCopyElision(data.val, input[1]);

    output[0] =
        val1 == 3 &&
        val2 == 12
        ? 1 : 0;

    output[1] = val1;
    output[2] = val2;
}

// BUF: 1