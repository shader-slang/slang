//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -g2 -emit-spirv-directly
//TEST:SIMPLE(filecheck=SEPARATE):-target spirv -entry main -stage fragment -g2 -emit-spirv-directly -separate-debug-info -separate-debug-info-output debug-info-opaque-types.dbg.spv

Texture2D texture;
SamplerState sampler;

struct Empty
{}

// Keep the empty struct in the emitted debug information so it can serve as a non-opaque control.
[noinline]
float4 preserveEmptyType(Empty value, float4 result)
{
    return result;
}

[shader("fragment")]
float4 main(float2 uv : TEXCOORD0) : SV_Target
{
    Empty ordinary;
    return preserveEmptyType(ordinary, texture.Sample(sampler, uv));
}

// CHECK-DAG: %[[DEBUG_INFO_NONE:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInfoNone
// CHECK-DAG: %[[TEXTURE_NAME:[0-9]+]] = OpString "Texture2D"
// CHECK-DAG: %[[TEXTURE_LINKAGE_NAME:[0-9]+]] = OpString "@Texture2D"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[TEXTURE_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[TEXTURE_LINKAGE_NAME]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
// CHECK-DAG: %[[SAMPLER_NAME:[0-9]+]] = OpString "SamplerState"
// CHECK-DAG: %[[SAMPLER_LINKAGE_NAME:[0-9]+]] = OpString "@SamplerState"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[SAMPLER_NAME]] %uint_0 %{{[0-9]+}} %uint_0 %uint_0 %{{[0-9]+}} %[[SAMPLER_LINKAGE_NAME]] %[[DEBUG_INFO_NONE]] %uint_131072{{$}}
// CHECK-DAG: %[[EMPTY_NAME:[0-9]+]] = OpString "Empty"
// CHECK-DAG: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugTypeComposite %[[EMPTY_NAME]] %uint_1 %{{[a-zA-Z0-9_]+}} %{{[a-zA-Z0-9_]+}} %{{[a-zA-Z0-9_]+}} %{{[a-zA-Z0-9_]+}} %[[EMPTY_NAME]] %uint_0 %uint_131072{{$}}

// SEPARATE-NOT: DebugInfoNone
// SEPARATE-NOT: DebugTypeComposite
// SEPARATE: DebugBuildIdentifier
// SEPARATE-NOT: DebugInfoNone
// SEPARATE-NOT: DebugTypeComposite
