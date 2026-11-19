//TEST:INTERPRET(filecheck=CHECK):
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda -shaderobj -output-using-type

// `countof` of a tuple type counts each plain element once and each `expand` as the size of
// its pack. The pattern expands a type pack (`Wrap<each T>`) or a value pack
// (`Layer<each S, each S>`), the pack can be empty or forwarded from another generic, and the
// tuple can be named as a type or built with `makeTuple`. The cases whose result differs from the
// number of tuple operands check that an `expand` is not counted as a single element.

struct Wrap<T>
{
}

struct Layer<int InSize, int OutSize>
{
}

int countWrapped<each T>()
{
    return countof(Tuple<expand Wrap<each T>>);
}

int countLayers<let each S : int>()
{
    return countof(Tuple<expand Layer<each S, each S>>);
}

int countMixed<each T>()
{
    return countof(Tuple<float, expand Wrap<each T>, int>);
}

int countTwoExpands<each T>()
{
    return countof(Tuple<expand Wrap<each T>, expand each T>);
}

int countForwarded<each T>()
{
    return countWrapped<int, expand each T>();
}

int countMade<each T>()
{
    return countof(makeTuple(expand Wrap<each T>()));
}

void main()
{
    printf(
        "types: %d %d %d\n",
        countWrapped<int>(),
        countWrapped<int, float, uint>(),
        countWrapped<int, int, int, int, int>());
    // CHECK: types: 1 3 5

    printf(
        "values: %d %d %d\n",
        countLayers<4>(),
        countLayers<4, 8, 16>(),
        countLayers<1, 2, 3, 4, 5>());
    // CHECK: values: 1 3 5

    printf("empty: %d\n", countWrapped());
    // CHECK: empty: 0

    printf("mixed: %d %d\n", countMixed(), countMixed<int, float, uint>());
    // CHECK: mixed: 2 5

    printf("two-expands: %d\n", countTwoExpands<int, float, uint>());
    // CHECK: two-expands: 6

    printf("forwarded: %d %d\n", countForwarded(), countForwarded<float, uint>());
    // CHECK: forwarded: 1 3

    printf("made: %d %d\n", countMade(), countMade<int, float, uint>());
    // CHECK: made: 0 3
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = countWrapped<int>();
    // BUF: 1
    outputBuffer[1] = countWrapped<int, float, uint>();
    // BUF-NEXT: 3
    outputBuffer[2] = countWrapped<int, int, int, int, int>();
    // BUF-NEXT: 5
    outputBuffer[3] = countLayers<4>();
    // BUF-NEXT: 1
    outputBuffer[4] = countLayers<4, 8, 16>();
    // BUF-NEXT: 3
    outputBuffer[5] = countLayers<1, 2, 3, 4, 5>();
    // BUF-NEXT: 5
    outputBuffer[6] = countWrapped();
    // BUF-NEXT: 0
    outputBuffer[7] = countMixed();
    // BUF-NEXT: 2
    outputBuffer[8] = countMixed<int, float, uint>();
    // BUF-NEXT: 5
    outputBuffer[9] = countTwoExpands<int, float, uint>();
    // BUF-NEXT: 6
    outputBuffer[10] = countForwarded();
    // BUF-NEXT: 1
    outputBuffer[11] = countForwarded<float, uint>();
    // BUF-NEXT: 3
    outputBuffer[12] = countMade();
    // BUF-NEXT: 0
    outputBuffer[13] = countMade<int, float, uint>();
    // BUF-NEXT: 3
}
