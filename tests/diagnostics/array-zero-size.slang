// array-zero-size.slang

// Test that array size cannot be zero

//TEST:SIMPLE:

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    bar();
}

func bar() -> int[0];  // expected-error 30025 "array size must be larger than zero." 