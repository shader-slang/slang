//TEST:SIMPLE(filecheck=CHECK): -target spirv -gdwarf -g3 -O0 -line-directive-mode glsl -stage fragment -entry main -emit-spirv-directly

struct VSOutput {
    int x;
};

float3x3 MatrixTranspose(float3x3 m) {
    return float3x3(
        m[0][0], m[1][0], m[2][0],
        m[0][1], m[1][1], m[2][1],
        m[0][2], m[1][2], m[2][2]
    );
}

float4 main(VSOutput input) : SV_TARGET {
    float3x3 m = MatrixTranspose(float3x3(1.0));
    return float4(1.0, 1.0, 1.0, 1.0);
}

// CHECK: %[[VECTOR:[0-9]+]] = OpExtInst %void %{{[a-zA-Z0-9_]+}} DebugTypeVector
// CHECK: %[[VSOUTPUT:[0-9]+]] = OpExtInst %void %{{[a-zA-Z0-9_]+}} DebugTypePointer
// CHECK: {{.*}} = OpExtInst %void %{{[a-zA-Z0-9_]+}} DebugTypeFunction %{{[a-zA-Z0-9_]+}} %[[VECTOR]] %[[VSOUTPUT]]
// CHECK: %[[MATRIX:[0-9]+]] = OpExtInst %void %{{[a-zA-Z0-9_]+}} DebugTypeMatrix
// CHECK: {{.*}} = OpExtInst %void %{{[a-zA-Z0-9_]+}} DebugTypeFunction %{{[a-zA-Z0-9_]+}} %[[MATRIX]] %[[MATRIX]]
