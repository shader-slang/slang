//DIAGNOSTIC_TEST:SIMPLE:

float nonDiff(float x)
{
    return x;
}

[Differentiable]
float f(float x)
{
    return x * x;
}

[Differentiable]
float g(float x)
{
    float val = f(x + 1); // Error: f must also be backward-differentiable
    return val;
}

[Differentiable]
float h(float x)
{
    float val = 0;

    // call to non-differentiable method 
    // (should error if the data-flow propagation works properly.)
    // 
    float y = nonDiff(x);

    // call to a differentiable method, using the
    // result of a non-differentiable call.
    // 
    val = f(y);

    return val;
}
