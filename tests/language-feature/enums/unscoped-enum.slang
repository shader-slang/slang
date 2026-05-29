//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -xslang -DDEFAULT_UNSCOPED=0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -xslang -DDEFAULT_UNSCOPED=1 -xslang -unscoped-enum

// Verifies that an enumeration is unscoped either by the attribute or
// by the compiler switch.

#if !DEFAULT_UNSCOPED
// default is scoped enums, so we need the attribute
[UnscopedEnum]
#endif
enum Fruit
{
    Orange, Apple
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 1
    outputBuffer[0] = (int)Apple;
}
