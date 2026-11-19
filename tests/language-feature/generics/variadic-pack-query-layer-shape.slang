// TODO(#14297): `countof(Tuple<expand ...>)` returns 1 instead of the number of expanded
// elements, so this test prints 1 for both calls.
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
