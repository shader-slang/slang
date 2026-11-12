//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage vertex -entry VSMain

// Regression test: issue #8957 (GLSL variant)
// Exercises createGLSLGlobalVaryingsImpl with extern struct associated types.

interface IObject
{
    associatedtype VertexInput;
    associatedtype VertexOutput;
}

struct MyVertexInput
{
    [[vk::location(0)]] float3 position;
}

struct MyVertexOutput
{
    float4 position : SV_Position;
}

struct MyObject : IObject
{
    typedef MyVertexInput VertexInput;
    typedef MyVertexOutput VertexOutput;
}

export struct Object : IObject = MyObject;

[shader("vertex")]
Object.VertexOutput VSMain(Object.VertexInput input)
{
    Object.VertexOutput output;
    return output;
}

//CHECK: void main()
//CHECK: gl_Position
