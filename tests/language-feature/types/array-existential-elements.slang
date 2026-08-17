// array-existential-elements.slang

// Test an array whose element type is an interface (existential) rather than a
// concrete struct. Each slot may hold a different conforming type, so `score()`
// must dispatch dynamically per element through the stored witness — this is what
// distinguishes it from a concrete `Fixed[]`/`Doubled[]` array that binds `score()`
// statically.

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

int sumScores(IScorer items[4])
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
    // A single existential array holding a mix of element types; the sum must
    // dispatch score() differently per slot: 1 + (2*2) + 3 + (4*2) = 16.
    IScorer mixed[4];
    mixed[0] = Fixed(1);
    mixed[1] = Doubled(2);
    mixed[2] = Fixed(3);
    mixed[3] = Doubled(4);

    // CHECK: 16
    outputBuffer[0] = sumScores(mixed);

    // Dynamic dispatch on a directly-indexed existential element.
    // CHECK: 4
    outputBuffer[1] = mixed[1].score();
    // CHECK: 8
    outputBuffer[2] = mixed[3].score();

    // An all-Fixed existential array still dispatches through the interface.
    IScorer allFixed[4];
    allFixed[0] = Fixed(10);
    allFixed[1] = Fixed(20);
    allFixed[2] = Fixed(30);
    allFixed[3] = Fixed(40);
    // CHECK: 100
    outputBuffer[3] = sumScores(allFixed);
}
