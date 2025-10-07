//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib

struct Output
{
  float4 Diffuse  : SV_Target0;
  float4 Normal   : SV_Target1;
  float4 Material : SV_Target2;
}

// METALLIB: define {{.*}} @fragMain
// METAL: color(0)
// METAL: color(1)
// METAL: color(2)

[shader("fragment")]
Output fragMain()
{
    return { float4(1), float4(2), float4(3) };
}
