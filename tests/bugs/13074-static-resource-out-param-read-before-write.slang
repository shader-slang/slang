//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target cpp -entry testMain -stage compute -validate-ir

Texture2D<float4> inputTexture;
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTexture;

[noinline]
void readThenReplace(out Texture2D<float4> value)
{
    Texture2D<float4>* address = (Texture2D<float4>*)&value;
    output[0] = (*address).Load(int3(0, 0, 0));
//CHECK:                       ^ use of uninitialized out parameter
//CHECK:                       ^ use of uninitialized out parameter 'value'
    value = inputTexture;
}

[numthreads(1, 1, 1)]
void testMain()
{
    readThenReplace(cachedTexture);
//CHECK:           ^ use of uninitialized variable
//CHECK:           ^ use of uninitialized variable 'cachedTexture'

    // The call reads the incoming value, but a normal return also assigns the complete `out`
    // parameter. The read above is invalid; the read after the call is valid.
    output[1] = cachedTexture.Load(int3(0, 0, 0));
}
