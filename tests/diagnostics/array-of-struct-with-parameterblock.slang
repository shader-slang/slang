// Test that arrays of structs containing ParameterBlocks are properly diagnosed
// with error 30027 instead of causing a compiler crash

//TEST:SIMPLE(filecheck=CHECK): -target spirv

// A struct that contains a ParameterBlock member
struct MyStruct
{
    ParameterBlock<IMyInterface> pb;
    int data;
}

interface IMyInterface
{
    int getValue();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // This should trigger error 30027: Arrays of ParameterBlock are not allowed
    // CHECK: ([[# @LINE+1]]): error 30027: Arrays of ParameterBlock are not allowed
    MyStruct arr[2];
    
    // Also test with dynamic arrays
    // CHECK: ([[# @LINE+1]]): error 30027: Arrays of ParameterBlock are not allowed
    MyStruct dynamicArr[];
}

// Test nested structs as well
struct OuterStruct
{
    MyStruct inner;
    float value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain2()
{
    // Nested struct containing ParameterBlock should also trigger the error
    // CHECK: ([[# @LINE+1]]): error 30027: Arrays of ParameterBlock are not allowed
    OuterStruct nestedArr[3];
}