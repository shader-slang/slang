//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):-std 2026

// A public generic method nested in two generic containers still reports a signature type that is
// genuinely less visible than the method, on both the method and its synthesized derivatives. The
// test is non-exhaustive because the `bwd_diff` diagnostic and one more point into the core module.
module "nested-generic-method-visibility-negative";

internal struct Hidden : IDifferentiable
{
    float v;
}

public struct Outer<T : IDifferentiable>
{
    public struct Inner<B>
    {
        [Differentiable]
        public Hidden identity<Index : __BuiltinIntegerType>(Hidden x, Index idx) { return x; }
        //CHECK:      ^^^^^^^^ 'identity' references less visible type 'Hidden'.
        //CHECK:      ^^^^^^^^ 'fwd_diff' references less visible type 'DifferentialPair<Hidden>'.
        //CHECK:      ^^^^^^^^ 'apply_bwd' references less visible type 'Hidden'.
        //CHECK:      ^^^^^^^^ 'remat' references less visible type 'Hidden'.
    }
}
