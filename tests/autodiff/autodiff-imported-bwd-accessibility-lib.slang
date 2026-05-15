//TEST_IGNORE_FILE:

module "autodiff-imported-bwd-accessibility-lib";

public RWStructuredBuffer<float> outputBuffer;

[Differentiable]
public void add(float value)
{
    outputBuffer[0] = detach(value + 10.0);
}