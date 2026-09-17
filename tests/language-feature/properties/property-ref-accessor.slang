//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// A property `ref` accessor returns the address of its storage. Writes and read-modify-writes
// through the property must therefore update the original field rather than a loaded copy.

struct Cell
{
    int valueStorage;
    property int value
    {
        get { return valueStorage; }
        [mutating] ref { return valueStorage; }
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Cell cell;
    cell.valueStorage = 0;
    cell.value = 13;
    outputBuffer[0] = cell.value;
    cell.value++;
    outputBuffer[1] = cell.value;
    cell.value += 20;
    outputBuffer[2] = cell.value;
    outputBuffer[3] = cell.valueStorage;
}

// CHECK: 13
// CHECK-NEXT: 14
// CHECK-NEXT: 34
// CHECK-NEXT: 34
