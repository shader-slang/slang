// Keep inferred type requirements distinct from unused or explicitly restricted methods.
// Regression for https://github.com/shader-slang/slang/issues/13793.
//TEST:SIMPLE: -target hlsl -entry methodMain -stage compute -no-codegen
//TEST:SIMPLE: -target hlsl -entry staticMethodMain -stage compute -no-codegen

[require(metal)]
struct MetalOnly
{
}

struct Input<T>
{
}

// An unused restricted method does not restrict its containing struct's type.
struct MethodContext
{
    [require(metal)]
    void metalMethod()
    {
    }
}

[numthreads(1, 1, 1)]
void methodMain(uniform Input<MethodContext> input)
{
}

[require(hlsl)]
void hlslHelper()
{
}

// The alias infers a requirement on the struct's type, but does not declare that
// requirement on its static method. The method only promises support for HLSL.
public struct StaticMethodContext
{
    typealias Element = MetalOnly;

    [require(hlsl)]
    public static void hlslMethod()
    {
        hlslHelper();
    }
}

[numthreads(1, 1, 1)]
void staticMethodMain()
{
    StaticMethodContext.hlslMethod();
}
