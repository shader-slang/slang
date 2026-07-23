// array-existential-elements.slang

// Test an array of structs that conform to an interface, dispatched
// through a generic function. Exercises type specialization across array elements.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

interface IScorer
{
    int score();
}

struct Fixed : IScorer
{
    int val;
    int score() { return val; }
}

struct Doubled : IScorer
{
    int val;
    int score() { return val * 2; }
}

int sumScores_Fixed(Fixed items[4])
{
    int total = 0;
    for (int i = 0; i < 4; i++)
        total = total + items[i].score();
    return total;
}

int sumScores_Doubled(Doubled items[4])
{
    int total = 0;
    for (int i = 0; i < 4; i++)
        total = total + items[i].score();
    return total;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Fixed fixedArr[4];
    fixedArr[0].val = 1;
    fixedArr[1].val = 2;
    fixedArr[2].val = 3;
    fixedArr[3].val = 4;

    // CHECK: 10
    outputBuffer[0] = sumScores_Fixed(fixedArr);

    Doubled dblArr[4];
    dblArr[0].val = 1;
    dblArr[1].val = 2;
    dblArr[2].val = 3;
    dblArr[3].val = 4;

    // CHECK: 20
    outputBuffer[1] = sumScores_Doubled(dblArr);

    // Single element access
    // CHECK: 3
    outputBuffer[2] = fixedArr[2].score();
    // CHECK: 8
    outputBuffer[3] = dblArr[3].score();
}
