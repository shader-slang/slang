import neural;

typealias Address = TensorViewAddress<float>;
typealias Vec4 = InlineVector<float, 4>;
typealias Vec2 = InlineVector<float, 2>;
typealias Layer = FFLayer<float, Vec4, Vec2, LinearLayout, IdentityActivation<float>, true>;

uniform TensorView<float> params;
uniform TensorView<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void forwardMain()
{
    Address baseAddr = Address(params);
    Address weightAddr = baseAddr.getOffset(0);
    Address biasAddr = baseAddr.getOffset(8);

    float[4] xArr = {1.0, 2.0, 3.0, 4.0};
    let x = Vec4(xArr);

    let layer = Layer();
    let y = layer.eval<Address>(x, weightAddr, biasAddr);

    // W = [[1,2,3,4],[5,6,7,8]], b = [9, 10]
    // y = W*x + b = [1+4+9+16+9, 5+12+21+32+10] = [39, 80]
    output.store(0u, y[0]);
    output.store(1u, y[1]);
}

[Differentiable]
Vec2 computeLayerOutput(Address weightAddr, Address biasAddr, Vec4 input, Layer layer)
{
    return layer.eval<Address>(input, weightAddr, biasAddr);
}

uniform TensorView<float> gradParams;
uniform TensorView<float> gradOutput;

[shader("compute")]
[numthreads(1, 1, 1)]
void backwardMain()
{
    Address baseAddr = Address(params);
    Address weightAddr = baseAddr.getOffset(0);
    Address biasAddr = baseAddr.getOffset(8);

    Address gradBaseAddr = Address(gradParams);
    Address dWeightAddr = gradBaseAddr.getOffset(0);
    Address dBiasAddr = gradBaseAddr.getOffset(8);

    float[4] xArr = {1.0, 2.0, 3.0, 4.0};
    let x = Vec4(xArr);
    let layer = Layer();

    var weightAddrPair = DifferentialPtrPair<Address>(weightAddr, dWeightAddr);
    var biasAddrPair = DifferentialPtrPair<Address>(biasAddr, dBiasAddr);
    var inputPair = diffPair(x);

    // dOutput = [1, 1]
    let dOut = Vec2(1.0);

    bwd_diff(computeLayerOutput)(weightAddrPair, biasAddrPair, inputPair, layer, dOut);

    // dInput = W^T * dOutput = [1+5, 2+6, 3+7, 4+8] = [6, 8, 10, 12]
    output.store(0u, inputPair.d[0]);
    output.store(1u, inputPair.d[1]);
    output.store(2u, inputPair.d[2]);
    output.store(3u, inputPair.d[3]);

    // dW = dOutput * input^T = [[1,2,3,4],[1,2,3,4]]
    // dBias = dOutput = [1, 1]
    // (already written to gradParams via atomicAdd)
}
