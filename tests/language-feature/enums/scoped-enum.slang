//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -xslang -DDEFAULT_UNSCOPED=0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -xslang -DDEFAULT_UNSCOPED=1 -xslang -unscoped-enum

// Complementary test for unscoped-enum.slang and unscoped-enum-negative.slang. This just verifies that
// the scoped enums work.

#if DEFAULT_UNSCOPED

// default is unscoped enums, so we need to use 'class' to make it scoped
enum class Fruit
{
    Orange, Apple
}

#else

// default is scoped enums
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
    // CHECK: 1
    outputBuffer[0] = (int)Fruit.Apple;
}
