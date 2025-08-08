// Simple test - just forward differentiable
interface ITest : IDifferentiable
{
    [ForwardDifferentiable]
    float getValue();
}

struct TestImpl : ITest
{
    typealias Differential = This;
    
    [ForwardDifferentiable]
    float getValue() { return 0.0; }
}

[shader("compute")]
void main()
{
    var test = TestImpl();
    float result = test.getValue();
}