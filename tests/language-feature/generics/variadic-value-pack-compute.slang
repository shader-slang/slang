//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void add(inout int a, int b) { a += b; }

int sum<let each D : int>()
{
    int s = 0;
    expand add(s, each D);
    return s;
}

struct Dims<let each D : int>
{
    int getDimCount() { return countof(D); }
}

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = sum<10, 20, 30>();
    // CHECK: 60

    Dims<1, 2, 3, 4> d;
    outputBuffer[1] = d.getDimCount();
    // CHECK: 4
}
