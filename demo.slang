// Network evaluation with FFLayer and built-in activations
[Differentiable]
OutputVec mlp_forward(Storage storage, InputVec input)
{
    uint addr = 0u;
    let h0 = Layer0(addr, addr + INPUT_DIM * HIDDEN_DIM, LeakyReLU<float>(LEAKY_RELU_SLOPE)).eval<Storage>(storage, input);
    addr = Layer0.nextAddress(addr);
    let h1 = Layer1(addr, addr + HIDDEN_DIM * HIDDEN_DIM, LeakyReLU<float>(LEAKY_RELU_SLOPE)).eval<Storage>(storage, h0);
    addr = Layer1.nextAddress(addr);
    return Layer2(addr, addr + HIDDEN_DIM * OUTPUT_DIM, ExpActivation<float>()).eval<Storage>(storage, h1);
}
