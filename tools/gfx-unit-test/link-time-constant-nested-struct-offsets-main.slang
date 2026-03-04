// Main module for testing nested struct with link-time constant array
// The lib module will define NESTED_ARRAY_SIZE

extern static const int NESTED_ARRAY_SIZE;

struct NestedStruct
{
    int x;
    float y;
    int nestedArray[NESTED_ARRAY_SIZE];  // Link-time sized array in nested struct
    int z;
};

struct ParentStruct
{
    int a;
    float b;
    NestedStruct nested;  // Contains the link-time sized array
    int c;
    float d;
};

RWStructuredBuffer<ParentStruct> buffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Write test values to the buffer
    buffer[0].a = 1;
    buffer[0].b = 2.0f;
    buffer[0].nested.x = 10;
    buffer[0].nested.y = 20.0f;
    buffer[0].nested.nestedArray[0] = 100;
    buffer[0].nested.nestedArray[1] = 101;
    buffer[0].nested.nestedArray[2] = 102;
    buffer[0].nested.z = 30;
    buffer[0].c = 3;
    buffer[0].d = 4.0f;
}