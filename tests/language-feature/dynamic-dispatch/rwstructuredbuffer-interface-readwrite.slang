// Writing existential values to RWStructuredBuffer<IFoo> and reading them back
// with dynamic dispatch. This tests the full pack/unpack pipeline: concrete values
// are boxed into existentials, stored to a buffer, loaded back as existentials,
// and dispatched through the interface.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute -shaderobj -output-using-type

[anyValueSize(8)]
interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v + 1.0; }
}

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 3.0; }
}

//TEST_INPUT: type_conformance ImplA:IFoo = 1
//TEST_INPUT: type_conformance ImplB:IFoo = 2

//TEST_INPUT: set rwBuffer = ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<IFoo> rwBuffer;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

ImplA createImplA(float v) { ImplA a; a.v = v; return a; }
ImplB createImplB(float v) { ImplB b; b.v = v; return b; }

//TEST_INPUT: ubuffer(data=[9 4 5], stride=4):name=inputParams
StructuredBuffer<uint> inputParams;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Write concrete types into the RW buffer as existentials
    rwBuffer[0] = createImplA(float(inputParams[0]));
    rwBuffer[1] = createImplB(float(inputParams[1]));

    // Read back and dispatch -- the buffer elements are now existentials
    // whose concrete types must be determined at runtime
    outputBuffer[0] = rwBuffer[0].getValue();  // ImplA: 9.0 + 1.0 = 10.0
    outputBuffer[1] = rwBuffer[1].getValue();  // ImplB: 4.0 * 3.0 = 12.0

    // Overwrite element 0 with ImplB to verify the buffer correctly
    // tracks type changes
    rwBuffer[0] = createImplB(float(inputParams[2]));
    outputBuffer[2] = rwBuffer[0].getValue();  // ImplB: 5.0 * 3.0 = 15.0

    // Verify element 1 is still ImplB
    outputBuffer[3] = rwBuffer[1].getValue();  // ImplB: 4.0 * 3.0 = 12.0

    // CHECK: 10.0
    // CHECK: 12.0
    // CHECK: 15.0
    // CHECK: 12.0
}
