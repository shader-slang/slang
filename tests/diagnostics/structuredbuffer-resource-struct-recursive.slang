//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct RecursiveFoo
{
    float value;
    //CHECK-DAG: ([[# @LINE+1]]): error 38204
    StructuredBuffer<RecursiveFoo> children;
}

struct MutualB
{
    //CHECK-DAG: ([[# @LINE+1]]): error 38204
    StructuredBuffer<MutualA> aBuffer;
}

struct MutualA
{
    //CHECK-DAG: ([[# @LINE+1]]): error 38204
    StructuredBuffer<MutualB> bBuffer;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
