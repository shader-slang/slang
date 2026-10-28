//TEST:COMPILE: tests/bugs/13306/type-kinds.hlsl -Gec -target hlsl -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/type-kinds.hlsl -Gec -target spirv -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/type-kinds.hlsl -Gec -target dxil -profile cs_6_0 -entry main -validate-ir

// Matrices and enums can be copied and assigned, including through struct fields.
enum Mode : uint { First, Second };
struct Data { float4x4 transform; Mode mode; };
uniform float4x4 transform;
uniform Mode mode;
uniform Data data;
RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    transform[0][0] += float(tid.x);
    mode = Mode.Second;
    data.transform = transform;
    data.mode = mode;
    output[tid.x] = data.transform[0] + float4(uint(data.mode));
}
