// TODO(#14296, #14297): the test does not compile (E39999), because `__trimLast` and `__trimFirst`
// do not work inside `expand`. Even when it compiles, `countof(Tuple<expand ...>)` returns 1. The test
// passed by accident before, because `CHECK: 3` matched `E39999`.
//DISABLE_TEST:INTERPRET(filecheck=CHECK):

struct Layer<int InSize, int OutSize>
{
}

int layerCount<let each LayerSize : int>()
{
    return countof(Tuple<expand Layer<each __trimLast(LayerSize), each __trimFirst(LayerSize)>>);
}

void main()
{
    printf("%d\n", layerCount<4, 8, 16, 32>());
    // CHECK: 3

    printf("%d\n", layerCount<8>());
    // CHECK: 0
}
