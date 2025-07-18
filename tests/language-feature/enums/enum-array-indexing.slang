//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type

// Test that enums can be used as array indices without explicit casting

enum Fruit { Apple, Orange, Banana };

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    int fruits[10];
    
    // Initialize arrays with some values
    for (int i = 0; i < 10; i++)
    {
        fruits[i] = i * 10;
    }
    
    // Test basic enum indexing - this should work with our fix
    int appleCost = fruits[Fruit::Apple];     // Should access fruits[0] = 0
    int orangeCost = fruits[Fruit::Orange];   // Should access fruits[1] = 10
    int bananaCost = fruits[Fruit::Banana];   // Should access fruits[2] = 20
    
    // CHECK: 0
    outputBuffer[0] = appleCost;
    // CHECK: 10
    outputBuffer[1] = orangeCost;
    // CHECK: 20
    outputBuffer[2] = bananaCost;
    // CHECK: 42
    outputBuffer[3] = 42; // Just a test value
}