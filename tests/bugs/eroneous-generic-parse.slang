//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=16):out
RWStructuredBuffer<int> outputBuffer;

// Previously this definition would lead to an infinite loop in parsing.
int doThing<1>() { return 2; }
//CHECK:    ^ unexpected token
//CHECK:    ^ unexpected integer literal, expected identifier

[numthreads(4, 4, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECK:
     ^^^^^^^^^^^ unexpected token
     ^^^^^^^^^^^ unexpected identifier, expected '>'
*/
{
    int index = dispatchThreadID.x;

    outputBuffer[index] = index;
}