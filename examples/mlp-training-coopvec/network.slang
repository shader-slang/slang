module network;

import common;
import mlp;

public struct MyNetwork
{
    public FeedForwardLayer<4, 16> layer1;
    public FeedForwardLayer<16, 4> layer2;

    [Differentiable]
    internal MLVec<4> encodeInput(NFloat x, NFloat y)
    {
        return MLVec<4>.fromArray({
                x,
                y,
                x*x,
                y*y,
            });
    }

    [Differentiable]
    internal MLVec<4> _eval(NFloat x, NFloat y)
    {
        let encoding = encodeInput(x, y);
        let layer1Output = layer1.eval(encoding);
        let leyer2Output = layer2.eval(layer1Output);
        return leyer2Output;
    }

    [Differentiable]
    public half4 eval(no_diff NFloat x, no_diff NFloat y)
    {
        let mlv = _eval(x, y);
        let arr = mlv.toArray();
        return half4(arr[0], arr[1], arr[2], arr[3]);
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

