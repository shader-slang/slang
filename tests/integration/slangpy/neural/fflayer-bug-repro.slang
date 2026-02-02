// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Test FFLayer autodiff (bug "assert failure: zeroMethod" is now fixed)
import slangpy;
import neural;

typealias Vec2 = InlineVector<float, 2>;
typealias Storage = StructuredBufferStorage<float>;
typealias Layer = FFLayer<float, Vec2, Vec2, Storage, IdentityActivation<float>, true>;

struct Network
{
    [Differentiable]
    Vec2 forward(Storage storage, no_diff Vec2 input)
    {
        // Storage passed to eval(), not constructor
        let layer = Layer(0u, 4u); // (weightAddr, biasAddr)
        return layer.eval<Storage>(storage, input);
    }
}

[Differentiable]
Vec2 loss(Storage storage, no_diff Vec2 input, no_diff Network network)
{
    // Wrapper where you can add more computation if desired.
    return network.forward(storage, input);
}

void calculate_grad(float2 input, RWStructuredBuffer<float> params, RWStructuredBuffer<float> dparams)
{
    Vec2 v; v[0] = input.x; v[1] = input.y;

    Storage p = Storage(params);
    Storage.Differential dp = Storage.Differential(dparams);
    let pair = DifferentialPtrPair<Storage>(p, dp);

    Network network;
    float seedArr[2] = { 1.0f, 0.0f };
    Vec2 seed = Vec2(seedArr);
    bwd_diff(loss)(pair, v, network, seed);
}
