//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib


// METAL:     struct [[pixelOutput:pixelOutput_[0-9]+]]
// METAL-NEXT:{
// METAL-NEXT:    float4 output{{.*}}{{\[\[}}color(0){{\]\]}};
// METAL-NEXT:};

// METAL:     struct pixelInput{{.*}}
// METAL-NEXT:{
// METAL-NEXT:    float4 vertexColor{{.*}}  {{\[\[}}user(_SLANG_ATTR){{\]\]}};
// METAL-NEXT:    float2 vertexUV{{.*}} {{\[\[}}user(_SLANG_ATTR_1){{\]\]}};
// METAL-NEXT:    float3 vertexNormal{{.*}} {{\[\[}}user(NORMAL){{\]\]}};
// METAL-NEXT:};

// METAL: {{\[\[}}fragment{{\]\]}} [[pixelOutput]] main_fragment(pixelInput{{.*}} {{\[\[}}stage_in{{\]\]}}, float4 position{{.*}} {{\[\[}}position{{\]\]}})

// METAL:     struct FragOut{{.*}}
// METAL-NEXT:{
// METAL-NEXT:    float4 color{{.*}}{{\[\[}}color(0){{\]\]}};
// METAL-NEXT:    float depth{{.*}} {{\[\[}}depth(any){{\]\]}};
// METAL-NEXT:};

// METAL:     struct [[vertexOutput:main_vertex_Result_[0-9]+]]
// METAL-NEXT:{
// METAL-NEXT:    float4 position{{.*}} {{\[\[}}position{{\]\]}};
// METAL-NEXT:    float4 vertexColor{{.*}} {{\[\[}}user(_SLANG_ATTR){{\]\]}};
// METAL-NEXT:    float2 vertexUV{{.*}} {{\[\[}}user(_SLANG_ATTR_1){{\]\]}};
// METAL-NEXT:    float3 vertexNormal{{.*}} {{\[\[}}user(NORMAL){{\]\]}};
// METAL-NEXT:};

// METAL:     struct vertexInput{{.*}}
// METAL-NEXT:{
// METAL-NEXT:    float4 position{{.*}} {{\[\[}}attribute(0){{\]\]}};
// METAL-NEXT:    float4 color{{.*}} {{\[\[}}attribute(1){{\]\]}};
// METAL-NEXT:};

// METAL: {{\[\[}}vertex{{\]\]}} [[vertexOutput]] main_vertex(vertexInput{{.*}}{{\[\[}}stage_in{{\]\]}}, uint vid{{.*}}{{\[\[}}vertex_id{{\]\]}}, uint instanceID{{.*}} {{\[\[}}instance_id{{\]\]}})

// METALLIB: define {{.*}} @main_vertex
// METALLIB: define {{.*}} @main_fragment
// METALLIB: define {{.*}} @main_fragment1

struct VIn
{
    float4 position : POSITION;
    float4 color : COLOR;
    uint vid : SV_VertexID;
    uint instanceID : SV_InstanceID;
}

struct VOut
{
    float4 position : SV_Position;
    float4 vertexColor;
    float2 vertexUV;
    float3 vertexNormal : NORMAL;
}

[shader("vertex")]
VOut main_vertex(VIn vertexIn)
{
    VOut vertexOut;
    vertexOut.position = vertexIn.position;
    vertexOut.vertexColor = vertexIn.color;
    vertexOut.vertexUV = float2(0.0, 1.0);
    return vertexOut;
}

[shader("fragment")]
float4 main_fragment(VOut fragmentIn) : SV_Target
{
    return fragmentIn.vertexColor;
}

struct FragOut
{
    float4 color : SV_Target;
    float depth : SV_Depth;
}

[shader("fragment")]
FragOut main_fragment1(VOut fragmentIn)
{
    FragOut fragOut;
    fragOut.color = fragmentIn.vertexColor;
    fragOut.depth = 0.5;
    return fragOut;
}
