// array-zero-size.slang

// Test that array size cannot be zero

//DIAGNOSTIC_TEST:SIMPLE:

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    bar();
}

func bar() -> int[0];
