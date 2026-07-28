//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// METAL: #include <metal_logging>

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // METAL: os_log_default.log("test");
    printf("test");

    // METAL: os_log_default.log("test1 \"%d %d\"", int(5), int(6));
    printf(R"(test1 "%d %d")", 5, 6);

    // METAL: os_log_default.log("tid = %u, f = %f\n", {{.*}}, {{.*}});
    printf("tid = %u, f = %f\n", tid.x, 1.5f);

    outputBuffer[0] = 1;
}

// METALLIB: define void @computeMain
