//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct MutualB
{
    StructuredBuffer<MutualA> aBuffer;
//CHECK:                      ^^^^^^^ recursive type in structured buffer
//CHECK:                      ^^^^^^^ structured buffer element type 'MutualA' contains recursive type references
}

struct MutualA
{
    StructuredBuffer<MutualB> bBuffer;
//CHECK:                      ^^^^^^^ recursive type in structured buffer
//CHECK:                      ^^^^^^^ structured buffer element type 'MutualB' contains recursive type references
}

StructuredBuffer<MutualA> mutualRoot;
//CHECK:                  ^^^^^^^^^^ recursive type in structured buffer
//CHECK:                  ^^^^^^^^^^ structured buffer element type 'MutualA' contains recursive type references
RWStructuredBuffer<float> output;

// External function to force consumption of recursive types
float consumeMutual(MutualA a, MutualB b);

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;

    float result = 0;
    // Force usage of mutual recursion
    MutualA a = mutualRoot[i];
    MutualB b = a.bBuffer[0];
    result += consumeMutual(a, b);

    output[i] = result;
}

