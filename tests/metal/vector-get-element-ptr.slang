//TEST:SIMPLE(filecheck=CHECK): -target metal

//TEST(smoke,compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -mtl

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void modify(inout int v)
{
    v = 2;
}

[numthreads(1,1,1)]
void computeMain(int3 v : SV_DispatchThreadID)
{
    int3 u = v;
    // CHECK: thread int3 [[TEMP:[a-zA-Z0-9_]+]] = int3(v{{.*}};
    // CHECK: modify{{.*}}(&[[TEMP]][int(0)])

    modify(u.x);

    // CHECK: outputBuffer{{.*}} = [[TEMP]].x + [[TEMP]].y;
    // BUF: 2
    outputBuffer[0] = u.x + u.y;
}
