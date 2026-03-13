//TEST:INTERPRET(filecheck=CHECK):

struct Layer<int InSize, int OutSize>
{
}

int layerCount<let each LayerSize : int>()
{
    return countof(Tuple<expand Layer<each __trimTail(LayerSize), each __trimHead(LayerSize)>>);
}

void main()
{
    printf("%d\n", layerCount<4, 8, 16, 32>());
    // CHECK: 3

    printf("%d\n", layerCount<8>());
    // CHECK: 0
}
