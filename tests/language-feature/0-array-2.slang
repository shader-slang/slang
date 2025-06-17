//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk

public struct ContextND<let N: int>
{
    public uint3 thread_id;
    public int[N] call_id;

    public ContextND<0> map(int)
    {
        return { thread_id };
    }

    public ContextND<M> map<let M : int>(int[M] mapping)
    {
        ContextND<M> result;
        result.thread_id = thread_id;
        for (int i = 0; i < M; ++i)
            result.call_id[i] = call_id[mapping[i]];
        return result;
    }
}

typealias Context = ContextND<0>;

//TEST_INPUT: set value = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> value;

void store(Context context, in int v) { value[0] = v; }

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    Context context = {dispatchThreadID};
    int c = 1;
    int m_c = 0;
    // CHECK: 1
    store(context.map(m_c), c);
}