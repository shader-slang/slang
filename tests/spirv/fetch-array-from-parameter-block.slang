//TEST:SIMPLE(filecheck=CHECK):-target spirv

//CHECK-NOT: OpCompositeConstruct
//CHECK-COUNT-1: OpStore

struct Data
{
    int bigArray[8];
    int fetch(int i)
    {
        return bigArray[i];
    }
}

ParameterBlock<Data> pData;
uniform int* result;

[numthreads(16,1,1)]
void main(int id : SV_DispatchThreadID)
{
    *result = pData.fetch(id);
}