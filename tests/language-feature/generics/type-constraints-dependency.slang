// TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type
// TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type


public interface IFoo<T> : IDifferentiablePtrType
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const int Val;
}

struct FooImp<T> : IFoo<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias Differential = FooImp<T.Differential>;
    static const int Val = 16;
}

public interface IBar<T> : IDifferentiable
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
     public associatedtype Differential : IBar<T.Differential>;
     static const int Val;
}

struct BarImp<T> : IBar<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias Differential = BarImp<T.Differential>;
    static const int Val = 32;
}

public struct MyStruct<T, Foo, Bar>
    where T : __BuiltinFloatingPointType
    where T == T.Differential           // checking this now
    where Foo : IFoo<T>          // checking this now.
    where Foo.Differential : IFoo<T.Differential>
    where Bar : IBar<T>
{
    static const int ParameterCount = Foo.Val * Bar.Val;
}

// TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    MyStruct<float, FooImp<float>, BarImp<float>> s;
    outputBuffer[0] = s.ParameterCount;
    // CHECK: 512
}

