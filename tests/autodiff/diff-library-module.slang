// Library half of module-prelink-keeps-autodiff.slang. It is precompiled to a
// .slang-module by that test's COMPILE step, so the prelink that produces the
// serialized module must keep its auto-diff annotations and witness-table
// entries complete: the library itself never differentiates anything, but its
// consumer does.
module diff_library_module;

[ForwardDifferentiable]
public float libSquare(float x)
{
    return x * x;
}
