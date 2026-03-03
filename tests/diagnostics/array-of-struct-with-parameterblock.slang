// Test that arrays of structs containing ParameterBlocks are properly diagnosed
// with error 30028 instead of causing a compiler crash

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv

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
    MyStruct arr[2];
//CHECK:     ^^^ arrays of non-addressable type not allowed

    // Also test with dynamic arrays
    MyStruct dynamicArr[];
//CHECK:     ^^^^^^^^^^ arrays of non-addressable type not allowed
//CHECK:     ^^^^^^^^^^ cannot instantiate unsized type
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
    OuterStruct nestedArr[3];
//CHECK:        ^^^^^^^^^ arrays of non-addressable type not allowed
}
