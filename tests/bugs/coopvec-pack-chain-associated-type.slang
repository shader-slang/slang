//DISABLE_TEST:SIMPLE: -target spirv -entry computeMain -stage compute

// A recursive value-pack chain can return different-sized ordinary arrays through this associated
// type pattern. Replacing each CoopVec below with an ordinary array makes this test compile. With
// CoopVec, type legalization aborts with `unexpected: non-simple operand(s)!`.

interface ILayerChain<let InputSize : int>
{
    associatedtype Output;
    Output eval(CoopVec<half, InputSize> input);
}

struct EmptyNode<let N : int> : ILayerChain<N>
{
    typealias Output = CoopVec<half, N>;

    Output eval(CoopVec<half, N> input)
    {
        return input;
    }
}

struct Layer<let InputSize : int, let OutputSize : int>
{
    CoopVec<half, OutputSize> eval(CoopVec<half, InputSize> input)
    {
        CoopVec<half, OutputSize> result;
        for (int outputIndex = 0; outputIndex < OutputSize; ++outputIndex)
            result[outputIndex] = input[outputIndex % InputSize];
        return result;
    }
}

struct EvalNode<let Head : int, let each Tail : int> : ILayerChain<Head>
    where nonempty(Tail)
{
    typealias Rest = __packBranch(
        __trimFirst(Tail),
        EmptyNode<__first(Tail)>,
        EvalNode<__first(Tail), __trimFirst(Tail)>);
    typealias Output = Rest::Output;

    Layer<Head, __first(Tail)> firstLayer;
    Rest restLayers;

    Output eval(CoopVec<half, Head> input)
    {
        return restLayers.eval(firstLayer.eval(input));
    }
}

struct MLP<let Head : int, let each Tail : int>
{
    typealias Layers = __packBranch(Tail, EmptyNode<Head>, EvalNode<Head, Tail>);
    typealias Output = Layers::Output;

    Layers layers;

    Output eval(CoopVec<half, Head> input)
    {
        return layers.eval(input);
    }
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
[require(spvCooperativeVectorNV)]
void computeMain()
{
    MLP<2, 3, 2> mlp = {};
    CoopVec<half, 2> input;
    input[0] = 1.0h;
    input[1] = 2.0h;
    let output = mlp.eval(input);
    outputBuffer[0] = float(output[0]);
}
