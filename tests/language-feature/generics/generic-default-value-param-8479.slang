//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

// Regression test for https://github.com/shader-slang/slang/issues/8479
// An unsuffixed integer literal default for a generic value parameter folded to a
// ConstantIntVal carrying the literal's own type instead of the parameter's declared
// type, so the element type of `y.hits` (specialized through the implicit default)
// compared unequal to the local annotation `Payload.Hit` (specialized through the
// solver), producing the identical-printing error:
//   error 30019: expected an expression of type 'Payload<16>.Hit', got 'Payload<16>.Hit'

struct Payload<uint32_t x = 16>
{
    struct Hit
    {
        int value;
    }
    Hit hits[x];
}

uniform Payload y;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // The issue repro: nested-type readback through the default-specialized uniform.
    Payload.Hit h = y.hits[0];

    // Explicit spelling must denote the same type as the implicit default.
    Payload<16> z;
    z.hits[0].value = 2;
    Payload.Hit h2 = z.hits[0];

    outputBuffer[0] = h.value + h2.value + 40;
    // CHECK: 42
}
