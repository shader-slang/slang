//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -emit-spirv-directly -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_IGNORE_CAPS): -target spirv -emit-spirv-directly -entry main -stage compute -ignore-capabilities
module test;

[require(spvAtomicFloat16AddEXT)]
interface IFoo
{
    [require(spvRayQueryKHR)]
    void method1();
//CHECK: ^^^^^^^ see declaration of 'method1'

    void method2();
//CHECK: ^^^^^^^ see declaration of 'method2'
}

interface IFoo2
{
    void method3();
}

[require(spvGroupNonUniformArithmetic)]
void useNonUniformArithmetic()
{}

[require(spvRayQueryKHR)]
void useRayQueryKHR()
{}

[require(spvAtomicFloat16AddEXT)]
void useAtomicFloat16()
{}

// This should be OK, uses nothing past what is declared in the interface.
struct Impl1 : IFoo
{
    void method1()
    {
        useAtomicFloat16();
        useRayQueryKHR();
    }

    void method2()
    {
        useAtomicFloat16();
    }
}

struct Impl2 : IFoo, IFoo2
{
    // error here because explicit requirement is on `method1`
    void method1()
//CHECK: ^^^^^^^ capability incompatible with interface
//CHECK: ^^^^^^^ 'method1' uses capability 'spvGroupNonUniformArithmetic' that is incompatable with the interface requirement
    {
        useRayQueryKHR();
        useNonUniformArithmetic();
    }

    // error here because capabilities are explicitly tagged on
    // the requirement parent `IFoo`
    void method2()
//CHECK: ^^^^^^^ capability incompatible with interface
//CHECK: ^^^^^^^ 'method2' uses capability 'spvGroupNonUniformArithmetic' that is incompatable with the interface requirement
    {
        useAtomicFloat16();
        useNonUniformArithmetic();
    }

    // do not error here because capabilities are not explicitly tagged
    // on the requirement parent `IFoo2` or requirment method
    void method3()
    {
        useAtomicFloat16();
        useNonUniformArithmetic();
    }
}

void main()
{}
