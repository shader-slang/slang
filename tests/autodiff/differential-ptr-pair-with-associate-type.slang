//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
interface IFoo<T>
{
    associatedtype Parameters : IDifferentiablePtrType;
    property DifferentialPtrPair<Parameters> parametersDual { get; }
}

extension<T: __BuiltinFloatingPointType> T: IDifferentiablePtrType
{
    typealias Differential = T;
}

struct Foo<T:__BuiltinFloatingPointType> : IFoo<T>
{
    typealias Parameters = Array<T, 1>;
    property DifferentialPtrPair<Parameters> parametersDual
    {
        get
        {
            Parameters primal = {T(1)};
            Parameters diff   = {T(2)};
            return DifferentialPtrPair<Parameters>(primal, diff);
        }
    }
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=output
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Foo<float> foo;
    let res = foo.parametersDual;
    output[0] = res.p[0];
    output[1] = res.d[0];

    // CHECK: 1.0
    // CHECK: 2.0
}
