// Fragment-stage `SV_PrimitiveID` maps to WGSL's `@builtin(primitive_index)`, which is a `u32`
// input that a module may only use after `enable primitive_index;`.
// https://github.com/shader-slang/slang/issues/13617

//TEST:SIMPLE(filecheck=PARAM): -target wgsl -stage fragment -entry paramMain
//TEST:SIMPLE(filecheck=FIELD): -target wgsl -stage fragment -entry fieldMain
//TEST:SIMPLE(filecheck=NESTED): -target wgsl -stage fragment -entry nestedMain
//TEST:SIMPLE(filecheck=GLOBAL): -target wgsl -stage fragment -entry globalMain
//TEST:SIMPLE(filecheck=INT): -target wgsl -stage fragment -entry intMain
//TEST:SIMPLE(filecheck=NONE): -target wgsl -stage fragment -entry noneMain
//DIAGNOSTIC_TEST:SIMPLE(diag=RTAI): -target wgsl -stage fragment -entry renderTargetArrayIndexMain

//PARAM: enable primitive_index;
//PARAM: @builtin(primitive_index) {{.*}}: u32
uint paramMain(uint primitive : SV_PrimitiveID) : SV_Target0
{
    return primitive;
}

struct FragmentInput
{
    float4 position : SV_Position;
    uint primitive : SV_PrimitiveID;
    float color : COLOR;
};

//FIELD: enable primitive_index;
//FIELD: @builtin(primitive_index) {{.*}}: u32
float4 fieldMain(FragmentInput input) : SV_Target0
{
    return float4(input.color + float(input.primitive));
}

// A nested struct field is flattened by `flattenInputParameters` instead of being hoisted to an
// entry-point parameter like a plain field, so `ensureStructHasUserSemantic<VaryingInput>` assigns
// its builtin before `legalizeSystemValue` sees the struct key.
struct InnerInput
{
    uint primitive : SV_PrimitiveID;
    float color : COLOR;
};

struct NestedInput
{
    float4 position : SV_Position;
    InnerInput inner;
};

//NESTED: enable primitive_index;
//NESTED: @builtin(primitive_index) {{.*}}: u32
float4 nestedMain(NestedInput input) : SV_Target0
{
    return float4(input.inner.color + float(input.inner.primitive));
}

// A global `in` varying is how GLSL's `gl_PrimitiveID` reaches the WGSL target.
in uint globalPrimitive : SV_PrimitiveID;

//GLOBAL: enable primitive_index;
//GLOBAL: @builtin(primitive_index) {{.*}}: u32
float4 globalMain() : SV_Target0
{
    return float4(float(globalPrimitive));
}

//INT: enable primitive_index;
//INT: @builtin(primitive_index) [[PRIM:[a-z_0-9]+]] : u32
//INT: i32([[PRIM]])
int intMain(int primitive : SV_PrimitiveID) : SV_Target0
{
    return primitive;
}

// A shader without an `SV_PrimitiveID` input must not require the extension.
//NONE-NOT: primitive_index
//NONE: @fragment
float4 noneMain(float4 position : SV_Position) : SV_Target0
{
    return position;
}

// WGSL has no render-target-array-index builtin, so this semantic stays unsupported.
float4 renderTargetArrayIndexMain(uint layer : SV_RenderTargetArrayIndex) : SV_Target0
//RTAI:                                ^^^^^ error E55202
//RTAI:                                ^^^^^ system value semantic 'sv_rendertargetarrayindex' is not supported for the current target.
{
    return float4(float(layer));
}
