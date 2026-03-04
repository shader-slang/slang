//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -emit-spirv-directly

RWStructuredBuffer<float> outputBuffer;

float testFunc(vector<vector<float, 3>, 2> v1)
//CHECK:                                   ^^ unsupported builtin type
//CHECK:                                   ^^ 'vector<vector<float,3>,2>' is not a supported builtin type for the target.
{
    return v1.x.x;
}

[numthreads(1,1,1)]
void main(uint v : SV_DispatchThreadID)
{
    vector<vector<float, 3>, 2> v1 = { { 1, 2, 3 }, { 4, 5, v } };
//CHECK:                        ^^ disallowed vector element type
//CHECK:                        ^^ vector with disallowed element type 'vector<float,3>' encountered
    outputBuffer[0] = testFunc(v1);
}