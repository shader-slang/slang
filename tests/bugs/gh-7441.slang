//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER-MTL):-metal -compute -output-using-type
//TEST:REFLECTION(filecheck=REFLECT):-stage compute -entry computeMain -target cuda -no-codegen
//TEST:REFLECTION(filecheck=REFLECT-MTL):-stage compute -entry computeMain -target metal -no-codegen


// Test struct for bool layout analysis
struct TestType
{
    uint value;
    bool f_bool;
    bool1 f_bool1;
    bool pad1;
    bool2 f_bool2;
    bool pad2;
    bool3 f_bool3;
    bool4 f_bool4;
    uint END;
};

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=buffer
RWStructuredBuffer<TestType> buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;

    // Initialize test data
    buffer[i].value = 7;
    buffer[i].f_bool = true;
    buffer[i].f_bool1 = bool1(true);
    buffer[i].pad1 = false;
    buffer[i].f_bool2 = bool2(true, true);
    buffer[i].pad2 = false;
    buffer[i].f_bool3 = bool3(true, false, true);
    buffer[i].f_bool4 = bool4(true, false, true, false);
    buffer[i].END = 0x12345678;
}

// BUFFER: 7
// BUFFER-NEXT: 101
// BUFFER-NEXT: 1000101
// BUFFER-NEXT: 100
// BUFFER-NEXT: 10001
// BUFFER-NEXT: 12345678

// Expected output for Metal (different struct layout)
// BUFFER-MTL: 7
// BUFFER-MTL-NEXT: 101
// BUFFER-MTL-NEXT: 101
// BUFFER-MTL-NEXT: 10001
// BUFFER-MTL-NEXT: 10001
// BUFFER-MTL-NEXT: 12345678

// REFLECT:         "name": "f_bool1",
// REFLECT:         "binding": {"kind": "uniform", "offset": 5, "size": 1, "elementStride": 1}
// REFLECT:         "name": "f_bool2",
// REFLECT:         "binding": {"kind": "uniform", "offset": 8, "size": 2, "elementStride": 1}
// REFLECT:         "name": "f_bool3",
// REFLECT:         "binding": {"kind": "uniform", "offset": 11, "size": 3, "elementStride": 1}
// REFLECT:         "name": "f_bool4",
// REFLECT:         "binding": {"kind": "uniform", "offset": 16, "size": 4, "elementStride": 1}

// Metal-specific reflection (different bool3 layout)
// REFLECT-MTL:         "name": "f_bool1",
// REFLECT-MTL:         "binding": {"kind": "uniform", "offset": 5, "size": 1, "elementStride": 1}
// REFLECT-MTL:         "name": "f_bool2",
// REFLECT-MTL:         "binding": {"kind": "uniform", "offset": 8, "size": 2, "elementStride": 1}
// REFLECT-MTL:         "name": "f_bool3",
// REFLECT-MTL:         "binding": {"kind": "uniform", "offset": 12, "size": 4, "elementStride": 1}
// REFLECT-MTL:         "name": "f_bool4",
// REFLECT-MTL:         "binding": {"kind": "uniform", "offset": 16, "size": 4, "elementStride": 1}
