// Test for issue #9836: Interface implementation containing pointer to interface
//
// This test verifies:
// 1. A struct implementing IFoo can have an IFoo* field (compiles)
// 2. True dynamic dispatch works when concrete type is runtime-determined
// 3. The IFoo* field participates correctly in existential packing/dispatch
//
// Unlike circular-pointer-breaks-cycle.slang (which tests cycle-breaking on
// CPU only), this test validates multi-backend support (CPU, CUDA, Vulkan, LLVM)
// with explicit type_conformance registration and runtime type selection.
//
// Note: The IFoo* pointer is only initialized to nullptr here. Testing actual
// pointer dereferencing (ptr->foo()) requires slang-test infrastructure to
// embed device addresses in buffer data. See #10015 for follow-up.
//
// anyValueSize is intentionally omitted to test the auto-computation path.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute -shaderobj -output-using-type

// Verify dynamic dispatch is actually generated (not optimized away)
//TEST:SIMPLE(filecheck=DISPATCH):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

// Input buffer to control which type is selected at runtime
// This forces TRUE dynamic dispatch - compiler cannot optimize it away
//TEST_INPUT:ubuffer(data=[1 2 1 2], stride=4):name=typeSelector
RWStructuredBuffer<uint> typeSelector;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    float foo();
}

// Implementation with pointer to interface - this is the key test case
// The pointer has fixed size (8 bytes) regardless of pointee type
struct GoodImpl : IFoo
{
    IFoo* next;  // OK: pointer to interface - fixed size
    float value;

    float foo() { return value; }
}

// Simple implementation without pointer for comparison
struct SimpleImpl : IFoo
{
    float value;
    float foo() { return value * 2.0; }
}

// Register type conformances - these IDs are used for runtime dispatch
//TEST_INPUT: type_conformance GoodImpl:IFoo = 1
//TEST_INPUT: type_conformance SimpleImpl:IFoo = 2

// Helper function that forces dynamic dispatch
// The concrete type depends on runtime value 'typeId'
float callThroughInterface(uint typeId, float inputValue)
{
    IFoo obj;
    
    // Runtime type selection - compiler CANNOT know which branch is taken
    if (typeId == 1)
    {
        GoodImpl g;
        g.value = inputValue;
        g.next = nullptr;  // Initialize pointer (not dereferenced in this test)
        obj = g;
    }
    else
    {
        SimpleImpl s;
        s.value = inputValue;
        obj = s;
    }
    
    // TRUE dynamic dispatch - compiler must emit switch on type tag
    return obj.foo();
    // DISPATCH: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test 1: GoodImpl (with IFoo* field) through dynamic dispatch
    // typeSelector[0] = 1 → GoodImpl, value = 42.0
    outputBuffer[0] = callThroughInterface(typeSelector[0], 42.0);  // CHECK: 42.0
    
    // Test 2: SimpleImpl through dynamic dispatch  
    // typeSelector[1] = 2 → SimpleImpl, value = 25.0, result = 25.0 * 2.0 = 50.0
    outputBuffer[1] = callThroughInterface(typeSelector[1], 25.0);  // CHECK: 50.0
    
    // Test 3: GoodImpl again to verify consistency
    // typeSelector[2] = 1 → GoodImpl, value = 10.0
    outputBuffer[2] = callThroughInterface(typeSelector[2], 10.0);  // CHECK: 10.0
    
    // Test 4: SimpleImpl again
    // typeSelector[3] = 2 → SimpleImpl, value = 4.0, result = 4.0 * 2.0 = 8.0
    outputBuffer[3] = callThroughInterface(typeSelector[3], 4.0);   // CHECK: 8.0
}
