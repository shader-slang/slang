// array-zero-size.slang

// Test that array size cannot be zero

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    bar();
}
func bar() -> int[-1];
//CHECK:           ^ array size must be non-negative
//CHECK:           ^ array size must be non-negative.
