// The autodiff supplement must stay unloaded until an explicit autodiff trigger is checked. The
// declarations below are nevertheless part of the ordinary language surface: plain kernels use
// the tensor types, plain code calls these helpers, and aggregate conformance synthesis needs the
// Array and Optional extensions.
//
//TEST:SIMPLE:

struct PlainTensorTypes
{
    TensorView<float> view;
    DiffTensorView<float> diffView;
    TorchTensor<float> tensor;
}

struct PlainAggregate : IDifferentiable
{
    float values[2];
    Optional<float> optionalValue;
}

PlainAggregate.Differential makeDifferential()
{
    return PlainAggregate.dzero();
}

float useAutodiffHelpersWithoutDifferentiating(float value)
{
    var pair = diffPair(value, 1.0);
    updatePrimal(pair, value + 1.0);
    updateDiff(pair, 2.0);
    updatePair(pair, value + 2.0, 3.0);
    let values = makeArrayFromElement<float, 2>(pair.p);
    return detach(values[0]);
}
