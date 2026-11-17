// A conformer's custom derivative must be used through the interface requirement it implements
// for two more requirement shapes than the sibling `custom-*-derivative-through-interface-
// requirement.slang` tests cover: a generic method requirement with a `[BackwardDerivative]`, and
// a `[ForwardDifferentiable]` requirement with a `[ForwardDerivative]`. As in those tests, the
// custom derivative deliberately differs from the true derivative of the primal body (`3 * s`)
// and counts its invocations in `outputBuffer[0]`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=GENERIC_METHOD):-cpu -shaderobj -output-using-type -Xslang... -DGENERIC_METHOD -X.
//TEST:COMPARE_COMPUTE(filecheck-buffer=FORWARD_ONLY):-cpu -shaderobj -output-using-type -Xslang... -DFORWARD_ONLY -X.

//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

#if defined(GENERIC_METHOD)
interface IView
{
    [Differentiable]
    float read<let N : int>(float s);
}

struct View : IView
{
    void read_bwd<let N : int>(inout DifferentialPair<float> s, float dOut)
    {
        outputBuffer[0] += 1.0;
        s = diffPair(s.p, 10.0 * N * dOut);
    }

    [BackwardDerivative(read_bwd<N>)]
    [Differentiable]
    float read<let N : int>(float s) { return 3.0 * s; }
}

[Differentiable]
float viaExistential(IView v, float s) { return v.read<2>(s); }

[Differentiable]
float viaGeneric<V : IView>(V v, float s) { return v.read<2>(s); }

[numthreads(1, 1, 1)]
void computeMain()
{
    // GENERIC_METHOD: {{^}}2.0
    // GENERIC_METHOD-NEXT: {{^}}20.0
    // GENERIC_METHOD-NEXT: {{^}}20.0
    var s = diffPair(1.0);
    bwd_diff(viaExistential)(View(), s, 1.0);
    outputBuffer[1] = s.d;
    s = diffPair(1.0);
    bwd_diff(viaGeneric<View>)(View(), s, 1.0);
    outputBuffer[2] = s.d;
}
#elif defined(FORWARD_ONLY)
interface IView
{
    [ForwardDifferentiable]
    float read(float s);
}

struct View : IView
{
    DifferentialPair<float> read_fwd(DifferentialPair<float> s)
    {
        outputBuffer[0] += 1.0;
        return diffPair(3.0 * s.p, 10.0 * s.d);
    }

    [ForwardDerivative(read_fwd)]
    [ForwardDifferentiable]
    float read(float s) { return 3.0 * s; }
}

[ForwardDifferentiable]
float viaExistential(IView v, float s) { return v.read(s); }

[ForwardDifferentiable]
float viaGeneric<V : IView>(V v, float s) { return v.read(s); }

[numthreads(1, 1, 1)]
void computeMain()
{
    // FORWARD_ONLY: {{^}}2.0
    // FORWARD_ONLY-NEXT: {{^}}10.0
    // FORWARD_ONLY-NEXT: {{^}}10.0
    outputBuffer[1] = fwd_diff(viaExistential)(View(), diffPair(1.0, 1.0)).d;
    outputBuffer[2] = fwd_diff(viaGeneric<View>)(View(), diffPair(1.0, 1.0)).d;
}
#endif
