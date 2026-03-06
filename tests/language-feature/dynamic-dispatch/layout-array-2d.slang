// Multi-dimensional fixed-size arrays as impl fields require nested array
// marshalling: the outer array elements are themselves arrays, each of
// which must be packed element-by-element into AnyValue storage.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IGrid
{
    float get(int row, int col);
    float rowSum(int row);
}

struct Grid2x3 : IGrid
{
    float data[2][3];

    float get(int row, int col) { return data[row][col]; }

    float rowSum(int row)
    {
        float s = 0.0;
        for (int j = 0; j < 3; j++)
            s += data[row][j];
        return s;
    }
}

struct Grid3x2 : IGrid
{
    float data[3][2];

    float get(int row, int col) { return data[row][col]; }

    float rowSum(int row)
    {
        float s = 0.0;
        for (int j = 0; j < 2; j++)
            s += data[row][j];
        return s;
    }
}

IGrid makeGrid(int id)
{
    if (id == 0)
    {
        Grid2x3 g;
        g.data[0][0] = 1.0; g.data[0][1] = 2.0; g.data[0][2] = 3.0;
        g.data[1][0] = 4.0; g.data[1][1] = 5.0; g.data[1][2] = 6.0;
        return g;
    }
    else
    {
        Grid3x2 g;
        g.data[0][0] = 10.0; g.data[0][1] = 20.0;
        g.data[1][0] = 30.0; g.data[1][1] = 40.0;
        g.data[2][0] = 50.0; g.data[2][1] = 60.0;
        return g;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // Grid2x3: row 0 = {1,2,3}, row 1 = {4,5,6}
    var g1 = makeGrid(id);
    outputBuffer[0] = g1.get(0, 2);
    // CHECK: 3.0
    outputBuffer[1] = g1.rowSum(0);
    // CHECK: 6.0
    outputBuffer[2] = g1.rowSum(1);
    // CHECK: 15.0

    // Grid3x2: row 0 = {10,20}, row 1 = {30,40}, row 2 = {50,60}
    var g2 = makeGrid(id + 1);
    outputBuffer[3] = g2.get(2, 1);
    // CHECK: 60.0
    outputBuffer[4] = g2.rowSum(0);
    // CHECK: 30.0
    outputBuffer[5] = g2.rowSum(1);
    // CHECK: 70.0
    outputBuffer[6] = g2.rowSum(2);
    // CHECK: 110.0
}
