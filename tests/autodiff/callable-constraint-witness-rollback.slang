//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A [BackwardDifferentiable] interface method adds callable constraints to
// the requirement. A direct candidate with only a legacy backward derivative
// can match the source signature and satisfy the backward constraint, but it
// must still be rejected because it does not satisfy the generated
// forward-differentiable constraint.

interface IModule<T : IDifferentiable>
{
    [BackwardDifferentiable]
    T forward(T input);
/*CHECK:
      ^^^^^^^ see interface requirement declaration of 'forward'
*/
}

struct InferenceModule : IModule<float>
{
    [BackwardDerivative(forward_bwd)]
    float forward(float input)
//CHECK:  ^^^^^^^ callable differentiability requirement not satisfied
//CHECK:  ^^^^^^^ member 'forward' does not satisfy the required forward-differentiable constraint for interface requirement 'forward'.
    {
        return input;
    }

    void forward_bwd(inout DifferentialPair<float> input, float dOut)
    {
        updateDiff(input, dOut);
    }
}
