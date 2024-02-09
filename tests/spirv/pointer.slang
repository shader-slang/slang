//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute -emit-spirv-directly


struct PP
{
    int data;
    int data2;
}
struct Data
{
    int data;
    PP* pNext;
};

void funcThatTakesPointer(PP* p)
{
    p.data = 2;
}
int* funcThatReturnsPointer(PP* p)
{
    return &p.data;
}

// CHECK: OpEntryPoint

StructuredBuffer<Data> buffer;
RWStructuredBuffer<int> output;
void main(int id : SV_DispatchThreadID)
{
    output[0] = buffer[0].pNext.data;
    let pData = &(buffer[0].pNext.data);
    // CHECK: OpPtrAccessChain
    int* pData1 = pData + 1;
    *pData1 = 3;
    *(int2*)pData = int2(1, 2);
    pData1[-1] = 2;
    buffer[0].pNext[1] = {5};
    // CHECK: OpConvertPtrToU
    // CHECK: OpINotEqual
    if (pData1)
    {
        *(funcThatReturnsPointer(buffer[0].pNext)) = 4;
    }
    if (pData1 > pData)
    {
        funcThatTakesPointer(buffer[0].pNext);
    }
}
