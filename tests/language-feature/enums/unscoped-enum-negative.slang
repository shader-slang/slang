//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -DDEFAULT_UNSCOPED=0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -unscoped-enum -DDEFAULT_UNSCOPED=1

// Complementary test for unscoped-enum.slang. This test verifies that
// an enumeration is not unscoped in the following cases:
// - default is scoped enums, regular enum without the [UnscopedEnum] attribute
// - default is unscoped enums but we're using an enum class, which is always scoped

#if DEFAULT_UNSCOPED

// enum class is always scoped, so we use it when the default is unscoped
enum class Fruit
{
    Orange, Apple
}

#else

// default is scoped
enum Fruit
{
    Orange, Apple
}

#endif


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = (int)Apple;
//CHECK:                   ^^^^^ undefined identifier
//CHECK:                   ^^^^^ undefined identifier 'Apple'.
}
