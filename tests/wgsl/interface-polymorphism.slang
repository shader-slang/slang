//TEST:SIMPLE(filecheck=WGSL): -stage vertex -entry vertexMain -target wgsl -conformance "Circle:IShape=0" -conformance "Rectangle:IShape=1"

// Test interface-based polymorphism with dynamic dispatch
// This tests the complex case that originally caused the WGSL generation issues

interface IShape
{
    float getArea();
}

struct Circle : IShape
{
    float radius;
    
    float getArea() { return 3.14159 * radius * radius; }
}

struct Rectangle : IShape  
{
    float width;
    float height;
    
    float getArea() { return width * height; }
}

struct ShapeBuffer
{
    // This should be filtered from function parameters and become a global
    // WGSL: @binding({{[0-9]+}}) @group(0) var<storage, read> {{.*}} : array<{{.*}}>;
    StructuredBuffer<IShape> shapes;
}

// Global shape buffer
ShapeBuffer shapes;

// Function parameters should be filtered, removing ShapeBuffer
// WGSL: fn getShapeArea_0( _S{{[0-9]+}} : u32) -> f32
float getShapeArea(ShapeBuffer buffer, uint index)
{
    IShape shape = buffer.shapes[index];
    return shape.getArea();
}

struct VertexInput
{
    float2 position : POSITION;
    float3 color : COLOR;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
    float area : TEXCOORD0;
};

VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position, 0.0, 1.0);
    output.color = input.color;
    
    ShapeBuffer shapeBuffer = shapes;
    // Function call should only pass non-buffer arguments
    // WGSL: getShapeArea{{.*}}(u32(0))
    output.area = getShapeArea(shapeBuffer, 0);
    
    return output;
}