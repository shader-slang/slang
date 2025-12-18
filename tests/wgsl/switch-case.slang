//TEST:SIMPLE(filecheck=WGSL): -target wgsl -conformance "Circle:IShape=0" -conformance "Rectangle:IShape=1"

[anyValueSize(16)]
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

struct ShapeDataBlob { uint type; uint payload[16]; }

struct ShapeBuffer
{
    StructuredBuffer<ShapeDataBlob> shapes;
    IShape getShape(uint index)
    {
      uint type = shapes[index].type;
      return createDynamicObject<IShape, ShapeDataBlob>(type, shapes[index]);
    }
}

struct VertexInput
{
    float2 position;
    float3 color;
}

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 color : TEXCOORD0;
    float area : TEXCOORD1;
}


[shader("vertex")]
func vs_main( input: VertexInput, shapes: ShapeBuffer)->VertexOutput
{
    VertexOutput output;
    output.position = float4(input.position, 0.0, 1.0);
    output.color = input.color;
    output.area = shapes.getShape(0).getArea(); 
    return output;
}

struct FragmentOutput
{
    float4 color : SV_TARGET;
}

[shader("fragment")]
func fs_main(VertexOutput input)->FragmentOutput
{
    FragmentOutput output;
    output.color = float4(input.color * input.area, 1.0);
    return output;
}

//WGSL: fn s_dispatch_IShape_getArea_{{[0-9]+}}( _S{{[0-9]+}} : u32, _S{{[0-9]+}} : AnyValue8) -> f32
//WGSL-NEXT:{
//WGSL-DAG:            return U_SR14switch_2Dxcase6Circle7getAreap0pf_wtwrapper_0(_S{{[0-9]+}});
//WGSL-DAG:            return U_SR14switch_2Dxcase9Rectangle7getAreap0pf_wtwrapper_0(_S{{[0-9]+}});
//WGSL:}
