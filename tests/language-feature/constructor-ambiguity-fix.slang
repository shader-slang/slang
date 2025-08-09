//TEST:SIMPLE(filecheck=CHECK): -target spirv

struct Vector<T, int N>
{
    T[N] data;

    __init(T value) {
        for (int i = 0; i < N; i++)
            data[i] = value;
    }

    __init(vector<T, N> data)
    {
        for (int i = 0; i < N; i++)
            this.data[i] = data[i];
    }
}

[shader("compute")]
void computeMain()
{
    // This should now work without ambiguity - prefers scalar constructor
    var normal = Vector<float, 16>(1.0);
    // CHECK: OpReturn
}