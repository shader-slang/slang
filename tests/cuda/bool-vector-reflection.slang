// bool-vector-reflection.slang

//TEST:REFLECTION:-stage compute -entry main -target cuda -no-codegen

// Test struct with various bool vector types to reproduce reflection issues
struct TestType {
    uint value;        // 4B
    bool f_bool;       // 1B  
    bool1 f_bool1;     // Should be 4B per the issue description
    bool2 f_bool2;     // Should be 8B per the issue description
    bool3 f_bool3;     // Should be 12B per the issue description
    bool4 f_bool4;     // Should be 16B per the issue description
    uint END;          // 4B
};

// Test various usage patterns that were mentioned in the issue
StructuredBuffer<bool2> sb_bool2;
ParameterBlock<bool2> pb_bool2;
uniform bool2 u_bool2;

// Additional test cases
RWStructuredBuffer<TestType> buffer;

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID) {
    uint i = tid.x;
    buffer[i].value = i+1;
    buffer[i].f_bool = true;
    buffer[i].f_bool1 = bool1(true);
    buffer[i].f_bool2 = bool2(true, true);
    buffer[i].f_bool3 = bool3(true, true, true);
    buffer[i].f_bool4 = bool4(true, true, true, true);
    buffer[i].END = 0x12345678;
}