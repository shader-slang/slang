//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage vertex -entry VSMain

// Regression test: issue #8957
// Associated type of link-time extern struct used as varying entry-point parameter/return type.

interface IObjectVertexInput {}
interface IObjectVertexOutput {}

interface IObject
{
    associatedtype VertexInput : IObjectVertexInput;
    associatedtype VertexOutput : IObjectVertexOutput;
}

struct MyObjectVertexInput : IObjectVertexInput
{
    [[vk::location(0)]] float3 position;
}

struct MyObjectVertexOutput : IObjectVertexOutput
{
    float4 position : SV_Position;
}

struct MyObject : IObject
{
    typedef MyObjectVertexInput VertexInput;
    typedef MyObjectVertexOutput VertexOutput;
}

export struct Object : IObject = MyObject;

[shader("vertex")]
Object.VertexOutput VSMain(Object.VertexInput input)
{
    Object.VertexOutput output;
    return output;
}

//CHECK: OpEntryPoint Vertex %VSMain
//CHECK: OpName %MyObjectVertexOutput "MyObjectVertexOutput"
//CHECK: OpDecorate %gl_Position BuiltIn Position
