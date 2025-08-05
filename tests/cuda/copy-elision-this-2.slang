//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda -compute
//TEST:SIMPLE(filecheck=CUDA): -stage compute -entry computeMain -target cuda -O3

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

//TEST_INPUT:uniform(data=[1]):name=globalData
uniform Data globalData;

//TEST_INPUT: set input = ubuffer(data=[1 2 3 4], stride=4)
RWStructuredBuffer<int> input;

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> output;

// CUDA: addCopyElision{{.*}}Data{{.*}}*{{.*}}data
int addCopyElision(Data data, int val)
{
    // ensure we do not introduce a temporary
    // CUDA-NOT: Data{{.*}};
    return data.val + val;
}

// CUDA: nested{{.*}}Data{{.*}}*{{.*}}data
int nested(Data data, int val)
{

    return addCopyElision(data, val);
}

// CUDA: addCopyElision{{.*}}FixedArray{{.*}}*{{.*}}data
int addCopyElision(int data[10], int val)
{
// ensure we do not introduce a temporary
// CUDA-NOT: {{.*}}FixedArray{{.*}};
    return data[1] + val;
}

// CUDA: nested{{.*}}Array{{.*}}*{{.*}}data
int nested(int data[10], int val)
{
    return addCopyElision(data, val);
}

void modify(inout int data[10])
{
    data[1] = input[0];
}
// CUDA: notDirectlyUsingParam{{.*}}Array{{.*}}*{{.*}}data
int notDirectlyUsingParam(int data[10], int val)
{
// ensure we create a temporary for the array
// CUDA: FixedArray{{.*}};
    modify(data);
    return data[1] + val;
}


// CUDA:computeMain
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{

    // struct
    Data data = Data(input[0]);
    int structVal = addCopyElision(data, input[1]);

    // struct which is globalParam
    int globalParamStructVal = addCopyElision(globalData, input[1]);

    // passing nested struct
    int nestedStructVal = nested(data, input[1]);

    // field
    DataWrapped dataWrapped = DataWrapped(input[0]);
    int fieldVal = addCopyElision(dataWrapped.field, input[1]);

    // element
    int elementVal = addCopyElision(dataWrapped.element[0], input[1]);

    // A non-variable
    int nonVariableVal = addCopyElision(Data(input[0]), input[1]);

    // array
    int val[10];
    val[1] = input[0];
    int arrayVal = addCopyElision(val, input[1]);

    // passing nested array
    int nestedArrayVal = nested(val, input[1]);

    // not directly using param
    int notDirectlyUsingParamVal = notDirectlyUsingParam(val, input[1]);

    output[0] = 
        structVal == 3 &&
        globalParamStructVal == 3 &&
        nestedStructVal == 3 &&
        fieldVal == 3 &&
        elementVal == 3 &&
        nonVariableVal == 3 &&
        arrayVal == 3 &&
        nestedArrayVal == 3 &&
        notDirectlyUsingParamVal == 3
        ? 1 : 0;

    // For debugging
    //output[1] = structVal;
    //output[2] = globalParamStructVal;
    //output[3] = nestedStructVal;
    //output[4] = fieldVal;
    //output[5] = elementVal;
    //output[6] = nonVariableVal;
    //output[7] = arrayVal;
    //output[8] = nestedArrayVal;
    //output[9] = notDirectlyUsingParamVal;
}

//BUF: 1