//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage vertex -entry main

// Test enum without explicit tag type (should default to int)
public enum SimpleEnum {
    value1,
    value2,
    value3
}

// Test enum with different tag types
public enum ByteEnum : uint8_t {
    a, b, c
}

public enum IntEnum : int {
    x = -1, y = 0, z = 1
}

struct VertexInput {
    float3 position : POSITION_ATTR;
};

public struct VertexOutput {
    float4 position : SV_Position;
    SimpleEnum simple : TEXCOORD0;
    IntEnum intVal : TEXCOORD1;
}

[shader("vertex")]
VertexOutput main(in VertexInput input) {
    VertexOutput output;
    output.position = float4(0);
    output.simple = SimpleEnum.value1;
    output.intVal = IntEnum.x;
    return output;
}

//CHECK: simple_0 : TEXCOORD0
//CHECK: intVal_0 : TEXCOORD1