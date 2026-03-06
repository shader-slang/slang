// Enum values returned from dynamically dispatched methods must be
// correctly packed into the AnyValue return slot and unpacked at the
// call site. This tests enum as a method return type (not just a field).

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum Status
{
    Idle = 0,
    Running = 1,
    Done = 2,
    Error = 3,
}

interface ITask
{
    Status getStatus();
    int getProgress();
}

struct PendingTask : ITask
{
    int progress;

    Status getStatus()
    {
        if (progress == 0) return Status.Idle;
        return Status.Running;
    }

    int getProgress() { return progress; }
}

struct CompletedTask : ITask
{
    int result;

    Status getStatus() { return Status.Done; }
    int getProgress() { return 100; }
}

struct FailedTask : ITask
{
    int errorCode;

    Status getStatus() { return Status.Error; }
    int getProgress() { return -1; }
}

ITask makeTask(int id)
{
    switch (id)
    {
    case 0: return PendingTask(0);
    case 1: return PendingTask(50);
    case 2: return CompletedTask(42);
    default: return FailedTask(404);
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // PendingTask(0): status=Idle(0), progress=0
    var t0 = makeTask(id);
    outputBuffer[0] = int(t0.getStatus());
    // CHECK: 0
    outputBuffer[1] = t0.getProgress();
    // CHECK: 0

    // PendingTask(50): status=Running(1), progress=50
    var t1 = makeTask(id + 1);
    outputBuffer[2] = int(t1.getStatus());
    // CHECK: 1
    outputBuffer[3] = t1.getProgress();
    // CHECK: 50

    // CompletedTask(42): status=Done(2), progress=100
    var t2 = makeTask(id + 2);
    outputBuffer[4] = int(t2.getStatus());
    // CHECK: 2
    outputBuffer[5] = t2.getProgress();
    // CHECK: 100

    // FailedTask(404): status=Error(3), progress=-1
    var t3 = makeTask(id + 3);
    outputBuffer[6] = int(t3.getStatus());
    // CHECK: 3
    outputBuffer[7] = t3.getProgress();
    // CHECK: -1
}
