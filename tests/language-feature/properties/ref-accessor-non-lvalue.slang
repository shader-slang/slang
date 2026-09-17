//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

struct Cell
{
    int valueStorage;
    property int value
    {
        [mutating] ref { return valueStorage + 1; }
//CHECK:                                     ^ the form of this l-value argument is not valid for a `ref` parameter
    }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Cell cell;
    cell.value = 3;
    outputBuffer[0] = cell.value;
}
