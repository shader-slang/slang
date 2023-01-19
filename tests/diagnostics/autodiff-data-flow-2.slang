//DIAGNOSTIC_TEST:SIMPLE:

float nonDiff(float x)
{
    return x;
}

[ForwardDifferentiable]
float f(float x)
{
    return x * x;
}

// error: function does not return a differentiable value.
[BackwardDifferentiable]
float g(float x)
{
    float val = f(x + 1); // Error: f must also be backward-differentiable
    return val;
}
/*
[BackwardDifferentiable]
float h(float x)
{
    float val = 0;
    // no diagnostic by clarifying intention.
    val = no_diff(f(x + 1));
    return val;
}
*/