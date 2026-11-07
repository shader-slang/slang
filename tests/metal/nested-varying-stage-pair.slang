//TEST:SIMPLE(filecheck=VS): -target metal -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=FS): -target metal -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=WVS): -target wgsl -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=WFS): -target wgsl -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=VSOUT): -target metal -stage vertex -entry vertexOutMain
//TEST:SIMPLE(filecheck=WBOTH): -target wgsl -entry vertexMain -stage vertex -entry fragmentMain -stage fragment

// A vertex output and the fragment input of the same type must get the same varying
// interface: the outer-most semantic wins on nested fields, as reflection reports.

struct Inner
{
    // VS-DAG: uv{{.*}} {{\[\[}}user(FOO_3)]]
    // FS-DAG: uv{{.*}} {{\[\[}}user(FOO_3)]]
    // WVS-DAG: @location(0) uv
    // WFS-DAG: @location(0) uv
    // VSOUT-DAG: uv{{.*}} {{\[\[}}user(FOO_3)]]
    // WBOTH-DAG: @location(0) uv
    float2 uv : TEXCOORD0;
    // VS-DAG: color{{.*}} {{\[\[}}user(FOO_4)]]
    // FS-DAG: color{{.*}} {{\[\[}}user(FOO_4)]]
    // WVS-DAG: @location(1) color
    // WFS-DAG: @location(1) color
    // VSOUT-DAG: color{{.*}} {{\[\[}}user(FOO_4)]]
    // WBOTH-DAG: @location(1) color
    float4 color;
};

struct Varyings
{
    float4 position : SV_Position;
    Inner nested : FOO3;
};

Varyings vertexMain(float4 position : POSITION)
{
    Varyings output;
    output.position = position;
    output.nested.uv = position.xy;
    output.nested.color = position;
    return output;
}

float4 fragmentMain(Varyings input) : SV_Target
{
    return float4(input.nested.uv, 0, 1) + input.nested.color;
}

// An `out` parameter next to a struct return goes through the out-parameter wrapper, whose
// result layout must still describe the nested fields.
// VSOUT-DAG: extra{{.*}} {{\[\[}}user(BAR_7)]]
Varyings vertexOutMain(float4 position : POSITION, out float2 extra : BAR7)
{
    Varyings output;
    output.position = position;
    output.nested.uv = position.xy;
    output.nested.color = position;
    extra = position.zw;
    return output;
}
