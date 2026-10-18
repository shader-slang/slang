// A conformer's `[BackwardDerivative]` must be used when its method is called through the
// interface requirement it implements: through an existential parameter, a generic parameter,
// or runtime dynamic dispatch. A direct call on the concrete type must also keep the user's
// derivative when a generic caller in the same module resolves the requirement to the same method
// (the MIXED mode). The custom derivatives below deliberately differ from the
// true derivative of the primal body (`3 * s`), and each counts its invocations, so the output
// shows which derivative ran. Forward-mode calls have no user derivative to honor and keep the
// synthesized one.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CONCRETE):-cpu -shaderobj -output-using-type -Xslang... -DCONCRETE -X.
//TEST:COMPARE_COMPUTE(filecheck-buffer=EXISTENTIAL):-cpu -shaderobj -output-using-type -Xslang... -DEXISTENTIAL -X.
//TEST:COMPARE_COMPUTE(filecheck-buffer=GENERIC):-cpu -shaderobj -output-using-type -Xslang... -DGENERIC -X.
//TEST:COMPARE_COMPUTE(filecheck-buffer=MIXED):-cpu -shaderobj -output-using-type -Xslang... -DMIXED -X.
//TEST:COMPARE_COMPUTE(filecheck-buffer=DYNAMIC):-cpu -shaderobj -output-using-type -Xslang... -DDYNAMIC -X.
//TEST:COMPARE_COMPUTE(filecheck-buffer=FORWARD):-cpu -shaderobj -output-using-type -Xslang... -DFORWARD -X.

// `outputBuffer[0]` counts custom-derivative invocations, and the remaining slots hold the
// derivative from each call path.
//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0 0.0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=selectorBuffer
RWStructuredBuffer<int> selectorBuffer;

[anyValueSize(16)]
interface IView
{
    [Differentiable]
    float read(float s);
}

struct View : IView
{
    void read_bwd(inout DifferentialPair<float> s, float dOut)
    {
        outputBuffer[0] += 1.0;
        s = diffPair(s.p, 10.0 * dOut);
    }

    [BackwardDerivative(read_bwd)]
    [Differentiable]
    float read(float s) { return 3.0 * s; }
}

struct OtherView : IView
{
    void read_bwd(inout DifferentialPair<float> s, float dOut)
    {
        outputBuffer[0] += 1.0;
        s = diffPair(s.p, 20.0 * dOut);
    }

    [BackwardDerivative(read_bwd)]
    [Differentiable]
    float read(float s) { return 3.0 * s; }
}

//TEST_INPUT: type_conformance View:IView = 0
//TEST_INPUT: type_conformance OtherView:IView = 1

[Differentiable]
float viaConcrete(View v, float s) { return v.read(s); }

[Differentiable]
float viaExistential(IView v, float s) { return v.read(s); }

[Differentiable]
float viaGeneric<V : IView>(V v, float s) { return v.read(s); }

IView pick(int selector)
{
    if (selector == 0)
        return View();
    return OtherView();
}

float backwardViaConcrete()
{
    var s = diffPair(1.0);
    bwd_diff(viaConcrete)(View(), s, 1.0);
    return s.d;
}

float backwardViaExistential(IView v)
{
    var s = diffPair(1.0);
    bwd_diff(viaExistential)(v, s, 1.0);
    return s.d;
}

float backwardViaGeneric()
{
    var s = diffPair(1.0);
    bwd_diff(viaGeneric<View>)(View(), s, 1.0);
    return s.d;
}

[numthreads(1, 1, 1)]
void computeMain()
{
#if defined(CONCRETE)
    // CONCRETE: {{^}}1.0
    // CONCRETE-NEXT: {{^}}10.0
    outputBuffer[1] = backwardViaConcrete();
#elif defined(EXISTENTIAL)
    // EXISTENTIAL: {{^}}1.0
    // EXISTENTIAL-NEXT: {{^}}10.0
    outputBuffer[1] = backwardViaExistential(View());
#elif defined(GENERIC)
    // GENERIC: {{^}}1.0
    // GENERIC-NEXT: {{^}}10.0
    outputBuffer[1] = backwardViaGeneric();
#elif defined(MIXED)
    // MIXED: {{^}}3.0
    // MIXED-NEXT: {{^}}10.0
    // MIXED-NEXT: {{^}}10.0
    // MIXED-NEXT: {{^}}10.0
    outputBuffer[1] = backwardViaConcrete();
    outputBuffer[2] = backwardViaExistential(View());
    outputBuffer[3] = backwardViaGeneric();
#elif defined(DYNAMIC)
    // DYNAMIC: {{^}}2.0
    // DYNAMIC-NEXT: {{^}}10.0
    // DYNAMIC-NEXT: {{^}}20.0
    outputBuffer[1] = backwardViaExistential(pick(selectorBuffer[0]));
    outputBuffer[2] = backwardViaExistential(pick(selectorBuffer[1]));
#elif defined(FORWARD)
    // FORWARD: {{^}}0.0
    // FORWARD-NEXT: {{^}}3.0
    // FORWARD-NEXT: {{^}}3.0
    // FORWARD-NEXT: {{^}}3.0
    View v;
    outputBuffer[1] = fwd_diff(viaConcrete)(v, diffPair(1.0, 1.0)).d;
    outputBuffer[2] = fwd_diff(viaExistential)(v, diffPair(1.0, 1.0)).d;
    outputBuffer[3] = fwd_diff(viaGeneric<View>)(v, diffPair(1.0, 1.0)).d;
#endif
}
