// static-const-in-generic-interface-methods.slang

// Test that static const variables in generic interface methods don't crash the compiler
// Reproduces the issue from GitHub issue #8148

interface IVector<T, int N> {}

interface IEncoder<T, int In>
{
    static const int In;
    static const int Out;

    [BackwardDifferentiable]
    OutputVector eval<InputVector, OutputVector>(InputVector input)
        where InputVector : IVector<T, In>
        where OutputVector : IVector<T, Out>;
}