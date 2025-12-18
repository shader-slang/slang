//TEST:INTERPRET(filecheck=CHECK):

// Test that we can call variadic generic [Differentiable] methods.

interface IParameterExtractor
{
    [Differentiable]
    void extract(no_diff uint x);
}

struct FloatParameterExtractor : IParameterExtractor
{
    [Differentiable]
    void extract(no_diff uint x)
    {
        printf("fff\n");
    }
}

[Differentiable]
void extract_parameters_helper<T : IParameterExtractor>(
    no_diff uint x,
    T arg,
    )
{
    arg.extract(x);
}

[Differentiable]
void wrapper1<each T>(
    uint x,
    expand each T args, // compiler will add no_diff modifier here.
    )
    where T : IParameterExtractor
{
    expand extract_parameters_helper(x, each args);
}

[Differentiable]
void wrapper2<each T>(
    uint x,
    expand each T args, // compiler will add no_diff modifier here.
    )
    where T : IParameterExtractor
{
    wrapper1(x, args);
}

void main()
{
    // There was a bug that causes the compiler failing to treat a `no_diff TypePack` as
    // a type pack, and thus diagnose an error when resolving the following call.
    //
    wrapper2(1, FloatParameterExtractor(), FloatParameterExtractor(), FloatParameterExtractor(), FloatParameterExtractor());
}

// CHECK: fff
// CHECK: fff
// CHECK: fff
// CHECK: fff