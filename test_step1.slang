// Test with BackwardDifferentiable on interface method
interface ITest : IDifferentiable
{
    [BackwardDifferentiable]
    float getValue();
}

// Remove [BackwardDifferentiable] from interface
struct TestImpl : ITest
{
    typealias Differential = This;
    
    [BackwardDifferentiable]
    float getValue() { return 0.0; }
}

[shader("compute")]
void main()
{
    var test = TestImpl();
    float result = test.getValue();
}