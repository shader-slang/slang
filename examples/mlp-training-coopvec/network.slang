module network;

import common;
import mlp;

// CoopVec lane reads are not natively differentiable, so provide the reverse rule needed to
// convert the network output to a conventional array.
internal NFloat[N] coopVecToArray<let N : int>(CoopVec<NFloat, N> value)
{
    NFloat[N] result;
    [ForceUnroll]
    for (int i = 0; i < N; i++)
        result[i] = value[i];
    return result;
}

[BackwardDerivativeOf(coopVecToArray)]
internal void coopVecToArrayBwd<let N : int>(
    inout DifferentialPair<CoopVec<NFloat, N>> value,
    NFloat[N] resultGrad)
{
    CoopVec<NFloat, N> valueGrad;
    [ForceUnroll]
    for (int i = 0; i < N; i++)
        valueGrad[i] = resultGrad[i];
    value = diffPair(value.p, valueGrad);
}

public struct MyNetwork
{
    public FeedForwardLayer<4, 16> layer1;
    public FeedForwardLayer<16, 4> layer2;

    [Differentiable]
    internal CoopVec<NFloat, 4> encodeInput(NFloat x, NFloat y)
    {
        return CoopVec<NFloat, 4>(x, y, x * x, y * y);
    }

    [Differentiable]
    internal CoopVec<NFloat, 4> _eval(NFloat x, NFloat y)
    {
        let encoding = encodeInput(x, y);
        let layer1Output = layer1.eval(encoding);
        let layer2Output = layer2.eval(layer1Output);
        return layer2Output;
    }

    [Differentiable]
    public half4 eval(no_diff NFloat x, no_diff NFloat y)
    {
        let result = _eval(x, y);
        let resultArray = coopVecToArray(result);
        return half4(resultArray[0], resultArray[1], resultArray[2], resultArray[3]);
    }
}

[Differentiable]
public half loss(MyNetwork* network, no_diff half x, no_diff half y)
{
    let networkResult = network.eval(x, y);
    let gt = no_diff groundtruth(x, y);
    let diff = networkResult - gt;
    return dot(diff, diff);
}

public half4 groundtruth(half x, half y)
{
    return {
        (x + y) / (1 + y * y),
        2 * x + y,
        0.5 * x * x + 1.2 * y,
        x + 0.5 * y * y,
    };
}
