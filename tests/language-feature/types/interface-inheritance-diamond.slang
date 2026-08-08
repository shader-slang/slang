// interface-inheritance-diamond.slang

// Test diamond interface inheritance: a struct conforms to two interfaces
// that both inherit from a common base. Verifies method resolution is unambiguous.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

interface IBase
{
    int id();
}

interface ILeft : IBase
{
    int leftVal();
}

interface IRight : IBase
{
    int rightVal();
}

struct Diamond : ILeft, IRight
{
    int x;

    int id() { return x; }
    int leftVal() { return x * 2; }
    int rightVal() { return x * 3; }
}

int useBase<T : IBase>(T obj) { return obj.id(); }
int useLeft<T : ILeft>(T obj) { return obj.leftVal(); }
int useRight<T : IRight>(T obj) { return obj.rightVal(); }

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Diamond d;
    d.x = 5;

    // CHECK: 5
    outputBuffer[0] = useBase(d);
    // CHECK: 10
    outputBuffer[1] = useLeft(d);
    // CHECK: 15
    outputBuffer[2] = useRight(d);
    // CHECK: 30
    outputBuffer[3] = useBase(d) + useLeft(d) + useRight(d);
}
