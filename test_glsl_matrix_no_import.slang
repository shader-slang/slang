[shader("fragment")]
void fragment_main(out float4 color : SV_Target) {
    float4x4 matrix = float4x4(1.0);
    float4 vector = float4(1.0);
    float4 result0 = mul(matrix, vector); 
    float4 result1 = mul(matrix, vector); 
    color = result0 + result1;
}