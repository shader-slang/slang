//TEST:INTERPRET(filecheck=CHECK):
//
// This test builds an integer-only MLP-shaped recursive PackBranch chain.
// Each Layer<InSize, OutSize> computes:
//   result[i] = (i + 1) + sum_j(input[j] * (i + j + 1))
// so MLP<2, 3, 2> on input [2, 1] evaluates to [63, 91].

interface ILayerChain<let InSize : int>
{
    associatedtype Output;
    __init();
    Output eval(int[InSize] input);
}

struct EmptyNode<let N : int> : ILayerChain<N>
{
    typealias Output = int[N];

    __init() {}

    Output eval(int[N] input)
    {
        return input;
    }
}

struct Layer<let InSize : int, let OutSize : int>
{
    __init() {}

    int[OutSize] eval(int[InSize] input)
    {
        int[OutSize] result = {};
        for (int i = 0; i < OutSize; ++i)
        {
            int sum = i + 1;
            for (int j = 0; j < InSize; ++j)
            {
                sum += input[j] * (i + j + 1);
            }
            result[i] = sum;
        }
        return result;
    }
}

struct EvalNode<let Head : int, let each Tail : int> : ILayerChain<Head>
    where nonempty(Tail)
{
    typealias Rest = __packBranch(
        __trimHead(Tail),
        EmptyNode<__first(Tail)>,
        EvalNode<__first(Tail), __trimHead(Tail)>);
    typealias Output = Rest::Output;

    Layer<Head, __first(Tail)> firstLayer = {};
    Rest restLayers = {};

    __init() {}

    Output eval(int[Head] input)
    {
        return restLayers.eval(firstLayer.eval(input));
    }
}

struct MLP<let Head : int, let each Tail : int>
{
    typealias Layers = __packBranch(Tail, EmptyNode<Head>, EvalNode<Head, Tail>);
    typealias Output = Layers::Output;

    Layers layers = {};

    __init() {}

    Output eval(int[Head] input)
    {
        return layers.eval(input);
    }
}

void main()
{
    MLP<2, 3, 2> mlp = {};
    int[2] input = {2, 1};
    let output = mlp.eval(input);
    printf("%d %d\n", output[0], output[1]);
    // CHECK: 63 91
}
